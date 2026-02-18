# Phase 5: Excitation & Room Tone - Research

**Researched:** 2026-02-18
**Domain:** Multiband soft saturation for physical room excitation modeling (Stage IV), shaped ambient noise generation for room tone (Stage V), oversampling for alias-free nonlinear processing, auto-gain compensation tuning for complete 6-stage chain, DAW transport detection for noise gating modes
**Confidence:** HIGH

## Summary

Phase 5 completes the six-stage DSP chain by implementing Stage IV (Excitation) and Stage V (Room Tone). These are the final two active processing stages before the signal reaches the output Mix/Gain section.

**Excitation (Stage IV)** models how a physical room responds nonlinearly to sound at different volumes. Unlike Crucible's SaturationSection which emulates preamp/tape character, Aether's excitation models acoustic phenomena: air compression at high SPL, surface response nonlinearity, and increased modal density in excited spaces. The key insight from acoustic physics is that real nonlinear air compression only becomes significant above ~120 dB SPL -- far beyond what room simulation targets. Therefore, the excitation model is perceptual rather than strictly physical: it uses frequency-dependent multiband soft saturation to simulate the subjective experience of a "live" room where sound gains density and liveliness at higher volumes. The implementation uses a 3-band Linkwitz-Riley crossover (low/mid/high) with per-band waveshaping curves, 2x-4x oversampling via `juce::dsp::Oversampling`, and material-coupled saturation curves from Stage I. The Drive knob (0-100%) controls intensity, with 0% still providing subtle room nonlinearity (a real room always has some acoustic effect) and bypass being the true off switch.

**Room Tone (Stage V)** adds a shaped ambient noise floor that simulates the background sound of a real room. The implementation generates white noise via `juce::Random`, shapes it through a Paul Kellett-style IIR filter cascade to produce pink-ish noise, then further sculpts it with biquad filters (HPF at ~80Hz, presence bump at 200-500Hz, LPF at ~8kHz) to match real room tone spectral characteristics. Left and right channels use independent noise generators with different random seeds for stereo decorrelation. The Ambience knob controls level (-40dB to -30dB below signal at maximum), Room Size couples to the spectral shape (larger rooms shift resonant character lower), and Shape couples to the spectral texture (different room geometries have different ambient characteristics). Three user-selectable gating modes (Always On, Signal-Gated, Transport-Only) address practical mixing concerns.

This phase also tunes the auto-gain compensation formula established in Phase 1 now that all six stages exist and contribute energy to the wet signal.

**Primary recommendation:** Implement Excitation as a self-contained multiband saturation stage using JUCE's `LinkwitzRileyFilter` for the 3-band split and `juce::dsp::Oversampling` for alias prevention. Use asymmetric soft-clipping curves (`tanh`-based, adapted from Crucible's proven pattern) with frequency-dependent drive scaling per band. Implement Room Tone as a filtered noise generator with Kellett IIR pink noise + biquad shaping, decorrelated stereo, three gating modes via `AudioPlayHead::PositionInfo::getIsPlaying()`, and Room Size/Shape coupling. Add a `tone_gate` AudioParameterChoice for the three gating modes. Tune auto-gain after both stages are operational.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

#### Saturation flavor
- Model physical room excitation, NOT preamp/console/tape character -- the nonlinearity should represent how sound excites a physical space at different volumes (air nonlinearity, surface response, modal density)
- Couple saturation curves to Material parameter from Stage I -- wood materials excite differently than stone/metal, reinforcing the physical model
- Band balance (low/mid/high curves): Claude's discretion based on physics-accurate room excitation modeling

#### Drive intensity range
- At 50%: "alive, not distorted" (per spec)
- At 90-100%: subtle edge/audible nonlinearity -- like a room being pushed hard. Noticeable but not harsh
- Input-sensitive: louder input = more excitation at same Drive setting, like a real room responding to dynamics
- At 0% Drive (stage not bypassed): subtle room nonlinearity still present -- a real room always has some acoustic effect. Drive scales from "minimal" to "lots." Bypass is the true off switch
- Couple to Room Size: small rooms excite faster/tighter, large rooms excite with more low-end bloom
- Drive response curve: Claude's discretion (sweet spot vs linear)

#### Room tone texture
- Three gating modes (user-selectable): Always On, Signal-Gated, Transport-Only (no noise when DAW transport is stopped)
- Spectral character couples to Room Size (per spec: larger = lower resonant character) AND Shape (hallway vs square room have different ambient character)
- Tone LEVEL is purely the Ambience knob -- no coupling to Proximity or other parameters. User gets full control over level for mix flexibility (critical for compressed sources like snare drums)
- Static vs modulated quality: Claude's discretion based on what sounds most like a real room

#### Excitation-to-room coupling
- Drive -> Diffuse Tail energy: Claude's discretion on whether/how to couple
- Drive -> Early Reflections energy: Claude's discretion on whether/how to couple
- Auto-gain compensation: explicitly tune in Phase 5 now that all 6 stages exist (deferred from Phase 2)
- DAW verification: verify Stages IV + V specifically (not full-chain re-verification of previously validated stages)

### Claude's Discretion

- Multiband saturation curve shapes and band crossover frequencies
- Drive response curve (sweet spot vs linear)
- Room tone modulation (static vs slowly evolving)
- Excitation coupling to downstream stages (Tail, Reflections)
- Oversampling factor within 2x-4x range
- Noise generation method for room tone

### Deferred Ideas (OUT OF SCOPE)

