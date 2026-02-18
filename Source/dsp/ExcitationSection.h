#pragma once
#include <JuceHeader.h>

// Stage IV: Excitation
// Phase 5: Frequency-dependent multiband soft saturation with oversampling.
// Adds density and liveliness without audible distortion.

class ExcitationSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);
    void reset();
    void setBypass (bool bypassed);

private:
    bool bypassed = false;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};
