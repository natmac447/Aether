#include "AetherKnob.h"
#include "AetherColours.h"
#include "AetherLookAndFeel.h"

//==============================================================================
AetherKnob::AetherKnob (const juce::String& name, int knobSize,
                          std::function<juce::String (float)> valueFormatter)
    : knobName (name),
      knobDiameter (knobSize),
      simpleFormatter (std::move (valueFormatter))
{
    // =========================================================================
    // Slider configuration
    // =========================================================================
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

    // 270 degree arc: 7 o'clock to 5 o'clock
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                juce::MathConstants<float>::pi * 2.75f,
                                true);
    slider.setScrollWheelEnabled (true);
    slider.setDoubleClickReturnValue (true, 0.5f);  // Updated per-param by attachToParameter

    // ---- Drag sensitivity (LOCKED DECISION: ~300px for full sweep) ----
    // Velocity mode with sensitivity 0.5 = user needs ~double default pixels (~300px)
    // Shift key activates fine mode (10x slower) via JUCE's built-in handling
    slider.setVelocityBasedMode (true);
    slider.setVelocityModeParameters (0.5, 1, 0.0, true,
                                       juce::ModifierKeys::shiftModifier);
    slider.setSliderSnapsToMousePosition (false);

    // Make the slider invisible -- all rendering happens in paint()
    slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    slider.setAlpha (0.0f);  // Completely invisible; paint() draws the knob

    slider.addListener (this);
    addAndMakeVisible (slider);

    // =========================================================================
    // Value display label
    // =========================================================================
    // Style: EB Garamond 11px italic, Ink Faint, centred
    valueLabel.setFont (juce::Font (juce::FontOptions ("Georgia", 11.0f, juce::Font::italic)));
    valueLabel.setColour (juce::Label::textColourId, juce::Colour (AetherColours::inkFaint));
    valueLabel.setJustificationType (juce::Justification::centred);

    // Click-to-type: single-click triggers edit
    valueLabel.setEditable (false, true, false);
    valueLabel.addListener (this);
    addAndMakeVisible (valueLabel);
}

AetherKnob::~AetherKnob()
{
    slider.removeListener (this);
    valueLabel.removeListener (this);
    stopTimer();
}

//==============================================================================
// Parameter attachment
//==============================================================================

void AetherKnob::attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                                     const juce::String& paramID)
{
    attachedAPVTS = &apvts;
    attachedParamID = paramID;

    // Create the attachment (syncs slider range and value)
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, paramID, slider);

    // Read parameter's default value for double-click reset
    if (auto* param = apvts.getParameter (paramID))
    {
        float defaultNormalised = param->getDefaultValue();
        float defaultDenorm = param->convertFrom0to1 (defaultNormalised);
        slider.setDoubleClickReturnValue (true, defaultDenorm);

        // Cache the parameter range for click-to-type
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*> (param))
            paramRange = rangedParam->getNormalisableRange();
    }

    // Update the value label font from the LookAndFeel if available
    if (auto* laf = dynamic_cast<AetherLookAndFeel*> (&getLookAndFeel()))
        valueLabel.setFont (laf->getBodyFontItalic (11.0f));

    // Initialise display state
    float norm = static_cast<float> (slider.proportionOfLengthToValue (slider.getValue()));
    // Simpler: use the normalised position
    if (auto* param = apvts.getParameter (paramID))
    {
        float currentNorm = param->getValue();
        displayValue = currentNorm;
        targetValue  = currentNorm;
    }

    updateValueLabel();
    repaint();
}

//==============================================================================
// Value formatting
//==============================================================================

void AetherKnob::setValueFormatter (
    std::function<juce::String (float normalised, const juce::String& paramText)> formatter)
{
    richFormatter = std::move (formatter);
    updateValueLabel();
}

//==============================================================================
// Bypassed state
//==============================================================================

void AetherKnob::setBypassed (bool shouldBeBypassed)
{
    if (bypassed != shouldBeBypassed)
    {
        bypassed = shouldBeBypassed;
        repaint();
    }
}

