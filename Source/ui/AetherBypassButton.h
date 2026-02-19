#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * AetherBypassButton - In/Out toggle button for section bypass in the Aether plugin.
 *
 * Synced with an AudioParameterBool via ButtonAttachment.
 * Bypass param semantics: false = active ("IN"), true = bypassed ("OUT").
 *
 * Visual states (handled by AetherLookAndFeel via "isBypass" property):
 * - Active ("IN"): Ink Light text, 1px Ink Light border, full opacity
 * - Bypassed ("OUT"): Ink Ghost text, 1px Ink Ghost border, 60% opacity
 * - Hover: full opacity regardless of state
 *
 * Size: ~28x16px, right-aligned in section header (positioned by parent).
 * Font: Spectral 7px uppercase, 2px tracking.
 */
class AetherBypassButton : public juce::TextButton
{
public:
    AetherBypassButton();
    ~AetherBypassButton() override = default;

    //==========================================================================
    // Parameter attachment

    /** Connect this bypass button to an AudioParameterBool via APVTS.
     *  Creates ButtonAttachment and syncs initial state. */
    void attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& paramID);

private:
    /** Update the button text based on current toggle state. */
    void updateText();

    //==========================================================================
    // APVTS attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherBypassButton)
};
