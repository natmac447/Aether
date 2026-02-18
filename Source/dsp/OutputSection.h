#pragma once
#include <JuceHeader.h>

// Output Section
// Output level trim with SmoothedValue gain (Crucible pattern).
// Range: -24dB to +6dB with 20ms smoothing ramp (ENG-01).

class OutputSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer, float gainDb);
    void reset();

private:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> gainSmoothed;
};
