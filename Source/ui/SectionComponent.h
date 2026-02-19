#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AetherBypassButton.h"

/**
 * SectionComponent - Reusable container for a DSP stage section.
 *
 * Displays a section label (Roman numeral + name) at the top with optional
 * bypass button right-aligned. Below the label row is the control area
 * where the parent editor places knobs, toggles, and dropdowns.
 *
 * When bypassed, draws a 40% parchment overlay over the control area
 * (below the label row). Controls remain interactive (LOCKED DECISION).
 */
class SectionComponent : public juce::Component
{
public:
    /**
     * @param numeral  Roman numeral prefix (e.g., "I.")
     * @param name     Section name (e.g., "CABINET RESONANCE")
     * @param hasBypass Whether this section has a bypass button (Output does not)
     */
    SectionComponent (const juce::String& numeral, const juce::String& name,
                      bool hasBypass = true);

    ~SectionComponent() override = default;

    //==========================================================================
    // Visual state

    /** Set bypassed state. Draws dimming overlay over control area. */
    void setBypassed (bool shouldBeBypassed);
    bool isBypassed() const noexcept { return bypassed; }

    //==========================================================================
    // Layout

    /** Returns the Rectangle below the label row where controls should be placed.
     *  Call this from resized() in the parent editor to position knobs/toggles. */
    juce::Rectangle<int> getControlArea() const noexcept { return controlArea; }

    //==========================================================================
    // Component overrides
    void paint (juce::Graphics& g) override;
    void resized() override;

    //==========================================================================
    // Public bypass button for attachment by parent
    AetherBypassButton bypassButton;

private:
    juce::String sectionNumeral;
    juce::String sectionName;
    bool         hasbypassButton;
    bool         bypassed = false;

    juce::Rectangle<int> controlArea;

    static constexpr int kLabelRowHeight = 26;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionComponent)
};
