#include "PresetSelector.h"
#include "AetherColours.h"
#include "AetherLookAndFeel.h"

//==============================================================================
PresetSelector::PresetSelector()
{
    // Populate with placeholder presets (Phase 8 will implement actual loading)
    comboBox.addItem ("-- Default --", 1);
    comboBox.addItem ("I. -- Tight Booth", 2);
    comboBox.addItem ("II. -- Vocal Room", 3);
    comboBox.addItem ("III. -- Live Stage", 4);
    comboBox.addItem ("IV. -- Stone Chapel", 5);
    comboBox.addItem ("V. -- Concert Hall", 6);
    comboBox.addItem ("VI. -- Church Hall", 7);

    comboBox.setSelectedId (1, juce::dontSendNotification);

    // Style: Ink Light text, Ink Ghost border, transparent background
    comboBox.setColour (juce::ComboBox::textColourId, juce::Colour (AetherColours::inkLight));
    comboBox.setColour (juce::ComboBox::outlineColourId, juce::Colour (AetherColours::inkGhost));
    comboBox.setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    comboBox.setColour (juce::ComboBox::arrowColourId, juce::Colour (AetherColours::inkFaint));

    addAndMakeVisible (comboBox);
}

//==============================================================================
void PresetSelector::resized()
{
    comboBox.setBounds (getLocalBounds());
}
