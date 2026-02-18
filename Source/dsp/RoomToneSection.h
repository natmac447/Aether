#pragma once
#include <JuceHeader.h>

// Stage V: Room Tone
// Phase 5: Shaped noise generator with decorrelated stereo.
// Character linked to Room Size parameter.

class RoomToneSection
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
