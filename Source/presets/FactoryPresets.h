#pragma once
#include <array>

/**
 * FactoryPresets - Constexpr data definitions for the 6 Aether factory presets.
 *
 * Each preset specifies raw (denormalised) values for all 15 non-bypass continuous/choice
 * parameters. When applying a preset, use param->convertTo0to1(rawValue) before calling
 * setValueNotifyingHost. Bypass parameters are always set to false (0.0f) when loading a preset.
 *
 * Parameter IDs map to ParamIDs in Parameters.h.
 */

struct PresetData
{
    const char* name;

    // Stage I: Resonance
    float resWeight;        // 0.0 - 1.0
    int   resMaterial;      // index: 0=Pine, 1=Oak, 2=Walnut, 3=Mahogany, 4=Iron, 5=Steel, 6=Copper, 7=Limestone, 8=Marble, 9=Granite

    // Stage II: Early Reflections
    float reflSize;         // 0.0 - 1.0
    int   reflShape;        // index: 0=Parlour, 1=Gallery, 2=Chamber, 3=Nave, 4=Alcove, 5=Crypt, 6=Conservatory
    float reflProx;         // 0.0 - 1.0
    float reflWidth;        // 0.0 - 1.0

    // Stage III: Air & Distance
    float airAmount;        // 0.0 - 1.0
    int   airChar;          // index: 0=Warm, 1=Neutral, 2=Cold

    // Stage IV: Excitation
    float excitDrive;       // 0.0 - 1.0

    // Stage V: Room Tone
    float toneAmb;          // 0.0 - 1.0
    int   toneGate;         // index: 0=Always On, 1=Signal-Gated, 2=Transport-Only

    // Stage VI: Diffuse Tail
    float tailDecay;        // 50.0 - 2000.0 ms (raw value, not normalised)
    float tailDiff;         // 0.0 - 1.0

    // Output
    float outMix;           // 0.0 - 1.0
    float outLevel;         // -24.0 - 6.0 dB
};

static constexpr int kNumPresets = 6;

static constexpr std::array<PresetData, kNumPresets> kFactoryPresets {{
    //----------------------------------------------------------------------
    // I. Tight Booth -- Small iso booth, dead and dry
    //----------------------------------------------------------------------
    {
        "I. \xe2\x80\x94 Tight Booth",
        0.35f, 0,           // res: weight, Pine
        0.10f, 4, 0.15f, 0.30f,  // refl: small, Alcove, near, narrow
        0.15f, 0,           // air: low, Warm
        0.10f,              // excit: subtle
        0.05f, 1,           // tone: minimal, Signal-Gated
        50.0f, 0.40f,       // tail: short, low diffusion
        0.55f, 0.0f         // out: moderate mix, 0dB
    },

    //----------------------------------------------------------------------
    // II. Live Room -- Professional studio live room, balanced and musical
    //----------------------------------------------------------------------
    {
        "II. \xe2\x80\x94 Live Room",
        0.50f, 3,           // res: medium, Mahogany
        0.40f, 2, 0.35f, 0.65f,  // refl: medium, Chamber, nr-mid, wide
        0.40f, 1,           // air: moderate, Neutral
        0.25f,              // excit: moderate
        0.10f, 1,           // tone: light, Signal-Gated
        150.0f, 0.60f,      // tail: medium, moderate diffusion
        0.70f, 0.0f         // out: full mix, 0dB
    },

    //----------------------------------------------------------------------
    // III. Recording Studio -- Controlled acoustic environment
    //----------------------------------------------------------------------
    {
        "III. \xe2\x80\x94 Recording Studio",
        0.45f, 1,           // res: moderate, Oak
        0.35f, 0, 0.30f, 0.55f,  // refl: sm-med, Parlour, nr-mid, moderate
        0.30f, 1,           // air: moderate, Neutral
        0.15f,              // excit: gentle
        0.08f, 1,           // tone: subtle, Signal-Gated
        120.0f, 0.55f,      // tail: controlled, moderate diffusion
        0.65f, 0.0f         // out: moderate mix, 0dB
    },

    //----------------------------------------------------------------------
    // IV. Concert Hall -- Large performance space, spacious and enveloping
    //----------------------------------------------------------------------
    {
        "IV. \xe2\x80\x94 Concert Hall",
        0.55f, 2,           // res: moderate-high, Walnut
        0.80f, 3, 0.70f, 0.85f,  // refl: med-lg, Nave, mid-far, very wide
        0.60f, 1,           // air: generous, Neutral
        0.30f,              // excit: moderate-high
        0.15f, 0,           // tone: moderate, Always On
        600.0f, 0.75f,      // tail: long, high diffusion
        0.70f, 0.0f         // out: full mix, 0dB
    },

    //----------------------------------------------------------------------
    // V. Church Hall -- Stone walls, high ceiling, bright reflections
    //----------------------------------------------------------------------
    {
        "V. \xe2\x80\x94 Church Hall",
        0.60f, 7,           // res: high, Limestone
        0.75f, 5, 0.65f, 0.80f,  // refl: med-lg, Crypt, mid-far, wide
        0.55f, 2,           // air: generous, Cold
        0.20f,              // excit: moderate
        0.12f, 0,           // tone: moderate, Always On
        500.0f, 0.70f,      // tail: long, high diffusion
        0.70f, 0.0f         // out: full mix, 0dB
    },

    //----------------------------------------------------------------------
    // VI. Cathedral -- Massive stone space, long decay, maximum grandeur
    //----------------------------------------------------------------------
    {
        "VI. \xe2\x80\x94 Cathedral",
        0.65f, 8,           // res: high, Marble
        0.95f, 3, 0.85f, 0.95f,  // refl: large, Nave, far, very wide
        0.70f, 2,           // air: high, Cold
        0.35f,              // excit: pronounced
        0.20f, 0,           // tone: moderate-high, Always On
        1200.0f, 0.85f,     // tail: very long, very high diffusion
        0.65f, 0.0f         // out: moderate mix, 0dB
    }
}};
