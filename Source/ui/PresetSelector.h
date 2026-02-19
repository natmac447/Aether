#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class MixLockButton;

/**
 * PresetSelector - Styled dropdown for preset selection in the Aether header.
 *
 * Populated with 7 items (Default + 6 factory presets) plus any user presets
 * saved to ~/Library/Audio/Presets/Cairn/Aether/. Selecting a preset applies
 * all DSP parameter values via APVTS. Selecting Default resets all parameters
 * to their APVTS default values.
 *
 * Style: EB Garamond 14px, Ink Light text, 1px Ink Ghost border, transparent background.
 * Size: ~220x24px (including Save button).
 */
class PresetSelector : public juce::Component
{
public:
    explicit PresetSelector (juce::AudioProcessorValueTreeState& apvts);
    ~PresetSelector() override = default;

    //==========================================================================
    // Component overrides
    void resized() override;

    //==========================================================================
    // Public accessors

    /** Expose the internal combo box for ArrowStepButton targeting. */
    juce::ComboBox& getComboBox() { return comboBox; }

    /** Set a pointer to the MixLockButton so preset loading can skip out_mix. */
    void setMixLockButton (MixLockButton* button) { mixLockBtn = button; }

    /** Rebuild the combo box item list (factory + user presets). Call after saving/deleting. */
    void rebuildPresetList();

private:
    void applyPreset (int presetIndex);

    juce::AudioProcessorValueTreeState& apvts;
    juce::ComboBox comboBox;
    juce::TextButton saveButton { "Save" };

    MixLockButton* mixLockBtn = nullptr;

    juce::Array<juce::File> userPresetFiles;   // Cached list of user preset files
    static constexpr int kUserPresetIdOffset = 100;  // User preset IDs start at 100

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetSelector)
};
