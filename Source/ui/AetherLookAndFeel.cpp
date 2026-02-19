#include "AetherLookAndFeel.h"
#include "AetherColours.h"
#include "BinaryData.h"

//==============================================================================
AetherLookAndFeel::AetherLookAndFeel()
{
    // =========================================================================
    // Load embedded fonts from binary data
    // =========================================================================
    displayTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::CormorantGaramondRegular_ttf,
        BinaryData::CormorantGaramondRegular_ttfSize);

    displayLightTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::CormorantGaramondLight_ttf,
        BinaryData::CormorantGaramondLight_ttfSize);

    displayItalicTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::CormorantGaramondItalic_ttf,
        BinaryData::CormorantGaramondItalic_ttfSize);

    bodyTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::EBGaramondRegular_ttf,
        BinaryData::EBGaramondRegular_ttfSize);

    bodyItalicTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::EBGaramondItalic_ttf,
        BinaryData::EBGaramondItalic_ttfSize);

    labelTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::SpectralRegular_ttf,
        BinaryData::SpectralRegular_ttfSize);

    labelItalicTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::SpectralItalic_ttf,
        BinaryData::SpectralItalic_ttfSize);

    // =========================================================================
    // Set JUCE colour IDs to match Aether parchment/ink palette
    // =========================================================================

    // Labels
    setColour (juce::Label::textColourId,       juce::Colour (AetherColours::ink));
    setColour (juce::Label::backgroundColourId,  juce::Colour (0x00000000));
    setColour (juce::Label::outlineColourId,     juce::Colour (0x00000000));

    // Sliders
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (AetherColours::inkLight));
    setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (AetherColours::ink));
    setColour (juce::Slider::thumbColourId,               juce::Colour (AetherColours::ink));
    setColour (juce::Slider::trackColourId,               juce::Colour (AetherColours::parchmentDark));
    setColour (juce::Slider::backgroundColourId,          juce::Colour (AetherColours::parchment));

    // Text buttons
    setColour (juce::TextButton::buttonColourId,    juce::Colour (0x00000000));
    setColour (juce::TextButton::buttonOnColourId,  juce::Colour (AetherColours::ink));
    setColour (juce::TextButton::textColourOffId,   juce::Colour (AetherColours::inkFaint));
    setColour (juce::TextButton::textColourOnId,    juce::Colour (AetherColours::parchment));

    // ComboBox
    setColour (juce::ComboBox::backgroundColourId,   juce::Colour (0x00000000));
    setColour (juce::ComboBox::textColourId,         juce::Colour (AetherColours::inkLight));
    setColour (juce::ComboBox::outlineColourId,      juce::Colour (AetherColours::inkGhost));
    setColour (juce::ComboBox::arrowColourId,        juce::Colour (AetherColours::inkFaint));

    // Popup menu
    setColour (juce::PopupMenu::backgroundColourId,            juce::Colour (AetherColours::parchment));
    setColour (juce::PopupMenu::textColourId,                  juce::Colour (AetherColours::ink));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (AetherColours::accentWarm));
    setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colour (AetherColours::parchment));

    // Window background
    setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (AetherColours::parchment));
}

//==============================================================================
// Font helpers
//==============================================================================

juce::Font AetherLookAndFeel::getDisplayFont (float size) const
{
    if (displayTypeface != nullptr)
        return juce::Font (juce::FontOptions (displayTypeface).withHeight (size));
    return juce::Font (juce::FontOptions ("Georgia", size, juce::Font::plain));
}

juce::Font AetherLookAndFeel::getDisplayFontLight (float size) const
{
    if (displayLightTypeface != nullptr)
        return juce::Font (juce::FontOptions (displayLightTypeface).withHeight (size));
    return juce::Font (juce::FontOptions ("Georgia", size, juce::Font::plain));
}

juce::Font AetherLookAndFeel::getDisplayFontItalic (float size) const
{
    if (displayItalicTypeface != nullptr)
        return juce::Font (juce::FontOptions (displayItalicTypeface).withHeight (size));
    return juce::Font (juce::FontOptions ("Georgia", size, juce::Font::italic));
}

juce::Font AetherLookAndFeel::getBodyFont (float size) const
{
    if (bodyTypeface != nullptr)
        return juce::Font (juce::FontOptions (bodyTypeface).withHeight (size));
    return juce::Font (juce::FontOptions ("Georgia", size, juce::Font::plain));
}

