#include "ReflectionsSection.h"
#include <cmath>

// =============================================================================
// 7 Room Geometry Shape Presets
// =============================================================================
// Each shape produces a maximally different reflection pattern.
// Delay times in ms (base values at Room Size 1.0, scaled linearly for smaller).
// Gains normalized per channel. Pan: -1.0 = hard left, 1.0 = hard right.

const ShapePreset ReflectionsSection::kShapes[ReflectionsSection::kNumShapes] =
{
    // 0: "The Parlour" -- Small symmetric room, regular even spacing
    // Rectangular room, ~4x4m. Short, evenly-spaced reflections.
    // Even tap spacing = regular flutter pattern, warm and intimate.
    {
        "The Parlour",
        "A modest, symmetric chamber of even proportion -- "
        "warm and intimate, with reflections arriving in orderly measure",
        // L taps: evenly spaced
        { 2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.5f, 15.0f, 18.0f },
        { 0.85f, 0.72f, 0.60f, 0.50f, 0.40f, 0.32f, 0.25f, 0.18f },
        { -0.3f, 0.2f, -0.4f, 0.3f, -0.2f, 0.4f, -0.3f, 0.2f },
        // R taps: offset by ~1ms
        { 2.5f, 4.5f, 6.5f, 8.5f, 10.5f, 13.0f, 15.5f, 18.5f },
        { 0.82f, 0.70f, 0.58f, 0.48f, 0.38f, 0.30f, 0.23f, 0.16f },
        { 0.3f, -0.2f, 0.4f, -0.3f, 0.2f, -0.4f, 0.3f, -0.2f },
        0.15f,   // absorption: moderate
        0.3f,    // tail diffusion bias: low (regular, not dense)
        0.2f     // tail modal character: even modes
    },

    // 1: "The Gallery" -- Long rectangular, strong lateral reflections
    // Long narrow room, ~12x4m. First reflections from side walls are early;
    // end-wall reflections are late. Creates left-right width.
    {
        "The Gallery",
        "A long, narrow hall of distinguished proportion -- "
        "sound rebounds between close walls before reaching the distant end",
        // L taps: clustered early (side walls) then sparse late (end walls)
        { 1.5f, 2.8f, 3.5f, 5.0f, 9.0f, 14.0f, 20.0f, 27.0f },
        { 0.90f, 0.78f, 0.65f, 0.55f, 0.35f, 0.25f, 0.18f, 0.12f },
        { -0.8f, 0.7f, -0.6f, 0.5f, -0.3f, 0.2f, -0.1f, 0.05f },
        // R taps
        { 1.8f, 3.2f, 4.0f, 5.5f, 9.5f, 14.5f, 20.5f, 27.5f },
        { 0.88f, 0.75f, 0.62f, 0.52f, 0.33f, 0.23f, 0.16f, 0.10f },
        { 0.8f, -0.7f, 0.6f, -0.5f, 0.3f, -0.2f, 0.1f, -0.05f },
        0.12f,
        0.4f,
        0.35f
    },

    // 2: "The Chamber" -- Medium square, moderate density
    // Square room ~6x6m. Reflections from all four walls arrive at similar times,
    // creating a dense cluster. Classic studio live room feel.
    {
        "The Chamber",
        "A square room of generous dimension -- "
        "reflections converge from all quarters in a rich, even chorus",
        // L taps: clustered in the 3-8ms range (similar wall distances)
        { 3.0f, 3.8f, 4.5f, 5.2f, 6.0f, 7.5f, 9.5f, 13.0f },
        { 0.80f, 0.75f, 0.70f, 0.65f, 0.55f, 0.42f, 0.30f, 0.20f },
        { -0.5f, 0.4f, -0.3f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f },
        // R taps
        { 3.3f, 4.2f, 5.0f, 5.7f, 6.5f, 8.0f, 10.0f, 13.5f },
        { 0.78f, 0.72f, 0.67f, 0.62f, 0.52f, 0.40f, 0.28f, 0.18f },
        { 0.5f, -0.4f, 0.3f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f },
        0.18f,
        0.5f,    // moderate tail diffusion
        0.45f    // slight mode clustering (square room modes)
    },

    // 3: "The Nave" -- Large cathedral-like, very long reverb path
    // Tall, long space ~20x8m with high ceiling. Reflections from
    // floor/ceiling arrive first, wall reflections spread across time.
    {
        "The Nave",
        "A vast vaulted space of soaring height -- "
        "sound ascends to the ceiling before cascading down in waves of grandeur",
        // L taps: floor/ceiling early, walls spread late
        { 1.0f, 2.5f, 5.0f, 8.5f, 13.0f, 18.0f, 24.0f, 30.0f },
        { 0.70f, 0.82f, 0.60f, 0.50f, 0.40f, 0.30f, 0.22f, 0.15f },
        { -0.2f, 0.1f, -0.6f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f },
        // R taps
        { 1.3f, 3.0f, 5.5f, 9.0f, 13.5f, 18.5f, 24.5f, 30.0f },
        { 0.68f, 0.80f, 0.58f, 0.48f, 0.38f, 0.28f, 0.20f, 0.13f },
        { 0.2f, -0.1f, 0.6f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f },
        0.08f,   // low absorption (stone/plaster surfaces)
        0.7f,    // high tail diffusion
        0.6f     // clustered modes (resonant space)
    },

    // 4: "The Alcove" -- Asymmetric, irregular angles
    // Non-rectangular room with alcoves and odd angles. Reflections
    // arrive in unpredictable clusters. Complex, lively character.
    {
        "The Alcove",
        "An irregular chamber of unexpected angles and hidden recesses -- "
        "sound wanders through asymmetric paths, alive with curious texture",
        // L taps: irregular spacing (asymmetric walls, alcoves)
        { 1.2f, 2.7f, 3.1f, 5.8f, 7.2f, 11.0f, 16.5f, 22.0f },
        { 0.75f, 0.68f, 0.80f, 0.45f, 0.55f, 0.35f, 0.28f, 0.15f },
        { -0.7f, 0.3f, -0.1f, 0.8f, -0.5f, 0.6f, -0.4f, 0.2f },
        // R taps: very different pattern (asymmetric room)
        { 1.8f, 2.2f, 4.5f, 6.0f, 8.5f, 12.5f, 17.0f, 23.5f },
        { 0.72f, 0.80f, 0.55f, 0.62f, 0.40f, 0.30f, 0.22f, 0.12f },
        { 0.5f, -0.6f, 0.8f, -0.2f, 0.4f, -0.7f, 0.3f, -0.5f },
        0.14f,
        0.55f,
        0.5f     // moderate mode clustering (irregular shapes scatter modes)
    },

    // 5: "The Crypt" -- Small, dense, stone-like
    // Low ceiling, irregular stone walls. Very dense early reflections
    // with rapid buildup. Dark character from stone absorption.
    {
        "The Crypt",
        "A low vault of ancient stone -- "
        "close walls conspire to multiply each sound into a dense, dark murmur",
        // L taps: very dense, short delays (low ceiling, close walls)
        { 0.8f, 1.5f, 2.0f, 2.8f, 3.5f, 4.8f, 6.5f, 9.0f },
        { 0.90f, 0.85f, 0.80f, 0.72f, 0.65f, 0.50f, 0.38f, 0.25f },
        { -0.4f, 0.3f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f, 0.3f },
        // R taps
        { 1.0f, 1.8f, 2.3f, 3.2f, 4.0f, 5.2f, 7.0f, 9.5f },
        { 0.88f, 0.82f, 0.77f, 0.70f, 0.62f, 0.48f, 0.35f, 0.22f },
        { 0.4f, -0.3f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f, -0.3f },
        0.22f,   // high absorption (rough stone)
        0.8f,    // very dense tail
        0.7f     // strong mode clustering
    },

    // 6: "The Conservatory" -- Bright, glass-like, wide
    // Large room with hard, reflective surfaces. Reflections are bright
    // and widely spread. Open, airy character.
    {
        "The Conservatory",
        "A luminous glass pavilion of generous span -- "
        "bright reflections dance between crystalline surfaces with brilliant clarity",
        // L taps: wide spread, moderate density
        { 2.0f, 4.5f, 7.0f, 10.0f, 14.0f, 18.5f, 23.0f, 28.0f },
        { 0.88f, 0.75f, 0.65f, 0.55f, 0.45f, 0.35f, 0.25f, 0.18f },
        { -0.9f, 0.8f, -0.7f, 0.6f, -0.5f, 0.7f, -0.6f, 0.5f },
        // R taps
        { 2.5f, 5.0f, 7.5f, 10.5f, 14.5f, 19.0f, 23.5f, 28.5f },
        { 0.85f, 0.72f, 0.62f, 0.52f, 0.42f, 0.32f, 0.23f, 0.15f },
        { 0.9f, -0.8f, 0.7f, -0.6f, 0.5f, -0.7f, 0.6f, -0.5f },
        0.05f,   // very low absorption (glass/hard surfaces)
        0.35f,   // moderate tail diffusion
        0.15f    // even modes (regular geometry)
    }
};

