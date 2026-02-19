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

    juce::Font numeralFont (juce::FontOptions ("Georgia", 12.0f, juce::Font::italic));
    juce::Font nameFont (juce::FontOptions ("Georgia", 13.0f, juce::Font::plain));

    if (laf != nullptr)
    {
        numeralFont = laf->getDisplayFontItalic (12.0f);
        nameFont = laf->getSpectralFont (13.0f);
    }

    // Center label over full width (bypass button overlaps but titles look better centred)
    float availWidth = static_cast<float> (getWidth());

    ParchmentElements::drawSectionLabel (g, sectionNumeral, sectionName,
                                          0.0f, 14.0f, // x, y (baseline)
                                          availWidth,
                                          numeralFont, nameFont);

    // Bypassed state is handled by reducing child component alpha (no overlay)
}

void SectionComponent::resized()
{
    auto bounds = getLocalBounds();

    // Label row at top
    auto labelRow = bounds.removeFromTop (kLabelRowHeight);

    // Position bypass button right-aligned in label row
    if (hasbypassButton)
    {
        bypassButton.setBounds (labelRow.removeFromRight (16).reduced (0, 5));
    }

    // Remaining area is the control area
    controlArea = bounds;
}
