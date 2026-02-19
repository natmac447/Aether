#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * MixLockButton - Padlock toggle button for mix value protection.
 *
 * When locked, preset loading skips the out_mix parameter so the user's
 * current mix ratio is preserved. Purely UI state -- not an APVTS parameter.
 *
 * Visual states:
 * - Locked: Closed padlock in accentWarm
 * - Unlocked: Open padlock in inkGhost
 * - Hover: accentCopper regardless of state
 *
 * Size: 20x20px, icon-only (no text).
 */
class MixLockButton : public juce::ToggleButton
{
public:
    MixLockButton();
    ~MixLockButton() override = default;

    /** Convenience accessor: returns true when the lock is engaged. */
    bool isLocked() const { return getToggleState(); }

    //==========================================================================
    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixLockButton)
};
