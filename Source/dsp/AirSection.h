#pragma once
#include <JuceHeader.h>
#include <signalsmith-dsp/filters.h>
#include <signalsmith-dsp/mix.h>

// Stage III: Air & Distance
// Physics-driven HF absorption via high shelf, allpass phase smearing for
// transient softening, three character variants (Warm/Neutral/Cold), bypass
// crossfade (10ms), character crossfade (30ms), smooth Air Amount ramping (25ms).

// =========================================================================
// Character Preset Structure
// =========================================================================
// Each character defines a complete filter parameter set for an environmental
// condition. Physics-derived from ISO 9613-1 air absorption data, adapted
// for indoor room distances (2-30m) and perceptual tuning.

struct AirCharacterPreset
{
    const char* name;

    // HF absorption shelf
    float shelfFreqHz;          // Transition frequency for HF rolloff
    float shelfOctaves;         // Shelf bandwidth/slope
    float minShelfDb;           // Shelf gain at Air = 0% (baseline)
    float maxShelfDb;           // Shelf gain at Air = 100%

    // LF character shelf (optional warmth/thinning)
    float lfShelfFreqHz;        // Low shelf frequency (0 = disabled)
    float lfMinShelfDb;         // LF shelf at Air = 0%
    float lfMaxShelfDb;         // LF shelf at Air = 100%

    // Allpass diffusion
    float allpassBaseFreqHz;    // Lowest allpass center frequency
    float allpassSpreadOctaves; // Range spread for allpass center frequencies
    float allpassMinQ;          // Q at Air = 0% (minimal phase shift)
    float allpassMaxQ;          // Q at Air = 100% (maximum smearing)
    int   allpassStages;        // Number of allpass stages (4-6)

    // Cross-stage coupling
    float tailDecayBias;        // -1.0 to 1.0: negative tightens, positive extends
    float reflDarkeningScale;   // How strongly this character darkens reflections (0.5-1.5x)
};

// =========================================================================
// Character Presets
// =========================================================================

static constexpr int kNumCharacters = 3;
static constexpr int kMaxAllpassStages = 6;

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
    // making air more transparent. Hard reflective surfaces.
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

// =========================================================================
// AirSection Class
// =========================================================================

class AirSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);
    void reset();

    void setBypass (bool bypassed);
    void setAmount (float airAmount);        // 0.0-1.0
    void setCharacter (int characterIndex);  // 0=Warm, 1=Neutral, 2=Cold

private:
    void updateFilters();
    void updatePendingFilters();

    // HF absorption shelf (current character)
    signalsmith::filters::BiquadStatic<float> hfShelfL, hfShelfR;

    // LF character shelf (current character)
    signalsmith::filters::BiquadStatic<float> lfShelfL, lfShelfR;

    // Allpass cascade (current character)
    signalsmith::filters::BiquadStatic<float> allpassL[kMaxAllpassStages];
    signalsmith::filters::BiquadStatic<float> allpassR[kMaxAllpassStages];

    // Pending character filters (for crossfade)
    signalsmith::filters::BiquadStatic<float> pendHfShelfL, pendHfShelfR;
    signalsmith::filters::BiquadStatic<float> pendLfShelfL, pendLfShelfR;
    signalsmith::filters::BiquadStatic<float> pendAllpassL[kMaxAllpassStages];
    signalsmith::filters::BiquadStatic<float> pendAllpassR[kMaxAllpassStages];

    // SmoothedValues
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> airSmoothed;   // 25ms ramp
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassBlend;   // 10ms ramp

    // Character crossfade state
    int   currentCharIndex  = 1;    // Neutral default
    int   pendingCharIndex  = -1;   // -1 when not crossfading
    float charCrossfade     = 1.0f; // 1.0 = fully on current character
    float charCrossfadeStep = 0.0f; // per-sample decrement for 30ms fade

    // State
    double currentSampleRate    = 44100.0;
    int    currentBlockSize     = 512;
    float  currentAirSmoothed   = 0.4f;  // cached for coefficient calculation
};
