#include "PresetSelector.h"
#include "MixLockButton.h"
#include "AetherColours.h"
#include "AetherLookAndFeel.h"
#include "../presets/FactoryPresets.h"
#include "../presets/UserPresetManager.h"
#include "../Parameters.h"

//==============================================================================
PresetSelector::PresetSelector (juce::AudioProcessorValueTreeState& apvtsRef)
    : apvts (apvtsRef)
{
    // Populate preset list (factory + user)
    rebuildPresetList();

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
            // If mix is locked, capture current value first
            float savedMix = 0.0f;
            bool lockMix = (mixLockBtn != nullptr && mixLockBtn->isLocked());
            if (lockMix)
            {
                if (auto* mixParam = apvts.getParameter (ParamIDs::outMix))
                    savedMix = mixParam->getValue();  // normalised 0-1
            }

            for (auto* param : apvts.processor.getParameters())
            {
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
                {
                    ranged->beginChangeGesture();
                    ranged->setValueNotifyingHost (ranged->getDefaultValue());
                    ranged->endChangeGesture();
                }
            }

            // Restore mix if locked
            if (lockMix)
            {
                if (auto* mixParam = apvts.getParameter (ParamIDs::outMix))
                {
                    mixParam->beginChangeGesture();
                    mixParam->setValueNotifyingHost (savedMix);
                    mixParam->endChangeGesture();
                }
            }
        }
        else if (selectedId >= 2 && selectedId <= 7)
        {
            applyPreset (selectedId - 2);
        }
        else if (selectedId >= kUserPresetIdOffset)
        {
            int userIndex = selectedId - kUserPresetIdOffset;
            if (userIndex >= 0 && userIndex < userPresetFiles.size())
            {
                // If mix is locked, capture current mix value
                float savedMix = 0.0f;
                bool lockMix = (mixLockBtn != nullptr && mixLockBtn->isLocked());
                if (lockMix)
                {
                    if (auto* mixParam = apvts.getParameter (ParamIDs::outMix))
                        savedMix = mixParam->getValue();  // normalised 0-1
                }

                UserPresetManager::loadPreset (userPresetFiles[userIndex], apvts);

                // Restore mix if locked
                if (lockMix)
                {
                    if (auto* mixParam = apvts.getParameter (ParamIDs::outMix))
                    {
                        mixParam->beginChangeGesture();
                        mixParam->setValueNotifyingHost (savedMix);
                        mixParam->endChangeGesture();
                    }
                }
            }
        }
    };

    addAndMakeVisible (comboBox);

    // Save button
    saveButton.setColour (juce::TextButton::textColourOffId, juce::Colour (AetherColours::inkFaint));
    saveButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    saveButton.onClick = [this]
    {
        auto* aw = new juce::AlertWindow ("Save Preset", "Enter a name for your preset:",
                                           juce::MessageBoxIconType::NoIcon);
        aw->addTextEditor ("name", "", "Preset name:");
        aw->addButton ("Save", 1);
        aw->addButton ("Cancel", 0);
        aw->enterModalState (true, juce::ModalCallbackFunction::create (
            [this, aw] (int result)
            {
                if (result == 1)
                {
                    auto name = aw->getTextEditorContents ("name");
                    if (name.isNotEmpty())
                    {
                        UserPresetManager::savePreset (name, apvts);
                        rebuildPresetList();
                    }
                }
                delete aw;
            }), false);
    };
    addAndMakeVisible (saveButton);
}

//==============================================================================
void PresetSelector::applyPreset (int presetIndex)
{
    jassert (presetIndex >= 0 && presetIndex < kNumPresets);
    const auto& preset = kFactoryPresets[static_cast<size_t> (presetIndex)];

    const bool lockMix = (mixLockBtn != nullptr && mixLockBtn->isLocked());

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

    // Output: skip out_mix if mix lock is engaged
    if (! lockMix)
        setParam ("out_mix", preset.outMix);
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
void PresetSelector::rebuildPresetList()
{
    comboBox.clear (juce::dontSendNotification);

    // Factory presets
    comboBox.addItem ("-- Default --", 1);
    comboBox.addItem ("I.  Tight Booth", 2);
    comboBox.addItem ("II.  Live Room", 3);
    comboBox.addItem ("III.  Recording Studio", 4);
    comboBox.addItem ("IV.  Concert Hall", 5);
    comboBox.addItem ("V.  Church Hall", 6);
    comboBox.addItem ("VI.  Cathedral", 7);

    // User presets
    userPresetFiles = UserPresetManager::getAvailablePresets();
    if (userPresetFiles.size() > 0)
    {
        comboBox.addSeparator();
        for (int i = 0; i < userPresetFiles.size(); ++i)
            comboBox.addItem (userPresetFiles[i].getFileNameWithoutExtension(),
                              kUserPresetIdOffset + i);
    }

    comboBox.setSelectedId (1, juce::dontSendNotification);
}

//==============================================================================
void PresetSelector::resized()
{
    auto bounds = getLocalBounds();
    saveButton.setBounds (bounds.removeFromRight (36));
    comboBox.setBounds (bounds);
}