//==============================================================================
// Component overrides
//==============================================================================

void AetherKnob::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    int componentWidth = bounds.getWidth();

    // ---- Draw the knob using LookAndFeel's drawRotarySlider with displayValue ----
    {
        // Knob area: centred at top, knobDiameter x knobDiameter
        int knobX = (componentWidth - knobDiameter) / 2;
        int knobY = 0;

        float startAngle = juce::MathConstants<float>::pi * 1.25f;
        float endAngle   = juce::MathConstants<float>::pi * 2.75f;

        // Use displayValue (animated) instead of slider's actual position
        getLookAndFeel().drawRotarySlider (g, knobX, knobY,
                                           knobDiameter, knobDiameter,
                                           displayValue,
                                           startAngle, endAngle,
                                           slider);
    }

    // ---- Draw the knob name label with letter spacing ----
    {
        int nameY = knobDiameter + 6;
        int nameHeight = 12;
        auto nameArea = juce::Rectangle<float> (0.0f, static_cast<float> (nameY),
                                                  static_cast<float> (componentWidth),
                                                  static_cast<float> (nameHeight));

        juce::Font nameFont (juce::FontOptions ("Georgia", 9.0f, juce::Font::plain));
        if (auto* laf = dynamic_cast<AetherLookAndFeel*> (&getLookAndFeel()))
            nameFont = laf->getSpectralFont (9.0f);

        g.setColour (juce::Colour (AetherColours::inkLight));
        drawSpacedLabel (g, knobName.toUpperCase(), nameArea, nameFont, 2.0f);
    }

    // ---- Bypassed overlay (LOCKED DECISION: 40% parchment overlay, controls interactive) ----
    if (bypassed)
    {
        g.setColour (juce::Colour (AetherColours::parchment).withAlpha (0.4f));
        int knobX = (componentWidth - knobDiameter) / 2;
        g.fillEllipse (static_cast<float> (knobX), 0.0f,
                        static_cast<float> (knobDiameter),
                        static_cast<float> (knobDiameter));
    }
}

void AetherKnob::resized()
{
    auto bounds = getLocalBounds();
    int componentWidth = bounds.getWidth();

    // Slider occupies the knob area (invisible but captures mouse events)
    int knobX = (componentWidth - knobDiameter) / 2;
    slider.setBounds (knobX, 0, knobDiameter, knobDiameter);

    // Value label below the name label
    // Layout: knobDiameter + 6px gap + 12px name + 2px gap + 14px value
    int valueLabelY = knobDiameter + 6 + 12 + 2;
    valueLabel.setBounds (0, valueLabelY, componentWidth, 14);
}

//==============================================================================
// Slider::Listener
//==============================================================================

void AetherKnob::sliderValueChanged (juce::Slider* /*s*/)
{
    // Calculate normalised position (0-1)
    float newNorm = 0.5f;
    if (attachedAPVTS != nullptr && attachedParamID.isNotEmpty())
    {
        if (auto* param = attachedAPVTS->getParameter (attachedParamID))
            newNorm = param->getValue();
    }
    else
    {
        // Fallback: compute from slider range
        auto range = slider.getRange();
        if (range.getLength() > 0.0)
            newNorm = static_cast<float> ((slider.getValue() - range.getStart()) / range.getLength());
    }

    targetValue = newNorm;

    if (isDragging || std::abs (targetValue - displayValue) < 0.05f)
    {
        // During drag or small changes: snap immediately
        displayValue = targetValue;
        stopTimer();
        repaint();
    }
    else
    {
        // Large jump (preset load, double-click reset): animate at 60 Hz
        if (! isTimerRunning())
            startTimerHz (60);
    }

    updateValueLabel();
}

void AetherKnob::sliderDragStarted (juce::Slider* /*s*/)
{
    isDragging = true;
}

void AetherKnob::sliderDragEnded (juce::Slider* /*s*/)
{
    isDragging = false;
}

//==============================================================================
// Label::Listener (click-to-type)
//==============================================================================

