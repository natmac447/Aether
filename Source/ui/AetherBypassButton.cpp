#include "AetherBypassButton.h"
#include "AetherColours.h"

//==============================================================================
AetherBypassButton::AetherBypassButton()
    : juce::TextButton ("IN")
{
    setClickingTogglesState (true);

    // Set the "isBypass" component property for AetherLookAndFeel styling
    getProperties().set ("isBypass", true);

    // Update text whenever toggle state changes
    // getToggleState() == true means bypass param is ON -> "OUT"
    // getToggleState() == false means bypass param is OFF -> "IN"
    onStateChange = [this] { updateText(); };
}

//==============================================================================
// Parameter attachment
//==============================================================================

void AetherBypassButton::attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                                             const juce::String& paramID)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, paramID, *this);

    // Sync button text with current parameter state
    updateText();
}

//==============================================================================
// Text update
//==============================================================================

void AetherBypassButton::updateText()
{
    // Bypass parameter: true = bypassed, false = active
    // ButtonAttachment: getToggleState() reflects the parameter value
    // Toggle ON (true) = bypassed = "OUT"
    // Toggle OFF (false) = active = "IN"
    if (getToggleState())
        setButtonText ("OUT");
    else
        setButtonText ("IN");
}
