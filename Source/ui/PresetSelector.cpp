#include "PresetSelector.h"
#include "AetherColours.h"
#include "AetherLookAndFeel.h"
#include "../presets/FactoryPresets.h"

//==============================================================================
PresetSelector::PresetSelector (juce::AudioProcessorValueTreeState& apvtsRef)
    : apvts (apvtsRef)
{
    // Populate with preset names (em-dash \u2014 used for display)
    comboBox.addItem ("-- Default --", 1);
    comboBox.addItem ("I.  — Tight Booth", 2);
    comboBox.addItem ("II.  — Live Room", 3);
    comboBox.addItem ("III.  — Recording Studio", 4);
    comboBox.addItem ("IV.  — Concert Hall", 5);
    comboBox.addItem ("V.  — Church Hall", 6);
    comboBox.addItem ("VI.  — Cathedral", 7);

    comboBox.setSelectedId (1, juce::dontSendNotification);

    // Style: Ink Light text, Ink Ghost border, transparent background
    comboBox.setColour (juce::ComboBox::textColourId, juce::Colour (AetherColours::inkLight));
    comboBox.setColour (juce::ComboBox::outlineColourId, juce::Colour (AetherColours::inkGhost));
    comboBox.setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    comboBox.setColour (juce::ComboBox::arrowColourId, juce::Colour (AetherColours::inkFaint));

    // Wire onChange to apply presets
    comboBox.onChange = [this]
    {
        const int selectedId = comboBox.getSelectedId();

        if (selectedId == 1)
        {
            // "Default" -- reset all parameters to their APVTS defaults
            for (auto* param : apvts.processor.getParameters())
            {
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
                {
                    ranged->beginChangeGesture();
                    ranged->setValueNotifyingHost (ranged->getDefaultValue());
                    ranged->endChangeGesture();
                }
            }
        }
        else if (selectedId >= 2 && selectedId <= 7)
        {
            applyPreset (selectedId - 2);
        }
    };

    addAndMakeVisible (comboBox);
}

//==============================================================================
void PresetSelector::applyPreset (int presetIndex)
{
    jassert (presetIndex >= 0 && presetIndex < kNumPresets);
    const auto& preset = kFactoryPresets[static_cast<size_t> (presetIndex)];

    // Helper: set a parameter by ID using its raw (denormalised) value
    auto setParam = [this] (const juce::String& paramId, float rawValue)
    {
        if (auto* param = apvts.getParameter (paramId))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (rawValue));
            param->endChangeGesture();
        }
    };

    // Stage I: Resonance
    setParam ("res_weight",   preset.resWeight);
    setParam ("res_material", static_cast<float> (preset.resMaterial));

    // Stage II: Early Reflections
    setParam ("refl_size",    preset.reflSize);
    setParam ("refl_shape",   static_cast<float> (preset.reflShape));
    setParam ("refl_prox",    preset.reflProx);
    setParam ("refl_width",   preset.reflWidth);

    // Stage III: Air & Distance
    setParam ("air_amount",   preset.airAmount);
    setParam ("air_char",     static_cast<float> (preset.airChar));

    // Stage IV: Excitation
    setParam ("excit_drive",  preset.excitDrive);

    // Stage V: Room Tone
    setParam ("tone_amb",     preset.toneAmb);
    setParam ("tone_gate",    static_cast<float> (preset.toneGate));

    // Stage VI: Diffuse Tail
    setParam ("tail_decay",   preset.tailDecay);
    setParam ("tail_diff",    preset.tailDiff);

    // Output
    setParam ("out_mix",      preset.outMix);
    setParam ("out_level",    preset.outLevel);

    // All bypass parameters: set to false (not bypassed)
    setParam ("res_bypass",   0.0f);
    setParam ("refl_bypass",  0.0f);
    setParam ("air_bypass",   0.0f);
    setParam ("excit_bypass", 0.0f);
    setParam ("tone_bypass",  0.0f);
    setParam ("tail_bypass",  0.0f);
}

//==============================================================================
void PresetSelector::resized()
{
    comboBox.setBounds (getLocalBounds());
}
