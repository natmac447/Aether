#pragma once
#include <JuceHeader.h>

// Stage I: Cabinet Resonance
// Phase 2: Short FDN for cabinet body resonance with resonant bandpass filtering.
// Three cabinet types: Open, Closed, Combo.

class CabinetSection
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
