#include "AirSection.h"
#include <cmath>

// =============================================================================
// prepare
// =============================================================================

void AirSection::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;

    // 25ms ramp for Air Amount changes
    airSmoothed.reset (sampleRate, 0.025);
    airSmoothed.setCurrentAndTargetValue (0.4f);  // match default Air Amount

    // 10ms bypass crossfade
    bypassBlend.reset (sampleRate, 0.010);
    bypassBlend.setCurrentAndTargetValue (1.0f);  // active (not bypassed)

    // Character state
    currentCharIndex  = 1;  // Neutral default
    pendingCharIndex  = -1;
    charCrossfade     = 1.0f;
    charCrossfadeStep = 1.0f / (0.030f * static_cast<float> (sampleRate));  // 30ms

    currentAirSmoothed = 0.4f;

    // Set initial filter coefficients
    updateFilters();

    // Reset all filter states
    hfShelfL.reset();   hfShelfR.reset();
    lfShelfL.reset();   lfShelfR.reset();

    for (int i = 0; i < kMaxAllpassStages; ++i)
    {
        allpassL[i].reset();
        allpassR[i].reset();
    }

    pendHfShelfL.reset();   pendHfShelfR.reset();
    pendLfShelfL.reset();   pendLfShelfR.reset();

    for (int i = 0; i < kMaxAllpassStages; ++i)
    {
        pendAllpassL[i].reset();
        pendAllpassR[i].reset();
    }
}

// =============================================================================
// reset
// =============================================================================

void AirSection::reset()
{
    hfShelfL.reset();   hfShelfR.reset();
    lfShelfL.reset();   lfShelfR.reset();

    for (int i = 0; i < kMaxAllpassStages; ++i)
    {
        allpassL[i].reset();
        allpassR[i].reset();
    }

    pendHfShelfL.reset();   pendHfShelfR.reset();
    pendLfShelfL.reset();   pendLfShelfR.reset();

    for (int i = 0; i < kMaxAllpassStages; ++i)
    {
        pendAllpassL[i].reset();
        pendAllpassR[i].reset();
    }

    airSmoothed.setCurrentAndTargetValue (0.4f);
    bypassBlend.setCurrentAndTargetValue (1.0f);
}

// =============================================================================
// setBypass
// =============================================================================

void AirSection::setBypass (bool shouldBypass)
{
    bypassBlend.setTargetValue (shouldBypass ? 0.0f : 1.0f);
}

// =============================================================================
// setAmount
// =============================================================================

void AirSection::setAmount (float airAmount)
{
    airSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, airAmount));
}

// =============================================================================
// setCharacter
// =============================================================================

void AirSection::setCharacter (int index)
{
    index = juce::jlimit (0, kNumCharacters - 1, index);

    if (index == currentCharIndex)
        return;

    if (pendingCharIndex >= 0 && index == pendingCharIndex)
        return;

    // If already crossfading, snap current to pending immediately
    if (pendingCharIndex >= 0)
    {
        currentCharIndex = pendingCharIndex;
        updateFilters();
    }

    pendingCharIndex = index;
    charCrossfade    = 1.0f;

    updatePendingFilters();

    // Reset pending filter states for clean crossfade
    pendHfShelfL.reset();   pendHfShelfR.reset();
    pendLfShelfL.reset();   pendLfShelfR.reset();

    for (int i = 0; i < kMaxAllpassStages; ++i)
    {
        pendAllpassL[i].reset();
        pendAllpassR[i].reset();
    }
}

// =============================================================================
// updateFilters
// =============================================================================

