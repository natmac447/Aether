#include "RoomToneSection.h"

void RoomToneSection::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;

    // Reset Kellett filters
    pinkL.reset();
    pinkR.reset();

    // Set up spectral shaping at default Room Size / Shape
    updateShapingFilters();

    // SmoothedValues
    ambienceSmoothed.reset (sampleRate, 0.025);  // 25ms ramp
    ambienceSmoothed.setCurrentAndTargetValue (0.1f);

    bypassBlend.reset (sampleRate, 0.010);  // 10ms ramp
    bypassBlend.setCurrentAndTargetValue (0.0f);  // default bypassed

    // Manual one-pole gate envelope coefficients (asymmetric attack/release)
    gateAttackCoeff  = 1.0f - std::exp (-1.0f / (static_cast<float> (sampleRate) * kGateAttackMs * 0.001f));
    gateReleaseCoeff = 1.0f - std::exp (-1.0f / (static_cast<float> (sampleRate) * kGateReleaseMs * 0.001f));
    gateLevel = 0.0f;

    // Reset LFO
    lfoPhase = 0.0f;
}

void RoomToneSection::updateShapingFilters()
{
    const float sr = static_cast<float> (currentSampleRate);

    // Room Size (0.0-1.0) shifts filter parameters:
    // HPF cutoff: 100Hz at roomSize=0.0 -> 60Hz at roomSize=1.0
    float hpfFreq = 100.0f - 40.0f * currentRoomSize;
    // Presence peak freq: 400Hz at small -> 200Hz at large
    float peakFreq = 400.0f - 200.0f * currentRoomSize;
    // Presence peak gain: +4dB base
    float peakGainDb = 4.0f;
    // LPF cutoff: 10000Hz at small -> 6000Hz at large
    float lpfFreq = 10000.0f - 4000.0f * currentRoomSize;

    // Apply Shape offsets from kShapeAmbience lookup table
    if (currentShapeIndex >= 0 && currentShapeIndex < 7)
    {
        const auto& shapeChar = kShapeAmbience[currentShapeIndex];
        peakFreq   += shapeChar.presenceFreqOffset;
        lpfFreq    += shapeChar.lpfCutoffOffset;
        peakGainDb += shapeChar.presenceGainDb;
    }

    // Clamp to safe ranges
    peakFreq   = juce::jlimit (100.0f, 800.0f, peakFreq);
    lpfFreq    = juce::jlimit (4000.0f, 12000.0f, lpfFreq);
    peakGainDb = juce::jlimit (1.0f, 7.0f, peakGainDb);

    // Configure BiquadStatic filters using Signalsmith API
    // All take normalized frequency (freq / sampleRate)
    hpfL.highpassQ (static_cast<double> (hpfFreq / sr), 0.707);
    hpfR.highpassQ (static_cast<double> (hpfFreq / sr), 0.707);

    presenceL.peakDbQ (static_cast<double> (peakFreq / sr), static_cast<double> (peakGainDb), 0.8);
    presenceR.peakDbQ (static_cast<double> (peakFreq / sr), static_cast<double> (peakGainDb), 0.8);

    lpfL.lowpassQ (static_cast<double> (lpfFreq / sr), 0.707);
    lpfR.lowpassQ (static_cast<double> (lpfFreq / sr), 0.707);
}

