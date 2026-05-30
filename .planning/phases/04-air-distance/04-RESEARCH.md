# Phase 4: Air & Distance - Research

**Researched:** 2026-02-18
**Domain:** Frequency-dependent air absorption filtering, allpass phase smearing for transient softening, three-character environmental simulation (Warm/Neutral/Cold), cross-stage coupling to Early Reflections and Diffuse Tail
**Confidence:** HIGH

## Summary

Phase 4 implements Stage III of Aether's processing chain: Air & Distance. This stage simulates the high-frequency absorption and phase dispersion that occurs as sound travels through air in a room. Unlike a simple tone control, the Air stage combines physics-driven HF rolloff with allpass-based phase smearing to produce a spatial impression of distance -- the signal sounds like it traveled through a room rather than being EQ'd. Three character variants (Warm, Neutral, Cold) model different environmental conditions: hot/humid air with thick absorption, moderate balanced conditions, and cold/dry air with crisp bright quality.

The core DSP architecture uses a cascaded high-shelf filter for HF absorption (not a lowpass -- shelf preserves HF content above the transition, just attenuated, avoiding the "blanket" effect), a series of 4-6 cascaded biquad allpass filters for phase smearing (spreading transient energy across time without changing frequency content), and a subtle low-shelf for character-dependent low-frequency behavior. The shelf-based approach is physically motivated: real air absorption follows an approximately 6 dB/octave rolloff above a frequency that depends on temperature and humidity. A high shelf models this more accurately than a lowpass, which would asymptotically remove all HF content. At 100% Air the signal sounds like "next room over" -- noticeably softer transients and reduced brightness, but still intelligible with life remaining.

Cross-stage coupling is the architecturally significant aspect of this phase. Air Amount already flows to DiffuseTailSection.setHFDamping() (wired in Phase 3, Plan 3). Phase 4 extends this by: (1) adding a new setter on ReflectionsSection for air-induced darkening of individual reflection taps, (2) adding character influence on both coupling paths so Warm extends perceived decay and Cold tightens it, and (3) implementing a subtle baseline absorption at 0% Air so the stage is never fully transparent. The existing processor-mediated forwarding pattern (Anti-Pattern 5 avoidance) applies: all cross-stage data flows through updateStageParams() in PluginProcessor.cpp.

**Primary recommendation:** Implement Air stage as a stereo biquad filter chain per channel: one high-shelf for primary HF absorption (physics-driven curve per character), one optional low-shelf for character warmth/thinning, and 4-6 cascaded biquad allpass filters for phase smearing. Use a 3-position AudioParameterChoice for character selection (consistent with existing Shape/Material choice parameters). Extend ReflectionsSection with a setAirDarkening() method for cross-stage coupling. Update the air_char parameter from 2 to 3 choices with a version bump.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

#### Filter Character
- Three character variants instead of two: **Warm, Neutral, Cold**
  - Warm = hot, humid day -- thick air, sound hangs longer, soft materials, plush carpet
  - Neutral = moderate room conditions, balanced absorption
  - Cold = cold, dry day -- bright, crisp sound with bite, hard reflective surfaces
- Each character should be noticeably different -- enough to clearly hear when switching between them, but not a monumental difference
- Physics-driven rolloff curves -- model real air absorption coefficients rather than analog-feel musical shaping. Accuracy drives the sound.
- Minimal baseline absorption at 0% Air -- even at 0%, a very subtle HF shelf. No room has zero air absorption. Stage is never completely transparent (bypass exists for that).
- At max Air (100%), signal should sound like "next room over" -- noticeable HF loss but still intelligible, not dramatically dark
- Must NOT sound like a lowpass filter (tone knob) or a blanket (muffled/smothered). Always spatial, not tonal. Even at max, there should be life in the signal.
- Air knob transitions smoothly ramped (~20-30ms) to avoid clicks or zipper noise during automation

#### Transient Softening
- At max Air, signal is allowed to get somewhat "washy" or smeared -- that's what real distance does to transients
- Allpass phase smearing should simulate air diffusion, not just be an EQ effect

#### Air-to-Stage Coupling
- Air couples to BOTH Early Reflections AND Diffuse Tail -- higher Air darkens individual reflection taps AND increases tail HF damping. More physically accurate: air affects all sound paths.
- Warm character subtly extends perceived decay (humid air holds sound longer). Cold tightens perceived decay. Character influences the coupling, not just the direct signal.

### Claude's Discretion

#### Filter
- Cold character approach: whether it's a different rolloff flavor (steeper, higher-frequency cut) or includes subtle HF presence boost. Pick what's physically plausible.
- Character selector UI control: 3-position toggle vs continuous blend knob. Pick based on UI consistency and DSP support.
- Whether characters also affect low-frequency behavior (Warm adding low warmth, Cold thinning lows). Pick what sounds most natural for room simulation.

#### Transients
- How noticeable the transient softening should be (clearly audible vs subtle byproduct). Pick the balance that serves room simulation.
- Whether diffusion scales linearly with Air or has a threshold. Pick what produces the most natural result.
- Whether the three characters influence transient diffusion amount (Warm = more diffusion, Cold = less). Pick based on physical plausibility.

#### Coupling
- How strongly Air influences tail/reflections darkness. Pick what sounds cohesive with the direct signal filtering.
- Whether character affects coupling strength or just direct signal. Pick what creates the most cohesive room impression.
- Whether minimal baseline at 0% Air also subtly affects tail/reflections. Pick based on coherence with the baseline decision.

### Deferred Ideas (OUT OF SCOPE)

