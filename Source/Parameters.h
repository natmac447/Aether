#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamIDs
{
    // Stage I: Resonance
    inline constexpr auto resWeight   { "res_weight" };
    inline constexpr auto resMaterial { "res_material" };
    inline constexpr auto resBypass   { "res_bypass" };

    // Stage II: Early Reflections
    inline constexpr auto reflSize    { "refl_size" };
    inline constexpr auto reflShape   { "refl_shape" };
    inline constexpr auto reflProx    { "refl_prox" };
    inline constexpr auto reflWidth   { "refl_width" };
    inline constexpr auto reflBypass  { "refl_bypass" };

    // Stage III: Air & Distance
    inline constexpr auto airAmount   { "air_amount" };
    inline constexpr auto airChar     { "air_char" };
    inline constexpr auto airBypass   { "air_bypass" };

    // Stage IV: Excitation
    inline constexpr auto excitDrive  { "excit_drive" };
    inline constexpr auto excitBypass { "excit_bypass" };

    // Stage V: Room Tone
    inline constexpr auto toneAmb     { "tone_amb" };
    inline constexpr auto toneGate    { "tone_gate" };
    inline constexpr auto toneBypass  { "tone_bypass" };

    // Stage VI: Diffuse Tail
    inline constexpr auto tailDecay   { "tail_decay" };
    inline constexpr auto tailDiff    { "tail_diff" };
    inline constexpr auto tailBypass  { "tail_bypass" };

    // Output
    inline constexpr auto outMix      { "out_mix" };
    inline constexpr auto outLevel    { "out_level" };
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // =========================================================================
    // Stage I: Resonance
    // =========================================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::resWeight, 2 },
        "I Resonance - Weight",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.5f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) {
                return juce::String (juce::roundToInt (value * 100.0f)) + "%";
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::resMaterial, 2 },
        "I Resonance - Material",
        juce::StringArray { "Pine", "Oak", "Walnut", "Mahogany", "Iron", "Steel", "Copper", "Limestone", "Marble", "Granite" },
        3  // default: Mahogany
    ));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::resBypass, 2 },
        "I Resonance - Bypass",
        false
    ));

    // =========================================================================
    // Stage II: Early Reflections
    // =========================================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::reflSize, 3 },
        "II Reflections - Room Size",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f, 0.4f),
        0.4f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) -> juce::String {
                if (value < 0.2f)  return "Small";
                if (value < 0.4f)  return "Sm-Med";
                if (value < 0.6f)  return "Medium";
                if (value < 0.8f)  return "Med-Lg";
                return "Large";
            })
            .withValueFromStringFunction ([] (const juce::String& text) -> float {
                if (text == "Small")  return 0.1f;
                if (text == "Sm-Med") return 0.3f;
                if (text == "Medium") return 0.5f;
                if (text == "Med-Lg") return 0.7f;
                if (text == "Large")  return 0.9f;
                return text.getFloatValue();
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::reflShape, 3 },
        "II Reflections - Shape",
        juce::StringArray { "The Parlour", "The Gallery", "The Chamber", "The Nave",
                            "The Alcove", "The Crypt", "The Conservatory" },
        0  // default: The Parlour
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::reflProx, 1 },
        "II Reflections - Proximity",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.3f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) -> juce::String {
                if (value < 0.2f)  return "Near";
                if (value < 0.4f)  return "Nr-Mid";
                if (value < 0.6f)  return "Middle";
                if (value < 0.8f)  return "Mid-Far";
                return "Far";
            })
            .withValueFromStringFunction ([] (const juce::String& text) -> float {
                if (text == "Near")    return 0.1f;
                if (text == "Nr-Mid")  return 0.3f;
                if (text == "Middle")  return 0.5f;
                if (text == "Mid-Far") return 0.7f;
                if (text == "Far")     return 0.9f;
                return text.getFloatValue();
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::reflWidth, 3 },
        "II Reflections - Width",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.7f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) {
                return juce::String (juce::roundToInt (value * 100.0f)) + "%";
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::reflBypass, 1 },
        "II Reflections - Bypass",
        false
    ));

    // =========================================================================
    // Stage III: Air & Distance
    // =========================================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::airAmount, 1 },
        "III Air - Amount",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.4f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) {
                return juce::String (juce::roundToInt (value * 100.0f)) + "%";
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::airChar, 2 },
        "III Air - Character",
        juce::StringArray { "Warm", "Neutral", "Cold" },
        1  // default: Neutral
    ));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::airBypass, 1 },
        "III Air - Bypass",
        false
    ));

    // =========================================================================
    // Stage IV: Excitation
    // =========================================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::excitDrive, 1 },
        "IV Excitation - Drive",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.25f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) {
                return juce::String (juce::roundToInt (value * 100.0f)) + "%";
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::excitBypass, 1 },
        "IV Excitation - Bypass",
        false
    ));

    // =========================================================================
    // Stage V: Room Tone
    // =========================================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::toneAmb, 1 },
        "V Room Tone - Ambience",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.1f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) {
                return juce::String (juce::roundToInt (value * 100.0f)) + "%";
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::toneGate, 1 },
        "V Room Tone - Gate",
        juce::StringArray { "Always On", "Signal-Gated", "Transport-Only" },
        0  // default: Always On
    ));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::toneBypass, 1 },
        "V Room Tone - Bypass",
        true  // Room Tone bypassed by default per design handoff
    ));

    // =========================================================================
    // Stage VI: Diffuse Tail
    // =========================================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::tailDecay, 3 },
        "VI Tail - Decay",
        juce::NormalisableRange<float> (50.0f, 2000.0f, 1.0f, 0.3f),
        150.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) {
                return juce::String (juce::roundToInt (value)) + " ms";
            })
            .withValueFromStringFunction ([] (const juce::String& text) {
                return text.trimCharactersAtEnd (" ms").getFloatValue();
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::tailDiff, 1 },
        "VI Tail - Diffusion",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.6f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) {
                return juce::String (juce::roundToInt (value * 100.0f)) + "%";
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::tailBypass, 1 },
        "VI Tail - Bypass",
        false
    ));

    // =========================================================================
    // Output
    // =========================================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::outMix, 1 },
        "Output - Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.7f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) {
                return juce::String (juce::roundToInt (value * 100.0f)) + "%";
            })
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::outLevel, 1 },
        "Output - Level",
        juce::NormalisableRange<float> (-24.0f, 6.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) {
                if (value <= -24.0f) return juce::String ("-inf dB");
                return juce::String (value, 1) + " dB";
            })
            .withValueFromStringFunction ([] (const juce::String& text) {
                if (text == "-inf dB") return -24.0f;
                return text.trimCharactersAtEnd (" dB").getFloatValue();
            })
    ));

    return layout;
}