juce::Font AetherLookAndFeel::getBodyFontItalic (float size) const
{
    if (bodyItalicTypeface != nullptr)
        return juce::Font (juce::FontOptions (bodyItalicTypeface).withHeight (size));
    return juce::Font (juce::FontOptions ("Georgia", size, juce::Font::italic));
}

juce::Font AetherLookAndFeel::getSpectralFont (float size) const
{
    if (labelTypeface != nullptr)
        return juce::Font (juce::FontOptions (labelTypeface).withHeight (size));
    return juce::Font (juce::FontOptions ("Georgia", size, juce::Font::plain));
}

juce::Font AetherLookAndFeel::getSpectralFontItalic (float size) const
{
    if (labelItalicTypeface != nullptr)
        return juce::Font (juce::FontOptions (labelItalicTypeface).withHeight (size));
    return juce::Font (juce::FontOptions ("Georgia", size, juce::Font::italic));
}

//==============================================================================
// Rotary slider: parchment gradient body with ink indicator line
//==============================================================================

void AetherLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y,
                                           int width, int height,
                                           float sliderPos,
                                           float startAngle, float endAngle,
                                           juce::Slider& /*slider*/)
{
    auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                           static_cast<float> (width), static_cast<float> (height));
    auto centre = bounds.getCentre();
    float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

    // ---- Drop shadow (subtle, offset 1px down) ----
    {
        auto shadowColour = juce::Colour (AetherColours::shadow);
        g.setColour (shadowColour);
        g.fillEllipse (centre.x - radius + 0.5f, centre.y - radius + 1.0f,
                        radius * 2.0f, radius * 2.0f);
    }

    // ---- Body: radial gradient (Parchment Light highlight at top-left,
    //      Parchment Dark shadow at bottom-right) ----
    {
        juce::ColourGradient bodyGrad (
            juce::Colour (AetherColours::parchmentLight),
            centre.x - radius * 0.4f, centre.y - radius * 0.4f,
            juce::Colour (AetherColours::parchmentDark),
            centre.x + radius * 0.4f, centre.y + radius * 0.4f,
            true);  // radial
        g.setGradientFill (bodyGrad);
        g.fillEllipse (centre.x - radius, centre.y - radius,
                        radius * 2.0f, radius * 2.0f);
    }

    // ---- Inner highlight: subtle white glow at top ----
    {
        g.setColour (juce::Colours::white.withAlpha (0.3f));
        float highlightR = radius - 1.0f;
        g.drawEllipse (centre.x - highlightR, centre.y - highlightR - 0.5f,
                        highlightR * 2.0f, highlightR * 2.0f, 0.5f);
    }

    // ---- Border: 1px Ink Light ellipse stroke ----
    {
        g.setColour (juce::Colour (AetherColours::inkLight));
        g.drawEllipse (centre.x - radius, centre.y - radius,
                        radius * 2.0f, radius * 2.0f, 1.0f);
    }

    // ---- Indicator line: 1.5px wide, from inner to outer radius ----
    {
        float currentAngle = startAngle + sliderPos * (endAngle - startAngle);

        // Indicator spans from (radius - 16px) to (radius - 2px) from center
        float indicatorLen = juce::jmin (16.0f, radius * 0.55f);
        float outerR = radius - 2.0f;
        float innerR = outerR - indicatorLen;

        float lineX1 = centre.x + innerR * std::sin (currentAngle);
        float lineY1 = centre.y - innerR * std::cos (currentAngle);
        float lineX2 = centre.x + outerR * std::sin (currentAngle);
        float lineY2 = centre.y - outerR * std::cos (currentAngle);

        g.setColour (juce::Colour (AetherColours::ink));
        g.drawLine (lineX1, lineY1, lineX2, lineY2, 1.0f);
    }
}

//==============================================================================
// Button background
//==============================================================================

void AetherLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& /*backgroundColour*/,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    bool isOn = button.getToggleState();
    float cornerR = 2.0f;

    // Check if this is a bypass button via component property
    bool isBypass = button.getProperties().getWithDefault ("isBypass", false);

    if (isBypass)
    {
        // Bypass button: power symbol (circle + top line)
        auto colour = isOn ? juce::Colour (AetherColours::inkLight)
                           : juce::Colour (AetherColours::inkGhost).withAlpha (0.6f);
        g.setColour (colour);

        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.32f;

        // Semi-circle with opening at top: draw arc at origin then rotate 180°
        juce::Path arc;
        const float gap = juce::MathConstants<float>::pi * 0.25f;
        // Original arc has gap at 9 o'clock (left)
        arc.addArc (-radius, -radius, radius * 2.0f, radius * 2.0f,
                    -juce::MathConstants<float>::pi + gap,
                     juce::MathConstants<float>::pi - gap, true);
        // Rotate 180° to move gap from left (9 o'clock) to top (12 o'clock)
        arc.applyTransform (juce::AffineTransform::rotation (
            -juce::MathConstants<float>::pi));
        // Translate to button centre
        arc.applyTransform (juce::AffineTransform::translation (cx, cy));
        g.strokePath (arc, juce::PathStrokeType (0.9f));

        // Vertical line through gap at top of circle
        float lineLen = radius * 0.75f;
        g.drawLine (cx, cy - radius - 0.5f, cx, cy - radius + lineLen, 0.9f);

        // Hover highlight
        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour (juce::Colour (AetherColours::inkLight).withAlpha (0.15f));
            g.fillRoundedRectangle (bounds, cornerR);
        }
    }
    else if (button.getClickingTogglesState()
             || bool (button.getProperties().getWithDefault ("isToggleOption", false)))
    {
        // Toggle button (e.g., air character, gate mode)
        if (isOn || shouldDrawButtonAsDown)
        {
            // Active: warm accent fill (subtler than full ink)
            g.setColour (juce::Colour (AetherColours::accentWarm));
            g.fillRoundedRectangle (bounds, cornerR);
        }
        else
        {
            // Inactive: transparent with 1px Ink Ghost border
            g.setColour (juce::Colour (AetherColours::inkGhost));
            g.drawRoundedRectangle (bounds, cornerR, 1.0f);

            if (shouldDrawButtonAsHighlighted)
            {
                g.setColour (juce::Colour (AetherColours::accentCopper).withAlpha (0.1f));
                g.fillRoundedRectangle (bounds, cornerR);
            }
        }
    }
    else
    {
        // Standard button: parchment background with Ink Ghost border
        g.setColour (juce::Colour (AetherColours::parchment));
        g.fillRoundedRectangle (bounds, cornerR);

        g.setColour (juce::Colour (AetherColours::inkGhost));
        g.drawRoundedRectangle (bounds, cornerR, 1.0f);

        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour (juce::Colour (AetherColours::accentCopper).withAlpha (0.08f));
            g.fillRoundedRectangle (bounds, cornerR);
        }

        if (shouldDrawButtonAsDown)
        {
            g.setColour (juce::Colour (AetherColours::ink).withAlpha (0.05f));
            g.fillRoundedRectangle (bounds, cornerR);
        }
    }
}

//==============================================================================
// Button text
//==============================================================================

void AetherLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                         bool /*shouldDrawButtonAsHighlighted*/,
                                         bool /*shouldDrawButtonAsDown*/)
{
    bool isOn = button.getToggleState();
    bool isBypass = button.getProperties().getWithDefault ("isBypass", false);

    juce::Colour textColour;
    juce::Font font;

    if (isBypass)
    {
        // Bypass buttons use symbol drawn in drawButtonBackground -- no text
        return;
    }
    else if (button.getClickingTogglesState()
             || bool (button.getProperties().getWithDefault ("isToggleOption", false)))
    {
        // Toggle buttons
        font = getSpectralFont (12.0f);
        textColour = isOn ? juce::Colour (AetherColours::parchmentLight)   // Light on warm accent
                          : juce::Colour (AetherColours::inkFaint);
    }
    else
    {
        // Standard buttons
        font = getSpectralFont (12.0f);
        textColour = juce::Colour (AetherColours::ink);
    }

    g.setFont (font);
    g.setColour (textColour.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    auto textBounds = button.getLocalBounds();
    g.drawFittedText (button.getButtonText(), textBounds,
                      juce::Justification::centred, 1);
}

//==============================================================================
// Label rendering
//==============================================================================

void AetherLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.fillAll (label.findColour (juce::Label::backgroundColourId));

    if (! label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        auto font = getLabelFont (label);

        g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);

        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          juce::jmax (1, static_cast<int> (static_cast<float> (textArea.getHeight()) / font.getHeight())),
                          label.getMinimumHorizontalScale());
    }
    else if (label.isEnabled())
    {
        g.setColour (label.findColour (juce::Label::outlineColourId));
    }
}

//==============================================================================
// Popup menu background: parchment with ink ghost border
//==============================================================================

void AetherLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (juce::Colour (AetherColours::parchment));

    // 1px Ink Ghost border
    g.setColour (juce::Colour (AetherColours::inkGhost));
    g.drawRect (0, 0, width, height, 1);
}

//==============================================================================
// Popup menu item: parchment theme with accent warm highlight
//==============================================================================

void AetherLookAndFeel::drawPopupMenuItem (juce::Graphics& g,
                                            const juce::Rectangle<int>& area,
                                            bool isSeparator, bool isActive,
                                            bool isHighlighted, bool isTicked,
                                            bool hasSubMenu,
                                            const juce::String& text,
                                            const juce::String& shortcutKeyText,
                                            const juce::Drawable* icon,
                                            const juce::Colour* textColour)
{
    juce::ignoreUnused (icon, shortcutKeyText, textColour);

    if (isSeparator)
    {
        auto sepArea = area.reduced (5, 0);
        g.setColour (juce::Colour (AetherColours::inkGhost));
        g.fillRect (sepArea.getX(), sepArea.getCentreY(), sepArea.getWidth(), 1);
        return;
    }

    // Reserve left margin for tick mark
    constexpr int tickMargin = 22;
    auto textArea = area.withTrimmedLeft (tickMargin).withTrimmedRight (8);

    g.setFont (getBodyFont (14.0f));

    if (isHighlighted && isActive)
    {
        // Highlighted: Accent Warm background, Parchment text
        g.setColour (juce::Colour (AetherColours::accentWarm));
        g.fillRect (area);
        g.setColour (juce::Colour (AetherColours::parchment));
    }
    else
    {
        // Normal: Parchment background, Ink text
        g.setColour (isActive ? juce::Colour (AetherColours::ink)
                              : juce::Colour (AetherColours::inkFaint));
    }

    // Tick mark in the left margin
    if (isTicked)
    {
        auto tickX = static_cast<float> (area.getX() + 6);
        auto tickCY = static_cast<float> (area.getCentreY());
        juce::Path tick;
        tick.startNewSubPath (tickX, tickCY);
        tick.lineTo (tickX + 4.0f, tickCY + 4.0f);
        tick.lineTo (tickX + 10.0f, tickCY - 4.0f);
        g.strokePath (tick, juce::PathStrokeType (1.5f));
    }

    g.drawFittedText (text, textArea, juce::Justification::centredLeft, 1);

    if (hasSubMenu)
    {
        auto arrowH = 0.5f * static_cast<float> (area.getHeight());
        auto arrowX = static_cast<float> (area.getRight()) - 10.0f;
        auto arrowMid = static_cast<float> (area.getCentreY());

        juce::Path arrow;
        arrow.addTriangle (arrowX, arrowMid - arrowH * 0.3f,
                           arrowX, arrowMid + arrowH * 0.3f,
                           arrowX + 4.0f, arrowMid);
        g.fillPath (arrow);
    }
}

//==============================================================================
// Font overrides for standard JUCE components
//==============================================================================

juce::Font AetherLookAndFeel::getTextButtonFont (juce::TextButton& /*button*/, int buttonHeight)
{
    return getSpectralFont (juce::jmin (14.0f, static_cast<float> (buttonHeight) * 0.6f));
}

juce::Font AetherLookAndFeel::getLabelFont (juce::Label& label)
{
    return getBodyFont (juce::jmin (15.0f, static_cast<float> (label.getHeight()) * 0.8f));
}

juce::Font AetherLookAndFeel::getPopupMenuFont()
{
    return getBodyFont (14.0f);
}

//==============================================================================
// ComboBox: centre text across full width (ignoring arrow button)
//==============================================================================

void AetherLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    // Place the label across the full width so centred text ignores the arrow
    label.setBounds (0, 0, box.getWidth(), box.getHeight());
    label.setJustificationType (juce::Justification::centred);
    label.setFont (getComboBoxFont (box));
}

juce::Font AetherLookAndFeel::getComboBoxFont (juce::ComboBox& /*box*/)
{
    return getBodyFont (14.0f);
}

//==============================================================================
// Constrain popup menus to parent editor bounds
//==============================================================================

juce::Component* AetherLookAndFeel::getParentComponentForMenuOptions (
    const juce::PopupMenu::Options& options)
{
    // Walk up the component tree to find the top-level parent (the editor)
    // so popup menus are constrained within the plugin window bounds
    if (auto* target = options.getTargetComponent())
    {
        auto* comp = target;
        while (comp->getParentComponent() != nullptr)
            comp = comp->getParentComponent();
        return comp;
    }
    return nullptr;
}
