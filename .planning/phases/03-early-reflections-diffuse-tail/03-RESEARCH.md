# Phase 3: Early Reflections & Diffuse Tail - Research

**Researched:** 2026-02-18
**Domain:** Multi-tap delay early reflections, FDN reverb tail, cross-stage parameter linking, stereo decorrelation, room geometry presets, parameter migration (float->choice)
**Confidence:** HIGH

## Summary

Phase 3 is the core spatial engine of Aether, delivering two DSP stages that together create the impression of a real room. The Early Reflections stage (Stage II) uses a stereo tapped delay line (12-16 taps) to simulate the first discrete echoes from walls, floor, and ceiling, controlled by Room Size (delay times 1-30ms), Shape (discrete room geometry presets affecting tap spacing and gain patterns), Proximity (direct/reflected ratio), and a new Width parameter (stereo spread). The Diffuse Tail stage (Stage VI) uses an 8-line Hadamard FDN with frequency-dependent feedback damping to create a smooth reverb decay (50ms-2s), with pre-delay automatically linked to Room Size and HF damping linked to the Air parameter (read from APVTS, anticipating Phase 4's Air stage).

This phase introduces several parameter changes: Shape converts from a continuous float (version 1) to an AudioParameterChoice with 6-8 discrete room geometry presets (version 3 bump); a new `refl_width` parameter is added; and the Tail Decay range extends from 50-500ms to 50-2000ms with a weighted knob curve. The cross-stage linking architecture follows the established anti-pattern avoidance from ARCHITECTURE.md -- the processor reads Room Size from APVTS and forwards derived values to the tail section via setters; no direct section-to-section coupling.

Signalsmith DSP v1.7.1 provides all primitives needed: `Delay<float>` for the tapped delay lines and FDN delay lines, `BiquadStatic<float>` for per-tap absorption filters and feedback loop damping, `Householder<float,N>` / `Hadamard<float,N>` for FDN mixing matrices, and `cheapEnergyCrossfade` for smooth transitions. The 8-line Hadamard matrix is the recommended upgrade from Phase 2's 4-line Householder -- it provides higher mixing density (N log N operations) needed for the longer delay times in a reverb tail, ensuring the echo density is sufficient to avoid metallic ringing.

**Primary recommendation:** Implement Early Reflections as a stereo tapped delay line with 16 taps (8 per channel, different times/gains for L and R) driven by room geometry presets, and Diffuse Tail as an 8-line Hadamard FDN with input allpass diffusion, frequency-dependent feedback damping, and automatic pre-delay. Forward Room Size and Air parameter values from the processor to both stages for cross-stage linking.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- Room Size range pushes into hall/cathedral territory at max (full 30ms reflection range)
- Nearly dry at minimum Room Size -- barely perceptible ambience, not fully anechoic
- Room Size knob curve weighted toward smaller rooms -- first 50% covers booth-to-studio, last 50% covers studio-to-hall. More resolution in the practical guitar zone
- Smooth morphing required when sweeping Room Size in real-time (delay time interpolation for DAW automation)
- Shape parameter changes from continuous float to discrete room geometry presets (6-8 shapes)
- Claude curates the shape set based on acoustic distinctiveness -- should produce maximally different reflection patterns
- Shape names use Victorian/evocative naming (e.g., "The Parlour", "The Nave", "The Gallery") -- not generic labels
- Parameter version bump required for Shape (float -> choice)
- Default settings should be subtle until pushed -- adds depth without screaming "reverb"
- Proximity Near extreme: very tight close-mic'd feel with just a breath of air (not fully anechoic). Important for non-mic'd sources like synths
- New `refl_width` parameter added for dedicated stereo width control (0-100%, natural to dramatic spread)
- Width defaults to natural stereo field; pushing it creates dramatic enveloping spread for solo instruments
- Characterful and textured reverb tail -- organic, you can hear the room's personality. Not perfectly smooth studio reverb
- Decay range extended to 50ms-2s (up from 50-500ms in original spec). Allows dramatic long tails
- Decay knob curve weighted toward shorter tails -- first 60-70% covers 50-500ms (practical zone), last 30% opens to 2s
- Room Size -> Tail pre-delay: purely automatic, always locked, not overridable
- Air -> Tail HF damping: purely automatic, always locked, not overridable
- Shape -> Tail character: room shape influences diffusion density and modal character in the tail (cathedral tail sounds different from studio tail)
- No visual feedback for links (UI stays clean) -- coupling happens behind the scenes

### Claude's Discretion
- Specific real-space mapping for room size extremes (abstract vs specific spaces)
- Whether Room Size affects reflection tonality (darker in larger rooms) vs keeping tone-neutral for Air stage
- Whether Stage I material influences reflection character (material -> reflection timbre coupling)
- Size-Shape interaction model (independent vs shape constraining size range)
- Far extreme Proximity behavior (how much direct signal floor to maintain)
- ER-to-Tail transition (seamless blend vs perceptible pre-delay gap)
- Room Size -> Tail pre-delay curve (proportional vs physically modeled)
- Whether Room Size also drives Tail FDN delay lengths (full room scaling vs pre-delay only)
- Minimum room size character (closet slapback vs subliminal ambience)
- Shape preset count and specific shapes (6-8, curated for acoustic distinctiveness)

### Deferred Ideas (OUT OF SCOPE)
- **Dedicated room surface material/absorption parameters** -- User wants wall material (stone vs wood vs carpet) to affect reflections independently from Stage I material. Warrants its own parameter set and possibly a future phase insertion. Would complement Shape (geometry) + Surface (material) for full room definition.
- **Dedicated stereo width as a separate stage or global control** -- Currently adding `refl_width` to reflections. Could expand to a global stereo field control affecting all stages.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| REFL-01 | User can control room size via Room Size knob (Small -> Large), scaling delay times from 1ms-30ms | Tapped delay line with 16 taps. Room Size maps to a delay time multiplier via a weighted curve (first 50% = 1-8ms booth-to-studio, last 50% = 8-30ms studio-to-hall). SmoothedValue on the delay multiplier with linear interpolation for zipper-free sweeps. Each shape preset defines base tap times that get scaled by the Room Size multiplier. |
| REFL-02 | User can control reflection distribution via Shape knob (Regular -> Irregular), adjusting tap spacing from even to randomized | Shape changes from float to AudioParameterChoice with 7 discrete room geometry presets. Each preset defines a complete tap pattern (16 delay times + gains + pan positions) representing a distinct room geometry. Presets range from regular/symmetric (The Parlour) to highly irregular (The Crypt). Version bump to 3 for this parameter. |
| REFL-03 | User can control mic distance via Proximity knob (Near -> Far), adjusting direct/reflected ratio and HF rolloff | Proximity controls a wet/dry blend within the reflections stage itself. Near = mostly direct signal with minimal reflections (subtle breath of air). Far = more reflected energy, reduced direct signal, additional HF rolloff on reflections (simulating distance). Direct signal floor at Far extreme: ~-12dB (never fully zero). |
| REFL-04 | Early Reflections use 8-16 individually filtered delay taps | 16 taps total (8 per stereo channel). Each tap has: delay time (from shape preset, scaled by Room Size), gain (from shape preset, scaled by Proximity), and a one-pole lowpass filter (cutoff linked to delay time -- longer delays get darker, simulating air absorption). |
| REFL-05 | Left and right channels have different delay tap times and pan positions for stereo decorrelation | L and R channels use offset tap patterns from the shape preset. Each preset defines 8 L-taps and 8 R-taps with different delay times (typically R-taps offset by 0.5-2ms from L). Width parameter controls the spread: at 0% L and R taps converge to identical (mono); at 100% the full decorrelation pattern applies plus additional pan spread via mid-side processing. |
| REFL-06 | User can bypass Early Reflections stage independently | Implemented via `refl_bypass` parameter with 10ms crossfade (established pattern from ResonanceSection). When bypassed, buffer passes through unchanged. |
| TAIL-01 | User can control reverb decay time via Decay knob (50ms-500ms RT60) | Extended to 50ms-2000ms per user decision. Decay parameter uses a weighted NormalisableRange curve: first 60-70% covers 50-500ms (practical zone), last 30% opens to 2000ms. RT60 maps to FDN per-line feedback gain via: `gain = 10^(-3 * delayLineSamples / (RT60 * sampleRate))`. |
| TAIL-02 | User can control reverb texture density via Diffusion knob (0-100%) | Diffusion controls allpass coefficient in the input diffusion network (4 cascaded allpass stages before the FDN feedback loop). At 0%: allpass coefficients near 0 (no diffusion, sparse echoes). At 100%: coefficients at ~0.7 (maximum smearing, dense texture). Shape preset also influences diffusion density -- some shapes have inherently denser feedback patterns. |
| TAIL-03 | Pre-delay is automatically linked to Room Size from Stage II (larger room = longer pre-delay) | Processor reads `reflSizeParam->load()` and calls `diffuseTailSection.setPreDelay(roomSizeValue)`. Pre-delay curve: physically modeled proportional scaling. Room Size 0.0 (1ms ER) = ~1ms pre-delay. Room Size 1.0 (30ms ER) = ~30ms pre-delay. Uses a dedicated pre-delay buffer (separate `Delay<float>` before the FDN). |
| TAIL-04 | HF damping is automatically linked to Air setting from Stage III (more air = more HF damping) | Processor reads `airAmountParam->load()` and calls `diffuseTailSection.setHFDamping(airAmount)`. Air=0: minimal HF damping in tail feedback (bright tail). Air=1: aggressive lowpass in feedback (dark, muffled tail). Maps to lowpass cutoff in the FDN feedback path: Air 0% = 12kHz cutoff, Air 100% = 2kHz cutoff. |
| TAIL-05 | Diffuse Tail uses FDN (Feedback Delay Network) architecture with stereo implementation | 8-line Hadamard FDN. Stereo input upmixed to 8 channels via StereoMultiMixer. Each line has: delay -> damping lowpass -> Hadamard matrix -> feedback gain -> back to delay input. Output downmixed from 8 channels back to stereo via StereoMultiMixer. Delay lengths chosen as coprime values (20-60ms range at 44.1kHz), logarithmically distributed for smooth mode density. |
| TAIL-06 | User can bypass Diffuse Tail stage independently | Implemented via `tail_bypass` parameter with 10ms crossfade. When bypassed, pre-delay and FDN are both skipped. |
</phase_requirements>

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| JUCE | 8.0.12 | Audio plugin framework, APVTS, SmoothedValue, AudioBuffer, DryWetMixer | Already established. Provides parameter infrastructure, buffer management, smoothing, and NormalisableRange for weighted knob curves. |
| Signalsmith DSP | 1.7.1 | Delay lines, biquad filters, mixing matrices, crossfade utilities | Header-only, already linked. Provides all DSP primitives needed for both stages. Verified API from Phase 2 work. |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `signalsmith::delay::Delay<float>` | 1.7.1 | Tapped delay lines (ER), FDN delay lines (Tail), pre-delay buffer | All delay needs. Linear interpolation (default) sufficient for ER taps; fractional delay reads needed for smooth Room Size sweeps. |
| `signalsmith::delay::Delay<float, InterpolatorCubic>` | 1.7.1 | Higher quality interpolation for delay time modulation | Use for ER taps that are swept by Room Size knob. Cubic interpolation avoids the slight lowpass coloring of linear interpolation during modulation. |
| `signalsmith::filters::BiquadStatic<float>` | 1.7.1 | Per-tap absorption filters (ER), feedback damping (Tail), HF damping (Tail) | All filter needs. lowpass() for damping, allpass() for diffusion network. |
| `signalsmith::mix::Hadamard<float, 8>` | 1.7.1 | FDN feedback matrix (8x8 orthogonal) | Tail FDN. Higher mixing density than Householder for 8 lines. N*log(N) = 24 operations vs Householder's 2N = 16, but much denser cross-coupling needed for smooth reverb tail. |
| `signalsmith::mix::StereoMultiMixer<float, 8>` | 1.7.1 | Stereo-to-8-channel upmix and 8-to-stereo downmix for FDN | Tail FDN stereo implementation. Energy-preserving rotation-based upmix/downmix. |
| `signalsmith::mix::cheapEnergyCrossfade` | 1.7.1 | Bypass crossfade, shape preset transitions | Smooth transitions when switching between shape presets or toggling bypass. |
| `juce::SmoothedValue<float>` | (JUCE 8) | Parameter smoothing for Room Size, Proximity, Width, Decay, Diffusion | All continuous parameter changes on audio thread. 20-50ms ramp times. |
| `juce::NormalisableRange<float>` | (JUCE 8) | Weighted knob curves for Room Size and Decay | Custom skew factors to weight smaller rooms and shorter decays. |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Hadamard<8> for Tail FDN | Householder<8> | Householder is simpler (2N ops) but provides less mixing density for 8 lines. For a reverb tail with 20-60ms delay lines, insufficient mixing leads to audible flutter echoes and metallic coloration. Hadamard's N*log(N) mixing is worth the small CPU cost. |
| 8-line FDN | 4-line FDN (as in Phase 2) | 4 lines worked for short cabinet resonance (1-5ms) but are insufficient for reverb tail. Mode density with 4 lines and 20-60ms delays leaves audible gaps. 8 lines provide the echo density needed for smooth decay. |
| Tapped delay line for ER | FDN for ER | FDN would provide self-sustaining echo density but is harder to control per-shape. TDL gives precise per-tap control of timing, gain, and pan -- essential for discrete room geometry presets. |
| Allpass diffusion before FDN | Diffusion within FDN feedback | Pre-FDN allpass diffusion smears the input before it enters the feedback loop, creating denser initial reflections without affecting the feedback stability. In-loop diffusion is more complex and can cause instability at high coefficients. |
| StereoMultiMixer for FDN | Manual L/R split | StereoMultiMixer uses rotation-based spreading that preserves energy and provides natural stereo decorrelation. Manual splitting (feeding L to even lines, R to odd) works but gives less smooth stereo image. |

## Architecture Patterns

### Recommended Changes to Project Structure

```
Source/
  Parameters.h           -> Add refl_width param, change refl_shape to Choice,
                            extend tail_decay range, version bump to 3
  PluginProcessor.h      -> Add refl_width cached param, add cross-stage forwarding
  PluginProcessor.cpp    -> Wire new params, add cross-stage linking in updateStageParams
  dsp/
    ReflectionsSection.h  -> Complete implementation with TDL, shape presets, stereo
    ReflectionsSection.cpp
    DiffuseTailSection.h  -> Complete implementation with FDN, pre-delay, diffusion
    DiffuseTailSection.cpp
```

No new files needed. All changes go into existing stubs and the parameter/processor files.

### Pattern 1: Room Geometry Shape Preset

**What:** Each shape is defined by a compile-time struct containing the complete 16-tap configuration for that room geometry.
**When to use:** Every shape definition.

```cpp
struct ShapePreset
{
    const char* name;               // Victorian display name
    const char* description;        // Tooltip text for Phase 6

    // Left channel taps (8 taps)
    float delayMsL[8];             // Base delay times (scaled by Room Size)
    float gainL[8];                // Tap gains (0.0-1.0)
    float panL[8];                 // Pan position (-1.0 to 1.0, -1=hard left)

    // Right channel taps (8 taps)
    float delayMsR[8];             // Base delay times (offset from L)
    float gainR[8];                // Tap gains
    float panR[8];                 // Pan position

    // Per-tap filter character
    float absorptionRate;           // How much each ms of delay darkens (lowpass cutoff reduction per ms)

    // Tail influence
    float tailDiffusionBias;        // 0.0-1.0: how much this shape densifies the tail
    float tailModalCharacter;       // 0.0-1.0: 0=even mode distribution, 1=clustered modes
};
```

### Pattern 2: Stereo Tapped Delay Line

**What:** Two banks of delay taps (L and R) reading from a shared delay buffer, with per-tap filtering and gain.
**When to use:** The core of the ReflectionsSection.

```
Input (from Stage I) ──► [Write to delay buffer]
                              |
                         [Read L taps]         [Read R taps]
                         8 taps, each:         8 taps, each:
                         - delay time          - delay time (offset)
                         - gain                - gain
                         - LP filter           - LP filter
                              |                     |
                         [Sum L taps]          [Sum R taps]
                              |                     |
                    ┌─────────┴─────────────────────┘
                    |
              [Width control: mid-side blend]
                    |
              [Proximity: blend direct/reflected]
                    |
                 Output L, R
```

### Pattern 3: 8-Line Hadamard FDN with Input Diffusion

**What:** Pre-diffusion allpass cascade feeding an 8-channel FDN with frequency-dependent feedback.
**When to use:** The core of the DiffuseTailSection.

```
Input ──► [Pre-delay buffer] ──► [Input Allpass Diffusion (4 stages)]
                                       |
                                  [StereoMultiMixer: 2ch -> 8ch]
                                       |
                              ┌────────┴────────┐
                              |   FDN LOOP:     |
                              |   8 delay lines |
                              |   ↓             |
                              |   damping LP    |
                              |   ↓             |
                              |   Hadamard<8>   |
                              |   ↓             |
                              |   × feedback    |
                              |   ↓             |
                              |   + input ──────┘
                              |   ↓
                              |   write to delay
                              └─────────────────
                                       |
                                  [StereoMultiMixer: 8ch -> 2ch]
                                       |
                                    Output L, R
```

### Pattern 4: Cross-Stage Parameter Linking via Processor

**What:** The processor reads parameter values from APVTS and forwards derived values to the tail section, following the established anti-pattern avoidance from ARCHITECTURE.md.
**When to use:** All inter-stage parameter relationships.

```cpp
// In AetherProcessor::updateStageParams()

float roomSize = reflSizeParam->load();
float airAmount = airAmountParam->load();
int shapeIndex = static_cast<int>(reflShapeParam->load());

// Forward to ReflectionsSection
reflectionsSection.setRoomSize(roomSize);
reflectionsSection.setShape(shapeIndex);
reflectionsSection.setProximity(reflProxParam->load());
reflectionsSection.setWidth(reflWidthParam->load());

// Cross-stage: Room Size -> Tail pre-delay
diffuseTailSection.setPreDelay(roomSize);

// Cross-stage: Air -> Tail HF damping
diffuseTailSection.setHFDamping(airAmount);

// Cross-stage: Shape -> Tail character
diffuseTailSection.setShapeInfluence(shapeIndex);
```

### Pattern 5: Smooth Delay Time Modulation

**What:** When Room Size changes, all delay tap times must interpolate smoothly to avoid pitch artifacts and zipper noise.
**When to use:** Room Size knob sweeps during DAW automation.

```cpp
// Use SmoothedValue for the Room Size multiplier
juce::SmoothedValue<float> roomSizeSmoothed;

// In prepare():
roomSizeSmoothed.reset(sampleRate, 0.050);  // 50ms ramp for delay changes

// In process(), per sample:
float currentSize = roomSizeSmoothed.getNextValue();
for (int t = 0; t < kNumTaps; ++t)
{
    float delayMs = shapePreset.delayMsL[t] * roomSizeMultiplier(currentSize);
    float delaySamples = delayMs * (float)sampleRate / 1000.0f;
    // Read with interpolation (linear or cubic) from the delay buffer
    float tapOutput = delayBuffer.read(delaySamples);
    // ...
}
```

The key insight: Signalsmith's `Delay<float>` already supports fractional sample reads with interpolation, so smoothly changing the read position is natively supported. The `roomSizeMultiplier` transforms the 0-1 knob value to an actual delay scaling factor using the weighted curve.

### Pattern 6: Weighted Knob Curves

**What:** Room Size and Decay knobs need non-linear mapping to provide more resolution in the practical range.
**When to use:** Parameter definition in Parameters.h.

```cpp
// Room Size: first 50% covers 1-8ms (booth to studio), last 50% covers 8-30ms (studio to hall)
// Use NormalisableRange with skew factor
// skew = log(0.5) / log((8-1)/(30-1)) = log(0.5) / log(0.2414) = 0.489
auto roomSizeRange = juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 0.4f);
// At skew 0.4, the midpoint (0.5) maps to ~0.24 of the range, i.e., ~8ms in a 1-30ms range

// Decay: first 60-70% covers 50-500ms, last 30% opens to 2000ms
// Skew factor: log(0.65) / log((500-50)/(2000-50)) = log(0.65) / log(0.231) = 0.294
auto decayRange = juce::NormalisableRange<float>(50.0f, 2000.0f, 1.0f, 0.3f);
// At skew 0.3, the midpoint (0.5) maps to ~310ms
```

### Anti-Patterns to Avoid

- **Direct section-to-section coupling:** Never have ReflectionsSection hold a pointer to DiffuseTailSection. All inter-stage data flows through the processor's updateStageParams(). This is explicitly called out in ARCHITECTURE.md as Anti-Pattern 5.
- **Recalculating tap patterns per sample:** Shape presets are static -- tap times/gains/pans are looked up from the preset struct and scaled by Room Size. Only the Room Size scaling changes per sample (via SmoothedValue). Never recompute the entire tap pattern every sample.
- **Integer-quantized delay reads during modulation:** When Room Size is being swept, delay read positions change continuously. Must use fractional sample reads (linear or cubic interpolation). Integer-only reads cause zipper noise and pitch steps.
- **Same delay times for L and R:** L and R must have different tap times for stereo decorrelation. If L and R use identical taps, the result is mono (no spatial impression).
- **FDN feedback gain >= 1.0:** As in Phase 2, total loop gain must be strictly < 1.0. With Hadamard scaling (1/sqrt(8)), the per-line feedback gain can be higher than with Householder, but the product must still ensure stability. Calculate per-line gain from RT60: `gain = 10^(-3 * delaySamples / (RT60_seconds * sampleRate))`.
- **Allocating delay buffers per shape change:** All delay buffers sized for maximum delay (30ms ER + margin) in prepare(). Shape changes only modify read positions, not buffer sizes.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Delay lines with interpolation | Custom circular buffer | `signalsmith::delay::Delay<float>` | Correct wraparound, linear/cubic/sinc interpolation, handles fractional samples. Phase 2 proved this works. |
| Orthogonal feedback matrix (8ch) | Manual matrix multiply | `signalsmith::mix::Hadamard<float, 8>::inPlace()` | Efficient butterfly implementation, correct scaling, tested. |
| Stereo <-> multichannel conversion | Manual L/R splitting | `signalsmith::mix::StereoMultiMixer<float, 8>` | Energy-preserving rotation-based upmix/downmix with proven stereo imaging. |
| Biquad filters | Manual coefficient math | `signalsmith::filters::BiquadStatic<float>` | Correct designs for lowpass, allpass, highpass, shelving. Phase 2 proved this works. |
| Energy-preserving crossfade | Custom sine/cos | `signalsmith::mix::cheapEnergyCrossfade()` | Max 1.06% energy error, efficient polynomial. |
| Parameter smoothing | Custom one-pole | `juce::SmoothedValue<float>` | Handles sample rate, isSmoothing() optimization, settable ramp time. |
| Weighted knob curves | Custom mapping function | `juce::NormalisableRange<float>` with skew | Standard JUCE pattern, integrates with APVTS, handles UI/automation correctly. |

**Key insight:** The creative work in this phase is designing the shape presets (tap patterns that produce convincing room geometries) and tuning the FDN for characterful reverb. All DSP infrastructure (delays, filters, matrices, smoothing) comes from libraries.

## Common Pitfalls

### Pitfall 1: Metallic Ringing from Insufficient FDN Echo Density
**What goes wrong:** The diffuse tail sounds metallic, ringy, or has audible discrete echoes instead of smooth reverb.
**Why it happens:** With only 4 FDN delay lines (Phase 2 size) and longer delay times (20-60ms), the echo density is too low. The ear can pick out individual echoes and their periodic repetitions.
**How to avoid:** Use 8 delay lines with Hadamard matrix for maximum mixing density. Choose delay lengths that are mutually coprime and logarithmically distributed across the 20-60ms range. Input allpass diffusion (4 cascaded stages) pre-smears the input to increase initial echo density.
**Warning signs:** Audible flutter echo, pitched "ringing," decay that sounds like repeated echoes rather than smooth wash.

### Pitfall 2: Zipper Noise When Sweeping Room Size
**What goes wrong:** Audible clicking, popping, or pitched artifacts when automating the Room Size knob.
**Why it happens:** Delay read positions change discontinuously (integer sample jumps) causing discontinuities in the output signal.
**How to avoid:** Three layers of defense: (1) SmoothedValue on the Room Size multiplier (50ms ramp), (2) fractional sample reads with cubic interpolation from Signalsmith's Delay class, (3) per-tap one-pole smoothing on the actual delay time in samples. The combination ensures that even rapid Room Size sweeps produce smooth pitch-like modulation rather than stepping artifacts.
**Warning signs:** Clicking during automation playback, audible pitch steps, different behavior at different buffer sizes.

### Pitfall 3: Shape Preset Transition Clicks
**What goes wrong:** Audible clicks or pops when switching between room shape presets.
**Why it happens:** All 16 tap times, gains, and pan positions change simultaneously, creating a discontinuity in the output signal.
**How to avoid:** Crossfade between old and new shape configurations over ~30ms. During the crossfade, both old and new tap patterns read from the same delay buffer simultaneously. Use `cheapEnergyCrossfade` for the blend. After crossfade completes, drop the old configuration.
**Warning signs:** Click on shape switch, louder click at larger Room Size (longer delays = more accumulated energy to discontinue).

### Pitfall 4: Parameter Version Mismatch After Shape Migration
**What goes wrong:** DAW sessions saved with Phase 1/2's float Shape parameter produce wrong shape selections when loaded with Phase 3's choice parameter.
**Why it happens:** Float 0-1 maps differently than Choice index 0-6. Normalized value 0.4 meant "40% between Regular and Irregular" but now maps to choice index 2 or 3 depending on item count.
**How to avoid:** Bump ParameterID version to 3 for `refl_shape` (and any other changed parameters). This tells JUCE the parameter semantics changed. Old saved values for version-1 parameters are discarded and defaults are used. Acceptable since Aether is pre-release.
**Warning signs:** Wrong shape selected on project load, automation lanes producing unexpected jumps.

### Pitfall 5: Feedback Instability at Long Decay Times
**What goes wrong:** The FDN diverges (output grows without bound) at the maximum 2-second decay setting.
**Why it happens:** The feedback gain formula `gain = 10^(-3 * D / (RT60 * SR))` produces values very close to 1.0 for long RT60 times with short delay lines. With 8 delay lines and potential rounding, the total loop gain can exceed 1.0.
**How to avoid:** Calculate the maximum safe RT60 that keeps all 8 per-line gains below 0.9999. If the user-selected RT60 would produce higher gains, clamp. Add a safety hard limiter on the FDN output (juce::jlimit at +/- 4.0f). Monitor for NaN in debug builds with jassert.
**Warning signs:** Output level climbing during sustained notes, eventual NaN or inf, DAW protection limiters engaging.

### Pitfall 6: Width Parameter Creating Phase Cancellation
**What goes wrong:** At certain Width settings, the signal partially cancels when summed to mono, causing thin sound on mono playback systems.
**Why it happens:** If Width is implemented as pure stereo spreading (e.g., mid-side where side is boosted and mid is cut), mono summing cancels the side component.
**How to avoid:** Implement Width as a blend between mono ER pattern (0%) and the full stereo decorrelated pattern (100%). At 0%, L and R get identical tap configurations. At 100%, L and R get their preset-defined decorrelated configurations. This way, mono summing at any Width setting never cancels -- it just reduces to the average of L and R patterns, which is always valid.
**Warning signs:** Volume drop when checking mono compatibility, thin-sounding ER at certain Width values.

### Pitfall 7: Pre-delay Creating Perceptible Gap
**What goes wrong:** At large Room Size, the pre-delay before the diffuse tail creates an audible gap between the early reflections and the tail onset, making them sound like separate effects.
**Why it happens:** If pre-delay linearly tracks Room Size (30ms ER = 30ms pre-delay), the gap is too long. Real rooms have overlapping ER and reverb onset.
**How to avoid:** Use a compressed pre-delay curve: pre-delay = Room Size in ms * 0.6 to 0.8, with a cap around 25ms. This keeps the tail onset overlapping with the later early reflections, creating a seamless transition. The input allpass diffusion also helps bridge the gap by smearing the input before it enters the FDN.
**Warning signs:** Two distinct "events" audible on transients (initial reflections, then a separate reverb swell).

## Code Examples

### Room Geometry Shape Presets (7 shapes)

```cpp
// Source: Custom design for Aether, informed by architectural acoustics literature
// Each shape produces a maximally different reflection pattern
// Delay times in ms (base values at Room Size = 1.0, scaled linearly down for smaller sizes)
// Gains normalized to sum to ~1.0 per channel
// Pan: -1.0 = hard left, 0.0 = center, 1.0 = hard right

static constexpr int kNumShapes = 7;
static constexpr int kTapsPerChannel = 8;

static const ShapePreset kShapes[kNumShapes] =
{
    // 0: "The Parlour" -- Small symmetric room, regular even spacing
    // Rectangular room, ~4x4m. Short, evenly-spaced reflections.
    // Even tap spacing = regular flutter pattern, warm and intimate.
    {
        "The Parlour",
        "A modest, symmetric chamber of even proportion -- "
        "warm and intimate, with reflections arriving in orderly measure",
        // L taps: evenly spaced
        { 2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.5f, 15.0f, 18.0f },
        { 0.85f, 0.72f, 0.60f, 0.50f, 0.40f, 0.32f, 0.25f, 0.18f },
        { -0.3f, 0.2f, -0.4f, 0.3f, -0.2f, 0.4f, -0.3f, 0.2f },
        // R taps: offset by ~1ms
        { 2.5f, 4.5f, 6.5f, 8.5f, 10.5f, 13.0f, 15.5f, 18.5f },
        { 0.82f, 0.70f, 0.58f, 0.48f, 0.38f, 0.30f, 0.23f, 0.16f },
        { 0.3f, -0.2f, 0.4f, -0.3f, 0.2f, -0.4f, 0.3f, -0.2f },
        0.15f,   // absorption: moderate
        0.3f,    // tail diffusion bias: low (regular, not dense)
        0.2f     // tail modal character: even modes
    },

    // 1: "The Gallery" -- Long rectangular, strong lateral reflections
    // Long narrow room, ~12x4m. First reflections from side walls are early;
    // end-wall reflections are late. Creates left-right width.
    {
        "The Gallery",
        "A long, narrow hall of distinguished proportion -- "
        "sound rebounds between close walls before reaching the distant end",
        // L taps: clustered early (side walls) then sparse late (end walls)
        { 1.5f, 2.8f, 3.5f, 5.0f, 9.0f, 14.0f, 20.0f, 27.0f },
        { 0.90f, 0.78f, 0.65f, 0.55f, 0.35f, 0.25f, 0.18f, 0.12f },
        { -0.8f, 0.7f, -0.6f, 0.5f, -0.3f, 0.2f, -0.1f, 0.05f },
        // R taps
        { 1.8f, 3.2f, 4.0f, 5.5f, 9.5f, 14.5f, 20.5f, 27.5f },
        { 0.88f, 0.75f, 0.62f, 0.52f, 0.33f, 0.23f, 0.16f, 0.10f },
        { 0.8f, -0.7f, 0.6f, -0.5f, 0.3f, -0.2f, 0.1f, -0.05f },
        0.12f,
        0.4f,
        0.35f
    },

    // 2: "The Chamber" -- Medium square, moderate density
    // Square room ~6x6m. Reflections from all four walls arrive at similar times,
    // creating a dense cluster. Classic studio live room feel.
    {
        "The Chamber",
        "A square room of generous dimension -- "
        "reflections converge from all quarters in a rich, even chorus",
        // L taps: clustered in the 3-8ms range (similar wall distances)
        { 3.0f, 3.8f, 4.5f, 5.2f, 6.0f, 7.5f, 9.5f, 13.0f },
        { 0.80f, 0.75f, 0.70f, 0.65f, 0.55f, 0.42f, 0.30f, 0.20f },
        { -0.5f, 0.4f, -0.3f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f },
        // R taps
        { 3.3f, 4.2f, 5.0f, 5.7f, 6.5f, 8.0f, 10.0f, 13.5f },
        { 0.78f, 0.72f, 0.67f, 0.62f, 0.52f, 0.40f, 0.28f, 0.18f },
        { 0.5f, -0.4f, 0.3f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f },
        0.18f,
        0.5f,    // moderate tail diffusion
        0.45f    // slight mode clustering (square room modes)
    },

    // 3: "The Nave" -- Large cathedral-like, very long reverb path
    // Tall, long space ~20x8m with high ceiling. Reflections from
    // floor/ceiling arrive first, wall reflections spread across time.
    {
        "The Nave",
        "A vast vaulted space of soaring height -- "
        "sound ascends to the ceiling before cascading down in waves of grandeur",
        // L taps: floor/ceiling early, walls spread late
        { 1.0f, 2.5f, 5.0f, 8.5f, 13.0f, 18.0f, 24.0f, 30.0f },
        { 0.70f, 0.82f, 0.60f, 0.50f, 0.40f, 0.30f, 0.22f, 0.15f },
        { -0.2f, 0.1f, -0.6f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f },
        // R taps
        { 1.3f, 3.0f, 5.5f, 9.0f, 13.5f, 18.5f, 24.5f, 30.0f },
        { 0.68f, 0.80f, 0.58f, 0.48f, 0.38f, 0.28f, 0.20f, 0.13f },
        { 0.2f, -0.1f, 0.6f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f },
        0.08f,   // low absorption (stone/plaster surfaces)
        0.7f,    // high tail diffusion
        0.6f     // clustered modes (resonant space)
    },

    // 4: "The Alcove" -- Asymmetric, irregular angles
    // Non-rectangular room with alcoves and odd angles. Reflections
    // arrive in unpredictable clusters. Complex, lively character.
    {
        "The Alcove",
        "An irregular chamber of unexpected angles and hidden recesses -- "
        "sound wanders through asymmetric paths, alive with curious texture",
        // L taps: irregular spacing (asymmetric walls, alcoves)
        { 1.2f, 2.7f, 3.1f, 5.8f, 7.2f, 11.0f, 16.5f, 22.0f },
        { 0.75f, 0.68f, 0.80f, 0.45f, 0.55f, 0.35f, 0.28f, 0.15f },
        { -0.7f, 0.3f, -0.1f, 0.8f, -0.5f, 0.6f, -0.4f, 0.2f },
        // R taps: very different pattern (asymmetric room)
        { 1.8f, 2.2f, 4.5f, 6.0f, 8.5f, 12.5f, 17.0f, 23.5f },
        { 0.72f, 0.80f, 0.55f, 0.62f, 0.40f, 0.30f, 0.22f, 0.12f },
        { 0.5f, -0.6f, 0.8f, -0.2f, 0.4f, -0.7f, 0.3f, -0.5f },
        0.14f,
        0.55f,
        0.5f     // moderate mode clustering (irregular shapes scatter modes)
    },

    // 5: "The Crypt" -- Small, dense, stone-like
    // Low ceiling, irregular stone walls. Very dense early reflections
    // with rapid buildup. Dark character from stone absorption.
    {
        "The Crypt",
        "A low vault of ancient stone -- "
        "close walls conspire to multiply each sound into a dense, dark murmur",
        // L taps: very dense, short delays (low ceiling, close walls)
        { 0.8f, 1.5f, 2.0f, 2.8f, 3.5f, 4.8f, 6.5f, 9.0f },
        { 0.90f, 0.85f, 0.80f, 0.72f, 0.65f, 0.50f, 0.38f, 0.25f },
        { -0.4f, 0.3f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f, 0.3f },
        // R taps
        { 1.0f, 1.8f, 2.3f, 3.2f, 4.0f, 5.2f, 7.0f, 9.5f },
        { 0.88f, 0.82f, 0.77f, 0.70f, 0.62f, 0.48f, 0.35f, 0.22f },
        { 0.4f, -0.3f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f, -0.3f },
        0.22f,   // high absorption (rough stone)
        0.8f,    // very dense tail
        0.7f     // strong mode clustering
    },

    // 6: "The Conservatory" -- Bright, glass-like, wide
    // Large room with hard, reflective surfaces. Reflections are bright
    // and widely spread. Open, airy character.
    {
        "The Conservatory",
        "A luminous glass pavilion of generous span -- "
        "bright reflections dance between crystalline surfaces with brilliant clarity",
        // L taps: wide spread, moderate density
        { 2.0f, 4.5f, 7.0f, 10.0f, 14.0f, 18.5f, 23.0f, 28.0f },
        { 0.88f, 0.75f, 0.65f, 0.55f, 0.45f, 0.35f, 0.25f, 0.18f },
        { -0.9f, 0.8f, -0.7f, 0.6f, -0.5f, 0.7f, -0.6f, 0.5f },
        // R taps
        { 2.5f, 5.0f, 7.5f, 10.5f, 14.5f, 19.0f, 23.5f, 28.5f },
        { 0.85f, 0.72f, 0.62f, 0.52f, 0.42f, 0.32f, 0.23f, 0.15f },
        { 0.9f, -0.8f, 0.7f, -0.6f, 0.5f, -0.7f, 0.6f, -0.5f },
        0.05f,   // very low absorption (glass/hard surfaces)
        0.35f,   // moderate tail diffusion
        0.15f    // even modes (regular geometry)
    }
};
```

### FDN Delay Length Selection

```cpp
// Source: FDN literature (Smith PASP, Jot 1992), adapted for Aether
// Delay lengths logarithmically distributed in the 20-60ms range
// Converted to samples and quantized to nearest prime for coprimality

static constexpr float kFDNDelayMs[8] = {
    22.0f, 26.5f, 31.5f, 37.0f, 43.5f, 50.0f, 56.5f, 63.0f
};
// Ratio between consecutive: ~1.15-1.18x (logarithmic distribution)
// At 44.1kHz: ~970, 1169, 1389, 1632, 1918, 2205, 2492, 2778 samples
// Nearest primes: 971, 1171, 1399, 1637, 1913, 2207, 2503, 2777

// In configureDelayLengths():
for (int i = 0; i < 8; ++i)
{
    int targetSamples = static_cast<int>(std::round(kFDNDelayMs[i] * sampleRate / 1000.0));
    fdnDelaySamples[i] = static_cast<float>(nearestPrime(juce::jmax(2, targetSamples)));
}
// Verify all 8 are distinct (bump duplicates upward)
```

### RT60 to Feedback Gain Calculation

```cpp
// Source: Standard FDN theory (Smith PASP)
// For each delay line i, feedback gain = 10^(-3 * D_i / (T60 * SR))
// where D_i = delay in samples, T60 = decay time in seconds, SR = sample rate

void DiffuseTailSection::updateDecay(float decayMs)
{
    float decaySeconds = decayMs / 1000.0f;
    for (int i = 0; i < kNumFDNLines; ++i)
    {
        float delaySamples = fdnDelaySamples[i];
        float gain = std::pow(10.0f, -3.0f * delaySamples / (decaySeconds * (float)currentSampleRate));

        // Safety: clamp to prevent instability
        feedbackGains[i] = juce::jlimit(0.0f, 0.9999f, gain);
    }
}
```

### Input Allpass Diffusion Network

```cpp
// Source: Signalsmith "Let's Write a Reverb" architecture
// 4 cascaded allpass stages with increasing delay times
// Coefficient controlled by Diffusion knob (0-100% maps to 0.0-0.7)

static constexpr int kNumDiffusionStages = 4;
static constexpr float kDiffusionDelayMs[kNumDiffusionStages] = {
    1.5f, 3.0f, 5.0f, 8.0f
};

// Each stage: y[n] = -g*x[n] + x[n-D] + g*y[n-D]
// where g = allpass coefficient (from Diffusion knob)
// D = delay in samples

// Using BiquadStatic.allpassQ() for each stage:
for (int stage = 0; stage < kNumDiffusionStages; ++stage)
{
    float delayMs = kDiffusionDelayMs[stage];
    float scaledFreq = 1.0 / (2.0 * delayMs * 0.001 * sampleRate);
    diffusionAllpass[stage].allpassQ(scaledFreq, diffusionCoeff);
}
```

Note: The allpass diffusion can alternatively be implemented with explicit delay lines and feedback, which gives finer control over the delay times. The biquad allpass approach is simpler but limits delay time to a few samples. For the diffusion network with longer delays (1.5-8ms), explicit delay+feedback allpass structures are preferred:

```cpp
// Explicit allpass: output = -g*input + delayed + g*delayed_output
float delayed = diffDelay[stage].read(delaySamples);
float output = -diffCoeff * input + delayed + diffCoeff * delayedOutput;
diffDelay[stage].write(input + diffCoeff * delayed);
// Store output for next feedback: delayedOutput is read next iteration
```

### Pre-delay with Room Size Linking

```cpp
// Source: Custom for Aether
// Pre-delay tracks Room Size with a compressed curve
// Room Size 0.0 -> 1ms pre-delay, Room Size 1.0 -> ~25ms pre-delay

void DiffuseTailSection::setPreDelay(float roomSizeNormalized)
{
    // Compressed curve: 60-80% of ER delay time
    // Map Room Size to actual ER delay (1-30ms via weighted curve)
    float erDelayMs = roomSizeToDelayMs(roomSizeNormalized);
    float preDelayMs = erDelayMs * 0.7f;  // 70% of ER time
    preDelayMs = juce::jlimit(0.5f, 25.0f, preDelayMs);

    float preDelaySamples = preDelayMs * (float)currentSampleRate / 1000.0f;
    preDelaySmoothed.setTargetValue(preDelaySamples);
}
```

### HF Damping Linked to Air Parameter

```cpp
// Source: Custom for Aether
// Air amount 0-1 maps to damping lowpass cutoff in FDN feedback
// Air 0%: bright tail (12kHz cutoff), Air 100%: dark tail (2kHz cutoff)

void DiffuseTailSection::setHFDamping(float airAmount)
{
    // Logarithmic mapping for perceptual linearity
    // cutoff = 12000 * (2000/12000)^airAmount = 12000 * 0.167^airAmount
    float cutoffHz = 12000.0f * std::pow(2000.0f / 12000.0f, airAmount);
    cutoffHz = juce::jlimit(1000.0f, 16000.0f, cutoffHz);

    float scaledFreq = cutoffHz / currentSampleRate;
    for (int i = 0; i < kNumFDNLines; ++i)
        dampingFilters[i].lowpass(scaledFreq);
}
```

### Width Control via Tap Pattern Interpolation

```cpp
// Source: Custom for Aether
// Width 0% = mono (L and R get averaged tap pattern)
// Width 100% = full stereo decorrelation (L and R use their distinct patterns)

void ReflectionsSection::processWithWidth(float widthNorm)
{
    for (int t = 0; t < kTapsPerChannel; ++t)
    {
        // Compute mono tap time (average of L and R)
        float monoDelayMs = (currentShape.delayMsL[t] + currentShape.delayMsR[t]) * 0.5f;

        // Interpolate between mono and stereo tap times
        float lDelayMs = monoDelayMs + widthNorm * (currentShape.delayMsL[t] - monoDelayMs);
        float rDelayMs = monoDelayMs + widthNorm * (currentShape.delayMsR[t] - monoDelayMs);

        // Same for gains and pan positions
        // ...
    }
}
```

## Discretion Recommendations

### Room Size Mapping: Abstract with Physical Anchors
**Recommendation:** Keep Room Size mapping abstract (0-1 continuous), not tied to specific meters or room names. However, use physical anchors for the display text:
- 0-10%: "Closet" (1-2ms, subliminal ambience)
- 10-30%: "Booth" (2-5ms, tight recording booth)
- 30-50%: "Studio" (5-8ms, standard live room)
- 50-70%: "Hall" (8-15ms, concert venue)
- 70-100%: "Cathedral" (15-30ms, vast space)

At minimum Room Size, the reflections should be subliminal -- you feel depth but don't hear discrete echoes. This is achieved by very short delay times (1-2ms) where the ear integrates the reflections with the direct sound (precedence effect threshold). Not fully anechoic because the taps still run.

### Room Size and Reflection Tonality: Subtle Darkening
**Recommendation:** Larger rooms should sound subtly darker due to increased air path length. Implement as a gentle lowpass cutoff reduction on the per-tap absorption filters as Room Size increases. At Room Size 0%: tap filters at 12kHz. At Room Size 100%: tap filters at 6kHz. This is physically motivated (air absorbs HF over distance) and keeps the darkening subtle enough that the Air stage (Phase 4) remains the primary tonal control.

### Stage I Material Influence: None
**Recommendation:** Do NOT couple Stage I material to reflection character. The Resonance stage simulates cabinet/body vibration; the Reflections stage simulates room acoustics. These are physically independent phenomena. Coupling them would create confusing parameter interactions where changing your "cabinet material" changes your "room sound." Keep stages orthogonal. The deferred "room surface material" parameter would be the correct place for room wall material influence.

### Size-Shape Interaction: Independent
**Recommendation:** Room Size and Shape should operate independently. Room Size scales all tap times uniformly (multiplier on the preset's base times). Shape selects the pattern of those taps. Any shape should work at any size. This gives the user maximum flexibility and avoids the complexity of shape-constrained size ranges. The Nave at minimum Room Size produces a tiny cathedral-shaped closet (physically absurd but musically interesting).

### Far Proximity Behavior: -12dB Direct Floor
**Recommendation:** At maximum Proximity (Far), maintain the direct signal at approximately -12dB below the reflected energy. Never fully remove the direct signal -- even in a far-field position, the direct sound is still the perceptual anchor. The reflected energy dominates but the direct path is always present. At Near, direct signal is at 0dB (full level) and reflected energy is at approximately -18dB (just a breath of air). The Proximity knob crossfades between these two extremes using an equal-power curve.

### ER-to-Tail Transition: Seamless Blend
**Recommendation:** The pre-delay should be shorter than the last early reflections, creating overlap between ER and tail onset. With pre-delay at 70% of ER time, the tail's diffused energy begins arriving while the later ERs are still decaying. Combined with the 4-stage allpass input diffusion, this creates a seamless transition. No perceptible gap.

### Room Size -> Tail Pre-delay: Compressed Proportional
**Recommendation:** Pre-delay = ER_delay_ms * 0.7, capped at 25ms. This is compressed proportional (not 1:1) because a 30ms pre-delay at maximum Room Size would create a perceptible gap. The 0.7 factor keeps the tail onset overlapping with late ERs. At minimum Room Size (1ms ER), pre-delay is ~0.7ms (negligible). At maximum (30ms ER), pre-delay is ~21ms.

### Room Size -> FDN Delay Lengths: Pre-delay Only
**Recommendation:** Room Size should NOT drive the FDN's internal delay lengths. The FDN delay lengths (20-60ms) are fixed and chosen for optimal mode density and echo density. Changing them with Room Size would alter the reverb character unpredictably and could create instability at extreme values. Room Size influence on the tail comes entirely through: (1) pre-delay length, and (2) the input signal itself (which already has the ER imprint from Stage II). This separation keeps the tail stable and predictable while still sounding room-size-appropriate.

### Minimum Room Size: Subliminal Ambience
**Recommendation:** At minimum Room Size, the reflections should be below the perceptual threshold for discrete echoes. With tap times of 1-2ms, the reflections fall within the precedence effect window (~5ms) where the brain integrates them with the direct sound. The result is subtle thickening/depth rather than audible echoes. Combined with Proximity at Near (mostly direct signal), this produces the "close-mic'd with a breath of air" character requested.

### Shape Count and Selection: 7 Presets
**Recommendation:** 7 shapes, curated for maximum acoustic distinctiveness:

| Index | Name | Character | Acoustic Basis |
|-------|------|-----------|---------------|
| 0 | The Parlour | Regular, symmetric, intimate | Small rectangular room (~4x4m), even reflection spacing |
| 1 | The Gallery | Wide, lateral, elongated | Long narrow room (~12x4m), strong side-wall reflections |
| 2 | The Chamber | Dense, even, studio-like | Medium square room (~6x6m), convergent reflections |
| 3 | The Nave | Grand, soaring, spacious | Cathedral-proportioned (~20x8m, high ceiling), floor/ceiling first |
| 4 | The Alcove | Irregular, complex, textured | Asymmetric room with recesses, unpredictable clusters |
| 5 | The Crypt | Dense, dark, close | Low-ceiling stone vault, very dense short reflections |
| 6 | The Conservatory | Bright, wide, open | Large glass room, hard surfaces, minimal absorption |

These 7 shapes cover the perceptual space: regular vs irregular, small vs large, bright vs dark, dense vs sparse, symmetric vs asymmetric. Each produces a distinctly different sonic fingerprint.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Continuous Shape float (0-1) | Discrete room geometry presets (AudioParameterChoice) | Phase 3 | Breaking change requiring version bump. Presets provide more musically meaningful choices than an abstract continuous knob. Each preset is acoustically designed rather than interpolated. |
| 4-line Householder FDN (Phase 2) | 8-line Hadamard FDN (Phase 3) | Phase 3 | Higher echo density for longer reverb delays. Hadamard mixing provides denser cross-coupling needed for smooth decay without metallic artifacts. |
| Fixed decay range 50-500ms | Extended range 50-2000ms with weighted curve | Phase 3 (per user decision) | Allows dramatic long tails while keeping practical range easily accessible. Weighted NormalisableRange gives fine control in the 50-500ms zone. |
| Static delay tap patterns | Room geometry presets with Victorian naming | Phase 3 | Each shape is an acoustically-modeled reflection pattern rather than a mathematical interpolation, producing more convincing room impressions. |

**Deprecated/outdated:**
- Continuous Shape float parameter (version 1): Replaced by AudioParameterChoice (version 3)
- 50-500ms Decay range: Extended to 50-2000ms per user decision

## Open Questions

1. **Exact shape preset tuning**
   - What we know: The 7 shapes are defined with starting-point delay times, gains, and pan positions based on architectural acoustics principles.
   - What's unclear: Whether these specific values produce the intended perceptual differences when heard in context with the full signal chain. The relative gains and absorption rates need ear-tuning.
   - Recommendation: Implement with the starting values documented above, then tune by ear during the verification task. The structure is correct; specific numbers are educated starting points.

2. **FDN delay length interaction with sample rate**
   - What we know: Delay lengths must be coprime at every sample rate. The nearestPrime() approach from Phase 2 works.
   - What's unclear: Whether the logarithmic distribution (22-63ms) produces optimal mode density at all sample rates, or whether different base distributions are needed for 44.1 vs 96 vs 192 kHz.
   - Recommendation: Use the same ms-based distribution at all sample rates (nearestPrime converts to appropriate sample counts). The mode density improves at higher sample rates (more samples per ms), so if it sounds good at 44.1kHz it will sound at least as good at higher rates.

3. **Input diffusion implementation: allpass cascade vs delay+feedback**
   - What we know: Both approaches work. Biquad allpass is simpler but limited in delay range. Explicit delay+feedback allpass gives longer delays.
   - What's unclear: Which approach sounds better for the specific delay times needed (1.5-8ms).
   - Recommendation: Use explicit delay+feedback allpass structures (not biquad). The 1.5-8ms delay times needed for input diffusion are too long for a biquad allpass, which is limited to a few samples of effective delay. Use `Delay<float>` for the delay line in each allpass stage.

4. **getTailLengthSeconds() update**
   - What we know: Currently returns 0.0. With a 2-second decay, this should be updated for correct DAW PDC and transport behavior.
   - What's unclear: Whether to report the maximum possible tail (2s) or the current parameter-dependent tail length.
   - Recommendation: Report a fixed 2.0 seconds (the maximum). JUCE's `getTailLengthSeconds()` is called infrequently and should return a conservative estimate. Reporting the dynamic value would require listener callbacks and adds complexity for minimal benefit.

5. **New parameter registration and version strategy**
   - What we know: `refl_width` is new (doesn't exist in current Parameters.h). `refl_shape` changes from float to choice. `tail_decay` range changes from 50-500 to 50-2000.
   - What's unclear: Whether to version all modified parameters to 3 or just the structurally changed ones.
   - Recommendation: Bump `refl_shape` to version 3 (type change). Register `refl_width` at version 3 (new parameter). Bump `tail_decay` to version 3 (range change affects normalized mapping). Keep other reflection/tail parameters at version 1 since their ranges/types are unchanged.

## Sources

### Primary (HIGH confidence)
- Signalsmith DSP v1.7.1 source code at `build/_deps/signalsmith-dsp-src/` -- delay.h (Delay with linear/cubic/sinc interpolation), filters.h (BiquadStatic with allpass/lowpass/highpass), mix.h (Hadamard<N>, Householder<N>, StereoMultiMixer<N>, cheapEnergyCrossfade). All APIs verified by direct source inspection during Phase 2 and this research.
- Aether Phase 2 codebase at `Source/` -- ResonanceSection.h/.cpp (working 4-line Householder FDN), ReflectionsSection.h/.cpp (stub), DiffuseTailSection.h/.cpp (stub), Parameters.h (current parameter layout), PluginProcessor.cpp (parameter caching, processBlock chain). Directly examined.
- [Signalsmith Audio - "Let's Write a Reverb"](https://signalsmith-audio.co.uk/writing/2021/lets-write-a-reverb/) -- Multi-channel FDN with Hadamard mixing, input diffusion stages, frequency-dependent decay. By the author of the signalsmith-dsp library Aether uses.
- [CCRMA FDN Reverberation](https://ccrma.stanford.edu/~jos/pasp/FDN_Reverberation.html) -- J.O. Smith, Physical Audio Signal Processing. FDN theory, feedback matrix requirements (orthogonal for stability), delay length selection (coprime), RT60-to-gain formula.
- [CCRMA Early Reflections](https://ccrma.stanford.edu/~jos/pasp/Early_Reflections.html) -- Tapped delay line implementation for early reflections, tap gain/timing for room geometry.

### Secondary (MEDIUM confidence)
- [ValhallaRoom: Early Controls](https://valhalladsp.com/2011/05/18/valhallaroom-the-early-controls/) -- Proximity control via Early/Late ratio (lower = closer), stereo cross-mixing control for width. Validates the direct/reflected ratio approach for Proximity.
- [ValhallaRoom: Early Reflections vs Early Energy](https://valhalladsp.com/2011/05/04/valhallaroom-early-reflections-versus-early-energy/) -- Psychoacoustic approach to ER design: parameter-driven impression rather than physical modeling. Validates Aether's preset-based approach.
- [Valhalla DSP: Diffusion, Allpass Delays, and Metallic Artifacts](https://valhalladsp.com/2011/01/21/reverbs-diffusion-allpass-delays-and-metallic-artifacts/) -- Allpass coefficient control as "Diffusion" parameter. Reducing coefficients reduces coloration but also reduces density. Practical range 0.0-0.7.
- [GitHub: FDN Reverb based on Signalsmith talk](https://gist.github.com/DawsonBruce/731623d598bbf1bf71505ce5efe79c14) -- Practical 8-channel FDN implementation with Hadamard mixing, velvet noise delay distribution, StereoMultiMixer upmix/downmix.
- [Audiokinetic: Image Source Approach to Dynamic Early Reflections](https://blog.audiokinetic.com/image-source-approach-to-dynamic-early-reflections/) -- Image source method for computing reflection times from room geometry. Validates the shoebox-to-cathedral approach for shape presets.
- [FDN Optimization (arxiv 2024)](https://arxiv.org/html/2402.11216v2) -- Recent FDN research confirming frequency-dependent attenuation filters for RT60 targeting, logarithmic delay distribution.
- [KVR: FDN Reverb discussion](https://www.kvraudio.com/forum/viewtopic.php?t=123095) -- Practical FDN implementation tips: Hadamard vs Householder, delay length selection, damping filter placement.
- [Buro Happold: Concert Hall Shapes](https://www.burohappold.com/articles/sculpting-sound-through-form/) -- Shoebox, fan-shaped, and cathedral acoustics. Validates the shape preset approach with real architectural acoustics data.

### Tertiary (LOW confidence)
- Room geometry tap patterns (specific delay times, gains, pan positions) -- Synthesized from architectural acoustics principles and the image source method. The relative patterns are physically motivated; specific numerical values are starting points that require ear-tuning. Shape names and descriptions are custom for Aether's Victorian aesthetic.
- Allpass diffusion delay times (1.5, 3.0, 5.0, 8.0 ms) -- Common starting values from reverb literature. Specific values may need adjustment based on how they interact with the FDN delay lengths.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH -- JUCE 8.0.12 and Signalsmith DSP 1.7.1 verified from Phase 2. All APIs confirmed from source inspection. Hadamard<8> and StereoMultiMixer<8> APIs confirmed from mix.h source.
- Architecture (ER): HIGH -- Tapped delay line for early reflections is the standard approach (Smith PASP, Valhalla Room). Shape presets with per-tap control are well-established. Stereo decorrelation via offset L/R tap times is standard practice.
- Architecture (Tail): HIGH -- 8-line Hadamard FDN with input allpass diffusion and frequency-dependent feedback damping is the standard modern reverb architecture (Signalsmith, Jot, Smith). RT60-to-gain formula is well-established.
- Cross-stage linking: HIGH -- Processor-mediated parameter forwarding is the established pattern from ARCHITECTURE.md. No novel patterns needed.
- Parameter migration: HIGH -- ParameterID version bumping is JUCE standard. Confirmed from Phase 2's successful version 1->2 bump.
- Shape presets: MEDIUM -- The DSP structure for shapes is sound, but specific tap times/gains/pan positions are educated starting points that require ear-tuning. The 7 shapes cover the perceptual space well but numerical values are not final.
- FDN tuning: MEDIUM -- Delay lengths, diffusion times, and damping curves are standard starting values that may need adjustment for Aether's specific "characterful not smooth" tail character.

**Research date:** 2026-02-18
**Valid until:** 2026-03-18 (stable domain -- no fast-moving dependencies)