// =============================================================================
// roomSizeToDelayMs -- Maps normalized 0-1 knob value to delay time in ms
// =============================================================================
// Room Size 0.0 -> 1ms base delay, Room Size 1.0 -> 30ms base delay.
// The perceptual weighting is already in the NormalisableRange skew (0.4).

float ReflectionsSection::roomSizeToDelayMs (float normalized)
{
    return 1.0f + normalized * 29.0f;  // Linear 1-30ms
}

// =============================================================================
// prepare
// =============================================================================

void ReflectionsSection::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Size delay lines for max 35ms (30ms + margin)
    int maxSamples = static_cast<int> (std::ceil (35.0 * sampleRate / 1000.0));

    delayLineL.resize (maxSamples + 1);
    delayLineL.reset();
    delayLineR.resize (maxSamples + 1);
    delayLineR.reset();

    // Reset all 16 tap filters
    for (int t = 0; t < kTapsPerChannel; ++t)
    {
        tapFiltersL[t].reset();
        tapFiltersR[t].reset();
    }

    // Initialize SmoothedValues
    roomSizeSmoothed.reset (sampleRate, 0.050);    // 50ms ramp for delay changes
    proximitySmoothed.reset (sampleRate, 0.020);    // 20ms ramp
    widthSmoothed.reset (sampleRate, 0.020);         // 20ms ramp
    bypassBlend.reset (sampleRate, 0.010);           // 10ms bypass crossfade

    bypassBlend.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);

    // Initialize shape crossfade state
    pendingShapeIndex = -1;
    shapeCrossfade = 1.0f;
    // 30ms crossfade: step per sample
    shapeCrossfadeStep = static_cast<float> (1.0 / (0.030 * sampleRate));
}

