#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * AetherKnob - Custom rotary knob control for the Aether plugin.
 *
 * Wraps a juce::Slider (RotaryHorizontalVerticalDrag) with:
 * - ~300px drag sensitivity (velocity mode, shift-drag 10x slower)
 * - Scroll wheel at ~1% increments
 * - Double-click resets to parameter default value
 * - Click-to-type value editing via editable value label
 * - Smooth animated indicator sweep on preset load / double-click reset (~150ms)
 * - Real-time value readout during drag
 * - Bypassed visual dimming (40% parchment overlay, controls remain interactive)
 *
 * Rendering delegates to AetherLookAndFeel::drawRotarySlider with displayValue-derived angle.
 */
class AetherKnob : public juce::Component,
                    public juce::Slider::Listener,
                    public juce::Label::Listener,
                    private juce::Timer
{
public:
    /**
     * @param name     Knob label text (e.g., "BODY", "AIR") -- displayed below the knob
     * @param knobSize Diameter in pixels: 56 (standard) or 64 (center room controls)
     * @param valueFormatter Optional custom value display function
     */
    AetherKnob (const juce::String& name, int knobSize = 56,
                std::function<juce::String (float)> valueFormatter = nullptr);

    ~AetherKnob() override;

    //==========================================================================
    // Parameter attachment

    /** Connect this knob to a parameter via APVTS. Creates SliderAttachment
     *  and reads the parameter's default for double-click reset. */
    void attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& paramID);

    //==========================================================================
    // Value formatting

    /** Set a custom value formatter that receives the normalised value (0-1)
     *  and the parameter's text representation. */
    void setValueFormatter (std::function<juce::String (float normalised,
                                                         const juce::String& paramText)> formatter);

    //==========================================================================
    // Visual state

    /** Set bypassed state. Draws translucent parchment overlay but controls
     *  remain interactive (LOCKED DECISION). */
    void setBypassed (bool shouldBeBypassed);
    bool isBypassed() const noexcept { return bypassed; }

    //==========================================================================
    // Component overrides
    void paint (juce::Graphics& g) override;
    void resized() override;

    //==========================================================================
    // Slider::Listener
    void sliderValueChanged (juce::Slider* slider) override;
    void sliderDragStarted (juce::Slider* slider) override;
    void sliderDragEnded (juce::Slider* slider) override;

    //==========================================================================
    // Label::Listener (click-to-type)
    void labelTextChanged (juce::Label* labelThatHasChanged) override;

    /** Direct access to the slider for advanced configuration. */
    juce::Slider& getSlider() noexcept { return slider; }

private:
    //==========================================================================
    // Timer (animation)
    void timerCallback() override;

    /** Update the value label text from the current slider value. */
    void updateValueLabel();

    /** Draw the knob name with letter spacing (char-by-char with GlyphArrangement). */
    void drawSpacedLabel (juce::Graphics& g, const juce::String& text,
                          const juce::Rectangle<float>& area, const juce::Font& font,
                          float letterSpacing) const;

    //==========================================================================
    juce::Slider slider;
    juce::Label  valueLabel;

    juce::String knobName;
    int          knobDiameter;

    // Animation state
    float displayValue  = 0.5f;  // What is visually drawn (0-1 normalised)
    float targetValue   = 0.5f;  // Actual parameter value (0-1 normalised)
    bool  isDragging    = false;

    // Bypass state
    bool bypassed = false;

    // Formatters
    std::function<juce::String (float)>                                    simpleFormatter;
    std::function<juce::String (float normalised, const juce::String&)>    richFormatter;

    // APVTS attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    // Cached parameter range for click-to-type parsing
    juce::NormalisableRange<float> paramRange { 0.0f, 1.0f };
    juce::String attachedParamID;
    juce::AudioProcessorValueTreeState* attachedAPVTS = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherKnob)
};