void AirSection::updateFilters()
{
    const auto& ch = kCharacterPresets[currentCharIndex];
    float sr  = static_cast<float> (currentSampleRate);
    float air = currentAirSmoothed;

    // HF absorption shelf: interpolate gain between min and max
    float shelfDb       = ch.minShelfDb + air * (ch.maxShelfDb - ch.minShelfDb);
    float scaledFreq    = ch.shelfFreqHz / sr;

    hfShelfL.highShelfDb (scaledFreq, shelfDb, ch.shelfOctaves);
    hfShelfR.highShelfDb (scaledFreq, shelfDb, ch.shelfOctaves);

    // Optional LF character shelf
    if (ch.lfShelfFreqHz > 0.0f)
    {
        float lfDb       = ch.lfMinShelfDb + air * (ch.lfMaxShelfDb - ch.lfMinShelfDb);
        float scaledLf   = ch.lfShelfFreqHz / sr;
        lfShelfL.lowShelfDb (scaledLf, lfDb);
        lfShelfR.lowShelfDb (scaledLf, lfDb);
    }

    // Allpass diffusion: scale Q with Air amount
    float q = ch.allpassMinQ + air * (ch.allpassMaxQ - ch.allpassMinQ);

    for (int i = 0; i < ch.allpassStages; ++i)
    {
        // Logarithmically spaced center frequencies
        float t      = (ch.allpassStages > 1)
                         ? static_cast<float> (i) / static_cast<float> (ch.allpassStages - 1)
                         : 0.0f;
        float freqHz = ch.allpassBaseFreqHz * std::pow (2.0f, t * ch.allpassSpreadOctaves);
        float sf     = freqHz / sr;

        allpassL[i].allpassQ (sf, q);
        // R channel offset by ~12% for stereo decorrelation
        allpassR[i].allpassQ (sf * 1.12f, q);
    }
}

// =============================================================================
// updatePendingFilters
// =============================================================================

void AirSection::updatePendingFilters()
{
    if (pendingCharIndex < 0)
        return;

    const auto& ch = kCharacterPresets[pendingCharIndex];
    float sr  = static_cast<float> (currentSampleRate);
    float air = currentAirSmoothed;

    // HF shelf
    float shelfDb    = ch.minShelfDb + air * (ch.maxShelfDb - ch.minShelfDb);
    float scaledFreq = ch.shelfFreqHz / sr;

    pendHfShelfL.highShelfDb (scaledFreq, shelfDb, ch.shelfOctaves);
    pendHfShelfR.highShelfDb (scaledFreq, shelfDb, ch.shelfOctaves);

    // LF shelf
    if (ch.lfShelfFreqHz > 0.0f)
    {
        float lfDb     = ch.lfMinShelfDb + air * (ch.lfMaxShelfDb - ch.lfMinShelfDb);
        float scaledLf = ch.lfShelfFreqHz / sr;
        pendLfShelfL.lowShelfDb (scaledLf, lfDb);
        pendLfShelfR.lowShelfDb (scaledLf, lfDb);
    }

    // Allpass
    float q = ch.allpassMinQ + air * (ch.allpassMaxQ - ch.allpassMinQ);

    for (int i = 0; i < ch.allpassStages; ++i)
    {
        float t      = (ch.allpassStages > 1)
                         ? static_cast<float> (i) / static_cast<float> (ch.allpassStages - 1)
                         : 0.0f;
        float freqHz = ch.allpassBaseFreqHz * std::pow (2.0f, t * ch.allpassSpreadOctaves);
        float sf     = freqHz / sr;

        pendAllpassL[i].allpassQ (sf, q);
        pendAllpassR[i].allpassQ (sf * 1.12f, q);
    }
}

// =============================================================================
// process
// =============================================================================