None -- discussion stayed within phase scope
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| AIR-01 | User can control high-frequency absorption via Air knob (0-100%), from no filtering to significant HF rolloff | High-shelf filter with Air-scaled gain. At 0%: subtle baseline shelf (-1 to -2dB at 12kHz). At 100%: significant shelf (-8 to -12dB above transition frequency). Transition frequency varies by character (Warm ~6kHz, Neutral ~8kHz, Cold ~10kHz). Uses `BiquadStatic::highShelfDb()` from Signalsmith DSP. SmoothedValue on shelf gain with 20-30ms ramp. |
| AIR-02 | User can select air character: Warm (gentle slope from ~6kHz, carpeted room feel) or Neutral (steeper slope from ~10kHz, hard-surfaced room feel) | Extended to three characters (Warm/Neutral/Cold) per user decision. Each character defines a complete filter parameter set: HF shelf frequency, HF shelf gain range, optional LF shelf, allpass center frequencies. 3-position AudioParameterChoice. Character switching uses 30ms crossfade (established pattern from ReflectionsSection shape switching). |
| AIR-03 | Air stage includes subtle allpass filtering for phase smearing (air diffusion simulation) | 4-6 cascaded biquad allpass filters per channel with center frequencies spread across the HF range (2-12kHz). Q values set to produce broadband phase dispersion. Allpass count and Q scale with Air amount: at low Air values, fewer effective allpass stages (lower Q); at high Air values, stronger phase dispersion. Uses `BiquadStatic::allpassQ()` from Signalsmith DSP. |
| AIR-04 | Higher Air values produce subtle transient softening | Achieved via the allpass cascade. Phase smearing spreads transient energy across time without changing frequency content. At 100% Air, transient peaks are reduced by approximately 2-4dB as broadband energy is temporally dispersed. The effect is "the sound becomes less immediate" rather than "the attack is filtered off." Character-dependent: Warm produces more diffusion (higher allpass Q), Cold produces less (lower Q). |
| AIR-05 | User can bypass Air & Distance stage independently | Implemented via `air_bypass` parameter with 10ms crossfade (established pattern from ResonanceSection and ReflectionsSection). When bypassed, all filtering and allpass processing is skipped. Bypass crossfade uses SmoothedValue blend between dry and processed signal. |
</phase_requirements>

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| JUCE | 8.0.12 | Audio plugin framework, APVTS, SmoothedValue, AudioParameterChoice | Already established. All parameter infrastructure, smoothing, and choice parameters verified from Phases 1-3. |
| Signalsmith DSP | 1.7.1 | BiquadStatic filters (high shelf, low shelf, allpass), delay lines (for allpass diffusion if needed) | Header-only, already linked. Provides all filter primitives needed. `highShelfDb()`, `lowShelfDb()`, `allpassQ()` APIs verified from source inspection. |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `signalsmith::filters::BiquadStatic<float>` | 1.7.1 | HF absorption shelf, LF character shelf, allpass diffusion stages | All filter needs. `highShelfDb(scaledFreq, db, octaves)` for HF absorption. `lowShelfDb(scaledFreq, db, octaves)` for LF character. `allpassQ(scaledFreq, q)` for phase smearing. All accept `scaledFreq = freq / sampleRate`. |
| `juce::SmoothedValue<float>` | (JUCE 8) | Air amount ramping, shelf gain smoothing, allpass parameter smoothing | All continuous parameter changes on audio thread. 20-30ms ramp per user decision. |
| `juce::AudioParameterChoice` | (JUCE 8) | Character selector (Warm/Neutral/Cold) | 3-position choice parameter, consistent with existing refl_shape and res_material pattern. |
| `signalsmith::mix::cheapEnergyCrossfade` | 1.7.1 | Character switch crossfade | Smooth transition when switching between Warm/Neutral/Cold characters. 30ms crossfade (same pattern as shape preset switching in ReflectionsSection). |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| High shelf for HF absorption | Lowpass filter | Lowpass asymptotically removes all HF content, creating the "blanket" effect the user explicitly wants to avoid. High shelf reduces HF by a fixed amount, preserving content above the transition. At -12dB the signal is quieter but still present -- spatial, not tonal. |
| Biquad allpass for phase smearing | Delay+feedback allpass (as in DiffuseTailSection) | Biquad allpass is simpler (single sample processing, no delay buffer) and provides broadband phase dispersion centered at a frequency. Delay+feedback allpass gives longer echo-like smearing (1-8ms) which would sound more like reverb than air diffusion. The biquad approach is more appropriate for subtle, continuous phase dispersion. |
| 3-position choice for character | Continuous blend knob | A continuous blend between Warm and Cold would give infinite positions but adds complexity (morphing filter curves) and is inconsistent with the existing UI pattern of discrete choices (Material, Shape). Three fixed characters are easier to tune, easier to name, and produce more predictable results. The user described three distinct environments, not a continuum. |
| Per-channel allpass cascade | Shared allpass cascade (mono then split) | Per-channel allpass with slightly different parameters (e.g., different center frequencies or Q values for L and R) provides subtle stereo decorrelation, adding to the spatial impression. Shared mono processing would narrow the stereo field. |

## Architecture Patterns

### Recommended Changes to Project Structure

```
Source/
  Parameters.h           -> Update air_char to 3 choices (Warm/Neutral/Cold), version bump
  PluginProcessor.h      -> No changes (air param pointers already cached)
  PluginProcessor.cpp    -> Extend updateStageParams() for Air forwarding + cross-stage coupling
  dsp/
    AirSection.h         -> Complete implementation: shelf filters, allpass cascade, character presets
    AirSection.cpp       -> Full DSP processing
    ReflectionsSection.h -> Add setAirDarkening(float) method
    ReflectionsSection.cpp -> Integrate air darkening into tap filter cutoff calculation
    DiffuseTailSection.h -> Add setCharacterDecayBias(float) method (optional, for character->decay coupling)
    DiffuseTailSection.cpp -> Integrate character decay bias
```

### Pattern 1: Character Preset Structure

**What:** Each character (Warm/Neutral/Cold) is defined by a compile-time struct containing the complete filter parameter set for that environmental condition.
**When to use:** Every character definition.

```cpp
struct AirCharacterPreset
{
    const char* name;

    // HF absorption shelf
    float shelfFreqHz;         // Transition frequency for HF rolloff
    float shelfOctaves;        // Shelf bandwidth/slope
    float minShelfDb;          // Shelf gain at Air = 0% (baseline)
    float maxShelfDb;          // Shelf gain at Air = 100%

    // LF character shelf (optional warmth/thinning)
    float lfShelfFreqHz;       // Low shelf frequency (0 = disabled)
    float lfMinShelfDb;        // LF shelf at Air = 0%
    float lfMaxShelfDb;        // LF shelf at Air = 100%

    // Allpass diffusion
    float allpassBaseFreqHz;   // Lowest allpass center frequency
    float allpassSpreadOctaves;// Range spread for allpass center frequencies
    float allpassMinQ;         // Q at Air = 0% (minimal phase shift)
    float allpassMaxQ;         // Q at Air = 100% (maximum smearing)
    int   allpassStages;       // Number of allpass stages (4-6)

    // Cross-stage coupling
    float tailDecayBias;       // -1.0 to 1.0: negative tightens, positive extends
    float reflDarkeningScale;  // How strongly this character darkens reflections (0.5-1.5x)
};
```

