#pragma once
#include <JuceHeader.h>
#include <signalsmith-dsp/delay.h>
#include <signalsmith-dsp/filters.h>
#include <signalsmith-dsp/mix.h>
#include <array>

// Stage I: Resonance -- 4-line Householder FDN for material body resonance
// with resonant bandpass filtering. 10 material types across wood, metal,
// and stone families. Weight knob controls wet/dry with quadratic curve.

struct MaterialParams
{
    const char* name;
    const char* description;    // Victorian-style tooltip text for Phase 6 UI
    float delayMs[4];           // FDN delay times in milliseconds
    float bandpassFreqHz;       // Resonant bandpass center frequency
    float bandpassQ;            // Bandpass Q factor
    float dampingFreqHz;        // Damping lowpass cutoff
    float feedbackGain;         // Must be < 1.0 for stability
    float lowShelfGainDb;       // Low shelf boost for body character
    float lowShelfFreqHz;       // Low shelf transition frequency
    float outputGain;           // Per-material output normalization
};

class ResonanceSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);
    void reset();
    void setBypass (bool shouldBypass);
    void setWeight (float weight);
    void setMaterial (int materialIndex);

    static constexpr int kNumDelayLines = 4;
    static constexpr int kNumMaterials = 10;
    static const MaterialParams kMaterials[kNumMaterials];

private:
    void configureForMaterial (const MaterialParams& mat);
    static int nearestPrime (int target);

    // FDN delay lines
    signalsmith::delay::Delay<float> delayLines[kNumDelayLines];

    // Per-line filters (inside feedback loop)
    signalsmith::filters::BiquadStatic<float> bandpassFilters[kNumDelayLines];
    signalsmith::filters::BiquadStatic<float> dampingFilters[kNumDelayLines];

    // Output shelf (outside feedback loop)
    signalsmith::filters::BiquadStatic<float> lowShelfFilter;

    // FDN state
    float currentDelayTimeSamples[kNumDelayLines] = {};
    float currentFeedbackGain = 0.0f;
    float currentOutputGain = 1.0f;

    // Smoothed parameters
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> weightSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassBlend;

    bool bypassed = false;
    int currentMaterialIndex = 3;   // Default: Mahogany (index 3)
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};