// =============================================================================
// process -- Per-sample stereo tapped delay line processing
// =============================================================================

void ReflectionsSection::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 1 || numSamples < 1)
        return;

    // Early exit: if fully bypassed and not smoothing, skip
    if (! bypassBlend.isSmoothing() && bypassBlend.getTargetValue() <= 0.0f)
        return;

    float* channelL = buffer.getWritePointer (0);
    float* channelR = (numChannels >= 2) ? buffer.getWritePointer (1) : nullptr;

    const float sr = static_cast<float> (currentSampleRate);
    const float minDelaySamples = sr * 0.001f;  // 1ms floor at any sample rate

    for (int s = 0; s < numSamples; ++s)
    {
        // a. Read smoothed parameter values
        float roomSize = roomSizeSmoothed.getNextValue();
        float proximity = proximitySmoothed.getNextValue();
        float widthNorm = widthSmoothed.getNextValue();
        float blend = bypassBlend.getNextValue();

        // b. Compute delay multiplier from Room Size
        float delayMs = roomSizeToDelayMs (roomSize);

        // Subtle room size darkening: larger rooms get darker absorption filters
        // baseCutoff = 12kHz at min size, 6kHz at max size
        // Cross-stage: Air darkening further reduces cutoff (up to 3600Hz at max)
        float airCutoffReduction = airDarkeningFactor * 3000.0f;
        float baseCutoff = 12000.0f - roomSize * 6000.0f - airCutoffReduction;
        baseCutoff = juce::jmax (2000.0f, baseCutoff);

        // c. Get input samples
        float inputL = channelL[s];
        float inputR = (channelR != nullptr) ? channelR[s] : inputL;

        // d. Write input samples to delay buffers
        delayLineL.write (inputL);
        delayLineR.write (inputR);

        // e. Accumulate taps from shapes
        float sumL = 0.0f;
        float sumR = 0.0f;

        // Shape crossfade: determine blend gains
        float currentGain = 1.0f;
        float pendingGain = 0.0f;

        if (pendingShapeIndex >= 0)
        {
            // Shape crossfade in progress
            shapeCrossfade -= shapeCrossfadeStep;
            if (shapeCrossfade <= 0.0f)
            {
                // Crossfade complete
                currentShapeIndex = pendingShapeIndex;
                pendingShapeIndex = -1;
                shapeCrossfade = 1.0f;
                currentGain = 1.0f;
                pendingGain = 0.0f;
            }
            else
            {
                currentGain = shapeCrossfade;
                pendingGain = 1.0f - shapeCrossfade;
            }
        }

        // Accumulate taps from current shape
        {
            const ShapePreset& sh = kShapes[currentShapeIndex];
            for (int t = 0; t < kTapsPerChannel; ++t)
            {
                // Width interpolation for delay times
                float monoDelay = (sh.delayMsL[t] + sh.delayMsR[t]) * 0.5f;
                float lDMs = monoDelay + widthNorm * (sh.delayMsL[t] - monoDelay);
                float rDMs = monoDelay + widthNorm * (sh.delayMsR[t] - monoDelay);

                // Scale by room size
                lDMs = lDMs * delayMs / 30.0f;
                rDMs = rDMs * delayMs / 30.0f;

                float lSamp = juce::jmax (minDelaySamples, lDMs * sr / 1000.0f);
                float rSamp = juce::jmax (minDelaySamples, rDMs * sr / 1000.0f);

                float tapL = delayLineL.read (lSamp);
                float tapR = delayLineR.read (rSamp);

                // Per-tap absorption filter
                float cutL = juce::jlimit (1000.0f, 16000.0f,
                    baseCutoff - sh.absorptionRate * lDMs * 200.0f);
                float cutR = juce::jlimit (1000.0f, 16000.0f,
                    baseCutoff - sh.absorptionRate * rDMs * 200.0f);

                // Only update filter coefficients for current shape (not during crossfade)
                if (pendingShapeIndex < 0)
                {
                    tapFiltersL[t].lowpass (cutL / sr);
                    tapFiltersR[t].lowpass (cutR / sr);
                }

                tapL = tapFiltersL[t] (tapL);
                tapR = tapFiltersR[t] (tapR);

                // Width interpolation for gains
                float monoGain = (sh.gainL[t] + sh.gainR[t]) * 0.5f;
                float gL = monoGain + widthNorm * (sh.gainL[t] - monoGain);
                float gR = monoGain + widthNorm * (sh.gainR[t] - monoGain);
                tapL *= gL;
                tapR *= gR;

                // Width interpolation for pan
                float monoPan = (sh.panL[t] + sh.panR[t]) * 0.5f;
                float pL = monoPan + widthNorm * (sh.panL[t] - monoPan);
                float pR = monoPan + widthNorm * (sh.panR[t] - monoPan);

                // Linear pan law
                float pLL = 0.5f * (1.0f - pL);
                float pLR = 0.5f * (1.0f + pL);
                float pRL = 0.5f * (1.0f - pR);
                float pRR = 0.5f * (1.0f + pR);

                sumL += (tapL * pLL + tapR * pRL) * currentGain;
                sumR += (tapL * pLR + tapR * pRR) * currentGain;
            }
        }

        // Accumulate taps from pending shape (during crossfade)
        if (pendingShapeIndex >= 0 && pendingGain > 0.0f)
        {
            const ShapePreset& sh = kShapes[pendingShapeIndex];
            for (int t = 0; t < kTapsPerChannel; ++t)
            {
                float monoDelay = (sh.delayMsL[t] + sh.delayMsR[t]) * 0.5f;
                float lDMs = monoDelay + widthNorm * (sh.delayMsL[t] - monoDelay);
                float rDMs = monoDelay + widthNorm * (sh.delayMsR[t] - monoDelay);

                lDMs = lDMs * delayMs / 30.0f;
                rDMs = rDMs * delayMs / 30.0f;

                float lSamp = juce::jmax (minDelaySamples, lDMs * sr / 1000.0f);
                float rSamp = juce::jmax (minDelaySamples, rDMs * sr / 1000.0f);

                float tapL = delayLineL.read (lSamp);
                float tapR = delayLineR.read (rSamp);

                // Use unfiltered taps during crossfade to avoid filter coefficient fights
                // The crossfade is short (30ms) so this is inaudible

                float monoGain = (sh.gainL[t] + sh.gainR[t]) * 0.5f;
                float gL = monoGain + widthNorm * (sh.gainL[t] - monoGain);
                float gR = monoGain + widthNorm * (sh.gainR[t] - monoGain);
                tapL *= gL;
                tapR *= gR;

                float monoPan = (sh.panL[t] + sh.panR[t]) * 0.5f;
                float pL = monoPan + widthNorm * (sh.panL[t] - monoPan);
                float pR = monoPan + widthNorm * (sh.panR[t] - monoPan);

                float pLL = 0.5f * (1.0f - pL);
                float pLR = 0.5f * (1.0f + pL);
                float pRL = 0.5f * (1.0f - pR);
                float pRR = 0.5f * (1.0f + pR);

                sumL += (tapL * pLL + tapR * pRL) * pendingGain;
                sumR += (tapL * pLR + tapR * pRR) * pendingGain;
            }
        }

        // f. Apply proximity blend
        // Near (0.0): direct at 0dB, reflected at -18dB
        // Far (1.0): direct at -12dB, reflected at 0dB
        static constexpr float kMinus12dB = 0.251189f; // pow(10, -12/20)
        static constexpr float kMinus18dB = 0.125893f; // pow(10, -18/20)
        float directGain = 1.0f - proximity * (1.0f - kMinus12dB);
        float reflGain = kMinus18dB + proximity * (1.0f - kMinus18dB);

        float outputL = inputL * directGain + sumL * reflGain;
        float outputR = inputR * directGain + sumR * reflGain;

        // g. Apply bypass crossfade
        float finalL = inputL * (1.0f - blend) + outputL * blend;
        float finalR = inputR * (1.0f - blend) + outputR * blend;

        // h. Write output
        channelL[s] = finalL;
        if (channelR != nullptr)
            channelR[s] = finalR;
    }
}