### Pattern 2: Physics-Driven HF Absorption via High Shelf

**What:** Air absorption modeled as a high shelf rather than lowpass, based on ISO 9613-1 air absorption coefficients scaled to indoor room distances.
**When to use:** The primary frequency-domain effect of the Air stage.

```
Real air absorption at 20C, 50% humidity (ISO 9613-1 derived):
  1 kHz:  0.0037 dB/m  -> At 10m: -0.037 dB  (negligible)
  2 kHz:  0.0097 dB/m  -> At 10m: -0.097 dB  (negligible)
  4 kHz:  0.0328 dB/m  -> At 10m: -0.328 dB  (barely perceptible)
  8 kHz:  0.117  dB/m  -> At 10m: -1.17  dB  (perceptible)
  16 kHz: ~0.4   dB/m  -> At 10m: -4.0   dB  (clearly audible)

For room distances (2-30m), real absorption is subtle:
  The difference between 2m and 30m is primarily above 4kHz.
  Below 2kHz, absorption is negligible at room scale.

For Aether's Air knob (0-100%):
  0% models ~1-2m (close mic, minimal absorption): -1dB shelf above 10kHz
  100% models ~20-30m (next room over): -8 to -12dB shelf above 4-6kHz
  The shelf slope (6dB/octave approximately) matches the physical behavior.
```

### Pattern 3: Allpass Cascade for Phase Smearing

**What:** 4-6 cascaded biquad allpass filters with center frequencies spread across the HF range to produce broadband phase dispersion that softens transients without changing frequency content.
**When to use:** The transient-softening component of the Air stage.

```
Signal Flow:
  Input -> [HF Shelf] -> [LF Shelf] -> [Allpass 1] -> [Allpass 2] -> ... -> [Allpass N] -> Output

Each allpass: BiquadStatic::allpassQ(centerFreq / sampleRate, Q)
  - Center frequencies: logarithmically spaced from ~2kHz to ~12kHz
  - Q controls the steepness of phase rotation around the center frequency
  - Higher Q = more concentrated phase shift = more noticeable smearing
  - Lower Q = gentle phase shift = subtle dispersion

Phase dispersion mechanism:
  - Each allpass rotates phase by 360 degrees at its center frequency
  - Cascading multiple allpass filters at different frequencies
    creates complex phase relationships across the spectrum
  - Transients (broadband impulses) are spread in time because
    their frequency components arrive at different times
  - The effect is perceptually "less immediate" rather than "filtered"
```

### Pattern 4: Cross-Stage Air Coupling via Processor

**What:** The processor reads Air Amount and Character from APVTS and forwards derived values to ReflectionsSection and DiffuseTailSection. Extends the existing cross-stage pattern from Phase 3.
**When to use:** All Air-to-other-stage parameter relationships.

```cpp
// In AetherProcessor::updateStageParams()

float airAmount = airAmountParam->load();
int   airChar   = static_cast<int>(airCharParam->load());

// Direct: Air -> AirSection
airSection.setAmount(airAmount);
airSection.setCharacter(airChar);
airSection.setBypass(airBypassParam->load() >= 0.5f);

// Cross-stage: Air -> Tail HF damping (already wired in Phase 3)
// Now enhanced with character influence
float decayBias = kCharacterPresets[airChar].tailDecayBias;
diffuseTailSection.setHFDamping(airAmount);
diffuseTailSection.setCharacterDecayBias(decayBias * airAmount);

// Cross-stage: Air -> Reflections darkening (NEW in Phase 4)
float reflDarkeningScale = kCharacterPresets[airChar].reflDarkeningScale;
reflectionsSection.setAirDarkening(airAmount * reflDarkeningScale);
```

### Pattern 5: Character Switch with Crossfade

**What:** When the user switches between Warm/Neutral/Cold, all filter parameters crossfade over 30ms to avoid clicks. Same pattern as ReflectionsSection shape switching.
**When to use:** Every character change.

```cpp
// Track current and pending character
int currentCharIndex = 0;
int pendingCharIndex = -1;
float charCrossfade = 1.0f;
float charCrossfadeStep;  // = 1.0 / (0.030 * sampleRate)

// In process(), per sample:
if (pendingCharIndex >= 0) {
    charCrossfade -= charCrossfadeStep;
    if (charCrossfade <= 0.0f) {
        currentCharIndex = pendingCharIndex;
        pendingCharIndex = -1;
        charCrossfade = 1.0f;
    }
}

// Apply filters for current character at charCrossfade blend
// Apply filters for pending character at (1-charCrossfade) blend
// Sum outputs weighted by blend factors
```

### Anti-Patterns to Avoid

- **Using a lowpass filter for HF absorption:** The user explicitly stated this must NOT sound like a lowpass / tone knob / blanket. A lowpass asymptotically removes HF content entirely. A high shelf reduces it by a fixed amount -- the signal retains HF presence, just quieter. This is the single most important design distinction.

- **Recalculating filter coefficients every sample:** Filter coefficient computation involves transcendental functions (sin, cos, sqrt). Calculate coefficients once per block or when parameters change (SmoothedValue target changes). During per-sample smoothing, interpolate the gain parameter and recalculate coefficients only when the smoothed value actually changes (check `isSmoothing()`).

- **Identical L/R allpass parameters:** Using identical allpass center frequencies and Q for both channels produces a mono phase rotation that narrows the stereo image. Offset L and R center frequencies by 10-15% for subtle stereo decorrelation.

- **Too many allpass stages:** More than 6-8 biquad allpass stages start to produce audible metallic coloration on percussive content (same issue as reverb diffusion). 4-6 stages provides noticeable transient softening without metallic artifacts.

- **Character switch without crossfade:** Switching filter coefficients instantaneously causes a click because the biquad state (x1, x2, y1, y2) is inconsistent with the new coefficients. Must crossfade between old and new processed signals over 30ms.

