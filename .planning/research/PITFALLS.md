# Pitfalls Research

**Domain:** JUCE C++ audio plugin -- room simulation / reverb / FDN / multiband saturation
**Researched:** 2026-02-18
**Confidence:** HIGH (established DSP domain with deep community knowledge; JUCE specifics verified via official docs and forums)

---

## Critical Pitfalls

### Pitfall 1: Memory Allocation on the Audio Thread

**What goes wrong:**
Any heap allocation inside `processBlock()` -- including hidden ones like `std::vector::push_back`, `juce::String` construction, `DBG()` macro usage, `juce::Path` or `juce::Image` creation, or even `std::function` captures that exceed the small-buffer optimization -- causes unbounded latency spikes. At 48 kHz / 128-sample buffer, the entire processing window is 2.66 ms. A single `malloc` call can take milliseconds when the OS memory allocator contends with other threads, producing audible clicks, pops, and dropouts.

**Why it happens:**
C++ makes allocation invisible. Containers resize silently. `DBG()` uses `juce::String` internally (which allocates). Parameter listener callbacks can trigger `repaint()` which enqueues work. Developers coming from non-realtime C++ do not have the reflex to audit every line for hidden allocations.

**How to avoid:**
- Pre-allocate ALL buffers, delay lines, and scratch memory in `prepareToPlay()`. Size them generously (2x `estimatedSamplesPerBlock` as the JUCE docs hint that the estimate is not a guarantee).
- Use `juce::ScopedNoDenormals` at the top of every `processBlock()` (JUCE puts this in the default template for good reason).
- Never call `DBG()` in the audio path. Use lock-free ring buffers to push debug data to a logging thread if needed.
- Cache `std::atomic` parameter loads into local variables at the top of `processBlock()` rather than calling `.load()` per-sample in tight loops (measurable overhead reduction).
- Build with `-fsanitize=address` during development and run pluginval at strictness >= 5 to catch allocations.

**Warning signs:**
- Sporadic clicks/pops under load, especially when GUI is open or parameters are being automated.
- CPU spikes visible in DAW meter that do not correlate with DSP complexity.
- pluginval failures at strictness level 5+.

**Phase to address:**
Phase 1 (project scaffolding / audio pipeline skeleton). Establish the allocation-free discipline from the very first `processBlock()` implementation. Retroactively removing allocations from mature code is far harder.

---

### Pitfall 2: FDN Feedback Matrix Instability

**What goes wrong:**
The FDN (both the short Cabinet Resonance network and the Diffuse Tail) grows unbounded or colors the sound with metallic ringing. Two distinct failure modes:

1. **Energy growth:** If the feedback matrix is not strictly unitary (or orthogonal for real-valued matrices), energy accumulates. Even tiny numerical errors compound over thousands of iterations. With absorption filters in the loop the system may appear stable at first but blow up at certain frequency-dependent decay settings.
2. **Spectral coloring / metallic ringing:** Poorly chosen delay line lengths (sharing common factors, or insufficient mode density) create audible periodic patterns. The ear perceives this as "metallic" or "fluttery" coloration, especially on longer tails.

**Why it happens:**
- Developers build the feedback matrix from arbitrary coefficients rather than using a known unitary construction (Hadamard, Householder, or a sequence of rotations).
- Delay lengths are chosen arbitrarily or from small primes that cluster too closely, producing insufficient mode density in some frequency bands.
- When absorption filters are added inside the feedback loop, the overall loop gain at certain frequencies can exceed unity if the filter gains are not carefully coordinated with the feedback matrix.

**How to avoid:**
- Use Householder reflection matrices (`H = I - (2/N) * ones`) for the feedback matrix. They provide moderate inter-channel mixing without the over-coupling of Hadamard matrices, and are computationally efficient (O(N) per sample rather than O(N^2) for a dense matrix multiply). Signalsmith's ADC 2021 reverb design recommends this approach.
- Choose delay line lengths as powers of distinct primes for the Cabinet Resonance FDN (allows runtime length changes while preserving mutual primality). For the Diffuse Tail FDN, use mutually prime lengths with a spread ratio of roughly 1.5x between shortest and longest to ensure uniform mode density.
- Keep absorption filters to first-order IIR per delay line for the initial implementation. Higher-order filters give finer RT60-per-band control but make stability analysis harder. Validate that the product `|H| * |filter_gain(f)|` < 1 for all frequencies at all parameter settings.
- Add a hard safety limiter (e.g., `if (abs(sample) > 10.0f) sample = 0.0f`) inside the feedback loop during development to catch runaways before they damage speakers or ears.

