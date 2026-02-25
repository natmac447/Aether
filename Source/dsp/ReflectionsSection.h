#pragma once
#include <JuceHeader.h>
#include <signalsmith-dsp/delay.h>
#include <signalsmith-dsp/filters.h>
#include <signalsmith-dsp/mix.h>
#include <array>

// Stage II: Early Reflections
// Stereo tapped delay line with 16 taps (8 per channel), 30 room geometry
// shape presets, Room Size/Shape/Proximity/Width controls, per-tap lowpass
// absorption, and 10ms bypass crossfade.

struct ShapePreset
{
    const char* name;
    const char* description;

    // Left channel taps (8 taps)
    float delayMsL[8];   // Base delay times (scaled by Room Size)
    float gainL[8];       // Tap gains (0.0-1.0)
    float panL[8];        // Pan position (-1.0 = hard left, 1.0 = hard right)

    // Right channel taps (8 taps)
    float delayMsR[8];   // Base delay times (offset from L for decorrelation)
    float gainR[8];       // Tap gains
    float panR[8];        // Pan position

    float absorptionRate;       // LP cutoff reduction per ms of delay
    float tailDiffusionBias;    // 0.0-1.0: how much this shape densifies the tail
    float tailModalCharacter;   // 0.0-1.0: 0=even mode distribution, 1=clustered modes
};

class ReflectionsSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);
    void reset();
    void setBypass (bool shouldBypass);
    void setRoomSize (float size);
    void setShape (int shapeIndex);
    void setProximity (float proximity);
    void setWidth (float width);
    void setAirDarkening (float darkening);

    const ShapePreset& getShapePreset (int index) const;

    static constexpr int kTapsPerChannel = 8;
    static constexpr int kNumShapes = 30;
    static const ShapePreset kShapes[kNumShapes];

private:
    // Maps normalized 0-1 Room Size to delay time in ms (1-30ms)
    static float roomSizeToDelayMs (float normalized);

    // Stereo delay buffers (one per channel, written once per sample, tapped multiple times)
    signalsmith::delay::Delay<float> delayLineL;
    signalsmith::delay::Delay<float> delayLineR;

    // Per-tap lowpass absorption filters
    signalsmith::filters::BiquadStatic<float> tapFiltersL[kTapsPerChannel];
    signalsmith::filters::BiquadStatic<float> tapFiltersR[kTapsPerChannel];

    // Smoothed parameters
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> roomSizeSmoothed;   // 50ms ramp
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> proximitySmoothed;  // 20ms ramp
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> widthSmoothed;      // 20ms ramp
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassBlend;        // 10ms ramp

    // Shape crossfade state (30ms crossfade between shapes)
    int currentShapeIndex = 0;
    int pendingShapeIndex = -1;
    float shapeCrossfade = 1.0f;     // 1.0 = fully on current shape
    float shapeCrossfadeStep = 0.0f; // per-sample step for crossfade

    // State
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    bool bypassed = false;

    // Cross-stage: Air darkening factor (set from PluginProcessor)
    float airDarkeningFactor = 0.0f;
};
