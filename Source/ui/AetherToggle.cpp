#include "AetherToggle.h"
#include "AetherColours.h"
#include "AetherLookAndFeel.h"

//==============================================================================
AetherToggle::AetherToggle (const juce::StringArray& options)
    : optionLabels (options)
{
    // Create one button per option
    for (int i = 0; i < options.size(); ++i)
    {
        auto* btn = buttons.add (new juce::TextButton (options[i]));
        btn->setClickingTogglesState (false);
        btn->getProperties().set ("isToggleOption", true);

        // Each button click sets the hidden ComboBox index
        btn->onClick = [this, i]
        {
            hiddenCombo.setSelectedItemIndex (i, juce::sendNotificationSync);
        };

        addAndMakeVisible (btn);
    }

    // Hidden ComboBox for APVTS attachment bridge
    addChildComponent (hiddenCombo);  // Not visible
    hiddenCombo.onChange = [this] { updateButtonStates(); };
}

AetherToggle::~AetherToggle() = default;

//==============================================================================
// Parameter attachment
//==============================================================================

void AetherToggle::attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                                       const juce::String& paramID)
{
    // Populate the hidden ComboBox with option strings
    hiddenCombo.clear (juce::dontSendNotification);
    hiddenCombo.addItemList (optionLabels, 1);  // IDs start at 1

    // Create the attachment (syncs selected index with parameter)
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, paramID, hiddenCombo);

    // Sync initial button states
    updateButtonStates();
}

//==============================================================================
// Visual state
//==============================================================================

void AetherToggle::setBypassed (bool shouldBeBypassed)
{
    if (bypassed != shouldBeBypassed)
    {
        bypassed = shouldBeBypassed;
        setAlpha (bypassed ? 0.35f : 1.0f);
    }
}

//==============================================================================
// Component overrides
//==============================================================================

void AetherToggle::paint (juce::Graphics& /*g*/)
{
    // No outer border -- individual buttons draw their own separated borders
}

void AetherToggle::paintOverChildren (juce::Graphics& /*g*/)
{
    // Bypassed state handled by setAlpha() -- no overlay needed
}

void AetherToggle::resized()
{
    auto bounds = getLocalBounds();
    int numButtons = buttons.size();
    if (numButtons == 0)
        return;

    constexpr int gap = 3;  // px between buttons for visual separation
    int totalGaps = (numButtons - 1) * gap;
    int buttonWidth = (bounds.getWidth() - totalGaps) / numButtons;

    for (int i = 0; i < numButtons; ++i)
    {
        int x = i * (buttonWidth + gap);
        int w = (i == numButtons - 1) ? (bounds.getWidth() - x) : buttonWidth;
        buttons[i]->setBounds (x, 0, w, bounds.getHeight());
    }
}

//==============================================================================
// Button state sync
//==============================================================================

void AetherToggle::updateButtonStates()
{
    int selectedIndex = hiddenCombo.getSelectedItemIndex();

    for (int i = 0; i < buttons.size(); ++i)
    {
        buttons[i]->setToggleState (i == selectedIndex, juce::dontSendNotification);
    }

    repaint();
}
