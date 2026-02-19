#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * AetherLookAndFeel - Master LookAndFeel for the Aether plugin.
 *
 * Victorian parchment/ink aesthetic:
 * - Parchment gradient knobs with ink indicator lines (drawRotarySlider)
 * - Ink-inverted toggle buttons (drawButtonBackground/drawButtonText)
 * - Three embedded font families: Cormorant Garamond, EB Garamond, Spectral
 * - Warm earth tones on parchment background
 */
class AetherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AetherLookAndFeel();
    ~AetherLookAndFeel() override = default;

    //==========================================================================
    // Font access helpers -- each returns the embedded typeface or Georgia fallback
    //
    // Display = Cormorant Garamond (title, numerals)
    // Body    = EB Garamond (preset selector, knob values)
    // Spectral = Spectral (labels, section names, captions, bypass buttons)

    juce::Font getDisplayFont (float size) const;
    juce::Font getDisplayFontLight (float size) const;
    juce::Font getDisplayFontItalic (float size) const;

    juce::Font getBodyFont (float size) const;
    juce::Font getBodyFontItalic (float size) const;

    juce::Font getSpectralFont (float size) const;
    juce::Font getSpectralFontItalic (float size) const;

    //==========================================================================
    // Rotary slider: parchment gradient body with ink indicator line
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    //==========================================================================
    // Button: ink-inverted toggles, parchment standard buttons
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    //==========================================================================
    // Label: parchment-themed label rendering
    void drawLabel (juce::Graphics& g, juce::Label& label) override;

    //==========================================================================
    // Popup menu: parchment background with ink text
    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu,
                            const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColour) override;

    //==========================================================================
    // Font overrides for standard JUCE components
    juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override;
    juce::Font getLabelFont (juce::Label& label) override;
    juce::Font getPopupMenuFont() override;

    //==========================================================================
    // ComboBox: no default arrow (ArrowStepButtons handle navigation)
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;
    juce::Font getComboBoxFont (juce::ComboBox& box) override;

    //==========================================================================
    // Constrain popup menus to parent editor bounds
    juce::Component* getParentComponentForMenuOptions (const juce::PopupMenu::Options& options) override;

private:
    // Embedded font typefaces
    juce::Typeface::Ptr displayTypeface;        // Cormorant Garamond Regular
    juce::Typeface::Ptr displayLightTypeface;   // Cormorant Garamond Light
    juce::Typeface::Ptr displayItalicTypeface;  // Cormorant Garamond Italic
    juce::Typeface::Ptr bodyTypeface;           // EB Garamond Regular
    juce::Typeface::Ptr bodyItalicTypeface;     // EB Garamond Italic
    juce::Typeface::Ptr labelTypeface;          // Spectral Regular
    juce::Typeface::Ptr labelItalicTypeface;    // Spectral Italic

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherLookAndFeel)
};