None -- discussion stayed within phase scope
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| EXCIT-01 | User can add nonlinear room excitation via Drive knob (0-100%), producing density and liveliness without audible distortion | 3-band multiband soft saturation with frequency-dependent curves. Drive 0-100% maps to subtle-to-moderate waveshaping intensity. At 0% (not bypassed): minimal but non-zero nonlinearity. Material coupling from Stage I adjusts saturation character per material family. |
| EXCIT-02 | Excitation uses frequency-dependent multiband soft saturation (3 bands: low, mid, high with different curves) | JUCE `LinkwitzRileyFilter` for LR4 3-band crossover. Low band (~200Hz crossover): gentlest saturation (bass needs least excitation). Mid band (~200Hz-3kHz): moderate saturation (primary density region). High band (~3kHz+): lightest saturation (prevents harshness). Per-band `tanh(x*G)/tanh(G)` waveshaping with different G ranges. |
| EXCIT-03 | Excitation uses 2x-4x oversampling to prevent aliasing artifacts | `juce::dsp::Oversampling` with `filterHalfBandFIREquiripple` and `useIntegerLatency=true`. Adaptive factor: 4x at 44.1/48kHz, 2x at 88.2/96kHz, bypass at 176.4/192kHz. Report latency via `setLatencySamples()`. Crucible's SaturationCore pattern (proven production reference). |
| EXCIT-04 | User can bypass Excitation stage independently | `excit_bypass` parameter already registered. 10ms bypass crossfade via `SmoothedValue<float>` (established pattern from ResonanceSection, AirSection). |
| EXCIT-05 | At 50% drive, the effect sounds like "louder/more alive" not "distorted" | Drive response curve with sweet spot around 40-60%: use pow(drive, 0.7) mapping so the 0-60% range covers most of the "alive" territory and 60-100% introduces progressively more edge. Auto-gain compensation within the stage to maintain perceived loudness. |
| TONE-01 | User can add shaped ambient noise floor via Ambience knob (0-100%) | Pink noise via Kellett IIR filter (white noise -> pink), further shaped by biquad filters for room tone spectral character. Ambience knob controls level with SmoothedValue ramping. At 100%: -40dB to -30dB below signal level. |
| TONE-02 | Noise is shaped to sound like actual room tone: rolled off below ~80Hz, presence bump 200-500Hz, rolled off above ~8kHz | 3-stage biquad shaping chain per channel: HPF at 80Hz (2nd order), peak filter at 300Hz (+3-5dB, Q=0.8), LPF at 8kHz (2nd order). These approximate measured room tone spectra. Room Size modulates filter parameters. |
| TONE-03 | Noise character subtly changes based on Room Size setting (larger rooms = lower resonant character) | Room Size 0.0-1.0 shifts: presence peak frequency (400Hz at small -> 200Hz at large), LPF cutoff (10kHz at small -> 6kHz at large), HPF cutoff (100Hz at small -> 60Hz at large). Larger rooms have lower-frequency ambient character due to longer modal wavelengths. |
| TONE-04 | Left and right channels use decorrelated noise generators | Two independent `juce::Random` instances with different seeds. Each generates its own white noise stream, independently filtered through separate Kellett IIR and biquad chains. Seeds offset by a large prime number for statistical independence. |
| TONE-05 | At 100%, noise is very quiet relative to signal (-40dB to -30dB below signal) | Ambience 0-100% maps to gain range of -inf to approximately -35dB (midpoint of spec). Exact level calibrated against a -18dBFS reference signal. Level is purely Ambience knob controlled -- no coupling to other parameters. |
| TONE-06 | Room Tone stage defaults to bypassed (Out) | `tone_bypass` parameter already registered with default `true` in Parameters.h. |
| TONE-07 | User can bypass Room Tone stage independently | Same 10ms bypass crossfade pattern as all other stages. When bypassed, noise generation can skip entirely (CPU savings). |
</phase_requirements>

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| JUCE | 8.0.12 | `dsp::Oversampling`, `dsp::LinkwitzRileyFilter`, `AudioPlayHead::PositionInfo`, `SmoothedValue`, `AudioParameterChoice`, `Random` | Already established in Phases 1-4. All needed APIs verified from official docs. |
| Signalsmith DSP | 1.7.1 | `BiquadStatic` filters for room tone shaping (HPF, LPF, peak), `cheapEnergyCrossfade` for bypass blending | Already linked. Provides all filter primitives for Room Tone spectral shaping. Consistent with AirSection pattern. |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `juce::dsp::Oversampling<float>` | (JUCE 8) | 2x-4x oversampling for Excitation waveshaping | Anti-aliasing for nonlinear processing. Crucible uses this exact class for saturation. Constructor: `Oversampling(2, order, filterHalfBandFIREquiripple, true, true)`. `processSamplesUp()` / `processSamplesDown()`. Reports latency via `getLatencyInSamples()`. |
| `juce::dsp::LinkwitzRileyFilter<float>` | (JUCE 8) | 3-band crossover for multiband saturation | LR4 (24dB/oct) crossover. Phase-coherent: low+high sums to allpass. Two instances create 3 bands: LP1+HP1 at low crossover, LP2+HP2 at high crossover applied to HP1 output. |
| `juce::Random` | (JUCE 8) | White noise generation for room tone | Thread-safe PRNG. Two instances with different seeds for L/R decorrelation. `nextFloat()` returns 0.0-1.0, scale to -1.0 to 1.0. |
| `juce::AudioPlayHead` | (JUCE 8) | Transport state detection for Transport-Only gating mode | `getPosition()` returns `Optional<PositionInfo>`. `getIsPlaying()` returns `Optional<bool>`. Must check for nullopt (not all hosts provide transport info). Access only from `processBlock()`. |
| `signalsmith::filters::BiquadStatic<float>` | 1.7.1 | Room tone shaping filters (HPF, peak, LPF) | `highpassQ()`, `peakDb()`, `lowpassQ()` for spectral sculpting. Same API used throughout AirSection. |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| JUCE `LinkwitzRileyFilter` for 3-band split | Signalsmith `BiquadStatic` with manual LR4 construction | JUCE LR filter is a single class that handles the cascaded Butterworth internally. More robust than manually cascading two biquads. Simpler API. |
| JUCE `Random` for white noise | `std::mt19937` / `std::uniform_real_distribution` | `juce::Random` is lighter weight, already available, and sufficient for noise generation quality. Mersenne Twister is overkill for ambient noise floor. |
| Kellett IIR for pink noise | Voss-McCartney algorithm (stacked octave generators) | Kellett IIR is simpler (7 state variables per channel), computationally lighter (7 multiply-accumulates per sample), and produces pink noise accurate to +/-0.05dB above 9.2Hz at 44.1kHz. Voss-McCartney requires more state and branching logic. For room tone that will be further filtered, Kellett's accuracy is more than sufficient. |
| `tanh(x*G)/tanh(G)` waveshaper | Polynomial soft clipper `1.5x - 0.5x^3` | tanh produces more natural-sounding saturation with better behavior at extreme inputs (always bounded). Polynomial clippers can produce hard discontinuities at the clipping threshold. Crucible's production-proven tanh waveshaper confirms the approach. |
| 3-band multiband | 2-band (sub protection only, like Crucible) | Aether's excitation needs frequency-dependent behavior across the full spectrum: less excitation in lows (prevents mud), moderate in mids (density/liveliness), gentler in highs (prevents harshness). 3 bands provide this control. Crucible uses 2-band because its sub crossover is protective, not creative. |

## Architecture Patterns

### Recommended Changes to Project Structure

```
Source/
  Parameters.h           -> Add tone_gate (3-choice: Always On, Signal-Gated, Transport-Only)
                            Version bump excit_drive and tone_amb if parameter behavior changes
  PluginProcessor.h      -> No structural changes (param pointers already cached)
  PluginProcessor.cpp    -> Extend updateStageParams() for Excitation (Drive, Material, RoomSize coupling)
                            and RoomTone (Ambience, RoomSize, Shape, Gate mode, transport state)
                            Update setLatencySamples() to include oversampling latency
                            Tune auto-gain compensation formula
  dsp/
    ExcitationSection.h  -> Complete implementation: 3-band crossover, per-band waveshaping,
                            oversampling, material coupling, room size coupling, auto-gain
    ExcitationSection.cpp -> Full DSP processing
    RoomToneSection.h    -> Complete implementation: noise gen, Kellett pink filter,
                            spectral shaping, stereo decorrelation, gating modes
    RoomToneSection.cpp  -> Full DSP processing
```

### Pattern 1: 3-Band Linkwitz-Riley Crossover for Multiband Saturation

**What:** Split audio into low, mid, and high bands using two LR4 crossover points. Process each band independently through waveshapers with different drive curves. Recombine by summation (LR4 guarantees phase-coherent reconstruction).

**When to use:** The core signal flow of the Excitation stage.

```
Signal Flow:
  Input -> [LR4 LP @ 200Hz]  -> lowBand  -> [waveshape_low]  ----+
        -> [LR4 HP @ 200Hz]  -> midHigh  -> [LR4 LP @ 3kHz] --->|
                                          -> [LR4 HP @ 3kHz] --->|
                                     midBand -> [waveshape_mid]  -+-> Sum -> Output
                                     highBand -> [waveshape_high]-+

3-band implementation using 2 LR4 crossover pairs:
  Crossover 1 (low/mid+high): LP1 + HP1 at ~200Hz
  Crossover 2 (mid/high): LP2 + HP2 at ~3kHz, applied to HP1 output

  lowBand  = LP1(input)
  midBand  = LP2(HP1(input))
  highBand = HP2(HP1(input))
  output   = lowBand + midBand + highBand
```

