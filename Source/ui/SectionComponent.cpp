#include "SectionComponent.h"
#include "AetherColours.h"
#include "AetherLookAndFeel.h"
#include "ParchmentElements.h"

//==============================================================================
SectionComponent::SectionComponent (const juce::String& numeral, const juce::String& name,
                                    bool hasBypass)
    : sectionNumeral (numeral),
      sectionName (name),
      hasbypassButton (hasBypass)
{
    if (hasbypassButton)
        addAndMakeVisible (bypassButton);
}

//==============================================================================
// Visual state
//==============================================================================

void SectionComponent::setBypassed (bool shouldBeBypassed)
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

void SectionComponent::paint (juce::Graphics& g)
{
    // Draw section label using ParchmentElements
    auto* laf = dynamic_cast<AetherLookAndFeel*> (&getLookAndFeel());

    juce::Font numeralFont (juce::FontOptions ("Georgia", 13.0f, juce::Font::italic));
    juce::Font nameFont (juce::FontOptions ("Georgia", 9.0f, juce::Font::plain));

    if (laf != nullptr)
    {
        numeralFont = laf->getDisplayFontItalic (13.0f);
        nameFont = laf->getSpectralFont (9.0f);
    }

    ParchmentElements::drawSectionLabel (g, sectionNumeral, sectionName,
                                          0.0f, 14.0f, // x, y (baseline)
                                          static_cast<float> (getWidth()),
                                          numeralFont, nameFont);

    // Bypassed overlay: 40% parchment over control area (below label row)
    if (bypassed && ! controlArea.isEmpty())
    {
        g.setColour (juce::Colour (AetherColours::parchment).withAlpha (0.4f));
        g.fillRect (controlArea);
    }
}

void SectionComponent::resized()
{
    auto bounds = getLocalBounds();

    // Label row at top
    auto labelRow = bounds.removeFromTop (kLabelRowHeight);

    // Position bypass button right-aligned in label row
    if (hasbypassButton)
    {
        bypassButton.setBounds (labelRow.removeFromRight (30).reduced (0, 2));
    }

    // Remaining area is the control area
    controlArea = bounds;
}