- **Direct section-to-section coupling:** All Air-to-Reflections and Air-to-Tail data must flow through updateStageParams() in the processor. Never have AirSection hold a reference to DiffuseTailSection.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| HF absorption filter | Custom coefficient math | `BiquadStatic::highShelfDb(scaledFreq, db, octaves)` | Correct shelf design with configurable slope. Handles Nyquist correctly. Already proven in the codebase. |
| LF character shelf | Custom low shelf | `BiquadStatic::lowShelfDb(scaledFreq, db, octaves)` | Same library, same patterns, consistent API. |
| Phase smearing allpass | Custom allpass topology | `BiquadStatic::allpassQ(scaledFreq, q)` | Biquad allpass is numerically stable for the short-delay phase dispersion needed. Single-sample processing, no additional buffers. |
| Parameter smoothing | Custom ramp | `juce::SmoothedValue<float>` | Handles sample rate, provides isSmoothing() optimization, settable ramp time. Established pattern. |
| Crossfade on character switch | Custom fade logic | `signalsmith::mix::cheapEnergyCrossfade()` | Max 1.06% energy error, efficient polynomial. Same pattern as shape preset crossfade in ReflectionsSection. |
| Bypass crossfade | Custom blend | `SmoothedValue<float>` for blend factor | Same 10ms bypass crossfade pattern from ResonanceSection and ReflectionsSection. |

**Key insight:** The creative work in this phase is tuning the three character presets to produce physically plausible, perceptually distinct environmental conditions. All DSP infrastructure (shelves, allpass, smoothing, crossfade) comes from libraries already used in Phases 1-3.

## Common Pitfalls

### Pitfall 1: "Blanket" / "Tone Knob" Sound
**What goes wrong:** The Air stage sounds like someone is rolling off treble with a tone control -- muffled and flat rather than spatial and distant.
**Why it happens:** Using a lowpass filter instead of a high shelf, or setting the shelf gain too extreme (below -15dB). A lowpass progressively removes all content above the cutoff. A shelf reduces it by a fixed amount.
**How to avoid:** Use high shelf exclusively for HF absorption. Keep maximum shelf gain at -8 to -12dB (never deeper). The allpass phase smearing adds the "distance" quality that distinguishes Air from EQ. Always A/B test: "Does this sound like the mic moved further away, or like someone turned down the treble knob?" If the latter, reduce shelf depth and increase allpass contribution.
**Warning signs:** No difference between Air at 100% and a simple EQ cut. Loss of articulation/intelligibility at moderate Air settings. Mono compatibility issues (shelf processing should not affect stereo image).

### Pitfall 2: Metallic Ringing from Allpass Cascade
**What goes wrong:** The allpass cascade produces audible metallic or "phaser-like" coloration, especially on percussive content.
**Why it happens:** Too many allpass stages (>8), allpass center frequencies too close together, or Q values too high (>2.0). Multiple allpass filters with similar frequencies create resonant peaks in the group delay that color the sound.
**How to avoid:** Use 4-6 stages maximum. Space center frequencies logarithmically (at least 1 octave apart). Keep Q below 1.5 even at maximum Air. The allpass effect should be felt (transient softening) rather than heard (no tonal coloration). Test with click tracks and snare drums -- these reveal metallic artifacts first.
**Warning signs:** Audible pitch shifting on sustained tones. Ringing "tail" after transients. "Phaser" or "flanging" quality when sweeping Air amount.

### Pitfall 3: Click on Character Switch
**What goes wrong:** Audible pop or click when switching between Warm/Neutral/Cold.
**Why it happens:** Filter coefficients change instantaneously while biquad internal state (x1, x2, y1, y2) retains values from the old coefficient set. The state is inconsistent with the new transfer function, producing a discontinuity.
**How to avoid:** Never switch coefficients in-place. Run both old and new filter chains simultaneously during a 30ms crossfade window. The old chain continues processing with old coefficients, the new chain starts from zeroed state with new coefficients, and the outputs are blended with cheapEnergyCrossfade. After crossfade completes, drop the old chain. This is the same pattern used for shape preset switching in ReflectionsSection.
**Warning signs:** Click on character switch. Louder click with higher Air amount (more filter state to discontinue). Different click amplitude at different Air settings.

### Pitfall 4: Zipper Noise When Automating Air Amount
**What goes wrong:** Audible stepping or buzzing when the Air knob is automated rapidly.
**Why it happens:** Filter coefficients change too abruptly. If shelf gain is updated every sample via coefficient recalculation, the transcendental functions in the coefficient formula can produce small discontinuities.
**How to avoid:** Smooth the Air Amount via SmoothedValue with 20-30ms ramp (user requirement). Only recalculate filter coefficients when the smoothed value changes (once per few samples during ramping, once per block when stable). The isSmoothing() check avoids unnecessary recalculation. During the ramp, the coefficient change per step is small enough to be inaudible.
**Warning signs:** Buzzing during fast automation sweeps. Different artifacts at different buffer sizes. CPU spike during automation (excessive coefficient recalculation).

### Pitfall 5: Cross-Stage Coupling Creating Muddy Sound
**What goes wrong:** When Air darkens reflections AND the direct signal AND the tail simultaneously, the combined effect is too dark -- everything loses clarity.
**Why it happens:** Three independent darkening effects (direct shelf, reflection tap filter darkening, tail HF damping) compound multiplicatively in the signal chain.
**How to avoid:** Scale the cross-stage coupling to be more subtle than the direct effect. A good ratio: direct shelf at full strength, reflection darkening at ~50-60% strength, tail HF damping at the existing curve (already tuned in Phase 3). The user should perceive a cohesive environmental shift, not triple-stacked darkness. Character-dependent scaling helps: Cold character applies less coupling (crisp, bright environment should not darken reflections as aggressively).
**Warning signs:** The combined effect at 50% Air sounds darker than expected. Switching Air from 0% to 100% produces a dramatic volume or brightness change. A/B with only the direct shelf reveals the coupling is contributing too much additional darkness.

### Pitfall 6: Baseline Absorption Breaking A/B Comparison
**What goes wrong:** The subtle baseline HF shelf at Air 0% makes it impossible to cleanly A/B the Air stage effect because even at 0% the stage is doing something.
**Why it happens:** The user decision requires non-zero baseline absorption. But if the baseline is too strong, users will perceive the stage as "always filtering" rather than "adding subtle air."
**How to avoid:** Keep baseline shelf very subtle: -1 to -2dB maximum, at a high transition frequency (10-12kHz). At these levels, the baseline is perceptually more like "removing digital harshness" than "filtering." Bypass (In/Out) gives the true flat comparison. Document this behavior clearly for the user -- the Air stage at 0% is "the room exists" and Bypass is "no room."
**Warning signs:** Users reporting that Air at 0% sounds different from Bypass. The difference should be subtle enough that it requires careful listening to detect. If it is immediately obvious, the baseline is too strong.

## Code Examples

### Character Presets (3 Characters)

