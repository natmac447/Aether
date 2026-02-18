#pragma once
#include <JuceHeader.h>

// Stage II: Early Reflections
// Phase 3: Multi-tap delay network with stereo decorrelation.
// Room Size, Shape, Proximity, Width controls.

class ReflectionsSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);
    void reset();
    void setBypass (bool bypassed);
    void setRoomSize (float size);
    void setShape (int shapeIndex);
    void setProximity (float proximity);
    void setWidth (float width);

private:
    bool bypassed = false;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};