**Warning signs:**
- Output level slowly creeps up over 10+ seconds of sustained input.
- "Ping-pong" or "metallic" tone on percussive transients that does not diminish with decay time adjustment.
- Different behavior at different sample rates (because delay lengths map to different physical times).

**Phase to address:**
Phase 2 (core DSP implementation). The FDN is the heart of both the Cabinet Resonance and Diffuse Tail stages. Get the matrix and delay length selection right before layering on frequency-dependent decay or modulation.

---

### Pitfall 3: Denormalized Float CPU Explosions in Feedback Paths

**What goes wrong:**
When input goes silent, IIR filters and FDN delay lines decay toward zero. On x86 CPUs, subnormal floating-point values (magnitudes below ~1.18e-38 for float) trigger a microcode assist path that can be 100x slower than normal float operations. A reverb plugin with 8+ delay lines, each containing absorption filters, is a perfect storm: it generates a massive number of subnormals simultaneously during the tail-out period. CPU usage spikes from 5% to 80%+ the moment audio input stops.

**Why it happens:**
Developers test with continuous audio input and never notice the problem. The tail-out period (silence after the last input) is where feedback paths decay through the denormal range. Every IIR filter state variable and every sample in every delay line becomes a denormal simultaneously.

**How to avoid:**
- Place `juce::ScopedNoDenormals` at the top of `processBlock()`. This sets the FTZ (Flush To Zero) and DAZ (Denormals Are Zero) CPU flags for the scope. It is zero-overhead (just two register writes on entry/exit).
- As a belt-and-suspenders measure, add a tiny DC offset (1e-18f, alternating sign per buffer to prevent DC buildup) at the input of each feedback path. This keeps values above the denormal threshold during decay.
- Test specifically by feeding impulses and then silence for 30+ seconds while monitoring CPU usage. This is the most reliable detection method.

**Warning signs:**
- CPU meter in DAW spikes dramatically after audio stops (while reverb tail is decaying).
- Host reports "audio overload" or drops out during silence.
- Problem is worse with longer RT60 settings.

**Phase to address:**
Phase 1 (scaffolding). `ScopedNoDenormals` should be in the very first `processBlock()`. The DC offset trick should be added when feedback paths are implemented in Phase 2.

---

### Pitfall 4: Oversampling Done Wrong in the Excitation Stage

**What goes wrong:**
The Excitation stage (multiband soft saturation with 2-4x oversampling) is the most CPU-expensive stage in the chain and the most likely source of aliasing artifacts. Three failure modes:

1. **Insufficient oversampling for the saturation curve:** Mild saturation needs 2x. Aggressive saturation (approaching hard clipping) may still alias significantly at 4x. Each polynomial order in the waveshaper doubles the aliased harmonic content.
2. **Oversampling filter artifacts:** The anti-aliasing lowpass filter in the downsampler introduces its own pre-ringing, phase shift, and passband ripple. These artifacts can be more audible than the aliasing they prevent, especially on transients.
3. **Incorrect latency reporting:** `juce::dsp::Oversampling` introduces latency. If this latency is not reported to the host via `setLatencySamples()`, the plugin will be out of time alignment in the DAW, causing comb filtering when used in parallel or on a bus.

**Why it happens:**
- Developers test saturation in isolation with simple signals rather than full mixes where aliasing products are masked. When users push the drive harder than expected, aliasing becomes audible.
- The oversampling filter design is set-and-forget during initialization but its artifacts depend on the content.
- Latency reporting is easy to forget because the plugin "sounds fine" in solo testing.