**Key implementation detail:** JUCE's `LinkwitzRileyFilter` provides `processSample()` for per-sample processing inside the oversampled loop. Each filter instance internally manages LR4 coefficients. The crossover frequencies can be fixed (no need to modulate them in real time).

### Pattern 2: Room-Excitation Waveshaping (Not Gear Emulation)

**What:** The waveshaping curves model how sound excites a physical space, not how electronic components distort. This means: symmetric saturation (no even-harmonic bias from DC offset), frequency-dependent intensity (rooms excite mids more than lows or highs), and input-sensitivity (louder input = more excitation, like a real room responding to dynamics).

**When to use:** All waveshaping within the Excitation stage.

```cpp
// Normalized symmetric waveshaper: tanh(x*G)/tanh(G)
// Unlike Crucible's asymmetric tape model (which adds DC bias for even harmonics),
// room excitation is symmetric -- a room excites positive and negative pressure
// waves identically.
float waveshape(float sample, float G)
{
    // G < 0.5: nearly linear passthrough (subtle room presence)
    // G = 1.0-2.0: alive, dense, lively
    // G > 3.0: audible edge (room being pushed hard)
    return std::tanh(sample * G) / std::tanh(G);
}

// Per-band drive scaling (physics motivation):
// - Low band: less excitation (bass wavelengths couple less to room surfaces)
// - Mid band: most excitation (primary resonance range of rooms)
// - High band: moderate excitation (absorbed more by surfaces, less sustained)
static constexpr float kLowDriveScale  = 0.5f;   // 50% of Drive
static constexpr float kMidDriveScale  = 1.0f;   // 100% of Drive
static constexpr float kHighDriveScale = 0.6f;    // 60% of Drive
```

### Pattern 3: Material-Coupled Excitation

**What:** The Material parameter from Stage I (Resonance) influences how the room excites. Wood materials produce warmer, more resonant excitation. Metal materials produce brighter, tighter excitation. Stone materials produce darker, more diffuse excitation. This is implemented as a per-band gain modifier based on material family.

**When to use:** Cross-stage coupling from Stage I to Stage IV.

```cpp
// Material families and their excitation character
// Woods (0-3): More mid excitation, warmer harmonic generation
// Metals (4-6): More high excitation, tighter, brighter
// Stones (7-9): More low excitation, diffuse, less responsive

struct MaterialExcitationBias
{
    float lowScale;   // Multiplier on low-band drive
    float midScale;   // Multiplier on mid-band drive
    float highScale;  // Multiplier on high-band drive
};

static constexpr MaterialExcitationBias kMaterialBias[] = {
    // Woods: warm, resonant -- boost mids, gentle highs
    { 0.9f, 1.15f, 0.7f },  // Pine (open, resonant)
    { 0.95f, 1.1f, 0.75f }, // Oak (dense, structured)
    { 0.9f, 1.12f, 0.72f }, // Walnut (rich, balanced)
    { 0.85f, 1.2f, 0.68f }, // Mahogany (deep, warm)
    // Metals: bright, focused -- boost highs, tighter lows
    { 0.7f, 0.95f, 1.15f }, // Iron (forceful, edgy)
    { 0.65f, 0.9f, 1.2f },  // Steel (brilliant, precise)
    { 0.75f, 1.0f, 1.1f },  // Copper (warm brilliance)
    // Stones: deep, diffuse -- boost lows, gentle highs
    { 1.15f, 0.95f, 0.65f },// Limestone (soft, diffuse)
    { 1.1f, 1.0f, 0.7f },   // Marble (polished, even)
    { 1.2f, 0.9f, 0.6f },   // Granite (massive, dense)
};
```

### Pattern 4: Room Size Coupling to Excitation

**What:** Small rooms excite faster and tighter (the nonlinearity onsets quickly with less low-end bloom). Large rooms excite with more low-end bloom (longer wavelengths have more opportunity to build up). This is implemented by modifying the per-band drive scales based on Room Size.

**When to use:** Cross-stage coupling from Stage II (Room Size parameter) to Stage IV.

```cpp
// Room Size influence on excitation character
// Small room (0.0): tighter excitation, less low bloom, faster response
// Large room (1.0): more low-end bloom, slower buildup, broader excitation

float roomSizeInfluence = roomSizeNormalized;  // 0.0-1.0

// Modify per-band scales
float effectiveLowScale  = kLowDriveScale  * (0.7f + 0.6f * roomSizeInfluence);
// Small=0.7*0.5=0.35, Large=1.3*0.5=0.65 (more low bloom in big rooms)

float effectiveMidScale  = kMidDriveScale;  // Mids stay consistent
// Room size doesn't dramatically change mid excitation

float effectiveHighScale = kHighDriveScale * (1.1f - 0.3f * roomSizeInfluence);
// Small=1.1*0.6=0.66, Large=0.8*0.6=0.48 (less high excitation in big rooms)
```

### Pattern 5: Kellett IIR Pink Noise Generator

**What:** Paul Kellett's refined method converts white noise to pink noise using 7 first-order IIR filter stages. Each stage is a simple one-pole filter at a different frequency, and their outputs sum to produce a -3dB/octave spectral slope. This is the most CPU-efficient method for high-quality pink noise generation.

**When to use:** The noise generation core of the Room Tone stage.

```cpp
// Kellett "refined" pink noise filter
// 7 state variables (b0-b6) per channel
// Accurate to +/-0.05dB above 9.2Hz at 44100Hz
// Source: musicdsp.org/en/latest/Filters/76-pink-noise-filter.html

struct KellettPinkNoise
{
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, b3 = 0.0f;
    float b4 = 0.0f, b5 = 0.0f, b6 = 0.0f;

    float process(float white)
    {
        b0 = 0.99886f * b0 + white * 0.0555179f;
        b1 = 0.99332f * b1 + white * 0.0750759f;
        b2 = 0.96900f * b2 + white * 0.1538520f;
        b3 = 0.86650f * b3 + white * 0.3104856f;
        b4 = 0.55000f * b4 + white * 0.5329522f;
        b5 = -0.7616f * b5 - white * 0.0168980f;
        float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
        b6 = white * 0.115926f;
        return pink * 0.11f;  // Normalize to approximately -1 to +1
    }

    void reset()
    {
        b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0.0f;
    }
};
```

**Note on sample rate dependence:** The Kellett coefficients are derived for 44.1kHz. At higher sample rates, the spectral shape will deviate slightly above ~10kHz. Since room tone is shaped by additional biquad filters that roll off above 8kHz anyway, this deviation is inaudible and does not require sample-rate-dependent coefficients.

### Pattern 6: Room Tone Gating Modes

**What:** Three user-selectable modes for when room tone is audible: Always On (constant), Signal-Gated (fades when input is silent), Transport-Only (muted when DAW transport is stopped). This addresses practical mixing concerns -- compressed sources like snare drums react very differently to constant room tone.

**When to use:** The gating logic in RoomToneSection's process loop.

