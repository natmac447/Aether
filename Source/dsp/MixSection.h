#pragma once
#include <JuceHeader.h>

// Mix Section
// Dry/wet mixing with DryWetMixer (sin3dB equal-power crossfade) and auto-gain compensation.
// Dry signal captured at input (OUT-04), mixed after 6 DSP stages.
// Auto-gain curve: -2.5dB * pow(mix, 1.5) to compensate perceived loudness increase (OUT-03).

class MixSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void pushDrySamples (const juce::dsp::AudioBlock<float>& dryBlock);
    void mixWetSamples (juce::dsp::AudioBlock<float>& wetBlock);
    void setMixLevel (float mixValue);
    void applyAutoGainCompensation (juce::AudioBuffer<float>& buffer, float mixValue);
    void setWetLatency (float samples);
    void reset();

private:
    juce::dsp::DryWetMixer<float> dryWetMixer;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> compensationSmoothed;
    float currentMix = 0.7f;
};