**How to avoid:**
- Use `juce::dsp::Oversampling` with `shouldUseIntegerLatency = true` so the reported latency is sample-accurate for DAW compensation.
- Report total latency in `prepareToPlay()` via `setLatencySamples(oversampling.getLatencyInSamples())`. Update this if oversampling factor changes at runtime.
- Consider ADAA (Antiderivative Anti-Aliasing) as a complement to or replacement for oversampling for the soft-clip function. ADAA provides ~50 dB of aliasing reduction for memoryless nonlinearities at zero latency cost, though it only works for static waveshaping (not stateful nonlinearities). For `tanh`-style soft clipping, the antiderivative is `log(cosh(x))`, which is straightforward.
- Test with a 10 kHz sine wave at high drive settings, inspect the output spectrum for aliased reflections below 10 kHz. This is the standard aliasing audit.

**Warning signs:**
- Harsh, inharmonic "digital" quality at high drive settings, especially on high-frequency material.
- Timing misalignment when plugin is used on a bus (songs sound "phasey").
- CPU usage dominated by the Excitation stage (oversampling multiplies all downstream computation).

**Phase to address:**
Phase 3 (Excitation stage implementation). But the latency reporting infrastructure must be established in Phase 1, and the oversampling object should be allocated in `prepareToPlay()` from Phase 1.

---

### Pitfall 5: Thread-Unsafe GUI-to-Processor Communication

**What goes wrong:**
The AudioProcessorEditor (GUI, message thread) and AudioProcessor (DSP, audio thread) share data unsafely. This manifests as:
- Rare crashes (data races on non-atomic shared state).
- Subtle audio glitches (GUI locks a mutex that the audio thread is waiting on).
- Parameter automation that "sticks" or "jumps" because listener callbacks fire on the wrong thread.

**Why it happens:**
JUCE's APVTS (AudioProcessorValueTreeState) provides safe atomic parameter access, but developers often bypass it for convenience -- storing extra state in plain member variables, using `std::mutex` to protect shared data, or calling `repaint()` from `parameterChanged()` callbacks (which can execute on the audio thread during automation).

**How to avoid:**
- ALL parameter communication flows through APVTS. No exceptions.
- For non-parameter state (e.g., meter levels, visualization data), use lock-free single-producer/single-consumer FIFOs (`juce::AbstractFifo` or a simple atomic ring buffer). Audio thread writes, GUI thread reads on a timer.
- Never call `repaint()`, allocate memory, or do any GUI work from `parameterChanged()` or `parameterValueChanged()`. Instead, set a `std::atomic<bool>` flag and poll it from a `juce::Timer` or `juce::VBlankAttachment` callback on the GUI thread.
- For the reactive visualization feature: push FFT or waveform data from the audio thread into a lock-free ring buffer. The GUI reads and renders on its own schedule (30-60 fps via timer, never driven by audio callbacks).

**Warning signs:**
- Rare crashes that only reproduce under heavy automation + GUI interaction.
- Audio glitches that appear only when the GUI is open.
- Thread sanitizer (`-fsanitize=thread`) reports data races.
- pluginval failures at high strictness levels.

**Phase to address:**
Phase 1 (scaffolding). Establish the APVTS-only communication pattern and the lock-free FIFO for visualization data before any DSP or GUI code is written. Retrofitting thread safety is extremely difficult.

---

### Pitfall 6: Delay Line Interpolation Artifacts in Modulated FDN

**What goes wrong:**
When FDN delay times are modulated (for diffusion in the Diffuse Tail stage), the delay line read position becomes fractional. Poor interpolation causes:
- Audible pitch modulation artifacts (warbling) from linear interpolation error.
- High-frequency aliasing inside the feedback loop, which compounds with each iteration.
- Tonal coloration from the interpolation filter's frequency response being folded back through feedback.

**Why it happens:**
Linear interpolation is a lowpass filter with poor stopband rejection. Inside a feedback loop, the interpolation error at each iteration compounds: after N iterations through an 8-channel FDN, the effective interpolation error is amplified significantly. Developers test with single-pass delay effects where linear interpolation sounds fine, then reuse the same code inside feedback loops.

**How to avoid:**
- For fixed-length delay lines (no modulation): use integer sample lengths only. Round at initialization time. No interpolation needed, no artifacts.
- For modulated delay lines: use allpass interpolation (first-order Thiran) instead of linear interpolation. Allpass interpolation preserves magnitude (critical inside a feedback loop) while introducing only phase distortion that is perceptually benign for delay modulation.
- Keep modulation depth small (1-3 samples at the internal sample rate) for subtle diffusion. Larger modulation creates audible chorus/pitch effects.
- Modulate only a subset of the FDN channels (e.g., 4 of 8). The feedback matrix spreads the detuning to the other channels over time, providing full diffusion without modulating every line.

