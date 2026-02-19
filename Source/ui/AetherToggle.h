#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * AetherToggle - Multi-option toggle switch for the Aether plugin.
 *
 * Displays 2 or 3 mutually exclusive options in a bordered row, synced with
 * an AudioParameterChoice via a hidden ComboBox + ComboBoxAttachment.
 *
 * Active option: Ink background, Parchment text (ink-inverted)
 * Inactive options: Transparent background, Ink Faint text
 * Outer border: 1px Ink Ghost rounded rect around the entire toggle row
 */
class AetherToggle : public juce::Component
{
public:
    /** @param options  The option labels (e.g., {"Warm", "Neutral", "Cold"}) */
    explicit AetherToggle (const juce::StringArray& options);

    ~AetherToggle() override;

    //==========================================================================
    // Parameter attachment

    /** Connect this toggle to an AudioParameterChoice via APVTS.
     *  Creates ComboBoxAttachment and syncs initial state. */
    void attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& paramID);

    //==========================================================================
    // Visual state

    /** Set bypassed state. Draws translucent parchment overlay but controls
     *  remain interactive (LOCKED DECISION). */
    void setBypassed (bool shouldBeBypassed);
    bool isBypassed() const noexcept { return bypassed; }

    //==========================================================================
    // Component overrides
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;

private:
    /** Refresh button toggle states from the hidden ComboBox's selected index. */
    void updateButtonStates();

    //==========================================================================
    juce::OwnedArray<juce::TextButton> buttons;
    juce::ComboBox hiddenCombo;

    juce::StringArray optionLabels;
    bool bypassed = false;

    // APVTS attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherToggle)
};