void AetherKnob::labelTextChanged (juce::Label* label)
{
    if (label != &valueLabel)
        return;

    // Parse the user's input text, clamp to parameter range, set slider value
    juce::String inputText = label->getText();

    // Try to extract a numeric value -- strip common suffixes
    juce::String cleaned = inputText.trimCharactersAtEnd (" %msdBs")
                                     .trimCharactersAtStart (" ");
    float parsedValue = cleaned.getFloatValue();

    // Determine if user entered a percentage (0-100) or an actual value
    // For normalised 0-1 parameters, interpret numbers > 1 as percentage
    if (paramRange.start >= 0.0f && paramRange.end <= 1.0f && parsedValue > 1.0f)
        parsedValue /= 100.0f;

    // Clamp to parameter range
    parsedValue = juce::jlimit (paramRange.start, paramRange.end, parsedValue);

    slider.setValue (static_cast<double> (parsedValue), juce::sendNotificationSync);
    updateValueLabel();
}

//==============================================================================
// Timer (animation)
//==============================================================================

void AetherKnob::timerCallback()
{
    // Exponential smoothing: ~150ms sweep at 60Hz
    displayValue += 0.15f * (targetValue - displayValue);

    if (std::abs (targetValue - displayValue) < 0.001f)
    {
        displayValue = targetValue;
        stopTimer();
    }

    repaint();
}

//==============================================================================
// Value label update
//==============================================================================

void AetherKnob::updateValueLabel()
{
    juce::String text;

    if (richFormatter)
    {
        // Rich formatter: receives normalised value + parameter's text
        float normalised = targetValue;
        juce::String paramText;
        if (attachedAPVTS != nullptr && attachedParamID.isNotEmpty())
        {
            if (auto* param = attachedAPVTS->getParameter (attachedParamID))
                paramText = param->getCurrentValueAsText();
        }
        text = richFormatter (normalised, paramText);
    }
    else if (simpleFormatter)
    {
        text = simpleFormatter (static_cast<float> (slider.getValue()));
    }
    else
    {
        // Default: use parameter's string representation if attached
        if (attachedAPVTS != nullptr && attachedParamID.isNotEmpty())
        {
            if (auto* param = attachedAPVTS->getParameter (attachedParamID))
                text = param->getCurrentValueAsText();
        }
        else
        {
            text = juce::String (slider.getValue(), 1);
        }
    }

    valueLabel.setText (text, juce::dontSendNotification);
}

//==============================================================================
// Spaced label drawing (char-by-char with GlyphArrangement)
//==============================================================================

void AetherKnob::drawSpacedLabel (juce::Graphics& g, const juce::String& text,
                                    const juce::Rectangle<float>& area,
                                    const juce::Font& font, float letterSpacing) const
{
    if (text.isEmpty())
        return;

    g.setFont (font);

    // Use GlyphArrangement to measure character widths (non-deprecated API)
    auto measureChar = [&] (const juce::String& ch) -> float
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (font, ch, 0.0f, 0.0f);
        if (ga.getNumGlyphs() > 0)
            return ga.getBoundingBox (0, ga.getNumGlyphs(), false).getWidth();
        return 0.0f;
    };

    // Calculate total width with letter spacing
    float totalWidth = 0.0f;
    for (int i = 0; i < text.length(); ++i)
    {
        totalWidth += measureChar (text.substring (i, i + 1));
        if (i < text.length() - 1)
            totalWidth += letterSpacing;
    }

    // Centre horizontally within the area
    float startX = area.getX() + (area.getWidth() - totalWidth) * 0.5f;

    float xPos = startX;
    for (int i = 0; i < text.length(); ++i)
    {
        juce::String ch = text.substring (i, i + 1);
        float charWidth = measureChar (ch);
        g.drawText (ch, static_cast<int> (xPos), static_cast<int> (area.getY()),
                    static_cast<int> (charWidth + 1.0f),
                    static_cast<int> (area.getHeight()),
                    juce::Justification::centred, false);
        xPos += charWidth + letterSpacing;
    }
}
