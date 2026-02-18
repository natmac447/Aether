#pragma once
#include <JuceHeader.h>

// Stage I: Resonance -- Short FDN for material body resonance with resonant
// bandpass filtering. 10 material types across wood, metal, and stone families.

class ResonanceSection
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

    // FDN state (Plan 02 fills in real DSP members)
    float weight = 0.5f;          // current weight value
    int materialIndex = 3;        // default: Mahogany, index 3
};
