#pragma once
#include <JuceHeader.h>
#include <signalsmith-dsp/filters.h>

// Stage V: Room Tone
// Shaped ambient noise floor with Kellett IIR pink noise, spectral shaping,
// stereo decorrelation, three gating modes, Room Size/Shape coupling,
// and slow LFO modulation for natural quality.

// =========================================================================
// Kellett Pink Noise Generator
// =========================================================================
// Paul Kellett's economy method: 7-state IIR filter converts white noise
// to pink noise (1/f) with excellent spectral accuracy.

struct KellettPinkNoise
{
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, b3 = 0.0f;
    float b4 = 0.0f, b5 = 0.0f, b6 = 0.0f;

    float process (float white)
    {
        b0 = 0.99886f * b0 + white * 0.0555179f;
        b1 = 0.99332f * b1 + white * 0.0750759f;
        b2 = 0.96900f * b2 + white * 0.1538520f;
        b3 = 0.86650f * b3 + white * 0.3104856f;
        b4 = 0.55000f * b4 + white * 0.5329522f;
        b5 = -0.7616f * b5 - white * 0.0168980f;
        float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
        b6 = white * 0.115926f;
        return pink * 0.11f;
    }

    void reset() { b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0.0f; }
};

// =========================================================================
// Shape Ambient Character
// =========================================================================
// Maps the 7 room shapes to spectral character offsets for room tone.
// Each shape subtly alters the ambient noise color.

struct ShapeAmbientCharacter
{
    float presenceFreqOffset;   // Hz offset to presence peak (negative = lower)
    float lpfCutoffOffset;      // Hz offset to LPF cutoff
    float presenceGainDb;       // dB adjustment to presence peak gain
};

// 7 shapes matching ReflectionsSection: Parlour, Gallery, Chamber, Nave, Alcove, Crypt, Conservatory
static const ShapeAmbientCharacter kShapeAmbience[] = {
    {   0.0f,     0.0f,  0.0f },   // Parlour: neutral reference
    { -30.0f,  -500.0f,  1.0f },   // Gallery: long narrow space, stronger low-mid resonance, darker
    {  20.0f,   300.0f, -0.5f },   // Chamber: balanced, slightly brighter
    { -50.0f,  -800.0f,  1.5f },   // Nave: large open space, strong low-mid, quite dark
    {  40.0f,   500.0f, -1.0f },   // Alcove: small enclosed, brighter, less bass
    { -40.0f,  -600.0f,  2.0f },   // Crypt: underground, strong low resonance, very dark
    {  10.0f,   200.0f,  0.5f },   // Conservatory: glass/metal, bright ambience
};

// =========================================================================
// Gate Mode
// =========================================================================

enum class GateMode { AlwaysOn = 0, SignalGated = 1, TransportOnly = 2 };

// =========================================================================
// RoomToneSection Class
// =========================================================================

class RoomToneSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);
    void reset();
    void setBypass (bool bypassed);

    void setAmbience (float ambienceNormalized);         // 0.0-1.0
    void setRoomSize (float roomSizeNormalized);         // 0.0-1.0 (from Stage II)
    void setShape (int shapeIndex);                      // 0-6 (from Stage II)
    void setGateMode (int gateModeIndex);                // 0/1/2
    void setTransportPlaying (bool isPlaying);           // from AudioPlayHead

private:
    void updateShapingFilters();

    // Noise generators (different seeds for stereo decorrelation)
    juce::Random rngL { 42 };
    juce::Random rngR { 12345679 };

    // Kellett pink noise filters
    KellettPinkNoise pinkL, pinkR;

    // Spectral shaping per channel (6 BiquadStatic filters total)
    signalsmith::filters::BiquadStatic<float> hpfL, hpfR;           // HPF ~80Hz
    signalsmith::filters::BiquadStatic<float> presenceL, presenceR; // Peak ~300Hz
    signalsmith::filters::BiquadStatic<float> lpfL, lpfR;           // LPF ~8kHz

    // SmoothedValues
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> ambienceSmoothed;  // 25ms
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassBlend;       // 10ms

    // Manual one-pole gate envelope (asymmetric attack/release)
    float gateLevel = 0.0f;
    float gateAttackCoeff  = 0.0f;
    float gateReleaseCoeff = 0.0f;

    // LFO state
    float lfoPhase = 0.0f;
    static constexpr float kLfoRateHz   = 0.15f;
    static constexpr float kLfoDepthDb  = 0.8f;

    // Gating
    GateMode gateMode = GateMode::AlwaysOn;
    bool transportPlaying = true;
    static constexpr float kGateThresholdDb = -60.0f;
    static constexpr float kGateAttackMs    = 50.0f;
    static constexpr float kGateReleaseMs   = 500.0f;

    // Coupling state
    float currentRoomSize   = 0.4f;
    int   currentShapeIndex = 0;

    // State
    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;
    bool   bypassed          = true;   // TONE-06: default bypassed

    // Level
    static constexpr float kMaxNoiseGainDb = -35.0f;   // midpoint of -40 to -30dB spec
};
