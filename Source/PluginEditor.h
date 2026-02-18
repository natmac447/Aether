#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

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
