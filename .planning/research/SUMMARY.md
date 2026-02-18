# Project Research Summary

**Project:** Aether
**Domain:** JUCE C++ audio plugin -- room simulation / environment modeling DSP (macOS first)
**Researched:** 2026-02-18
**Confidence:** HIGH

## Executive Summary

Aether is a six-stage room environment simulator built as a JUCE 8 audio plugin, targeting guitar amp sim users who lack realistic room ambience in their signal chain. The expert approach for this class of product is a serial DSP chain of physically-motivated stages (Cabinet Resonance, Early Reflections, Air & Distance, Excitation, Room Tone, Diffuse Tail), each independently bypassable, communicating through a central APVTS parameter tree. The stack is well-established: JUCE 8.0.12 with CMake/Ninja, C++20, Signalsmith DSP for high-quality delay primitives, and Catch2 for testing. A production reference implementation (Crucible) exists within the same brand, providing battle-tested architectural patterns that Aether will follow directly.

The recommended approach is to build the project in layers: establish the plugin scaffold and parameter tree first, then implement DSP stages incrementally starting with Early Reflections (the most connected stage), followed by the FDN-based stages (Cabinet Resonance, Diffuse Tail), then the nonlinear Excitation stage with its oversampling requirements. The Victorian parchment UI and acoustic ray visualization can develop in parallel once the APVTS contract is established. The six-stage serial chain with Householder FDN matrices, Signalsmith polyphase delay interpolation, and JUCE's built-in oversampling provides a clean, well-documented path to implementation.

