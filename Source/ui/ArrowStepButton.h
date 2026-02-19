#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * ArrowStepButton - Small triangle button that steps a ComboBox up or down.
 *
 * Clicking Up decrements the selected ID (wrapping to last), clicking Down
 * increments (wrapping to first). Sends sendNotificationAsync so ComboBox
 * attachments pick up the change.
 *
 * Visual: Filled triangle in inkFaint, accentCopper on hover.
 * Size: 14x12px.
 */
class ArrowStepButton : public juce::Button
{
public:
    enum Direction { Up, Down };

    ArrowStepButton (Direction direction, juce::ComboBox& targetCombo);
    ~ArrowStepButton() override = default;

    //==========================================================================
    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

    void clicked() override;

private:
    Direction dir;
    juce::ComboBox& comboBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrowStepButton)
};