```cpp
// Source: Physics-derived from ISO 9613-1 air absorption data,
// adapted for indoor room distances (2-30m) and perceptual tuning.

static constexpr int kNumCharacters = 3;

// Indices matching AudioParameterChoice order
static constexpr int kCharWarm    = 0;
static constexpr int kCharNeutral = 1;
static constexpr int kCharCold    = 2;

static const AirCharacterPreset kCharacterPresets[kNumCharacters] =
{
    // 0: Warm -- hot humid day, thick air, sound hangs longer
    // Physics: high humidity increases molecular relaxation absorption
    // above ~4kHz. Warm materials (carpet, drapes) add broad absorption.
    // Perceptually: gentle, gradual HF loss starting lower in frequency.
    {
        "Warm",
        6000.0f,     // shelfFreqHz: lower transition (warm air absorbs sooner)
        2.0f,        // shelfOctaves: gentle slope (gradual rolloff)
        -1.5f,       // minShelfDb: baseline at Air 0%
        -10.0f,      // maxShelfDb: at Air 100% (gentle but deep)

        250.0f,      // lfShelfFreqHz: subtle low warmth
        0.0f,        // lfMinShelfDb: no LF change at Air 0%
        1.5f,        // lfMaxShelfDb: subtle LF boost at Air 100% (+1.5dB)

        2000.0f,     // allpassBaseFreqHz: lower range
        2.5f,        // allpassSpreadOctaves: wide spread
        0.15f,       // allpassMinQ: minimal smearing at Air 0%
        0.8f,        // allpassMaxQ: more diffusion (warm air)
        5,           // allpassStages: 5 stages

        0.15f,       // tailDecayBias: positive = extends perceived decay
        1.2f         // reflDarkeningScale: stronger coupling (thick air)
    },

    // 1: Neutral -- moderate room conditions, balanced absorption
    // Physics: standard indoor conditions (20C, 50% RH).
    // Reference absorption curve from ISO 9613-1.
    {
        "Neutral",
        8000.0f,     // shelfFreqHz: mid-range transition
        1.8f,        // shelfOctaves: moderate slope
        -1.0f,       // minShelfDb: subtle baseline
        -8.0f,       // maxShelfDb: moderate depth

        0.0f,        // lfShelfFreqHz: 0 = no LF shelf (neutral)
        0.0f,        // lfMinShelfDb: N/A
        0.0f,        // lfMaxShelfDb: N/A

        2500.0f,     // allpassBaseFreqHz
        2.0f,        // allpassSpreadOctaves
        0.1f,        // allpassMinQ
        0.6f,        // allpassMaxQ
        4,           // allpassStages: 4 stages

        0.0f,        // tailDecayBias: neutral (no change)
        1.0f         // reflDarkeningScale: standard coupling
    },

    // 2: Cold -- cold dry day, bright and crisp with bite
    // Physics: low humidity reduces molecular relaxation absorption,
    // making air more transparent. But cold dry air has slightly
    // higher classical absorption. Hard reflective surfaces.
    // Perceptually: HF stays present longer, with a crisp edge.
    {
        "Cold",
        10000.0f,    // shelfFreqHz: higher transition (HF preserved longer)
        1.5f,        // shelfOctaves: steeper slope (more abrupt cutoff)
        -0.5f,       // minShelfDb: very subtle baseline
        -6.0f,       // maxShelfDb: less overall absorption

        250.0f,      // lfShelfFreqHz: subtle LF thinning
        0.0f,        // lfMinShelfDb: no change at Air 0%
        -1.0f,       // lfMaxShelfDb: slight LF cut (cold, thin quality)

        3000.0f,     // allpassBaseFreqHz: higher range
        1.5f,        // allpassSpreadOctaves: tighter spread
        0.08f,       // allpassMinQ: minimal smearing
        0.4f,        // allpassMaxQ: less diffusion (dry air)
        4,           // allpassStages: 4 stages

        -0.1f,       // tailDecayBias: negative = tightens perceived decay
        0.7f         // reflDarkeningScale: weaker coupling (bright environment)
    }
};
```

### Air Absorption Filter Implementation

```cpp
// Source: Adapted from ISO 9613-1 air absorption principles
// High shelf models the approximately 6 dB/octave rolloff above transition frequency

void AirSection::updateFilters()
{
    const AirCharacterPreset& ch = kCharacterPresets[currentCharIndex];
    float sr = static_cast<float>(currentSampleRate);
    float air = currentAirSmoothed;  // 0.0 to 1.0

    // HF absorption shelf: interpolate gain between min and max
    float shelfDb = ch.minShelfDb + air * (ch.maxShelfDb - ch.minShelfDb);
    float scaledShelfFreq = ch.shelfFreqHz / sr;

    hfShelfL.highShelfDb(scaledShelfFreq, shelfDb, ch.shelfOctaves);
    hfShelfR.highShelfDb(scaledShelfFreq, shelfDb, ch.shelfOctaves);

    // Optional LF character shelf
    if (ch.lfShelfFreqHz > 0.0f)
    {
        float lfDb = ch.lfMinShelfDb + air * (ch.lfMaxShelfDb - ch.lfMinShelfDb);
        float scaledLfFreq = ch.lfShelfFreqHz / sr;
        lfShelfL.lowShelfDb(scaledLfFreq, lfDb);
        lfShelfR.lowShelfDb(scaledLfFreq, lfDb);
    }

    // Allpass diffusion: scale Q with Air amount
    float q = ch.allpassMinQ + air * (ch.allpassMaxQ - ch.allpassMinQ);
    for (int i = 0; i < ch.allpassStages; ++i)
    {
        // Logarithmically spaced center frequencies
        float t = static_cast<float>(i) / static_cast<float>(ch.allpassStages - 1);
        float freqHz = ch.allpassBaseFreqHz
            * std::pow(2.0f, t * ch.allpassSpreadOctaves);
        float scaledFreq = freqHz / sr;

        allpassL[i].allpassQ(scaledFreq, q);
        // R channel offset by ~12% for stereo decorrelation
        allpassR[i].allpassQ(scaledFreq * 1.12f, q);
    }
}
```

### Process Loop

