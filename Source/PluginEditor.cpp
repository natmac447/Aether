#include "PluginEditor.h"

AetherEditor::AetherEditor (AetherProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (900, 530);
}

AetherEditor::~AetherEditor() {}

void AetherEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xfff0e6d3));
}

void AetherEditor::resized()
{
}
