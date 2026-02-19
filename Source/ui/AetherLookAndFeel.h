#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AetherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AetherLookAndFeel();
    ~AetherLookAndFeel() override = default;
};