**Warning signs:**
- Chorus-like artifacts or pitch wobble on sustained tones through the reverb.
- High-frequency darkening that increases with reverb time (interpolation lowpass compounding).
- Different tonal character at 44.1 kHz vs 96 kHz (interpolation error is sample-rate dependent).

**Phase to address:**
Phase 2 (Diffuse Tail FDN). Start with unmodulated delay lines and add modulation only after the static FDN sounds correct. This isolates interpolation artifacts from other tuning issues.

---

### Pitfall 7: State Save/Restore Failures Across Hosts

**What goes wrong:**
Plugin state (presets, parameter values) fails to restore correctly in some hosts. Specific failure modes:
- Host calls `setCurrentProgram()` after `setStateInformation()`, overwriting the restored state with a preset.
- `getStateInformation()` is called during plugin construction in some hosts (e.g., FL Studio with VST3), before parameters are initialized.
- AU and VST3 have different state restoration ordering guarantees.
- Non-APVTS state (e.g., custom UI state like which tab is selected) is lost on restore.

**Why it happens:**
Different DAW hosts call the state save/restore methods at different times, in different orders, and with different assumptions. JUCE abstracts some of this but not all of it. Developers test in one DAW and ship.

**How to avoid:**
- Use APVTS exclusively for all parameter state. Its XML serialization handles versioning and missing parameters gracefully.
- In `setStateInformation()`, guard against being called with empty or malformed data (null check the parsed XML).
- Store a schema version number in saved state to handle future parameter additions without breaking old presets.
- If you implement `getNumPrograms()` returning > 0, be aware that hosts may call `setCurrentProgram()` at unexpected times. Consider returning 0 programs and handling presets entirely through your own UI.
- Test in at least: Logic Pro (AU), Ableton Live (VST3/AU), Reaper (VST3), FL Studio (VST3). Each has distinct state restoration behavior.

**Warning signs:**
- Parameters reset to defaults when reopening a saved project.
- Different behavior between VST3 and AU formats in the same DAW.
- pluginval state restoration test failures.

**Phase to address:**
Phase 1 (scaffolding). The APVTS and state serialization structure should be established early. Phase 5 (validation/polish) should include cross-host state restoration testing.

---

## Technical Debt Patterns

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Hardcoded delay line lengths | Faster prototyping | Cannot adapt to sample rate changes; sounds different at 44.1 vs 96 kHz | Never -- always compute from sample rate in `prepareToPlay()` |
| Linear interpolation in feedback loops | Simple, fast | Compounds through feedback; colors the sound; harder to fix later because tuning depends on it | Only for non-modulated delay lines at fixed integer lengths (where interpolation is not needed at all) |
| Skipping latency reporting | Plugin "works" in solo | Out-of-phase when used on buses or in parallel; users blame the plugin for "phasey" sound | Never -- trivial to implement correctly from day one |
| `std::mutex` for GUI-audio sync | Quick fix for shared state | Audio thread blocks on mutex; causes glitches under contention; impossible to fully test | Never in audio-thread code. Acceptable in GUI-only paths |
| Global/static state | Avoids passing references | Multiple plugin instances share state; one instance's settings bleed into another | Never -- JUCE runs multiple instances in-process |
| Single-DAW testing | Faster iteration | Format-specific bugs ship to users | Only during early development; must multi-DAW test before any release |

## Integration Gotchas

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| DAW automation | Reading APVTS parameters directly per-sample without smoothing | Use `juce::SmoothedValue` (linear for gains, multiplicative for frequencies). Reset smoothing target in `processBlock()`, advance per-sample. ~10ms smoothing time for most parameters, ~50ms for delay times |
| AU format on macOS | Plugin not found after build | Copy to `~/Library/Audio/Plug-Ins/Components/`. Run `killall -9 AudioComponentRegistrar` to force re-scan. Company ID must have first letter uppercase, rest lowercase for GarageBand compatibility |
| VST3 format | Manifest generation issues with CMake | Set `VST3_AUTO_MANIFEST FALSE` in `juce_add_plugin` if you see duplicate manifest errors. Ensure `juce_add_module` is called before `juce_add_plugin` |
| `prepareToPlay()` not called on config change | Assuming `prepareToPlay()` fires on every sample rate or buffer size change | Some hosts (AUv3, Windows Standalone) do NOT call `prepareToPlay()` on buffer size changes. Defensively handle mismatched buffer sizes in `processBlock()` by checking buffer length and re-initializing if needed |
| pluginval validation | Shipping without validation | Run pluginval at strictness >= 5 in CI. At strictness > 5, pluginval also runs `auval` for AU format. Expect to discover 5+ bugs on first run |

