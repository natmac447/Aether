#pragma once
#include <JuceHeader.h>
#include <signalsmith-dsp/delay.h>
#include <signalsmith-dsp/filters.h>
#include <signalsmith-dsp/mix.h>
#include <array>
#include <cmath>

// Stage VI: Diffuse Tail
// 8-line Hadamard FDN reverb with input allpass diffusion, automatic pre-delay
// linked to Room Size, frequency-dependent feedback damping linked to Air,
// and shape-influenced tail character.

class DiffuseTailSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);
    void reset();
    void setBypass (bool shouldBypass);
    void setDecay (float decayMs);
    void setDiffusion (float diffusionNormalized);
    void setPreDelay (float roomSizeNormalized);
    void setHFDamping (float airAmount);
    void setCharacterDecayBias (float bias);
    void setShapeInfluence (int shapeIndex);

    static constexpr int kNumFDNLines = 8;
    static constexpr int kNumDiffusionStages = 4;

    // FDN base delay times in ms (logarithmically distributed 22-63ms)
    static constexpr float kFDNDelayMs[kNumFDNLines] = {
        22.0f, 26.5f, 31.5f, 37.0f, 43.5f, 50.0f, 56.5f, 63.0f
    };

    // Input diffusion delay times in ms
    static constexpr float kDiffusionDelayMs[kNumDiffusionStages] = {
        1.5f, 3.0f, 5.0f, 8.0f
    };

private:
    void configureDelayLengths();
    void updateDecay (float decayMs);
    void updateDampingFilters();
    static int nearestPrime (int target);

    // Pre-delay buffers (stereo)
    signalsmith::delay::Delay<float> preDelayL;
    signalsmith::delay::Delay<float> preDelayR;

    // Input diffusion allpass delay lines (per channel, 4 stages)
    // Explicit delay+feedback allpass (not biquad -- research recommends for 1.5-8ms delays)
    signalsmith::delay::Delay<float> diffDelayL[kNumDiffusionStages];
    signalsmith::delay::Delay<float> diffDelayR[kNumDiffusionStages];

    // FDN delay lines (8 channels)
    signalsmith::delay::Delay<float> fdnDelayLines[kNumFDNLines];

    // FDN damping lowpass (per line, inside feedback loop)
    signalsmith::filters::BiquadStatic<float> dampingFilters[kNumFDNLines];

    // FDN state
    float fdnDelaySamples[kNumFDNLines] = {};
    float feedbackGains[kNumFDNLines] = {};

    // Diffusion delay state (slightly different L/R for stereo decorrelation)
    float diffDelaySamplesL[kNumDiffusionStages] = {};
    float diffDelaySamplesR[kNumDiffusionStages] = {};

    // Smoothed parameters
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> decaySmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> diffusionSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> preDelaySmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassBlend;

    // HF damping cutoff (set by Air link)
    float dampingCutoffHz = 12000.0f;

    // Shape influence on tail character
    float shapeDiffusionBias = 0.3f;
    float shapeModalCharacter = 0.2f;

    // Current state
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    bool bypassed = false;

    // Track whether decay needs updating per sample during smoothing
    float lastDecayMs = 150.0f;

    // Cross-stage: Character decay bias (+/- 10% max)
    float characterDecayBias = 0.0f;
};
