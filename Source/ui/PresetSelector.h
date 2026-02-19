#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * PresetSelector - Styled dropdown for preset selection in the Aether header.
 *
 * Populated with 7 items (Default + 6 factory presets). Selecting a preset
 * applies all DSP parameter values via APVTS. Selecting Default resets all
 * parameters to their APVTS default values.
 *
 * Style: EB Garamond 14px, Ink Light text, 1px Ink Ghost border, transparent background.
 * Size: ~180x24px.
 */
class PresetSelector : public juce::Component
{
public:
    explicit PresetSelector (juce::AudioProcessorValueTreeState& apvts);
    ~PresetSelector() override = default;

    //==========================================================================
    // Component overrides
    void resized() override;

private:
    void applyPreset (int presetIndex);

    juce::AudioProcessorValueTreeState& apvts;
    juce::ComboBox comboBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetSelector)
};
