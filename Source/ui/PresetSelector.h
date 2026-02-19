#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * PresetSelector - Styled dropdown for preset selection in the Aether header.
 *
 * Currently a placeholder -- populated with 7 items (Default + 6 preset names).
 * Changing preset does nothing; the actual preset system is Phase 8.
 *
 * Style: EB Garamond 14px, Ink Light text, 1px Ink Ghost border, transparent background.
 * Size: ~180x24px.
 */
class PresetSelector : public juce::Component
{
public:
    PresetSelector();
    ~PresetSelector() override = default;

    //==========================================================================
    // Component overrides
    void resized() override;

private:
    juce::ComboBox comboBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetSelector)
};