```cpp
enum class GateMode { AlwaysOn = 0, SignalGated = 1, TransportOnly = 2 };

// In processBlock (PluginProcessor), before calling roomToneSection.process():
// Query transport state for Transport-Only mode
bool transportPlaying = true;  // default: assume playing
if (auto* playHead = getPlayHead())
{
    if (auto pos = playHead->getPosition())
    {
        if (auto playing = pos->getIsPlaying())
            transportPlaying = *playing;
    }
}
roomToneSection.setTransportPlaying(transportPlaying);

// In RoomToneSection::process():
// Signal-Gated: measure input RMS, fade noise with envelope follower
// Transport-Only: use transportPlaying flag
// Always On: no gating, noise is always present
float gateMultiplier = 1.0f;

if (gateMode == GateMode::TransportOnly && !transportPlaying)
    gateMultiplier = 0.0f;  // Mute when transport stopped

else if (gateMode == GateMode::SignalGated)
{
    // Simple envelope follower on input signal
    float inputLevel = computeRMS(buffer);
    // Attack: ~5ms, Release: ~200ms (noise fades slowly when signal stops)
    float target = (inputLevel > gateThreshold) ? 1.0f : 0.0f;
    gateEnvelope.setTargetValue(target);
    gateMultiplier = gateEnvelope.getNextValue();
}
// gateMultiplier applied to noise level before mixing into signal
```

### Pattern 7: Adaptive Oversampling Based on Sample Rate

**What:** The oversampling factor adapts to the base sample rate to maintain a reasonable CPU budget. At 44.1/48kHz, 4x oversampling processes at 176.4/192kHz. At 88.2/96kHz, 2x oversampling processes at 176.4/192kHz. At 176.4/192kHz, no oversampling (the base rate is already high enough that aliasing from soft saturation is above the audible range).

**When to use:** In ExcitationSection::prepare(), when creating the Oversampling instance.

```cpp
void ExcitationSection::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Adaptive oversampling: target ~176-192kHz processing rate
    if (sampleRate <= 50000.0)       // 44.1kHz, 48kHz
        oversamplingOrder = 2;       // 4x (2^2)
    else if (sampleRate <= 100000.0) // 88.2kHz, 96kHz
        oversamplingOrder = 1;       // 2x (2^1)
    else                              // 176.4kHz, 192kHz
        oversamplingOrder = 0;       // 1x (no oversampling)

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        2,  // stereo
        oversamplingOrder,
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
        true,   // isMaxQuality
        true);  // useIntegerLatency
    oversampler->initProcessing(static_cast<size_t>(samplesPerBlock));

    // Report latency to processor for DAW PDC
    latencySamples = static_cast<int>(oversampler->getLatencyInSamples());
}
```

### Pattern 8: Auto-Gain Compensation Tuning

**What:** The existing auto-gain formula from Phase 1 (`-2.5dB * pow(mix, 1.5)`) was designed for the full 6-stage chain but tuned when stages were passthrough. Now that all stages add energy, the formula needs empirical adjustment. The key insight: Excitation adds energy (saturation generates harmonics), while other stages may add or subtract energy depending on settings.

**When to use:** After both Excitation and Room Tone stages are operational. This is a calibration task, not a design task.

```
Current formula: compensationDb = -2.5 * pow(mix, 1.5)
  At Mix=0%:   0.0 dB
  At Mix=50%: -0.88 dB
  At Mix=100%: -2.5 dB

Tuning approach:
  1. Set all stages to moderate settings (mid-range knobs)
  2. Measure output level at Mix=0% vs Mix=100%
  3. Adjust the -2.5dB coefficient and 1.5 exponent until perceived
     loudness is approximately matched between Mix=0% and Mix=100%
  4. Test with multiple signal types (DI guitar, snare, full mix)
  5. Accept imperfection: auto-gain is approximate, not perfect
```

### Anti-Patterns to Avoid

- **Asymmetric saturation for room excitation:** Unlike Crucible's tape saturation (which uses DC bias for even harmonics), room excitation is physically symmetric -- air compression affects positive and negative pressure waves identically. Do NOT add DC bias to the waveshaper. If even harmonics are desired for warmth, achieve this through the frequency-dependent band balance (more mid saturation creates warmth) rather than asymmetry.

- **Over-coupling Excitation to downstream stages:** The user left Drive-to-Tail and Drive-to-Reflections coupling as Claude's discretion. My recommendation is NO direct coupling in Phase 5. The Excitation stage's output naturally feeds into Room Tone and Diffuse Tail in the serial chain. The signal already carries the excitation character downstream. Adding explicit parameter coupling (e.g., Drive increases Tail energy) would create a feedback-like compounding effect that is hard to control and potentially confusing to the user. The implicit signal-flow coupling is sufficient and physically correct.

- **Allocating noise buffers in processBlock:** Pre-allocate all noise work buffers in `prepare()`. The pink noise filter state (7 floats per channel) is trivially small. The biquad shaping filters are member variables. No allocation is needed in the process loop.

- **Using `std::rand()` for noise generation:** `std::rand()` is not thread-safe and produces low-quality random numbers. Use `juce::Random` which is designed for audio applications.

- **Noise generation at oversampled rate:** Room Tone noise does NOT need oversampling. It is a purely additive signal, not a nonlinear operation. Generate noise at the base sample rate and add it to the signal after the Excitation stage's oversampled processing completes.

