#pragma once
#include <JuceHeader.h>

// Stage VI: Diffuse Tail
// Phase 3: FDN reverb with Decay/Diffusion controls.
// Auto pre-delay linked to Room Size.

class DiffuseTailSection
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
