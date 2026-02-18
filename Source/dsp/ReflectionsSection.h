#pragma once
#include <JuceHeader.h>

// Stage II: Early Reflections
// Phase 3: Multi-tap delay network with stereo decorrelation.
// Room Size, Shape, Proximity controls.

class ReflectionsSection
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