- **Gate mode switching without crossfade:** When the user changes gate mode (e.g., from Always On to Transport-Only while noise is playing), the noise level must crossfade over ~20ms to avoid a click. Use `SmoothedValue` on the gate multiplier.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Multiband crossover | Custom Butterworth cascade | `juce::dsp::LinkwitzRileyFilter` | JUCE's LR filter handles the internal cascaded Butterworth correctly and guarantees phase-coherent reconstruction. Manual construction is error-prone (coefficient calculation, filter state management). |
| Oversampling | Custom upsampling/downsampling with polyphase filters | `juce::dsp::Oversampling` | Production-proven in Crucible. Handles filter design, latency reporting, buffer management. Integer latency mode for DAW PDC. |
| Pink noise generation | Voss-McCartney stacked generator | Kellett IIR filter on white noise | 7 multiply-accumulates per sample vs branching logic + multiple generator states. Simpler, lighter, accurate enough for room tone that gets further filtered. |
| White noise generation | `std::rand()` or custom PRNG | `juce::Random` | Thread-safe, audio-quality randomness, already available in JUCE. |
| Transport state detection | Custom MIDI clock parsing | `AudioPlayHead::PositionInfo::getIsPlaying()` | Standard JUCE API. Works across all DAW hosts. Handles edge cases (standalone mode, hosts that don't report transport). |
| Parameter smoothing | Custom ramp | `juce::SmoothedValue<float>` | Established pattern. Handles sample rate, provides `isSmoothing()` optimization. |
| DC blocking | Custom highpass | `juce::dsp::IIR::Filter` with 5Hz HPF coefficients | Crucible's pattern. Removes DC offset from any residual asymmetry in the waveshaper. |
| Bypass crossfade | Custom blend | `SmoothedValue<float>` for blend factor | Same 10ms pattern from ResonanceSection, AirSection. |

**Key insight:** The creative work in this phase is tuning the per-band saturation curves and room tone spectral shaping to produce a physically motivated, musically useful result. All DSP infrastructure (crossover, oversampling, noise generation, filtering, smoothing) comes from JUCE and libraries already proven in Phases 1-4 and Crucible.

## Common Pitfalls

### Pitfall 1: Excitation Sounds Like Guitar Distortion Instead of Room Liveliness
**What goes wrong:** The Drive knob produces a sound that resembles guitar amp distortion -- crunchy, edgy, with obvious harmonic content. Users perceive it as a distortion effect, not room character.
**Why it happens:** Drive range is too wide, allowing the waveshaper to push into heavy saturation territory. The saturation curves are too aggressive, or the mid band dominates too much.
**How to avoid:** Keep the maximum Drive (100%) at subtle-to-moderate levels. The maximum G value for the waveshaper should produce noticeable but not harsh nonlinearity. A good test: at Drive 100%, the effect should sound like "the room is being pushed hard and you can hear the walls vibrating" not "someone turned on an overdrive pedal." Cap the effective G per band: low G_max ~1.5, mid G_max ~2.5, high G_max ~1.8. At 50% Drive, G should be in the ~0.5-1.0 range where tanh is barely nonlinear.
**Warning signs:** Users comparing Drive to a distortion plugin rather than to "room character." Visible harmonic distortion on a spectrum analyzer at moderate Drive settings.

### Pitfall 2: Oversampling Latency Not Reported to DAW
**What goes wrong:** DAW plugin delay compensation is incorrect. Parallel tracks drift. Side-chained signals are mis-aligned.
**Why it happens:** The oversampling latency is not forwarded to `setLatencySamples()` in the processor. Or the latency changes when sample rate changes (adaptive oversampling) but `setLatencySamples()` is not called in `prepareToPlay()`.
**How to avoid:** In `prepareToPlay()`, after creating the Oversampling instance, call `setLatencySamples(excitationSection.getLatencySamples())`. The Excitation stage is the ONLY stage that introduces latency (all other stages are zero-latency). When sample rate changes trigger a different oversampling order, the new latency is automatically reported via `prepareToPlay()`.
**Warning signs:** Phase cancellation when mixing the plugin with a parallel dry track. Audio drift when automating bypass on/off.

### Pitfall 3: Aliasing at High Frequencies Despite Oversampling
**What goes wrong:** High-frequency test tones (8kHz+) produce audible alias frequencies that were not present in the input, despite oversampling being active.
**Why it happens:** The oversampling order is insufficient for the saturation intensity. Or the waveshaper is applied before upsampling rather than to the oversampled signal.
**How to avoid:** Process the waveshaping inside the oversampled block (between `processSamplesUp()` and `processSamplesDown()`). At 44.1kHz with 4x oversampling, the Nyquist is 88.2kHz -- harmonics up to the 4th are alias-free for content up to 22kHz. For the mild saturation Aether uses (G max ~2.5), 4x is more than sufficient. Verify with a 10kHz sine at 100% Drive: the output should contain 10kHz and 20kHz (2nd harmonic) with no content below 10kHz.
**Warning signs:** New spectral content below the input frequency. Metallic or harsh quality at high frequencies. Different artifacts at different sample rates.

### Pitfall 4: Room Tone Sounds Like White Noise, Not a Real Room
**What goes wrong:** The noise floor sounds obviously synthetic -- hissy, bright, even across the spectrum. Users perceive it as "noise" rather than "room ambience."
**Why it happens:** Insufficient spectral shaping. Pink noise alone is not enough -- real room tone has characteristic resonant peaks and steep rolloffs.
**How to avoid:** The shaping chain must include: (1) Pink noise base (Kellett IIR), (2) HPF at 80Hz to remove sub-bass rumble, (3) Resonant peak at 200-500Hz (the "room mode" region), (4) LPF at 6-10kHz (room absorption kills high frequencies). Additionally, very slow amplitude modulation (0.1-0.3Hz, +/-1dB) prevents the noise from sounding static. Real room tone has subtle level fluctuations from air currents and environmental changes. Test: record yourself in a quiet room with a high-gain mic. Compare to the generated room tone.
**Warning signs:** The noise sounds the same at all Room Size settings. The noise sounds obviously "layered on top" rather than "part of the room." Brightness above 8kHz is immediately audible.

### Pitfall 5: Signal-Gated Mode Creates Pumping Artifacts
**What goes wrong:** In Signal-Gated mode, the room tone audibly "pumps" -- rising when signal is present and dropping when it stops, creating a distracting modulation effect.
**Why it happens:** The gate envelope follower's attack and release times are too fast, or the threshold is too sensitive.
**How to avoid:** Use a slow attack (~50-100ms) and very slow release (~500ms-1s) for the gate envelope. The noise should fade in gradually when signal arrives and linger for up to a second after signal stops. The threshold should be set low enough that only true silence (no signal at all) triggers the gate -- background noise from the input should keep the gate open. A good default threshold: -60dBFS.
**Warning signs:** Audible noise level changes during sustained playing. The gate opens and closes during natural playing dynamics (should only close during true silence). Noise "breathes" in sync with the music.

### Pitfall 6: Auto-Gain Overcompensation After Full Chain Is Active
**What goes wrong:** After Excitation and Room Tone are implemented, the auto-gain compensation is too aggressive or too mild. At Mix=100%, the output is noticeably louder or quieter than at Mix=0%.
**Why it happens:** The Phase 1 auto-gain formula (`-2.5dB * pow(mix, 1.5)`) was designed for the full chain but calibrated when all stages were passthrough. Now that stages add energy (especially Excitation), the compensation needs recalibration.
**How to avoid:** After both stages are implemented, perform empirical calibration: set all stages to moderate settings, play a reference signal, compare output at Mix=0% vs Mix=100%, adjust the compensation coefficient and exponent. Accept that auto-gain is approximate -- it cannot perfectly compensate across all parameter combinations. The goal is "close enough that the user doesn't notice a volume jump when sweeping Mix."
**Warning signs:** Users reporting volume changes when sweeping the Mix knob. Level meter jumping when toggling between Mix=0% and Mix=100%.

## Code Examples

### ExcitationSection Class Interface

```cpp
// Source: Adapted from Crucible's SaturationSection pattern,
// modified for room excitation modeling (symmetric, multiband, material-coupled)

class ExcitationSection
{
public:
    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();
    void setBypass(bool bypassed);

    void setDrive(float driveNormalized);      // 0.0-1.0
    void setMaterial(int materialIndex);        // 0-9 (from Stage I)
    void setRoomSize(float roomSizeNormalized); // 0.0-1.0 (from Stage II)

    int getLatencySamples() const;

private:
    void updateDriveParams();  // Recompute per-band G values from Drive + Material + RoomSize

    // 3-band crossover (2 LR4 filter pairs)
    juce::dsp::LinkwitzRileyFilter<float> lowCrossLP, lowCrossHP;   // @ ~200Hz
    juce::dsp::LinkwitzRileyFilter<float> highCrossLP, highCrossHP; // @ ~3kHz

    // Oversampling
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    int oversamplingOrder = 2;  // 4x default at 44.1kHz

    // DC blocker (removes any residual offset)
    juce::dsp::IIR::Filter<float> dcBlockerL, dcBlockerR;

    // Work buffers (pre-allocated in prepare)
    juce::AudioBuffer<float> lowBand, midBand, highBand;

    // Per-band waveshaping gain (G parameter in tanh(x*G)/tanh(G))
    float lowG  = 0.3f;
    float midG  = 0.5f;
    float highG = 0.35f;

    // Smoothed parameters
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassBlend;

    // Coupling state
    int currentMaterialIndex = 3;    // Mahogany default
    float currentRoomSize = 0.4f;    // Medium default
    float currentDrive = 0.25f;      // 25% default

    // State
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    bool bypassed = false;
    int latencySamples = 0;

    // Crossover frequencies (Hz)
    static constexpr float kLowCrossFreq  = 200.0f;
    static constexpr float kHighCrossFreq = 3000.0f;

    // Drive curve: pow(drive, 0.7) for sweet spot in 30-60% range
    // Then scaled per band and per material
    static constexpr float kDriveCurveExponent = 0.7f;

    // Base per-band drive scaling (before material and room size modifiers)
    static constexpr float kBaseLowScale  = 0.5f;
    static constexpr float kBaseMidScale  = 1.0f;
    static constexpr float kBaseHighScale = 0.6f;

    // Maximum G values (caps distortion at reasonable levels)
    static constexpr float kMaxLowG  = 1.5f;
    static constexpr float kMaxMidG  = 2.5f;
    static constexpr float kMaxHighG = 1.8f;

    // Minimum G at Drive=0% (subtle room presence when not bypassed)
    static constexpr float kMinG = 0.15f;
};
```

### RoomToneSection Class Interface

```cpp
class RoomToneSection
{
public:
    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();
    void setBypass(bool bypassed);

    void setAmbience(float ambienceNormalized);     // 0.0-1.0
    void setRoomSize(float roomSizeNormalized);      // 0.0-1.0 (from Stage II)
    void setShape(int shapeIndex);                   // 0-6 (from Stage II)
    void setGateMode(int gateModeIndex);             // 0=AlwaysOn, 1=SignalGated, 2=TransportOnly
    void setTransportPlaying(bool isPlaying);        // From AudioPlayHead

private:
    void updateShapingFilters();  // Recalculate filter params from RoomSize + Shape

    // Noise generators (decorrelated L/R)
    juce::Random rngL, rngR;

    // Kellett pink noise filters (one per channel)
    KellettPinkNoise pinkL, pinkR;

    // Spectral shaping filters (one set per channel)
    signalsmith::filters::BiquadStatic<float> hpfL, hpfR;       // HPF ~80Hz
    signalsmith::filters::BiquadStatic<float> presenceL, presenceR; // Peak ~300Hz
    signalsmith::filters::BiquadStatic<float> lpfL, lpfR;       // LPF ~8kHz

    // Smoothed parameters
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> ambienceSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassBlend;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gateEnvelope;

    // Slow modulation LFO (very slow amplitude variation for natural quality)
    float lfoPhase = 0.0f;
    static constexpr float kLfoRateHz = 0.15f;  // ~7 second cycle
    static constexpr float kLfoDepthDb = 0.8f;  // +/- 0.8dB variation

    // Gating
    enum class GateMode { AlwaysOn = 0, SignalGated = 1, TransportOnly = 2 };
    GateMode gateMode = GateMode::AlwaysOn;
    bool transportPlaying = true;
    static constexpr float kGateThresholdDb = -60.0f;
    static constexpr float kGateAttackMs = 50.0f;
    static constexpr float kGateReleaseMs = 500.0f;

    // Room size / shape coupling
    float currentRoomSize = 0.4f;
    int currentShapeIndex = 0;

    // State
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    bool bypassed = true;  // Default bypassed (TONE-06)

    // Level calibration: at Ambience=100%, noise is -35dB below -18dBFS reference
    // -18dBFS + (-35dB) = -53dBFS noise floor
    static constexpr float kMaxNoiseGainDb = -35.0f;
};
```

### Excitation Process Loop (Core)

```cpp
void ExcitationSection::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 1 || numSamples < 1)
        return;

    // Early exit if fully bypassed and not smoothing
    if (!bypassBlend.isSmoothing() && bypassBlend.getTargetValue() <= 0.0f)
        return;

    // Store dry copy for bypass crossfade
    // (Use pre-allocated buffer, NOT allocation)
    juce::AudioBuffer<float> dryCopy;  // Actually a member, pre-allocated in prepare()
    dryCopy.makeCopyOf(buffer);

    // 1. Split into 3 bands using LR4 crossovers
    //    lowBand = LP(input) at kLowCrossFreq
    //    midHighBand = HP(input) at kLowCrossFreq
    //    midBand = LP(midHighBand) at kHighCrossFreq
    //    highBand = HP(midHighBand) at kHighCrossFreq

    // Process per-sample through crossovers...
    // (LR filters are per-sample, working on the buffer directly)

    // 2. Oversample each band, waveshape, downsample
    //    Each band processed through oversampler separately
    //    OR: sum bands, oversample once, waveshape with band-dependent G
    //    (Implementation detail: band separation before oversampling is cleaner)

    // 3. Per-band waveshaping at oversampled rate:
    //    for each oversampled sample:
    //      lowBand[s]  = tanh(lowBand[s] * lowG) / tanh(lowG)
    //      midBand[s]  = tanh(midBand[s] * midG) / tanh(midG)
    //      highBand[s] = tanh(highBand[s] * highG) / tanh(highG)

    // 4. Recombine bands: output = lowBand + midBand + highBand

    // 5. DC blocker at original rate

    // 6. Bypass crossfade
    //    output = dry * (1 - blend) + wet * blend
}
```

### Room Tone Process Loop (Core)

```cpp
void RoomToneSection::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 1 || numSamples < 1)
        return;

    // Early exit if fully bypassed and not smoothing
    if (!bypassBlend.isSmoothing() && bypassBlend.getTargetValue() <= 0.0f)
        return;

    float* channelL = buffer.getWritePointer(0);
    float* channelR = (numChannels >= 2) ? buffer.getWritePointer(1) : nullptr;

    for (int s = 0; s < numSamples; ++s)
    {
        float blend = bypassBlend.getNextValue();
        float ambience = ambienceSmoothed.getNextValue();

        // Compute gate multiplier based on mode
        float gate = 1.0f;
        if (gateMode == GateMode::TransportOnly && !transportPlaying)
            gate = 0.0f;
        else if (gateMode == GateMode::SignalGated)
            gate = gateEnvelope.getNextValue();

        // Generate decorrelated white noise
        float whiteL = rngL.nextFloat() * 2.0f - 1.0f;
        float whiteR = rngR.nextFloat() * 2.0f - 1.0f;

        // Pink noise via Kellett IIR
        float pinkSampleL = pinkL.process(whiteL);
        float pinkSampleR = pinkR.process(whiteR);

        // Spectral shaping: HPF -> Presence Peak -> LPF
        float shapedL = hpfL(pinkSampleL);
        shapedL = presenceL(shapedL);
        shapedL = lpfL(shapedL);

        float shapedR = hpfR(pinkSampleR);
        shapedR = presenceR(shapedR);
        shapedR = lpfR(shapedR);

        // Slow modulation for natural quality
        float lfoValue = std::sin(lfoPhase * 2.0f * juce::MathConstants<float>::pi);
        float modGainDb = lfoValue * kLfoDepthDb;
        float modGain = juce::Decibels::decibelsToGain(modGainDb, -100.0f);
        lfoPhase += kLfoRateHz / static_cast<float>(currentSampleRate);
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // Apply level: ambience knob * gate * modulation
        float noiseGainDb = kMaxNoiseGainDb * ambience;  // Scales from -inf to kMaxNoiseGainDb
        // Better: map ambience 0-1 to gain curve
        float noiseGain = (ambience > 0.001f)
            ? juce::Decibels::decibelsToGain(kMaxNoiseGainDb + (1.0f - ambience) * 40.0f, -100.0f)
            : 0.0f;
        float totalGain = noiseGain * gate * modGain * blend;

        // Additive: noise is ADDED to signal, not replacing it
        channelL[s] += shapedL * totalGain;
        if (channelR != nullptr)
            channelR[s] += shapedR * totalGain;
    }
}
```

### New Parameter: tone_gate

```cpp
// In Parameters.h: Add gate mode choice parameter
namespace ParamIDs
{
    // ... existing params ...
    inline constexpr auto toneGate { "tone_gate" };
}

// In createParameterLayout():
layout.add(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID { ParamIDs::toneGate, 1 },
    "V Room Tone - Gate",
    juce::StringArray { "Always On", "Signal-Gated", "Transport-Only" },
    0  // default: Always On
));
```

### Transport State Forwarding

```cpp
// In PluginProcessor::processBlock(), before calling roomToneSection.process():
bool transportPlaying = true;  // Safe default
if (auto* playHead = getPlayHead())
{
    if (auto posInfo = playHead->getPosition())
    {
        if (auto playing = posInfo->getIsPlaying())
            transportPlaying = *playing;
    }
}
roomToneSection.setTransportPlaying(transportPlaying);
```

### Latency Reporting

```cpp
// In PluginProcessor::prepareToPlay():
// After all sections are prepared...
int totalLatency = excitationSection.getLatencySamples();
// Only Excitation adds latency (oversampling). All other stages are zero-latency.
setLatencySamples(totalLatency);
```

## Discretion Recommendations

### Multiband Saturation Curve Shapes: Symmetric Tanh with Band-Dependent Scaling
**Recommendation:** Use `tanh(x*G)/tanh(G)` for all three bands (symmetric, no DC bias). This is physically correct for room excitation: air compression is symmetric. Differentiate bands by their G ranges:
- **Low band (< 200Hz):** G range 0.15 to 1.5. Gentlest saturation. Bass needs the least excitation and is most sensitive to distortion artifacts. Low-frequency saturation generates intermodulation products that sound muddy.
- **Mid band (200Hz-3kHz):** G range 0.15 to 2.5. Strongest saturation. This is the primary "liveliness and density" region. Most room excitation is perceptible in the mid-range.
- **High band (> 3kHz):** G range 0.15 to 1.8. Moderate saturation. Too much high-frequency saturation creates harshness. Enough to add "sparkle" and presence without brittleness.

### Band Crossover Frequencies: 200Hz and 3kHz
**Recommendation:** Fixed crossover points at 200Hz (low/mid boundary) and 3kHz (mid/high boundary). These are standard multiband dynamics crossover points that align with acoustic physics:
- 200Hz separates bass fundamentals from the lower midrange where room modes are most active
- 3kHz separates the primary consonance/clarity region from the presence/brilliance range
- These do not need to be modulated. Room excitation character is about per-band intensity, not frequency placement.

### Drive Response Curve: Sweet Spot via Power Curve
**Recommendation:** Map Drive 0-100% through `pow(drive, 0.7)` before scaling to G values. This creates a sweet spot in the 30-60% range where most of the "alive" character lives. Above 60%, the curve steepens into more obvious nonlinearity. This prevents the first half of the knob from sounding identical (problem with linear mapping) while keeping the bottom 30% clearly audible (problem with exponential mapping). The 0.7 exponent is a starting point -- tune by ear.

### Room Tone Modulation: Slowly Evolving (LFO)
**Recommendation:** Add very slow amplitude modulation to the room tone: a sine LFO at ~0.15Hz (approximately 7-second cycle) with +/-0.8dB depth. This prevents the noise from sounding static and synthetic. Real room tone has subtle level fluctuations from air currents, HVAC systems, and environmental changes. The modulation should be imperceptible as a rhythm -- it should just make the noise feel "alive." Additionally, very slight spectral modulation (LFO on the presence peak frequency, +/-10Hz at the same rate) adds further realism.

### Excitation Coupling to Downstream Stages: None (Signal Flow Is Sufficient)
**Recommendation:** Do NOT add explicit parameter coupling from Drive to Tail or Reflections energy. The serial signal chain already provides implicit coupling: the Excitation stage's output (with its added harmonics and density) feeds directly into Room Tone and then Diffuse Tail. When Drive is higher, the signal entering the Tail has more harmonic content, which naturally produces a richer, denser reverb tail. This is the physical reality -- in a real room, the "excited" sound propagates through the room and its excitation character carries into the reflections and tail. Explicit parameter coupling would double-compound the effect and create confusing interdependencies for the user.

### Oversampling Factor: 4x at 44.1/48kHz, Adaptive
**Recommendation:** Use the adaptive scheme described in Pattern 7. 4x at 44.1/48kHz provides excellent alias rejection for the mild saturation levels used. 2x at 88.2/96kHz maintains the same effective processing rate (~176-192kHz). No oversampling at 176.4/192kHz because the Nyquist is already high enough that aliasing from soft saturation is inaudible. Crucible uses fixed 4x, but Crucible has more aggressive saturation. Aether's mild saturation can tolerate 2x at higher rates.

### Noise Generation Method: Kellett IIR Pink + Biquad Shaping
**Recommendation:** Use the Kellett IIR method (described in Pattern 5) for pink noise, followed by the 3-stage biquad shaping chain. This is the most CPU-efficient approach and produces noise that, after shaping, is perceptually indistinguishable from a real room tone recording. The Voss-McCartney algorithm would also work but requires more state and branching logic with no audible benefit since the output is further filtered anyway.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| ExcitationSection stub (passthrough) | Full multiband saturation with oversampling | Phase 5 | Completes Stage IV of the processing chain |
| RoomToneSection stub (passthrough) | Full noise generation with spectral shaping and gating | Phase 5 | Completes Stage V of the processing chain |
| Auto-gain formula tuned for passthrough stages | Auto-gain empirically calibrated for full 6-stage chain | Phase 5 | Corrects volume compensation now that all stages add energy |
| Zero latency reported | Oversampling latency reported via setLatencySamples | Phase 5 | Enables correct DAW plugin delay compensation |
| 2 parameters for Room Tone (tone_amb, tone_bypass) | 3 parameters (+ tone_gate) | Phase 5 | Adds user-selectable gating mode for practical mixing |

**Deprecated/outdated:**
- Passthrough stubs for ExcitationSection and RoomToneSection
- `setLatencySamples(0)` in prepareToPlay (will become oversampling latency value)

## Open Questions

1. **Per-band saturation G value tuning**
   - What we know: G ranges are physics-motivated (low gentlest, mid strongest, high moderate). The tanh waveshaper is well-understood.
   - What's unclear: Whether the specific G_max values (1.5, 2.5, 1.8) produce the intended perceptual result of "alive not distorted" at 50% and "subtle edge" at 100%.
   - Recommendation: Implement with documented values, then tune by ear during verification. Test with DI guitar, acoustic guitar, snare, and full mix. Adjust G ranges until the user experience matches the spec.

2. **Material coupling audibility**
   - What we know: Material from Stage I should influence excitation character (wood warmer, metal brighter, stone darker). The per-band scale modifiers are physically motivated.
   - What's unclear: Whether the material coupling produces an audible and meaningful difference, or whether it is too subtle to perceive over the frequency-dependent saturation itself.
   - Recommendation: Start with the documented bias values. If material switching does not produce an audible excitation change during verification, increase the bias range (widen the scale differences). If it produces too dramatic a change, narrow the range.

3. **Room tone level calibration**
   - What we know: At 100% Ambience, noise should be -40dB to -30dB below signal. The spec gives a range, not a fixed value.
   - What's unclear: What reference signal level to use for calibration. A -18dBFS reference is standard for mixing, but guitar DI signals vary widely.
   - Recommendation: Calibrate against a -18dBFS peak reference signal. At Ambience=100%, the noise floor should measure approximately -53dBFS (i.e., -35dB below reference). This puts it at the midpoint of the -40 to -30dB spec range. Verify it is audible on headphones but not obtrusive in a mix.

4. **Auto-gain compensation retuning specifics**
   - What we know: The current formula is `-2.5dB * pow(mix, 1.5)`. With all stages now adding energy, this may undercompensate or overcompensate.
   - What's unclear: How much energy the full chain adds at typical settings. This depends on all parameter values and signal content.
   - Recommendation: Measure empirically after implementation. Set all stages to moderate defaults, play a reference signal, measure the level difference between Mix=0% and Mix=100%. Adjust the -2.5dB coefficient (likely needs to increase to -3.0 to -4.0dB) and possibly the 1.5 exponent. Document the final values.

5. **3-band crossover with single oversampler vs per-band oversamplers**
   - What we know: The crossover splits into 3 bands. Each band needs waveshaping. Waveshaping needs oversampling.
   - What's unclear: Whether to use one oversampler for the combined signal (crossover inside oversampled loop) or three separate oversamplers (one per band). Single oversampler is more CPU-efficient but means the crossover operates at the oversampled rate (which is fine since LR filters are inexpensive). Three oversamplers triple the anti-aliasing filter cost.
   - Recommendation: Use a single oversampler. Upsample the full signal, split into bands at the oversampled rate, waveshape each band, recombine, then downsample. This is more CPU-efficient and avoids potential phase issues from having three independent oversampling paths.

## Sources

### Primary (HIGH confidence)
- Crucible SaturationSection at `~/Projects/Crucible/Source/dsp/SaturationSection.h/.cpp` -- Production-proven oversampling + waveshaping architecture in the same brand. `tanh(x*G)/tanh(G)` waveshaper, `juce::dsp::Oversampling` usage, auto-gain compensation pattern, bypass crossfade pattern. Directly examined source code.
- Crucible SaturationCore at `~/Projects/Crucible/Source/dsp/saturation/SaturationCore.h/.cpp` -- Oversampling implementation: 4x FIR equiripple, integer latency, stereo processing, DC blocker pattern. Directly examined source code.
- Crucible SubCrossover at `~/Projects/Crucible/Source/dsp/saturation/SubCrossover.h` -- Linkwitz-Riley crossover pattern: split/combine API, phase-coherent reconstruction. Directly examined source code.
- Aether codebase Phases 1-4 at `Source/` -- All existing DSP section patterns (prepare/process/reset/setBypass, SmoothedValue, bypass crossfade, parameter caching, cross-stage coupling via updateStageParams). Directly examined source code.
- [JUCE dsp::Oversampling API](https://docs.juce.com/master/classjuce_1_1dsp_1_1Oversampling.html) -- Constructor parameters, processSamplesUp/Down, getLatencyInSamples, filter types, integer latency mode. Verified via official documentation.
- [JUCE dsp::LinkwitzRileyFilter API](https://docs.juce.com/master/classdsp_1_1LinkwitzRileyFilter.html) -- LR4 24dB/oct crossover, processSample, setCutoffFrequency. Verified via official documentation.
- [JUCE AudioPlayHead::PositionInfo API](https://docs.juce.com/master/classAudioPlayHead_1_1PositionInfo.html) -- getIsPlaying() returns Optional<bool>, access only from processBlock(). Verified via official documentation.
- Signalsmith DSP v1.7.1 source at `build/_deps/signalsmith-dsp-src/filters.h` -- BiquadStatic API: highpassQ, lowpassQ, peakDb, bandpassQ. Directly examined source code.

### Secondary (MEDIUM confidence)
- [Nonlinear Acoustics - Wikipedia](https://en.wikipedia.org/wiki/Nonlinear_acoustics) -- Physical thresholds for nonlinear air behavior (~120-154 dB SPL). Confirms that real air nonlinearity is irrelevant at room simulation levels; Aether's excitation is perceptual, not physical. Multiple academic sources cited.
- [Paul Kellett Pink Noise Filter](https://www.musicdsp.org/en/latest/Filters/76-pink-noise-filter.html) -- Kellett "refined" method coefficients for IIR pink noise generation. Accurate to +/-0.05dB above 9.2Hz at 44.1kHz. Widely used in audio DSP community. Coefficients verified across multiple sources.
- [DSP Generation of Pink Noise](https://www.firstpr.com.au/dsp/pink-noise/) -- Comprehensive comparison of pink noise algorithms: Kellett IIR, Voss-McCartney, staggered update. Confirms Kellett IIR as efficient choice for real-time audio.
- [Valhalla DSP: Diffusion and Metallic Artifacts](https://valhalladsp.com/2011/01/21/reverbs-diffusion-allpass-delays-and-metallic-artifacts/) -- Allpass cascade metallic artifact mitigation. Relevant to understanding why the Excitation stage should NOT use allpass chains for saturation (unlike Air stage).
- [JUCE Oversampling implementation guide](https://daudio.dev/explore/HowToImplementOversamplingInJuce) -- Practical oversampling tutorial with JUCE 8 code examples. Confirms constructor pattern, up/down processing flow, latency reporting.
- [JUCE Forum: Dealing with Oversampler Latency](https://forum.juce.com/t/dealing-with-oversampler-latency/36329) -- Community guidance on latency reporting with juce::dsp::Oversampling. Confirms integer latency mode for DAW PDC.
- [CCRMA Soft Clipping](https://ccrma.stanford.edu/~jos/pasp/Soft_Clipping.html) -- Julius O. Smith's reference on soft clipping / waveshaping. Confirms tanh as standard soft clipper with well-understood harmonic generation behavior.

### Tertiary (LOW confidence)
- Per-band G value ranges (kMaxLowG=1.5, kMaxMidG=2.5, kMaxHighG=1.8) -- Educated starting points based on Crucible's saturation range adapted for milder room excitation levels. Specific values require ear-tuning.
- Material excitation bias values -- Physics-motivated (wood absorbs, metal reflects, stone diffuses) but specific numerical biases are artistic choices, not measured data.
- Room tone spectral shaping parameters (HPF 80Hz, peak 300Hz, LPF 8kHz) -- Based on typical room tone spectral measurements but not calibrated against a specific room recording. The exact filter parameters are starting points for ear-tuning.
- Drive curve exponent (0.7) -- Chosen for sweet-spot behavior in the 30-60% range. May need adjustment based on perceived knob feel during verification.
- LFO modulation parameters (0.15Hz, +/-0.8dB) -- Artistic choice for natural quality. No measured reference for "how much real room tone fluctuates."

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH -- JUCE 8.0.12 and Signalsmith DSP 1.7.1 verified from Phases 1-4. All needed APIs (Oversampling, LinkwitzRileyFilter, AudioPlayHead, Random, BiquadStatic) confirmed from official docs and source inspection. No new libraries needed.
- Architecture (Excitation): HIGH -- Multiband saturation with oversampling is a well-established pattern. Crucible's production-proven SaturationSection provides a direct reference for the oversampling/waveshaping flow. The 3-band approach follows standard multiband dynamics architecture. The only novel aspect is the room-excitation (not gear-emulation) framing, which is a tuning difference, not an architectural difference.
- Architecture (Room Tone): HIGH -- Filtered noise generation is straightforward. Kellett IIR is well-documented and widely used. Spectral shaping with biquad filters is the established Aether pattern. Gating modes use standard JUCE APIs (AudioPlayHead, SmoothedValue).
- Tuning parameters: MEDIUM -- Per-band G ranges, material biases, room tone spectral shaping, auto-gain coefficients, and drive curve exponent are all educated starting points that need ear-tuning during implementation.
- Cross-stage coupling: HIGH -- Material coupling to Excitation follows the same processor-mediated forwarding pattern as all existing cross-stage links. Room Size coupling to both stages is straightforward.

**Research date:** 2026-02-18
**Valid until:** 2026-03-18 (stable domain -- no fast-moving dependencies)
