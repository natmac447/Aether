#pragma once
#include <JuceHeader.h>

// Stage IV: Excitation
// Frequency-dependent multiband soft saturation with oversampling.
// Models how sound excites a physical space at different volumes --
// producing density and liveliness, not gear-style distortion.
//
// Signal flow:
//   Input -> Upsample -> 3-band LR4 crossover -> per-band tanh waveshaping
//         -> recombine -> Downsample -> DC block -> bypass crossfade -> Output
//
// Cross-stage coupling:
//   Material (Stage I) -> per-band drive scaling (wood warmer, metal brighter, stone darker)
//   Room Size (Stage II) -> band balance (small=tight, large=low bloom)

// =========================================================================
// Material Excitation Bias
// =========================================================================
// Per-band scale modifiers for each of the 10 materials.
// These multiply the base per-band drive scales to produce
// material-coupled excitation character.

struct MaterialExcitationBias
{
    float lowScale;
    float midScale;
    float highScale;
};

static constexpr MaterialExcitationBias kMaterialBias[10] =
{
    // Woods: warm, resonant -- boost mids, gentle highs
    { 0.9f,  1.15f, 0.7f  },  // 0: Pine (open, resonant)
    { 0.95f, 1.1f,  0.75f },  // 1: Oak (dense, structured)
    { 0.9f,  1.12f, 0.72f },  // 2: Walnut (rich, balanced)
    { 0.85f, 1.2f,  0.68f },  // 3: Mahogany (deep, warm)

    // Metals: bright, focused -- boost highs, tighter lows
    { 0.7f,  0.95f, 1.15f },  // 4: Iron (forceful, edgy)
    { 0.65f, 0.9f,  1.2f  },  // 5: Steel (brilliant, precise)
    { 0.75f, 1.0f,  1.1f  },  // 6: Copper (warm brilliance)

    // Stones: deep, diffuse -- boost lows, gentle highs
    { 1.15f, 0.95f, 0.65f },  // 7: Limestone (soft, diffuse)
    { 1.1f,  1.0f,  0.7f  },  // 8: Marble (polished, even)
    { 1.2f,  0.9f,  0.6f  },  // 9: Granite (massive, dense)
};

// =========================================================================
// ExcitationSection Class
// =========================================================================

class ExcitationSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);
    void reset();
    void setBypass (bool bypassed);

    void setDrive (float driveNormalized);       // 0.0-1.0
    void setMaterial (int materialIndex);         // 0-9 (from Stage I)
    void setRoomSize (float roomSizeNormalized);  // 0.0-1.0 (from Stage II)

    int getLatencySamples() const;

private:
    void updateDriveParams();

    // 3-band crossover: two LR4 filter pairs
    // Low crossover at kLowCrossFreq: splits input into lowBand and midHigh
    // High crossover at kHighCrossFreq: splits midHigh into midBand and highBand
    juce::dsp::LinkwitzRileyFilter<float> lowCross;   // provides LP/HP at kLowCrossFreq
    juce::dsp::LinkwitzRileyFilter<float> highCross;   // provides LP/HP at kHighCrossFreq

    // Oversampling
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    int oversamplingOrder = 2;  // 4x default at 44.1kHz

    // DC blocker (5Hz HPF to remove any residual DC offset from waveshaping)
    juce::dsp::IIR::Filter<float> dcBlockerL, dcBlockerR;

    // Work buffer for dry copy (pre-allocated in prepare, used for bypass crossfade)
    juce::AudioBuffer<float> dryCopy;

    // Per-band waveshaping G values: tanh(x*G)/tanh(G)
    float lowG  = 0.3f;
    float midG  = 0.5f;
    float highG = 0.35f;

    // Smoothed parameters
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveSmoothed;  // 25ms ramp
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassBlend;    // 10ms ramp

    // Coupling state
    int   currentMaterialIndex = 3;     // Mahogany default
    float currentRoomSize      = 0.4f;  // Medium default
    float currentDrive         = 0.25f; // 25% default

    // State
    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;
    int    latencySamples    = 0;

    // =========================================================================
    // Constants
    // =========================================================================

    // Crossover frequencies (Hz)
    static constexpr float kLowCrossFreq  = 200.0f;
    static constexpr float kHighCrossFreq = 3000.0f;

    // Drive curve: pow(drive, 0.7) for sweet spot in 30-60% range
    static constexpr float kDriveCurveExponent = 0.7f;

    // Base per-band drive scaling (before material and room size modifiers)
    static constexpr float kBaseLowScale  = 0.5f;
    static constexpr float kBaseMidScale  = 1.0f;
    static constexpr float kBaseHighScale = 0.6f;

    // Maximum G values per band (caps saturation at reasonable levels)
    static constexpr float kMaxLowG  = 1.5f;
    static constexpr float kMaxMidG  = 2.5f;
    static constexpr float kMaxHighG = 1.8f;

    // Minimum G at Drive=0% (subtle room presence when not bypassed)
    static constexpr float kMinG = 0.15f;
};