The key risks are FDN feedback matrix instability (metallic ringing or energy blowup from incorrect matrix construction), CPU budget management (the Excitation stage's 4x oversampling dominates cost), and cross-host state restoration failures. All three are well-understood pitfalls with known prevention strategies: use Householder reflection matrices with mutually prime delay lengths, make oversampling factor sample-rate-aware, and establish APVTS-only state serialization from day one. The single largest technical unknown is tuning the Excitation stage's multiband saturation to sound "alive" rather than "distorted" -- this requires iterative listening and is the hardest stage to get right.

## Key Findings

### Recommended Stack

The stack mirrors the proven Crucible architecture with domain-specific additions. JUCE 8.0.12 via CMake FetchContent provides the plugin framework, DSP primitives (filters, oversampling, delay lines), and UI system. Signalsmith DSP v1.7.1 provides superior fractional delay line interpolation critical for modulated FDN reverb. All dependencies are fetched via CMake FetchContent with pinned git tags for reproducibility.

**Core technologies:**
- **JUCE 8.0.12:** Plugin framework (VST3/AU/Standalone) -- industry standard, matches Crucible, CMake-native
- **C++20:** Language standard -- designated initializers, concepts, `std::span` useful for DSP; matches Crucible
- **CMake 3.22+ / Ninja:** Build system -- fast incremental builds, FetchContent for dependency management
- **Signalsmith DSP v1.7.1:** Polyphase delay line interpolation -- critical for modulated FDN without metallic artifacts
- **Catch2 v3:** Unit testing -- BDD-style, matches Crucible pattern
- **pluginval:** Plugin validation -- run at strictness 5+ for host compatibility

**Key technical decisions:**
- Householder reflection matrices for FDN (not Hadamard) -- works for any channel count, O(N) per sample
- 4x oversampling only in Excitation stage (the only nonlinear stage) -- saves significant CPU
- VST3 first, AU + Standalone in Phase 2, CLAP deferred to JUCE 9

### Expected Features

The competitive analysis of 12+ products confirms Aether occupies an uncontested niche: no existing plugin combines six physically-motivated processing stages targeted specifically at amp sim users. The feature set is well-defined by the design handoff.

**Must have (table stakes):**
- Dry/Wet Mix with auto-gain compensation
- Per-stage bypass (In/Out toggle for each of 6 stages)
- Room Size / Shape / Proximity as primary spatial controls
- Early Reflections engine with stereo decorrelation
- Diffuse reverb tail with 50-500ms decay (deliberately short -- this is a room sim, not a reverb plugin)
- 6 factory presets covering small-to-large room range
- Full DAW parameter automation with zipper-free smoothing
- Latency reporting for DAW PDC (oversampling in Excitation)
- Sample-rate independence (44.1 through 192 kHz)

**Should have (differentiators):**
- Six-stage physically-motivated processing chain -- Aether's core identity, no competitor has this
- Cabinet Resonance stage (Open/Closed/Combo) -- bridges cab sim and room reverb gap
- Room Tone (shaped noise floor) -- psychoacoustically powerful, trivially cheap to implement
- Excitation (multiband soft saturation) -- "room aliveness" that no competitor offers
- Air & Distance as independent stage -- separates "how far" from "how big"
- Acoustic ray visualization reactive to Room Size/Shape/Proximity
- Auto-linked parameters across stages (Room Size drives pre-delay, HF damping, etc.)
- Victorian parchment UI aesthetic -- visually distinctive in a sea of dark guitar plugin UIs

**Defer (v2+):**
- Windows build (macOS-first for v1; architecture already supports cross-platform)
- CLAP format (wait for JUCE 9 native support)
- AAX format (Pro Tools only, requires Avid SDK)
- Surround/Atmos support (target audience works in stereo)
- Convolution/IR loading (contradicts the interactive algorithmic approach)
- Expanded preset library beyond 6 (add based on user feedback)

### Architecture Approach

The architecture follows a strict layered pattern proven in Crucible: a central AudioProcessor owns all DSP sections and the APVTS parameter tree, calling each section's `prepare()/process()/reset()` interface in series within `processBlock()`. DSP sections are plain C++ classes (no JUCE base class inheritance) that take `AudioBuffer<float>&` by reference. UI communicates exclusively through APVTS attachments and lock-free atomic metering. The plugin UI uses a fixed 900x530 base resolution with AffineTransform scaling via a ContentWrapper pattern.

**Major components:**
1. **AetherProcessor** -- Owns APVTS, all 6 DSP sections, meter sources. Orchestrates serial processBlock signal flow. Caches atomic parameter pointers.
2. **DSP Sections (6 + MixOutput)** -- Each is a self-contained class with prepare/process/reset interface. Uses SmoothedValue for parameter ramps. Pre-allocates all memory in prepare().
3. **DSP Core Primitives** -- Shared building blocks (DelayLine, AllpassFilter, FeedbackMatrix, NoiseGenerator, Oversampler, filter types) used across multiple stages.
4. **AetherEditor + Section Panels** -- Three-column layout with ContentWrapper scaling. Per-section panels bind to APVTS via attachments. Visualization is parameter-driven, not audio-driven.
5. **AetherLookAndFeel** -- Single custom LookAndFeel for the Victorian parchment aesthetic (custom knobs, toggles, typography, colour palette).
6. **PresetManager** -- File-based preset I/O using APVTS ValueTree XML serialization. Factory presets embedded as binary data.

### Critical Pitfalls

1. **Memory allocation on the audio thread** -- Pre-allocate ALL buffers in `prepareToPlay()`. Never use DBG(), construct strings, or resize containers in processBlock. Use ScopedNoDenormals. Validate with pluginval at strictness 5+ and address sanitizer. Establish this discipline from the very first processBlock implementation.

2. **FDN feedback matrix instability** -- Use Householder reflection matrices (unitary by construction). Choose mutually prime delay line lengths with 1.5x spread ratio. Keep absorption filters to first-order IIR initially. Add a hard safety limiter inside feedback loops during development. Start with unmodulated delay lines; add modulation only after static FDN sounds correct.

3. **Denormalized float CPU explosions** -- Place `ScopedNoDenormals` at processBlock top (zero overhead). Add tiny alternating DC offset at feedback path inputs. Test by feeding impulses then 30+ seconds of silence while monitoring CPU.

4. **Oversampling done wrong in Excitation** -- Use `juce::dsp::Oversampling` with integer latency. Report latency via `setLatencySamples()`. Make oversampling factor sample-rate-aware (4x at 44.1kHz, 2x at 96kHz). Consider ADAA as complement for the tanh soft-clip. Test with 10 kHz sine at high drive settings.

5. **Thread-unsafe GUI-to-processor communication** -- ALL parameter communication through APVTS, no exceptions. Lock-free atomics for meter data. Never call repaint() from parameterChanged(). Set atomic flag + poll from Timer instead. Establish this pattern before any DSP or GUI code.

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: Project Scaffold and Audio Pipeline

**Rationale:** The architecture research identifies clear dependency layers -- Parameters.h, DSP core primitives, and the processor shell must exist before any stage can be built. The pitfalls research demands that real-time safety patterns (ScopedNoDenormals, atomic parameter caching, pre-allocated buffers) be established from the first processBlock. This phase also sets up the build system following the proven Crucible/CMake/FetchContent pattern.

**Delivers:** Building, loading, and passing audio through a VST3 plugin with complete APVTS parameter tree, stub DSP sections, dry/wet mix, and passing pluginval at strictness 5.

**Addresses:** DAW parameter automation, latency reporting infrastructure, sample-rate independence foundation, build system (CMake + Ninja + FetchContent)

**Avoids:** Memory allocation on audio thread (Pitfall 1), thread-unsafe communication (Pitfall 5), denormals (Pitfall 3), state save/restore structure (Pitfall 7)

### Phase 2: Core DSP Stages (Early Reflections + Diffuse Tail + Cabinet Resonance)

**Rationale:** Early Reflections is the most connected parameter in the plugin (drives Diffuse Tail pre-delay, Room Tone character, and visualization). It must come first for integration testing. Diffuse Tail and Cabinet Resonance both use FDN architecture with the same core primitives (DelayLine, FeedbackMatrix), making them natural companions. Building these three stages together validates the FDN implementation patterns before moving to the more specialized stages.

**Delivers:** The core spatial engine -- users can hear room character from Cabinet Resonance through Early Reflections into the Diffuse Tail. Cross-stage parameter linking (Room Size to pre-delay) is functional.

**Addresses:** Early Reflections engine, Cabinet Resonance with 3 types, Diffuse Tail with decay/diffusion, cross-stage parameter linking, Room Size / Shape / Proximity controls

**Avoids:** FDN feedback instability (Pitfall 2), delay line interpolation artifacts (Pitfall 6), parameter smoothing zipper noise

### Phase 3: Remaining DSP Stages (Air & Distance + Excitation + Room Tone)

**Rationale:** Air & Distance influences Diffuse Tail HF damping (cross-link), so it should follow Phase 2. Excitation is the most CPU-expensive stage (oversampling) and the hardest to tune -- it needs the rest of the chain working to evaluate in context. Room Tone is low-complexity and can slot in alongside.

**Delivers:** All 6 DSP stages operational. The full physically-motivated processing chain is audible. Per-stage bypass works across all stages. CPU budget can be measured for the complete chain.

**Addresses:** Air & Distance with Warm/Neutral character, Excitation with multiband saturation, Room Tone with shaped noise, Air-to-Diffuse-Tail HF damping link, oversampling infrastructure

**Avoids:** Oversampling artifacts and latency reporting (Pitfall 4), CPU budget exceeded (keep Excitation oversampling sample-rate-aware)

### Phase 4: UI Implementation (Victorian Parchment Theme + Visualization)

**Rationale:** The architecture research confirms UI and DSP are decoupled through APVTS -- UI development could start earlier, but the custom Victorian aesthetic requires significant LookAndFeel work that is best done once all parameters are finalized. The visualization is parameter-driven (not audio-driven) and depends on Room Size/Shape/Proximity parameters being stable.

**Delivers:** Full custom UI with three-column layout, all 6 section panels, custom knobs/toggles/bypass buttons, acoustic ray visualization with breathing animation, ContentWrapper scaling, header/footer with preset selector.

**Addresses:** Victorian parchment UI, acoustic ray visualization, per-stage bypass UI, factory preset selection UI, auto-gain compensation on Mix knob

**Avoids:** GUI repaint performance (limit to 30fps, pre-allocate Path objects), audio-driven visualization anti-pattern

### Phase 5: Presets, Polish, and Validation

**Rationale:** Factory presets require all 6 stages working and tuned. Cross-host validation must be the final gate. This phase is where the "looks done but isn't" checklist from pitfalls research gets verified.

**Delivers:** 6 factory presets tuned for amp sim users, AU + Standalone formats, cross-host state restoration verified, full sample-rate testing, mono compatibility check, pluginval at strictness 7+.

**Addresses:** 6 factory presets, AU/Standalone format support, user preset save/load, state versioning, cross-host compatibility

**Avoids:** State save/restore failures (Pitfall 7), AU format gotchas, sample-rate handling bugs, buffer-size independence issues

### Phase Ordering Rationale

- **Phase 1 before everything:** Pitfalls research is emphatic that real-time safety patterns, APVTS structure, and thread-safety discipline must be established before any DSP code. Retrofitting these is "extremely difficult" (direct quote from pitfalls).
- **Phase 2 groups FDN-based stages:** Cabinet Resonance, Early Reflections, and Diffuse Tail all share FDN/delay-line primitives. Building them together validates the core DSP building blocks (DelayLine, FeedbackMatrix, AllpassFilter) before they are reused.
- **Phase 3 follows Phase 2:** Air & Distance has a cross-link to Diffuse Tail (HF damping). Excitation needs the full chain to evaluate whether its saturation sounds "alive" or "distorted" in context. Room Tone depends on Room Size being implemented.
- **Phase 4 after DSP stabilizes:** The UI binds to APVTS parameters. Adding/removing/renaming parameters after UI is built creates rework. Better to finalize the parameter set in Phases 2-3, then build UI.
- **Phase 5 last:** Presets configure all stages (impossible before Phase 3). Cross-host validation is meaningless before the plugin is feature-complete.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 2 (FDN stages):** FDN delay line length selection, Householder matrix implementation, frequency-dependent decay tuning. The Signalsmith ADC 2021 reference and JOS academic material provide strong foundations, but specific tuning for Aether's short-decay room simulation (50-500ms) differs from general-purpose reverb and may need experimentation.
- **Phase 3 (Excitation):** Multiband saturation tuning is explicitly called out as "the hardest stage to tune correctly." ADAA vs oversampling tradeoffs need evaluation. The crossover filter design for 3-band split needs specification.
- **Phase 4 (UI):** The Victorian parchment aesthetic is well-defined in the design handoff but custom LookAndFeel implementation in JUCE requires understanding of the drawing API. melatonin_inspector will help debug layout.

Phases with standard patterns (skip research-phase):
- **Phase 1 (Scaffold):** Directly follows Crucible patterns. CMake setup, APVTS construction, processBlock skeleton are all documented in the architecture research with code examples.
- **Phase 5 (Validation):** pluginval, cross-host testing, and preset creation are standard workflows with no novel research needed.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | JUCE 8.0.12, CMake, Signalsmith DSP all verified with official sources and pinned versions. Crucible provides production reference for the exact same build configuration. |
| Features | MEDIUM-HIGH | Competitive analysis covers 12+ products. Feature set is well-defined by design handoff. The one uncertainty is whether 6 factory presets are enough for launch (may need 10-12). |
| Architecture | HIGH | Directly follows Crucible patterns verified in production. APVTS, section interface, ContentWrapper scaling, lock-free metering all observed from working source code. Build-order implications are clear. |
| Pitfalls | HIGH | FDN instability, denormals, allocation-free audio, oversampling artifacts, and cross-host state restoration are all well-documented in DSP literature and JUCE community. Prevention strategies are concrete and actionable. |

**Overall confidence:** HIGH

### Gaps to Address

- **Excitation stage tuning:** The multiband saturation is flagged as "the hardest stage to tune correctly." No amount of upfront research replaces iterative listening. Plan for 2-3 tuning iterations in Phase 3.
- **Factory preset values:** The 6 preset names are defined (Tight Booth through Church Hall) but the actual parameter values require the DSP chain to be auditioned. Cannot be finalized until Phase 5.
- **CPU budget at 64-sample buffers:** The performance analysis assumes 128-sample buffers. Users tracking guitar in real-time may use 64-sample buffers (1.33ms window). Need to verify the full 6-stage chain fits within budget at this setting -- may require fallback to 2x oversampling.
- **Mono compatibility:** Stereo decorrelation in Early Reflections and Room Tone may cause phase cancellation on mono sum. Needs explicit testing but not architectural changes.
- **User preset management UI:** The design handoff includes factory presets but not a detailed user preset save/load workflow. This needs UI design work in Phase 5.

## Sources

### Primary (HIGH confidence)
- Crucible reference implementation (`~/Projects/Crucible`) -- production-proven JUCE 8 + CMake + C++20 architecture
- Aether Design Handoff (`/Users/nathanmcmillan/Downloads/Aether Files/AETHER-DESIGN-HANDOFF.md`) -- definitive product specification
- [JUCE 8.0.12 Release](https://github.com/juce-framework/JUCE/releases/tag/8.0.12) -- verified latest version
- [JUCE CMake API docs](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md) -- official build system reference
- [Signalsmith DSP v1.7.1](https://github.com/Signalsmith-Audio/dsp/tags) -- delay line and interpolation library
- [Signalsmith Reverb Example (ADC 2021)](https://github.com/Signalsmith-Audio/reverb-example-code) -- FDN architecture reference
- [JOS FDN Reverberation](https://ccrma.stanford.edu/~jos/pasp/FDN_Reverberation.html) -- academic FDN theory
- [JOS Lossless Feedback Matrix](https://www.dsprelated.com/freebooks/pasp/Choice_Lossless_Feedback_Matrix.html) -- unitary matrix construction

### Secondary (MEDIUM confidence)
- [UAD Ocean Way Studios](https://www.uaudio.com/uad-plugins/reverbs/ocean-way-studios.html) -- competitive analysis
- [Valhalla Room](https://valhalladsp.com/shop/reverb/valhalla-room/) -- competitive analysis
- [LiquidSonics Seventh Heaven](https://www.liquidsonics.com/software/seventh-heaven/) -- competitive analysis
- [Melatonin JUCE tips](https://melatonin.dev/blog/big-list-of-juce-tips-and-tricks/) -- practitioner knowledge
- [Pamplejuce template](https://github.com/sudara/pamplejuce) -- JUCE 8 + CMake + CI reference
- [Kemper Forum: room mic simulation](https://forum.kemper-amps.com/forum/thread/27074-best-reverb-plugin-to-simulate-room-mics/) -- target audience validation
- [ADAA aliasing reduction (Esqueda et al., EUSIPCO 2015)](https://www.eurasip.org/Proceedings/Eusipco/Eusipco2015/papers/1570104119.pdf) -- peer-reviewed

### Tertiary (LOW confidence)
- [JUCE Forum: FDN software design](https://forum.juce.com/t/feedback-delay-network-software-design-considerations/22510) -- community discussion, aligns with academic sources
- [KVR Forum: FDN Reverb](https://www.kvraudio.com/forum/viewtopic.php?t=123095) -- practitioner discussion

---
*Research completed: 2026-02-18*
*Ready for roadmap: yes*
