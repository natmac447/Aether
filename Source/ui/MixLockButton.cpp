#include "MixLockButton.h"
#include "AetherColours.h"

//==============================================================================
MixLockButton::MixLockButton()
{
    setSize (20, 20);
    setClickingTogglesState (true);
}

//==============================================================================
void MixLockButton::paintButton (juce::Graphics& g,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool /*shouldDrawButtonAsDown*/)
{
    const bool locked = getToggleState();

    // Choose colour
    juce::Colour colour;
    if (shouldDrawButtonAsHighlighted)
        colour = juce::Colour (AetherColours::accentCopper);
    else if (locked)
        colour = juce::Colour (AetherColours::accentWarm);
    else
        colour = juce::Colour (AetherColours::inkGhost);

    g.setColour (colour);

    // Draw padlock centred in bounds
    const auto bounds = getLocalBounds().toFloat();
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();

    // Body: 8x7 rectangle
    const float bodyW = 8.0f;
    const float bodyH = 7.0f;
    const float bodyX = cx - bodyW / 2.0f;
    const float bodyY = cy;

    g.fillRoundedRectangle (bodyX, bodyY, bodyW, bodyH, 1.0f);

    // Shackle (arc on top of body)
    const float shackleW = 6.0f;
    const float shackleH = 5.0f;

    juce::Path shackle;

    if (locked)
    {
        // Closed shackle: symmetric arc from body top-left to top-right
        const float sx = cx - shackleW / 2.0f;
        const float sy = bodyY;

        shackle.startNewSubPath (sx, sy);
        shackle.cubicTo (sx, sy - shackleH,
                         sx + shackleW, sy - shackleH,
                         sx + shackleW, sy);
    }
    else
    {
        // Open shackle: rigid U-shape lifted up, right leg floats above body
        const float lift = 3.0f;
        const float sx = cx - shackleW / 2.0f;
        const float sy = bodyY - lift;

        shackle.startNewSubPath (sx, bodyY);           // left leg at body
        shackle.lineTo (sx, sy);                       // straight up to lifted arc
        shackle.cubicTo (sx, sy - shackleH,            // symmetric arc (same as closed)
                         sx + shackleW, sy - shackleH,
                         sx + shackleW, sy);            // right leg ends lifted
    }

    g.strokePath (shackle, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    // Small keyhole dot on body
    g.setColour (locked ? juce::Colour (AetherColours::parchment)
                        : juce::Colour (AetherColours::parchment).withAlpha (0.5f));
    g.fillEllipse (cx - 1.0f, bodyY + bodyH * 0.35f, 2.0f, 2.0f);
}