## Performance Traps

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Oversampling multiplies all downstream cost | CPU usage 2-4x higher than expected for the Excitation stage | Place oversampling only around the nonlinear saturation function, not the entire signal chain. Use the minimum oversampling ratio that achieves acceptable aliasing (test with 10 kHz sine) | At 4x oversampling with 4 bands, each sample goes through 16x the processing. At 96 kHz base rate this becomes 384 kHz effective, consuming most of a CPU core |
| 6 serial DSP stages without CPU budget | Total CPU exceeds the 2.66ms window at 128-sample buffer / 48 kHz | Profile each stage independently. Budget: Cabinet Resonance ~10%, Early Reflections ~10%, Air & Distance ~5%, Excitation ~40% (oversampled), Room Tone ~5%, Diffuse Tail ~25%. Leave 5% headroom. Use Perfetto for profiling | When users run at 64-sample buffers (1.33ms window) or stack multiple instances |
| GUI repaint blocking the message thread | UI animations (reactive visualization) consume 30%+ CPU, causing DAW UI sluggishness | Limit visualization repaint rate to 30 fps via `juce::Timer` or `juce::VBlankAttachment`. Use `JUCE_ENABLE_REPAINT_DEBUGGING=1` to find wasteful repaints. Pre-allocate all `juce::Path` and gradient objects as member variables; never construct them in `paint()` | Large plugin windows (800x600+), complex visualizations, or when user resizes the window |
| FDN channel count too high | 16-channel FDN sounds great but uses 16x the memory bandwidth and filter computation | Start with 8 channels for Diffuse Tail, 4 channels for Cabinet Resonance. 8 channels is the sweet spot for quality vs. cost in the literature. The Householder matrix for N channels costs O(N) per sample, but each channel has its own delay line + absorption filter | With absorption filters and modulation, a 16-channel FDN at 96 kHz can consume an entire CPU core |
| Atomic parameter loads in tight loops | Measurable overhead from `std::atomic::load()` called per-sample across many parameters | Cache all parameter values into local `float` variables at the start of `processBlock()`. With 20+ parameters and 6 stages, per-sample atomic loads add up | At small buffer sizes (32-64 samples) the per-load overhead as a fraction of total work becomes significant |

## UX Pitfalls

| Pitfall | User Impact | Better Approach |
|---------|-------------|-----------------|
| Delay time smoothing causes pitch artifacts | When user adjusts reverb size/time, the smooth interpolation creates audible pitch bending (doppler effect) | For large parameter changes (room size, decay time), crossfade between old and new delay configurations rather than smoothly interpolating delay lengths. Small changes can use smooth interpolation |
| No dry/wet mix control | Users on insert chains cannot blend the effect; must set up a send | Always provide a mix knob with 0-100% range. Ensure dry signal is latency-compensated to match the wet signal (account for oversampling latency) |
| Exposed DSP parameters instead of perceptual ones | Users see "FDN channel count" or "absorption filter frequency" instead of "Room Size" or "Brightness" | Map perceptual controls (Room Size, Decay, Brightness, Width, Distance) to multiple internal DSP parameters simultaneously. Hide implementation details |
| Visualization not matching what you hear | Reactive visualization shows frequency content but does not reflect the actual reverb character | Visualize the impulse response envelope or RT60 curve, not just a spectrum analyzer. Users of reverb plugins expect to "see" the room shape |
| Parameter ranges that produce bad sound | Extreme settings (e.g., RT60 = 0ms or RT60 = 60s) cause silence, blowups, or unusable results | Constrain parameter ranges to the musically useful region. Use `NormalisableRange` with skew to give more resolution in the useful range. Soft-clip internal values even if the UI allows extremes |