void RoomToneSection::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numSamples == 0)
        return;

    // TONE-07: When bypassed and not smoothing, skip noise generation entirely for CPU savings
    if (bypassed && ! bypassBlend.isSmoothing())
        return;

    float* channelL = buffer.getWritePointer (0);
    float* channelR = numChannels >= 2 ? buffer.getWritePointer (1) : nullptr;

    const float invSampleRate = 1.0f / static_cast<float> (currentSampleRate);

    for (int s = 0; s < numSamples; ++s)
    {
        const float blend    = bypassBlend.getNextValue();
        const float ambience = ambienceSmoothed.getNextValue();

        // Compute gate target based on mode
        float gateTarget = 1.0f;
        if (gateMode == GateMode::TransportOnly && ! transportPlaying)
        {
            gateTarget = 0.0f;
        }
        else if (gateMode == GateMode::SignalGated)
        {
            // Measure input level from current sample
            float inputPower = channelL[s] * channelL[s];
            if (channelR != nullptr)
                inputPower = (inputPower + channelR[s] * channelR[s]) * 0.5f;

            float inputDb = juce::Decibels::gainToDecibels (std::sqrt (inputPower), -100.0f);
            gateTarget = (inputDb > kGateThresholdDb) ? 1.0f : 0.0f;
        }

        // Asymmetric one-pole envelope follower
        float coeff = (gateTarget > gateLevel) ? gateAttackCoeff : gateReleaseCoeff;
        gateLevel += coeff * (gateTarget - gateLevel);

        // Generate decorrelated white noise
        float whiteL = rngL.nextFloat() * 2.0f - 1.0f;
        float whiteR = rngR.nextFloat() * 2.0f - 1.0f;

        // Pink via Kellett IIR
        float pinkSampleL = pinkL.process (whiteL);
        float pinkSampleR = pinkR.process (whiteR);

        // Spectral shaping: HPF -> Presence -> LPF
        float shapedL = lpfL (presenceL (hpfL (pinkSampleL)));
        float shapedR = lpfR (presenceR (hpfR (pinkSampleR)));

        // Slow LFO modulation for natural quality
        float lfoValue = std::sin (lfoPhase * 2.0f * juce::MathConstants<float>::pi);
        float modGain  = juce::Decibels::decibelsToGain (lfoValue * kLfoDepthDb, -100.0f);
        lfoPhase += kLfoRateHz * invSampleRate;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;

        // Level calculation: map ambience 0-1 to gain
        // At ambience=0: silence. At ambience=1: kMaxNoiseGainDb (-35dB)
        // Use a curve that gives useful range across the knob
        float noiseGain = (ambience > 0.001f)
            ? juce::Decibels::decibelsToGain (kMaxNoiseGainDb + (1.0f - ambience) * (-60.0f - kMaxNoiseGainDb), -100.0f)
            : 0.0f;
        // This maps: ambience=0 -> -inf (silence), ambience=0.5 -> ~-47.5dB, ambience=1.0 -> -35dB

        float totalGain = noiseGain * gateLevel * modGain * blend;

        // ADDITIVE: noise added to signal (not replacing)
        channelL[s] += shapedL * totalGain;
        if (channelR != nullptr)
            channelR[s] += shapedR * totalGain;
    }
}

void RoomToneSection::reset()
{
    // Reset Kellett pink noise filters
    pinkL.reset();
    pinkR.reset();

    // Reset biquad spectral shaping filters
    hpfL.reset();
    hpfR.reset();
    presenceL.reset();
    presenceR.reset();
    lpfL.reset();
    lpfR.reset();

    // Re-seed RNG (constructor seeds are preserved by juce::Random)
    rngL = juce::Random (42);
    rngR = juce::Random (12345679);

    // Reset SmoothedValues
    ambienceSmoothed.setCurrentAndTargetValue (0.1f);
    bypassBlend.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);

    // Reset gate envelope and LFO
    gateLevel = 0.0f;
    lfoPhase  = 0.0f;
}

void RoomToneSection::setBypass (bool shouldBypass)
{
    bypassed = shouldBypass;
    bypassBlend.setTargetValue (shouldBypass ? 0.0f : 1.0f);
}

void RoomToneSection::setAmbience (float ambienceNormalized)
{
    ambienceSmoothed.setTargetValue (ambienceNormalized);
}

void RoomToneSection::setRoomSize (float roomSizeNormalized)
{
    if (std::abs (currentRoomSize - roomSizeNormalized) > 0.001f)
    {
        currentRoomSize = roomSizeNormalized;
        updateShapingFilters();
    }
}

void RoomToneSection::setShape (int shapeIndex)
{
    if (currentShapeIndex != shapeIndex)
    {
        currentShapeIndex = shapeIndex;
        updateShapingFilters();
    }
}

void RoomToneSection::setGateMode (int gateModeIndex)
{
    gateMode = static_cast<GateMode> (juce::jlimit (0, 2, gateModeIndex));
}

void RoomToneSection::setTransportPlaying (bool isPlaying)
{
    transportPlaying = isPlaying;
}