```cpp
void AirSection::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 1 || numSamples < 1)
        return;

    // Early exit: if fully bypassed and not smoothing
    if (!bypassBlend.isSmoothing() && bypassBlend.getTargetValue() <= 0.0f)
        return;

    float* channelL = buffer.getWritePointer(0);
    float* channelR = (numChannels >= 2) ? buffer.getWritePointer(1) : nullptr;

    // Determine if we need per-sample coefficient updates
    bool airSmoothing = airSmoothed.isSmoothing();

    for (int s = 0; s < numSamples; ++s)
    {
        float blend = bypassBlend.getNextValue();

        // Update air smoothing and filters when parameter is ramping
        if (airSmoothing)
        {
            currentAirSmoothed = airSmoothed.getNextValue();
            // Recalculate coefficients periodically during smoothing
            // (every 16 samples to balance CPU vs smoothness)
            if ((s & 15) == 0)
                updateFilters();
        }

        float inL = channelL[s];
        float inR = (channelR != nullptr) ? channelR[s] : inL;

        // a. HF absorption shelf
        float outL = hfShelfL(inL);
        float outR = hfShelfR(inR);

        // b. LF character shelf (if active)
        if (kCharacterPresets[currentCharIndex].lfShelfFreqHz > 0.0f)
        {
            outL = lfShelfL(outL);
            outR = lfShelfR(outR);
        }

        // c. Allpass phase smearing cascade
        int stages = kCharacterPresets[currentCharIndex].allpassStages;
        for (int i = 0; i < stages; ++i)
        {
            outL = allpassL[i](outL);
            outR = allpassR[i](outR);
        }

        // d. Bypass crossfade
        float finalL = inL * (1.0f - blend) + outL * blend;
        float finalR = inR * (1.0f - blend) + outR * blend;

        channelL[s] = finalL;
        if (channelR != nullptr)
            channelR[s] = finalR;
    }

    // After block, check if smoothing completed
    if (airSmoothing && !airSmoothed.isSmoothing())
        updateFilters();  // Final coefficient update at target value
}
```

### Cross-Stage Coupling: Air -> Reflections

```cpp
// In ReflectionsSection: add air darkening to existing tap filter cutoff
// airDarkening: 0.0 (no air darkening) to ~1.2 (Warm at 100% Air)

void ReflectionsSection::setAirDarkening(float darkening)
{
    airDarkeningFactor = juce::jlimit(0.0f, 2.0f, darkening);
}

// In process(), modify the existing baseCutoff calculation:
// Original (Phase 3):
//   float baseCutoff = 12000.0f - roomSize * 6000.0f;
// With Air coupling:
//   float airCutoffReduction = airDarkeningFactor * 3000.0f;
//   float baseCutoff = 12000.0f - roomSize * 6000.0f - airCutoffReduction;
//   baseCutoff = juce::jmax(2000.0f, baseCutoff);  // floor at 2kHz
```

### Cross-Stage Coupling: Character -> Tail Decay

```cpp
// In DiffuseTailSection: character decay bias modifies feedback gains
// bias > 0: longer perceived decay (Warm -- humid air holds sound)
// bias < 0: shorter perceived decay (Cold -- dry air lets sound die)
// bias = 0: no change (Neutral)

void DiffuseTailSection::setCharacterDecayBias(float bias)
{
    // Clamp to +/- 10% decay time modification
    characterDecayBias = juce::jlimit(-0.10f, 0.10f, bias);
}

// In updateDecay():
void DiffuseTailSection::updateDecay(float decayMs)
{
    // Apply character bias: Warm extends, Cold tightens
    float biasedDecayMs = decayMs * (1.0f + characterDecayBias);
    biasedDecayMs = juce::jlimit(50.0f, 2200.0f, biasedDecayMs);

    float decaySec = biasedDecayMs / 1000.0f;
    float sr = static_cast<float>(currentSampleRate);

    for (int i = 0; i < kNumFDNLines; ++i)
    {
        float gain = std::pow(10.0f, -3.0f * fdnDelaySamples[i] / (decaySec * sr));
        feedbackGains[i] = juce::jlimit(0.0f, 0.9999f, gain);
    }
}
```

### Parameter Update: air_char Version Bump

```cpp
// In Parameters.h: Update air_char from 2 choices to 3 with version bump

layout.add(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID { ParamIDs::airChar, 2 },  // Version 2 (was 1 with 2 choices)
    "III Air - Character",
    juce::StringArray { "Warm", "Neutral", "Cold" },
    1  // default: Neutral (was 0/Warm with 2 choices)
));
```

## Discretion Recommendations