// =============================================================================
// setRoomSize
// =============================================================================

void ReflectionsSection::setRoomSize (float size)
{
    roomSizeSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, size));
}

// =============================================================================
// setShape -- triggers 30ms crossfade to new shape
// =============================================================================

void ReflectionsSection::setShape (int shapeIndex)
{
    shapeIndex = juce::jlimit (0, kNumShapes - 1, shapeIndex);

    // Only trigger crossfade if actually changing
    if (shapeIndex != currentShapeIndex && shapeIndex != pendingShapeIndex)
    {
        pendingShapeIndex = shapeIndex;
        shapeCrossfade = 1.0f;
    }
}

// =============================================================================
// setProximity
// =============================================================================

void ReflectionsSection::setProximity (float proximity)
{
    proximitySmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, proximity));
}

// =============================================================================
// setWidth
// =============================================================================

void ReflectionsSection::setWidth (float width)
{
    widthSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, width));
}

// =============================================================================
// setAirDarkening -- Cross-stage coupling from Air Amount * Character scale
// =============================================================================

void ReflectionsSection::setAirDarkening (float darkening)
{
    airDarkeningFactor = juce::jlimit (0.0f, 2.0f, darkening);
}

// =============================================================================
// setBypass
// =============================================================================

void ReflectionsSection::setBypass (bool shouldBypass)
{
    bypassed = shouldBypass;
    bypassBlend.setTargetValue (shouldBypass ? 0.0f : 1.0f);
}

// =============================================================================
// getShapePreset
// =============================================================================

const ShapePreset& ReflectionsSection::getShapePreset (int index) const
{
    return kShapes[juce::jlimit (0, kNumShapes - 1, index)];
}

// =============================================================================
// reset
// =============================================================================

void ReflectionsSection::reset()
{
    delayLineL.reset();
    delayLineR.reset();

    for (int t = 0; t < kTapsPerChannel; ++t)
    {
        tapFiltersL[t].reset();
        tapFiltersR[t].reset();
    }

    roomSizeSmoothed.setCurrentAndTargetValue (0.4f);
    proximitySmoothed.setCurrentAndTargetValue (0.3f);
    widthSmoothed.setCurrentAndTargetValue (0.7f);
    bypassBlend.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);

    pendingShapeIndex = -1;
    shapeCrossfade = 1.0f;
}
