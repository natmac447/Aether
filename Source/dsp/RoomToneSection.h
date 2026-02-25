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

// 30 shapes matching ReflectionsSection (10 small, 10 medium, 10 large)
static const ShapeAmbientCharacter kShapeAmbience[] = {
    // Small Rooms (0-9)
    {   0.0f,     0.0f,  0.0f },   //  0: Parlour: neutral reference
    {  40.0f,   500.0f, -1.0f },   //  1: Alcove: small enclosed, brighter
    { -40.0f,  -600.0f,  2.0f },   //  2: Crypt: underground, strong low resonance
    {  10.0f,   200.0f, -0.5f },   //  3: Vestibule: hard walls, slightly bright
    {  30.0f,  -200.0f, -1.5f },   //  4: Closet: fabric dampens, proximity presence
    { -20.0f,  -300.0f,  0.5f },   //  5: Study: books absorb, warm, mid-forward
    {  50.0f,   800.0f, -2.0f },   //  6: Telephone Box: metallic, very bright, thin
    { -10.0f,  -100.0f,  0.0f },   //  7: Pantry: mixed surfaces, nearly neutral
    { -25.0f,  -400.0f,  1.0f },   //  8: Confessional: wood absorbs, dark, intimate
    {  35.0f,   600.0f, -1.0f },   //  9: Powder Room: ceramic tiles, bright
    // Medium Rooms (10-19)
    {  20.0f,   300.0f, -0.5f },   // 10: Chamber: balanced, slightly brighter
    { -30.0f,  -500.0f,  1.0f },   // 11: Gallery: long narrow, stronger low-mid
    {  10.0f,   200.0f,  0.5f },   // 12: Conservatory: glass/metal, bright ambience
    { -35.0f,  -400.0f,  1.5f },   // 13: Scriptorium: stone, dark medieval
    { -45.0f,  -700.0f,  0.5f },   // 14: Library: books absorb, very dark/warm
    {  -5.0f,   100.0f,  0.0f },   // 15: Drawing Room: balanced, refined
    {  15.0f,   400.0f, -0.5f },   // 16: Workshop: hard surfaces, slightly bright
    { -20.0f,  -300.0f,  1.0f },   // 17: Refectory: stone+wood, warm
    {  25.0f,   300.0f, -0.5f },   // 18: Solarium: glass, bright and airy
    {   5.0f,     0.0f,  0.5f },   // 19: Apothecary: complex scattering, slight presence
    // Large Rooms (20-29)
    { -50.0f,  -800.0f,  1.5f },   // 20: Nave: large open space, strong low-mid
    { -10.0f,   100.0f,  0.0f },   // 21: Ballroom: balanced, grand
    {   5.0f,   200.0f, -0.5f },   // 22: Atrium: open, some brightness from stone
    { -40.0f,  -600.0f,  1.5f },   // 23: Chapel: stone, reverberant, dark
    {  20.0f,   300.0f, -1.0f },   // 24: Warehouse: harsh, bright, metallic
    { -60.0f, -1000.0f,  2.5f },   // 25: Cistern: underground, extreme low resonance
    { -15.0f,  -200.0f,  0.5f },   // 26: Observatory: dome focuses sound, moderate
    { -25.0f,  -400.0f,  1.0f },   // 27: Great Hall: timber+stone, warm
    {  30.0f,   500.0f, -1.5f },   // 28: Greenhouse: all glass, very bright
    { -45.0f,  -700.0f,  2.0f },   // 29: Mausoleum: marble, cold, resonant
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
    void setShape (int shapeIndex);                      // 0-29 (from Stage II)
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
