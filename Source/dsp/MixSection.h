#pragma once
#include <JuceHeader.h>
#include <signalsmith-dsp/filters.h>

// Mix Section
// Dry/wet mixing with DryWetMixer (sin3dB equal-power crossfade) and auto-gain compensation.
// Dry signal captured at input (OUT-04), mixed after 6 DSP stages.
// Auto-gain curve: -3.5dB * pow(mix, 1.4) to compensate perceived loudness increase (OUT-03).

class MixSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void pushDrySamples (const juce::dsp::AudioBlock<float>& dryBlock);
    void mixWetSamples (juce::dsp::AudioBlock<float>& wetBlock);
    void setMixLevel (float mixValue);
    void applyAutoGainCompensation (juce::AudioBuffer<float>& buffer, float mixValue,
                                     float driveValue = 0.0f, float decayNorm = 0.0f,
                                     float diffusionValue = 0.0f,
                                     float roomSizeNorm = 0.4f, float proximity = 0.3f);
    void setWetLatency (float samples);
    void reset();

private:
    juce::dsp::DryWetMixer<float> dryWetMixer;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> compensationSmoothed;
    float currentMix = 0.7f;

    static constexpr int kDryDecorrelationStages = 2;
    signalsmith::filters::BiquadStatic<float> dryDecorrelationL[kDryDecorrelationStages];
    signalsmith::filters::BiquadStatic<float> dryDecorrelationR[kDryDecorrelationStages];

    // Gentle low shelf on wet path to restore low-end body lost to
    // phase cancellation during dry/wet mixing (100-250Hz region).
    signalsmith::filters::BiquadStatic<float> wetLowShelfL;
    signalsmith::filters::BiquadStatic<float> wetLowShelfR;

    double storedSampleRate = 44100.0;
};