## "Looks Done But Isn't" Checklist

- [ ] **Reverb tail:** Often missing tail kill on bypass -- toggling bypass leaves a decaying reverb tail bleeding through. Verify that bypass immediately silences the wet signal (or gracefully fades it).
- [ ] **Sample rate handling:** Often broken at 96 kHz or 192 kHz -- delay line lengths, filter coefficients, and smoothing times must all be recomputed in `prepareToPlay()`. Test at 44.1, 48, 88.2, 96, and 192 kHz.
- [ ] **Mono compatibility:** Often untested -- a stereo reverb that sums to mono may have phase cancellation from the decorrelation. Test mono summing explicitly.
- [ ] **Latency compensation:** Often missing or incorrect -- verify with a null test (plugin in parallel with dry signal, invert phase, should cancel perfectly at mix=0%).
- [ ] **State versioning:** Often missing -- if you add a parameter in v1.1, sessions saved in v1.0 will fail to restore the new parameter. Verify default values are applied for missing parameters.
- [ ] **Multiple instances:** Often broken -- two instances of the plugin in the same DAW session must not share any state. JUCE runs them in the same process with shared static variables. Verify no globals or statics hold per-instance data.
- [ ] **Buffer size independence:** Often assumed -- `processBlock()` can receive ANY buffer size at any time, not just the hint from `prepareToPlay()`. Verify with buffer sizes of 1, 13, 64, 128, 512, 2048.
- [ ] **Parameter automation:** Often broken at extremes -- automate every parameter from min to max while audio plays. Listen for clicks, verify no crashes, check that values restore correctly.

## Recovery Strategies

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Memory allocation in audio thread | MEDIUM | Audit every line of `processBlock()` and all functions it calls. Replace containers with pre-allocated fixed-size buffers. Use address sanitizer to catch remaining allocations |
| FDN instability / metallic sound | HIGH | Requires redesigning delay line lengths and feedback matrix. Cannot be patched incrementally -- the tuning is interdependent. Start over with a known-good construction (Householder + mutually prime delays) |
| Missing latency reporting | LOW | Add `setLatencySamples()` call in `prepareToPlay()`. Sum oversampling latency + any other fixed delay. One-line fix but requires re-testing in all hosts |
| Thread-unsafe GUI communication | HIGH | Requires architectural refactor. Every shared variable must be audited. Introducing APVTS and lock-free FIFOs after the fact means rewriting the parameter flow and GUI update paths |
| Oversampling aliasing | MEDIUM | Increase oversampling factor (CPU cost increase), add ADAA to the waveshaper, or redesign the saturation curve to produce fewer harmonics. Each approach has different tradeoffs |
| State restore failures | MEDIUM | Add schema versioning, add defensive null checks in `setStateInformation()`, test in multiple DAWs. Usually a few days of work but requires careful testing |
| Denormal CPU spikes | LOW | Add `ScopedNoDenormals` to `processBlock()`. 30-second fix if caught early, but if the architecture has spawned additional processing threads, each thread needs its own denormal protection |

## Pitfall-to-Phase Mapping

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Memory allocation in audio thread | Phase 1: Scaffolding | pluginval at strictness 5+, address sanitizer, CPU profiling during silence |
| FDN feedback matrix instability | Phase 2: Core DSP | Impulse response test: feed impulse, verify exponential decay without growth over 60 seconds. Spectrum analysis for metallic coloring |
| Denormalized float CPU spikes | Phase 1 (ScopedNoDenormals) + Phase 2 (DC offset in feedback) | CPU monitoring during 30-second silence after impulse. CPU should return to near-zero |
| Oversampling artifacts / latency | Phase 3: Excitation stage | 10 kHz sine aliasing test, null-test for latency compensation, CPU profiling of oversampled path |
| Thread-unsafe GUI communication | Phase 1: Scaffolding | Thread sanitizer, pluginval with GUI tests, stress test with rapid automation + GUI interaction |
| Delay line interpolation artifacts | Phase 2: Diffuse Tail FDN | A/B test: modulated vs unmodulated FDN on sustained tones. Spectrum analysis for unexpected sidebands |
| State save/restore failures | Phase 1 (structure) + Phase 5 (validation) | pluginval state restoration test, manual test in Logic/Ableton/Reaper/FL Studio, version migration test |
| Parameter smoothing / zipper noise | Phase 2: All DSP stages | Automate every parameter min-to-max while audio plays. Listen on headphones for clicks/zippers |
| Sample rate handling | Phase 2 + Phase 5 | Run full test suite at 44.1, 48, 96, 192 kHz. Verify delay times, filter responses, and smoothing times scale correctly |
| AU/VST3 format validation | Phase 5: Validation | pluginval at strictness 7+ (runs auval), `killall -9 AudioComponentRegistrar` before AU tests, test in Logic + Ableton |
| CPU budget exceeded | Phase 4: Integration | Profile all 6 stages running together. Verify total CPU < 50% of budget at 128-sample buffer / 48 kHz to leave headroom for user's other plugins |
| GUI repaint performance | Phase 4: Custom UI | `JUCE_ENABLE_REPAINT_DEBUGGING=1`, Perfetto profiling of paint calls, verify < 5% CPU for GUI at 30fps |

