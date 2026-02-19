#pragma once
#include <juce_graphics/juce_graphics.h>

/**
 * ParchmentElements - Static drawing helpers for Victorian parchment decorative elements.
 *
 * All functions draw at specified coordinates using the AetherColours palette.
 * Used by the editor paint() and section components (Plan 04) to render:
 * - Cached paper texture + vignette background
 * - Diamond dividers between sections
 * - Corner L-brackets at plugin frame corners
 * - Double-rule Victorian publication borders
 * - Letter-spaced text rendering (JUCE has no built-in letter-spacing)
 * - Section labels with Roman numerals
 */
namespace ParchmentElements
{
    /**
     * Pre-renders the full background (parchment fill + ruled-paper texture + radial vignette)
     * to a cached juce::Image. Call once on construction and on resize; draw with drawImageAt().
     *
     * - Paper texture: faint horizontal lines every 4px at Sepia 0.018 alpha
     * - Edge vignette: radial gradient from transparent centre to Sepia 0.08 alpha at corners,
     *   darkening starts at 60% from centre (~30px visible inward effect)
     */
    juce::Image generateBackground (int width, int height);

    /**
     * Draws a horizontal divider with a centered diamond ornament.
     * Line colour: Ink Ghost. Diamond: 5x5px rotated square stroke, 1px.
     * Lines extend from leftX to midX-8 and midX+8 to rightX.
     */
    void drawDiamondDivider (juce::Graphics& g, float y, float leftX, float rightX);

    /**
     * Draws L-shaped brackets at all four corners of the given bounds.
     * Bracket size: 20x20px, 1px Ink Ghost stroke, positioned 6px from edges.
     */
    void drawCornerBrackets (juce::Graphics& g, juce::Rectangle<int> bounds);

    /**
     * Draws a Victorian double-rule border (two distinct 1px lines with 3px gap).
     * Primary line at y in full Ink Ghost. Secondary line at y+3 (isTop) or y-3 (!isTop)
     * at 40% opacity of Ink Ghost.
     */
    void drawDoubleRule (juce::Graphics& g, float y, float leftX, float rightX, bool isTop);

    /**
     * Draws text with custom letter spacing (tracking).
     * Uses GlyphArrangement to offset each glyph's x position by glyphIndex * tracking.
     */
    void drawLetterSpacedText (juce::Graphics& g, const juce::String& text,
                               float x, float y, float width, float tracking,
                               const juce::Font& font, juce::Colour colour,
                               juce::Justification justification);

    /**
     * Draws "I. Cabinet Resonance" style section labels.
     * Numeral in Cormorant Garamond italic, name in Spectral uppercase with 3px tracking.
     * Both rendered in Ink Faint colour, with 4px gap between numeral and name.
     */
    void drawSectionLabel (juce::Graphics& g, const juce::String& numeral,
                           const juce::String& name, float x, float y, float width,
                           const juce::Font& numeralFont, const juce::Font& nameFont);
}
