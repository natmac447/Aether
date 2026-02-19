#pragma once
#include <JuceHeader.h>
#include "ui/AetherLookAndFeel.h"
#include "ui/AetherKnob.h"
#include "ui/AetherToggle.h"
#include "ui/AetherBypassButton.h"
#include "ui/SectionComponent.h"
#include "ui/PresetSelector.h"
#include "ui/VisualizationComponent.h"

class AetherProcessor;

/**
 * AetherEditor - Complete Victorian parchment UI for the Aether plugin.
 *
 * Three-column layout at 900x530px:
 *   Left (220px):  Stage I (Resonance), Stage IV (Excitation), Stage V (Room Tone)
 *   Center (460px): Stage II (Early Reflections) with visualization placeholder
 *   Right (220px): Stage III (Air), Stage VI (Diffuse Tail), Output
 *
 * All 21 APVTS parameters connected to custom controls:
 *   11 knobs, 2 dropdowns, 2 toggles, 6 bypass buttons
 */
class AetherEditor : public juce::AudioProcessorEditor,
                     private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit AetherEditor (AetherProcessor&);
    ~AetherEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==========================================================================
    // APVTS Listener for bypass state propagation
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    //==========================================================================
    AetherProcessor& processorRef;

    // LookAndFeel (owned, set on this component)
    AetherLookAndFeel lookAndFeel;

    // Cached background image (parchment + texture + vignette)
    juce::Image backgroundImage;

    //==========================================================================
    // Header
    PresetSelector presetSelector;

    //==========================================================================
    // Stage I: Resonance (left panel)
    SectionComponent resSection { "", "Cabinet Resonance" };
    AetherKnob       resWeightKnob { "Body", 56 };
    juce::ComboBox   materialCombo;

    // Stage II: Early Reflections (center panel)
    SectionComponent reflSection { "", "Early Reflections" };
    AetherKnob       reflSizeKnob { "Room Size", 56 };
    AetherKnob       reflProxKnob { "Proximity", 56 };
    AetherKnob       reflWidthKnob { "Width", 56 };
    juce::ComboBox   reflShapeCombo;

    // Stage III: Air & Distance (right panel)
    SectionComponent airSection { "", "Air & Distance" };
    AetherKnob       airAmountKnob { "Air", 56 };
    AetherToggle     airCharToggle { juce::StringArray { "Warm", "Neutral", "Cold" } };

    // Stage IV: Excitation (left panel)
    SectionComponent excitSection { "", "Excitation" };
    AetherKnob       excitDriveKnob { "Drive", 56 };

    // Stage V: Room Tone (left panel)
    SectionComponent toneSection { "", "Room Tone" };
    AetherKnob       toneAmbKnob { "Ambience", 56 };
    AetherToggle     toneGateToggle { juce::StringArray { "Always", "Gated", "Transport" } };

    // Stage VI: Diffuse Tail (right panel)
    SectionComponent tailSection { "", "Diffuse Tail" };
    AetherKnob       tailDecayKnob { "Decay", 56 };
    AetherKnob       tailDiffKnob { "Diffusion", 56 };

    // Output (right panel, no bypass)
    SectionComponent outputSection { "", "Output", false };
    AetherKnob       mixKnob { "Mix", 56 };
    AetherKnob       levelKnob { "Level", 56 };

    //==========================================================================
    // Visualization
    VisualizationComponent vizComponent;

    //==========================================================================
    // Parameter attachments (created AFTER addAndMakeVisible)
    // -- SliderAttachments are managed inside AetherKnob::attachToParameter
    // -- ComboBox attachments for Material and Shape dropdowns
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> materialAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherEditor)
};