## Sources

- [The big list of JUCE tips and tricks - Melatonin](https://melatonin.dev/blog/big-list-of-juce-tips-and-tricks/) -- HIGH confidence, comprehensive practitioner knowledge
- [Let's Write A Reverb - Signalsmith Audio (ADC 2021)](https://signalsmith-audio.co.uk/writing/2021/lets-write-a-reverb/) -- HIGH confidence, authoritative FDN design guide
- [FDN Reverberation - Julius O. Smith III (CCRMA/Stanford)](https://ccrma.stanford.edu/~jos/pasp/FDN_Reverberation.html) -- HIGH confidence, academic reference for FDN theory
- [Choice of Lossless Feedback Matrix - Julius O. Smith III](https://www.dsprelated.com/freebooks/pasp/Choice_Lossless_Feedback_Matrix.html) -- HIGH confidence, unitary matrix theory
- [JUCE Forum: Locks and memory allocations in the processing thread](https://forum.juce.com/t/locks-and-memory-allocations-in-the-processing-thread/39964) -- MEDIUM confidence, community discussion
- [JUCE Forum: APVTS Updates & Thread/Realtime Safety](https://forum.juce.com/t/apvts-updates-thread-realtime-safety/36928) -- MEDIUM confidence, community discussion
- [JUCE Forum: State of the Art Denormal Prevention](https://forum.juce.com/t/state-of-the-art-denormal-prevention/16802) -- MEDIUM confidence, community discussion
- [JUCE Official Docs: SmoothedValue](https://docs.juce.com/master/classSmoothedValue.html) -- HIGH confidence, official API reference
- [JUCE Official Docs: dsp::Oversampling](https://docs.juce.com/master/classjuce_1_1dsp_1_1Oversampling.html) -- HIGH confidence, official API reference
- [Pluginval is a plugin dev's best friend - Melatonin](https://melatonin.dev/blog/pluginval-is-a-plugin-devs-best-friend/) -- MEDIUM confidence, practitioner guide
- [Using Perfetto with JUCE - Melatonin](https://melatonin.dev/blog/using-perfetto-with-juce-for-dsp-and-ui-performance-tuning/) -- MEDIUM confidence, profiling methodology
- [Aliasing reduction in soft-clipping algorithms (Esqueda et al., EUSIPCO 2015)](https://www.eurasip.org/Proceedings/Eusipco/Eusipco2015/papers/1570104119.pdf) -- HIGH confidence, peer-reviewed
- [KVR Forum: Feedback Delay Network (FDN) Reverb](https://www.kvraudio.com/forum/viewtopic.php?t=123095) -- MEDIUM confidence, practitioner discussion
- [JUCE Forum: prepareToPlay not called on buffer size change (AUv3)](https://forum.juce.com/t/preparetoplay-is-not-called-when-changing-the-buffer-size-in-the-auv3-host/59244) -- MEDIUM confidence, confirmed bug report
- [Floating point denormals - EarLevel Engineering](https://www.earlevel.com/main/2019/04/19/floating-point-denormals/) -- HIGH confidence, authoritative DSP engineering reference

---
*Pitfalls research for: JUCE C++ audio plugin -- room simulation / reverb / FDN / multiband saturation*
*Researched: 2026-02-18*
