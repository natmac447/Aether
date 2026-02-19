#pragma once
#include <juce_graphics/juce_graphics.h>

/**
 * AetherColours - Complete colour palette for the Aether plugin.
 *
 * Victorian parchment/ink aesthetic from the design handoff.
 * Light theme - unique among Cairn plugins.
 */
namespace AetherColours
{
    // =========================================================================
    // Parchment tones (backgrounds, knob body)
    // =========================================================================
    inline constexpr juce::uint32 parchment       = 0xfff0e6d3;  // Primary background
    inline constexpr juce::uint32 parchmentLight   = 0xfff5edd9;  // Highlight areas, knob face highlights
    inline constexpr juce::uint32 parchmentDark    = 0xffe0d2b8;  // Knob body shadows, depressed areas
    inline constexpr juce::uint32 paperEdge        = 0xffd4c4a8;  // Plugin outer border, aged-edge vignette

    // =========================================================================
    // Ink tones (text, linework)
    // =========================================================================
    inline constexpr juce::uint32 ink              = 0xff2a2118;  // Primary text, darkest linework, knob indicators
    inline constexpr juce::uint32 inkLight         = 0xff5a4a38;  // Secondary text, acoustic ray lines
    inline constexpr juce::uint32 inkFaint         = 0xff9a8a72;  // Labels, annotations, section numerals
    inline constexpr juce::uint32 inkGhost         = 0xffc8b89a;  // Borders, dividers, ornamental lines, bypassed

    // =========================================================================
    // Accent tones
    // =========================================================================
    inline constexpr juce::uint32 accentWarm       = 0xff8b6b3d;  // Active states, accent highlights
    inline constexpr juce::uint32 accentCopper     = 0xffa67c52;  // Hover states, warm glow
    inline constexpr juce::uint32 sepia            = 0xff704214;  // Deep accent, figure labels

    // =========================================================================
    // Utility
    // =========================================================================
    inline constexpr juce::uint32 shadow           = 0x262a2118;  // rgba(42, 33, 24, 0.15) drop shadows

    // Bypass dimming
    inline constexpr float bypassedOpacity = 0.5f;

    // =========================================================================
    // Convenience helper
    // =========================================================================
    inline juce::Colour colour (juce::uint32 argb) { return juce::Colour (argb); }
}
