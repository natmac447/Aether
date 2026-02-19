#include "ParchmentElements.h"
#include "AetherColours.h"

namespace ParchmentElements
{

//==============================================================================
// Background: parchment fill + ruled-paper texture + radial vignette
//==============================================================================
juce::Image generateBackground (int width, int height)
{
    juce::Image image (juce::Image::ARGB, width, height, true);
    juce::Graphics g (image);

    // 1. Base parchment fill
    g.setColour (juce::Colour (AetherColours::parchment));
    g.fillAll();

    // 2. Paper texture: faint horizontal ruled lines every 4px
    //    Sepia at 0.018 alpha (within locked 0.015-0.02 range)
    g.setColour (juce::Colour (AetherColours::sepia).withAlpha (0.018f));
    for (int y = 0; y < height; y += 4)
        g.fillRect (0, y, width, 1);

    // 3. Edge vignette: radial gradient from transparent centre to sepia at corners
    //    Darkening starts at 60% from centre, reaching ~30px visible inward effect
    const float cx = static_cast<float> (width)  / 2.0f;
    const float cy = static_cast<float> (height) / 2.0f;
    const float cornerDist = std::sqrt (cx * cx + cy * cy);

    juce::ColourGradient vignette (
        juce::Colours::transparentBlack, cx, cy,
        juce::Colour (AetherColours::sepia).withAlpha (0.08f), cx + cornerDist, cy,
        true);  // radial
    vignette.addColour (0.6, juce::Colours::transparentBlack);

    g.setGradientFill (vignette);
    g.fillRect (0, 0, width, height);

    return image;
}

//==============================================================================
// Diamond divider: horizontal lines with centered 5x5 rotated square
//==============================================================================
void drawDiamondDivider (juce::Graphics& g, float y, float leftX, float rightX)
{
    const float midX = (leftX + rightX) / 2.0f;

    g.setColour (juce::Colour (AetherColours::inkGhost));

    // Left line
    g.drawLine (leftX, y, midX - 8.0f, y, 1.0f);

    // Diamond: 5x5px rotated square, stroke only
    juce::Path diamond;
    diamond.addRectangle (-2.5f, -2.5f, 5.0f, 5.0f);
    diamond.applyTransform (
        juce::AffineTransform::rotation (juce::MathConstants<float>::pi / 4.0f)
            .translated (midX, y));
    g.strokePath (diamond, juce::PathStrokeType (1.0f));

    // Right line
    g.drawLine (midX + 8.0f, y, rightX, y, 1.0f);
}

//==============================================================================
// Corner L-brackets: 20x20px at all four corners, 6px from edges, 1px Ink Ghost
//==============================================================================
void drawCornerBrackets (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    const float w = static_cast<float> (bounds.getWidth());
    const float h = static_cast<float> (bounds.getHeight());
    const float bx = static_cast<float> (bounds.getX());
    const float by = static_cast<float> (bounds.getY());

    constexpr float inset  = 6.0f;
    constexpr float length = 14.0f;
    constexpr float thick  = 1.0f;

    g.setColour (juce::Colour (AetherColours::inkGhost));

    // Top-left
    g.drawLine (bx + inset, by + inset, bx + inset + length, by + inset, thick);
    g.drawLine (bx + inset, by + inset, bx + inset, by + inset + length, thick);

    // Top-right
    g.drawLine (bx + w - inset - length, by + inset, bx + w - inset, by + inset, thick);
    g.drawLine (bx + w - inset, by + inset, bx + w - inset, by + inset + length, thick);

    // Bottom-left
    g.drawLine (bx + inset, by + h - inset, bx + inset + length, by + h - inset, thick);
    g.drawLine (bx + inset, by + h - inset - length, bx + inset, by + h - inset, thick);

    // Bottom-right
    g.drawLine (bx + w - inset - length, by + h - inset, bx + w - inset, by + h - inset, thick);
    g.drawLine (bx + w - inset, by + h - inset - length, bx + w - inset, by + h - inset, thick);
}

//==============================================================================
// Double-rule border: two 1px lines with 3px gap, secondary at 40% opacity
//==============================================================================
void drawDoubleRule (juce::Graphics& g, float y, float leftX, float rightX, bool isTop)
{
    const auto inkGhost = juce::Colour (AetherColours::inkGhost);

    // Primary line: full Ink Ghost
    g.setColour (inkGhost);
    g.drawLine (leftX, y, rightX, y, 1.0f);

    // Secondary line: 40% opacity, offset by 3px
    const float secondaryY = isTop ? (y + 3.0f) : (y - 3.0f);
    g.setColour (inkGhost.withAlpha (0.4f));
    g.drawLine (leftX, secondaryY, rightX, secondaryY, 1.0f);
}

//==============================================================================
// Letter-spaced text: offset each glyph by glyphIndex * tracking
//==============================================================================
void drawLetterSpacedText (juce::Graphics& g, const juce::String& text,
                           float x, float y, float width, float tracking,
                           const juce::Font& font, juce::Colour colour,
                           juce::Justification justification)
{
    if (text.isEmpty())
        return;

    // Build glyph arrangement for the full string
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText (font, text, 0.0f, 0.0f);

    // Apply tracking: offset each glyph by its index * tracking
    for (int i = 0; i < glyphs.getNumGlyphs(); ++i)
    {
        auto glyph = glyphs.getGlyph (i);
        glyphs.moveRangeOfGlyphs (i, 1, static_cast<float> (i) * tracking, 0.0f);
    }

    // Calculate total width after tracking
    const float totalWidth = glyphs.getBoundingBox (0, glyphs.getNumGlyphs(), true).getWidth();

    // Calculate X offset based on justification
    float offsetX = x;
    if (justification.testFlags (juce::Justification::horizontallyCentred))
        offsetX = x + (width - totalWidth) / 2.0f;
    else if (justification.testFlags (juce::Justification::right))
        offsetX = x + width - totalWidth;

    // Move all glyphs to final position
    glyphs.moveRangeOfGlyphs (0, -1, offsetX, y);

    // Draw
    g.setColour (colour);
    glyphs.draw (g);
}

//==============================================================================
// Section label: "I. Cabinet Resonance" with Roman numeral + tracked name
//==============================================================================
void drawSectionLabel (juce::Graphics& g, const juce::String& numeral,
                       const juce::String& name, float x, float y, float width,
                       const juce::Font& numeralFont, const juce::Font& nameFont)
{
    const auto labelColour = juce::Colour (AetherColours::inkFaint);
    const float tracking = 3.0f;
    const juce::String upperName = name.toUpperCase();

    // Measure numeral width
    float numeralWidth = 0.0f;
    if (numeral.isNotEmpty())
    {
        juce::GlyphArrangement ng;
        ng.addLineOfText (numeralFont, numeral, 0.0f, 0.0f);
        numeralWidth = ng.getBoundingBox (0, ng.getNumGlyphs(), true).getWidth();
    }

    // Measure tracked name width
    float nameWidth = 0.0f;
    if (upperName.isNotEmpty())
    {
        juce::GlyphArrangement ng;
        ng.addLineOfText (nameFont, upperName, 0.0f, 0.0f);
        for (int i = 0; i < ng.getNumGlyphs(); ++i)
            ng.moveRangeOfGlyphs (i, 1, static_cast<float> (i) * tracking, 0.0f);
        nameWidth = ng.getBoundingBox (0, ng.getNumGlyphs(), true).getWidth();
    }

    // Total width and gap
    const float gap = (numeral.isNotEmpty() && upperName.isNotEmpty()) ? 5.0f : 0.0f;
    const float totalWidth = numeralWidth + gap + nameWidth;

    // Center within available width
    const float startX = x + (width - totalWidth) / 2.0f;

    // Draw numeral
    if (numeral.isNotEmpty())
    {
        juce::GlyphArrangement ng;
        ng.addLineOfText (numeralFont, numeral, 0.0f, 0.0f);
        ng.moveRangeOfGlyphs (0, -1, startX, y);
        g.setColour (labelColour);
        ng.draw (g);
    }

    // Draw tracked name
    if (upperName.isNotEmpty())
    {
        const float nameX = startX + numeralWidth + gap;
        drawLetterSpacedText (g, upperName, nameX, y, nameWidth + 10.0f, tracking,
                              nameFont, labelColour, juce::Justification::left);
    }
}

} // namespace ParchmentElements
