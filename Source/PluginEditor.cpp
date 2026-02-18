#include "PluginEditor.h"
#include "PluginProcessor.h"

AetherEditor::AetherEditor (AetherProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // 900x530 matches the final design dimensions from the design handoff
    setSize (900, 530);
}

AetherEditor::~AetherEditor() {}

void AetherEditor::paint (juce::Graphics& g)
{
    // Parchment background placeholder color (#f0e6d3)
    g.fillAll (juce::Colour (0xfff0e6d3));
}

void AetherEditor::resized()
{
    // Layout will be implemented in Phase 6
}