### Cold Character Approach: Steeper Rolloff at Higher Frequency
**Recommendation:** Cold should use a steeper shelf slope (1.5 octaves vs Warm's 2.0) starting at a higher frequency (10kHz vs 6kHz). This means cold air preserves HF content for longer but drops off more abruptly when it does roll off. Do NOT add an HF presence boost -- this would violate the physics-driven design principle. In cold dry air, molecular relaxation absorption (which peaks around 4-10kHz depending on conditions) is reduced, so frequencies stay present longer. The "bite" of cold air comes from the preservation of upper harmonics, not from boosting them. Adding a subtle LF cut (-1dB at 250Hz) gives Cold a thinner quality that complements the bright top end.

### Character Selector: 3-Position AudioParameterChoice
**Recommendation:** Use a 3-position AudioParameterChoice, consistent with the existing refl_shape (7-position choice) and res_material (10-position choice) patterns. A continuous blend knob would require interpolating between filter curves and allpass parameters in complex ways, adding significant DSP complexity for marginal benefit. The user described three distinct environmental conditions (hot muggy day / moderate room / cold dry day), not a continuum. A 3-position toggle will appear as a 3-position selector in the DAW automation list, making it easy to automate between environments. The Phase 6 UI will render this as a 3-position toggle switch.

### Characters Affecting Low-Frequency Behavior: Yes, Subtly
**Recommendation:** Warm should add a subtle low shelf boost (+1.5dB at 250Hz) at maximum Air. This models the way thick, humid air and soft materials (carpet, drapes) allow low frequencies to linger, giving warmth and body. Cold should add a subtle low shelf cut (-1.0dB at 250Hz) at maximum Air, modeling the thinner quality of cold dry air and hard reflective surfaces. Neutral has no LF modification. The LF effect should be subtle enough that it is felt rather than analytically heard -- it contributes to the "vibe" of each character without drawing attention to itself. At 0% Air, there is no LF modification for any character (LF changes scale from 0 at Air 0% to full at Air 100%).

### Transient Softening: Clearly Audible at Maximum, Subtle Byproduct Below 50%
**Recommendation:** At 100% Air, the transient softening should be clearly audible -- pick attacks on guitar, cymbal transients, and snare hits should all sound noticeably less "immediate." This is the "washy" quality the user described as acceptable at maximum. Below 50% Air, the allpass effect should be a subtle byproduct that you notice when A/B'd against bypass but not during normal listening. This is achieved by the Q scaling: at 50% Air the Q is roughly 0.3-0.4 (gentle phase dispersion), while at 100% it reaches 0.6-0.8 (pronounced smearing). The threshold-like behavior emerges naturally from the Q curve -- linear Q scaling produces exponential perceptual change because phase dispersion increases nonlinearly with Q.

### Diffusion Scaling: Slightly Exponential (Naturally Threshold-Like)
**Recommendation:** Scale allpass Q linearly with Air amount. Due to the nonlinear relationship between Q and perceived phase dispersion, this produces a naturally threshold-like behavior: subtle diffusion grows slowly from 0-50% Air, then becomes increasingly noticeable from 50-100%. No explicit threshold needed. The linear mapping `q = minQ + air * (maxQ - minQ)` is simple, predictable, and produces the desired perceptual curve.

### Character Influence on Diffusion: Yes, Physically Motivated
**Recommendation:** Warm should produce more phase diffusion than Cold at the same Air amount. This is physically plausible: humid air has more molecular interactions (both H2O and O2/N2 relaxation contribute), creating more phase dispersion per unit distance. Warm uses Q range 0.15-0.8 and 5 allpass stages. Cold uses Q range 0.08-0.4 and 4 stages. Neutral sits between at 0.1-0.6 with 4 stages. The difference in allpass stage count (4 vs 5) provides additional diffusion differentiation beyond Q alone.

### Air-to-Coupling Strength: Direct > Reflections > Tail
**Recommendation:** The direct signal filtering (high shelf) should be the strongest effect. Reflection tap darkening should be at ~50-70% of the direct effect strength (controlled by reflDarkeningScale: Warm=1.2, Neutral=1.0, Cold=0.7). Tail HF damping already has its own curve from Phase 3 (Air 0%=12kHz, Air 100%=2kHz) which is sufficiently strong. The hierarchy ensures the direct signal change dominates the perception, with reflections and tail supporting it coherently. At 0% Air, the baseline is too subtle to meaningfully affect reflections (baseCutoff reduction of ~0dB), so baseline coupling to reflections and tail is effectively zero.

### Character Affecting Coupling Strength: Yes
**Recommendation:** Character should affect both direct signal filtering AND cross-stage coupling strength. Warm darkens reflections more aggressively (reflDarkeningScale=1.2) and extends tail decay slightly (+15% at max Air). Cold darkens reflections less (reflDarkeningScale=0.7) and tightens tail decay slightly (-10% at max Air). This creates a cohesive environmental impression: in a warm, humid room, EVERYTHING sounds warmer and hangs longer. In a cold, dry room, EVERYTHING sounds crisper and tighter. Without character affecting coupling, switching characters would only change the direct signal while reflections and tail remain unchanged, breaking the illusion.

### Baseline at 0% Air Affecting Cross-Stage: No
**Recommendation:** The baseline absorption at Air 0% should NOT propagate to reflections or tail. At 0% Air, the baseline shelf (-0.5 to -1.5dB above 6-10kHz) is so subtle that pushing it through cross-stage coupling would be inaudible AND would add complexity (non-zero darkening at the reflection stage's default state). The baseline exists to make the direct signal path never fully transparent -- it is a Stage III local decision. Cross-stage coupling scales from 0 at Air 0% up to full at Air 100%. This keeps the architecture clean: `reflDarkeningAmount = airAmount * characterScale`, where airAmount=0 produces zero coupling.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| 2-choice character (Warm/Neutral) | 3-choice character (Warm/Neutral/Cold) | Phase 4 (per user decision) | Breaking change requiring version bump on air_char. Cold provides the bright/crisp environmental condition that was missing. |
| Air couples only to Tail HF damping | Air couples to BOTH Reflections AND Tail | Phase 4 (per user decision) | More physically accurate. All sound paths through the room are affected by air absorption, not just the tail. |
| Character affects only direct signal | Character influences coupling strength | Phase 4 (per user decision) | Creates cohesive environmental impression. Warm = everything warmer. Cold = everything crisper. |

**Deprecated/outdated:**
- 2-choice air_char parameter (version 1): Replaced by 3-choice (version 2)
- Air coupling to Tail only: Extended to Reflections as well

## Open Questions

1. **Exact character preset tuning**
   - What we know: The three character presets define shelf frequencies, gains, allpass parameters, and coupling factors based on physics-derived principles.
   - What's unclear: Whether the specific numerical values produce the intended perceptual differences when heard in context. The shelf gain ranges (-6 to -10dB) and allpass Q ranges (0.08-0.8) are educated starting points.
   - Recommendation: Implement with the documented values, then tune by ear during verification. The architecture is correct; specific numbers need listening.

2. **Allpass stage count vs metallic artifacts**
   - What we know: 4-6 biquad allpass stages provide noticeable phase smearing without metallic coloration per the Valhalla DSP analysis.
   - What's unclear: Whether 5 stages (Warm) vs 4 stages (Neutral/Cold) produces a meaningful perceptual difference, or whether the Q range alone is sufficient for differentiation.
   - Recommendation: Start with the differentiated stage counts. If 4 vs 5 stages does not produce an audible difference, consolidate to 4 stages for all characters and rely solely on Q for differentiation.

3. **Filter coefficient update rate during Air amount smoothing**
   - What we know: Biquad coefficient calculation involves transcendental functions. Updating every sample is wasteful; updating too infrequently causes stepping.
   - What's unclear: The optimal update interval during smoothing (every sample vs every 16 vs every 32 samples).
   - Recommendation: Start with every 16 samples (proposed in the code example). At 44.1kHz this means coefficient updates at 2756Hz -- well above the perceptual threshold for EQ changes. If zipper artifacts are heard, reduce to every 8 samples. If CPU is a concern, increase to every 32.

4. **Reflection tap darkening integration**
   - What we know: ReflectionsSection already has per-tap lowpass filters whose cutoff is modulated by room size. Air darkening would further reduce this cutoff.
   - What's unclear: Whether adding air darkening to the existing baseCutoff calculation (subtractive) is the cleanest approach, or whether a separate air-controlled filter per tap is needed.
   - Recommendation: Use the subtractive approach (reduce baseCutoff by airDarkeningFactor * 3000Hz). This is simple, integrates with the existing tap filter logic, and avoids adding 16 additional biquad filters. The existing jlimit(1000, 16000) prevents the cutoff from going too low.

## Sources

### Primary (HIGH confidence)
- Signalsmith DSP v1.7.1 source code at `build/_deps/signalsmith-dsp-src/filters.h` -- BiquadStatic API verified: `highShelfDb(scaledFreq, db, octaves)`, `lowShelfDb(scaledFreq, db, octaves)`, `allpassQ(scaledFreq, q)`, `lowpass(scaledFreq)`. All accept `scaledFreq = freq / sampleRate`. BiquadDesign options (bilinear, cookbook, oneSided, vicanek) verified. Directly examined source code.
- Aether Phase 3 codebase at `Source/` -- ReflectionsSection.h/.cpp (per-tap lowpass filter infrastructure, shape crossfade pattern, baseCutoff modulation by room size), DiffuseTailSection.h/.cpp (setHFDamping already wired, dampingCutoffHz curve, feedback gain calculation with safety clamp), PluginProcessor.cpp (updateStageParams cross-stage forwarding pattern, air param pointers already cached). Directly examined.
- Aether Phase 3 Research at `.planning/phases/03-early-reflections-diffuse-tail/03-RESEARCH.md` -- Cross-stage parameter linking pattern via processor forwarding. Anti-Pattern 5 avoidance (no direct section-to-section coupling). Verified architecture consistency.

### Secondary (MEDIUM confidence)
- [ISO 9613-1 / ISO 9613-2 air absorption coefficients](https://www.iso.org/standard/17426.html) -- Standard reference for atmospheric sound absorption. Attenuation coefficients: 1kHz=0.0037 dB/m, 2kHz=0.0097 dB/m, 4kHz=0.0328 dB/m, 8kHz=0.117 dB/m at 20C/50%RH. Confirms frequency-squared relationship above ~2kHz. Values from military standard table (SAE ARP 866A) cross-referenced with ISO data.
- [Approximating Atmospheric Absorption with a Simple Filter](https://computingandrecording.wordpress.com/2017/07/05/approximating-atmospheric-absorption-with-a-simple-filter/) -- One-pole lowpass as first approximation. Relationship: alpha = 3.0/r for cutoff frequency. We use high shelf instead (avoids blanket effect), but the distance-to-absorption mapping informs our Air knob scaling.
- [Valhalla DSP: Diffusion, Allpass Delays, and Metallic Artifacts](https://valhalladsp.com/2011/01/21/reverbs-diffusion-allpass-delays-and-metallic-artifacts/) -- Five mitigation strategies for allpass metallic artifacts: fewer stages, more stages, modulation, lower coefficients, embrace coloration. Confirms 4-6 stages as practical range. Q/coefficient control as "Diffusion" parameter.
- [DSP Related: Artificial Reverberation / Physical Audio Signal Processing (J.O. Smith)](https://www.dsprelated.com/freebooks/pasp/Artificial_Reverberation.html) -- One-pole lowpass in feedback for air absorption modeling in reverbs. Frequency-dependent decay via lowpass-feedback-comb-filters. Freeverb damping coefficient d=0.2 as reference. Confirms high shelf + lowpass combination for professional reverb implementations.
- [GullDSP Circulate: Open Source Allpass Phase Smearing VST](https://github.com/GullDSP/Circulate-VST) -- Production reference: up to 64 allpass stages for phase smearing. Center frequency + Q control. Confirms biquad allpass bank architecture for transient dispersion.
- [UAD Ocean Way Studios](https://www.uaudio.com/uad-plugins/reverbs/ocean-way-studios.html) -- Reference plugin for distance-as-spatial-effect. Distance control applies filtering and phase shift to simulate mic distance. Validates the approach of combining frequency and phase processing for distance simulation.
- [Penn State: Absorption and Attenuation of Sound](https://www.acs.psu.edu/drussell/Demos/Absorption/Absorption.html) -- Classical absorption proportional to frequency squared. Molecular relaxation in O2 and N2 as additional mechanism. Temperature and humidity dependence. ANSI S1-26:1995 reference.
- [Sengpiel Audio: Air Absorption Calculator](https://sengpielaudio.com/calculator-air.htm) -- Interactive calculator implementing ISO 9613-1 formulas. Confirms attenuation values used in this research.

### Tertiary (LOW confidence)
- Character preset numerical values (shelf gains, allpass Q ranges, coupling factors) -- Synthesized from physics principles and perceptual tuning experience. The relative relationships between characters (Warm darker than Neutral, Cold brighter) are well-motivated. Specific numerical values are educated starting points that require ear-tuning.
- Allpass center frequency spacing (2-12kHz logarithmic) -- Common starting values for broadband phase dispersion. The specific range may need adjustment based on how it interacts with the shelf filtering and the content being processed.
- LF shelf amounts (Warm +1.5dB, Cold -1.0dB) -- Perceptual tuning values, not physics-derived. Indoor environments do have frequency-dependent absorption in the low-mid range from materials (carpet, curtains), but quantifying this as a specific dB value is an artistic choice.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH -- JUCE 8.0.12 and Signalsmith DSP 1.7.1 verified from Phases 1-3. All filter APIs confirmed from source inspection (highShelfDb, lowShelfDb, allpassQ). No new libraries needed.
- Architecture (filter chain): HIGH -- High shelf for HF absorption is the standard approach for modeling air absorption without lowpass "blanket" effect. Biquad allpass cascade for phase smearing is well-established (Circulate VST, Valhalla DSP analysis, J.O. Smith). Cross-stage coupling via processor forwarding is the established Aether pattern.
- Architecture (cross-stage coupling): HIGH -- Extends existing pattern from Phase 3 Plan 3. ReflectionsSection already has per-tap lowpass infrastructure. DiffuseTailSection already has setHFDamping. Adding new setters follows the established interface.
- Character presets: MEDIUM -- The DSP structure for characters is sound, but specific numerical values (shelf gains, allpass Q ranges, coupling factors) are educated starting points. The three characters cover the environmental space well (hot/moderate/cold), but specific tuning requires listening.
- Allpass tuning: MEDIUM -- 4-6 biquad allpass stages with Q 0.08-0.8 and frequencies 2-12kHz are reasonable starting points. The exact feel of "transient softening via phase smearing" depends on interaction with the specific content being processed. May need adjustment after listening.

**Research date:** 2026-02-18
**Valid until:** 2026-03-18 (stable domain -- no fast-moving dependencies)