void AirSection::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 1 || numSamples < 1)
        return;

    // Early exit: fully bypassed and not smoothing
    if (! bypassBlend.isSmoothing() && bypassBlend.getTargetValue() <= 0.0f)
        return;

    float* channelL = buffer.getWritePointer (0);
    float* channelR = (numChannels >= 2) ? buffer.getWritePointer (1) : nullptr;

    bool airSmoothing = airSmoothed.isSmoothing();

    for (int s = 0; s < numSamples; ++s)
    {
        float blend = bypassBlend.getNextValue();

        // Update air smoothing and filters when parameter is ramping
        if (airSmoothing)
        {
            currentAirSmoothed = airSmoothed.getNextValue();

            // Recalculate coefficients every 16 samples to balance CPU vs smoothness
            if ((s & 15) == 0)
            {
                updateFilters();

                if (pendingCharIndex >= 0)
                    updatePendingFilters();
            }
        }

        float inL = channelL[s];
        float inR = (channelR != nullptr) ? channelR[s] : inL;

        // -----------------------------------------------------------------
        // Process through current character filters
        // -----------------------------------------------------------------

        // a. HF absorption shelf
        float outL = hfShelfL (inL);
        float outR = hfShelfR (inR);

        // b. LF character shelf (if active for current character)
        if (kCharacterPresets[currentCharIndex].lfShelfFreqHz > 0.0f)
        {
            outL = lfShelfL (outL);
            outR = lfShelfR (outR);
        }

        // c. Allpass phase smearing cascade
        int stages = kCharacterPresets[currentCharIndex].allpassStages;
        for (int i = 0; i < stages; ++i)
        {
            outL = allpassL[i] (outL);
            outR = allpassR[i] (outR);
        }

        // -----------------------------------------------------------------
        // Character crossfade (if switching characters)
        // -----------------------------------------------------------------
        if (pendingCharIndex >= 0)
        {
            // Process input through pending character filters
            float pendOutL = pendHfShelfL (inL);
            float pendOutR = pendHfShelfR (inR);

            if (kCharacterPresets[pendingCharIndex].lfShelfFreqHz > 0.0f)
            {
                pendOutL = pendLfShelfL (pendOutL);
                pendOutR = pendLfShelfR (pendOutR);
            }

            int pendStages = kCharacterPresets[pendingCharIndex].allpassStages;
            for (int i = 0; i < pendStages; ++i)
            {
                pendOutL = pendAllpassL[i] (pendOutL);
                pendOutR = pendAllpassR[i] (pendOutR);
            }

            // Advance crossfade: charCrossfade goes from 1.0 -> 0.0
            charCrossfade -= charCrossfadeStep;

            // cheapEnergyCrossfade: x=0 -> toCoeff=0,fromCoeff=1; x=1 -> toCoeff=1,fromCoeff=0
            // We want: as charCrossfade goes from 1.0 to 0.0, blend from current to pending.
            // So x = (1.0 - charCrossfade) gives: at start x=0 (all current), at end x=1 (all pending).
            float pendGain = 0.0f, currGain = 0.0f;
            float xfade = juce::jlimit (0.0f, 1.0f, 1.0f - charCrossfade);
            signalsmith::mix::cheapEnergyCrossfade (xfade, pendGain, currGain);

            outL = outL * currGain + pendOutL * pendGain;
            outR = outR * currGain + pendOutR * pendGain;

            // Check if crossfade is complete
            if (charCrossfade <= 0.0f)
            {
                // Swap pending to current
                currentCharIndex = pendingCharIndex;
                pendingCharIndex = -1;
                charCrossfade    = 1.0f;

                // Copy pending filters (coefficients + state) to current
                // so processing continues seamlessly from where pending left off
                hfShelfL = pendHfShelfL;
                hfShelfR = pendHfShelfR;
                lfShelfL = pendLfShelfL;
                lfShelfR = pendLfShelfR;

                for (int i = 0; i < kMaxAllpassStages; ++i)
                {
                    allpassL[i] = pendAllpassL[i];
                    allpassR[i] = pendAllpassR[i];
                }
            }
        }

        // -----------------------------------------------------------------
        // Bypass blend: dry * (1-blend) + wet * blend
        // -----------------------------------------------------------------
        float finalL = inL * (1.0f - blend) + outL * blend;
        float finalR = inR * (1.0f - blend) + outR * blend;

        channelL[s] = finalL;
        if (channelR != nullptr)
            channelR[s] = finalR;
    }

    // After loop: if airSmoothing just completed, do final coefficient update
    if (airSmoothing && ! airSmoothed.isSmoothing())
    {
        currentAirSmoothed = airSmoothed.getTargetValue();
        updateFilters();

        if (pendingCharIndex >= 0)
            updatePendingFilters();
    }
}
