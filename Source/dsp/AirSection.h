#pragma once
#include <JuceHeader.h>

// Stage III: Air & Distance
// Phase 4: Frequency-dependent absorption with Warm/Neutral character.
// Includes allpass filtering for phase smearing.

class AirSection
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
