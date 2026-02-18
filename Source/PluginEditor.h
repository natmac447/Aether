#pragma once
#include <JuceHeader.h>

class AetherProcessor;

// Minimal editor shell. Currently not used -- createEditor() returns
// GenericAudioProcessorEditor for parameter testing. This class will
// become the custom Victorian parchment UI in Phase 6.
class AetherEditor : public juce::AudioProcessorEditor
{
public:
    explicit AetherEditor (AetherProcessor&);
    ~AetherEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AetherProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherEditor)
};
