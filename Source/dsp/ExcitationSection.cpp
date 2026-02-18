#include "ExcitationSection.h"
#include <cmath>

// =============================================================================
// prepare
// =============================================================================

void ExcitationSection::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;

    // -------------------------------------------------------------------------
    // Adaptive oversampling: target ~176-192kHz processing rate
    // -------------------------------------------------------------------------
    if (sampleRate <= 50000.0)
        oversamplingOrder = 2;       // 4x (2^2) at 44.1/48kHz
    else if (sampleRate <= 100000.0)
        oversamplingOrder = 1;       // 2x (2^1) at 88.2/96kHz
    else
        oversamplingOrder = 0;       // 1x (no oversampling) at 176.4/192kHz

    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
        2,                           // stereo
        static_cast<size_t> (oversamplingOrder),
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
        true,                        // isMaxQuality
        true);                       // useIntegerLatency

    oversampler->initProcessing (static_cast<size_t> (samplesPerBlock));
    latencySamples = static_cast<int> (oversampler->getLatencyInSamples());

    // -------------------------------------------------------------------------
    // 3-band LR4 crossover filters
    // -------------------------------------------------------------------------
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate * static_cast<double> (1 << oversamplingOrder);
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock * (1 << oversamplingOrder));
    spec.numChannels      = 2;

    // Low crossover: allpass mode so we can extract both LP and HP outputs
    // via the two-output processSample overload
    lowCross.setType (juce::dsp::LinkwitzRileyFilterType::allpass);
    lowCross.setCutoffFrequency (kLowCrossFreq);
    lowCross.prepare (spec);

    highCross.setType (juce::dsp::LinkwitzRileyFilterType::allpass);
    highCross.setCutoffFrequency (kHighCrossFreq);
    highCross.prepare (spec);

    // -------------------------------------------------------------------------
    // DC blocker: 5Hz HPF (removes DC offset from waveshaping)
    // -------------------------------------------------------------------------
    juce::dsp::ProcessSpec dcSpec;
    dcSpec.sampleRate       = sampleRate;
    dcSpec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    dcSpec.numChannels      = 1;

    auto dcCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 5.0f);
    dcBlockerL.coefficients = dcCoeffs;
    dcBlockerR.coefficients = dcCoeffs;
    dcBlockerL.prepare (dcSpec);
    dcBlockerR.prepare (dcSpec);

    // -------------------------------------------------------------------------
    // Work buffers (pre-allocated, no allocation in process)
    // -------------------------------------------------------------------------
    dryCopy.setSize (2, samplesPerBlock);

    // -------------------------------------------------------------------------
    // SmoothedValues
    // -------------------------------------------------------------------------
    driveSmoothed.reset (sampleRate, 0.025);  // 25ms ramp
    driveSmoothed.setCurrentAndTargetValue (currentDrive);

    bypassBlend.reset (sampleRate, 0.010);    // 10ms ramp
    bypassBlend.setCurrentAndTargetValue (1.0f);  // active (not bypassed)

    // Compute initial G values
    updateDriveParams();
}

// =============================================================================
// reset
// =============================================================================

void ExcitationSection::reset()
{
    if (oversampler != nullptr)
        oversampler->reset();

    lowCross.reset();
    highCross.reset();

    dcBlockerL.reset();
    dcBlockerR.reset();

    driveSmoothed.setCurrentAndTargetValue (currentDrive);
    bypassBlend.setCurrentAndTargetValue (1.0f);
}

// =============================================================================
// setBypass
// =============================================================================

void ExcitationSection::setBypass (bool shouldBypass)
{
    bypassBlend.setTargetValue (shouldBypass ? 0.0f : 1.0f);
}

// =============================================================================
// setDrive / setMaterial / setRoomSize
// =============================================================================

void ExcitationSection::setDrive (float driveNormalized)
{
    currentDrive = juce::jlimit (0.0f, 1.0f, driveNormalized);
    driveSmoothed.setTargetValue (currentDrive);
}

void ExcitationSection::setMaterial (int materialIndex)
{
    currentMaterialIndex = juce::jlimit (0, 9, materialIndex);
}

void ExcitationSection::setRoomSize (float roomSizeNormalized)
{
    currentRoomSize = juce::jlimit (0.0f, 1.0f, roomSizeNormalized);
}

// =============================================================================
// getLatencySamples
// =============================================================================

int ExcitationSection::getLatencySamples() const
{
    return latencySamples;
}

// =============================================================================
// updateDriveParams
// =============================================================================

void ExcitationSection::updateDriveParams()
{
    // 1. Map drive through power curve for sweet spot in 30-60% range
    float curved = std::pow (currentDrive, kDriveCurveExponent);

    // 2. Look up material bias
    int matIdx = juce::jlimit (0, 9, currentMaterialIndex);
    const auto& mat = kMaterialBias[matIdx];

    // 3. Compute room size influence on per-band scales
    //    Small rooms (0.0): tighter excitation, less low bloom
    //    Large rooms (1.0): more low-end bloom, less high excitation
    float effectiveLowScale  = kBaseLowScale  * (0.7f + 0.6f * currentRoomSize) * mat.lowScale;
    float effectiveMidScale  = kBaseMidScale  * mat.midScale;  // mids stable across room sizes
    float effectiveHighScale = kBaseHighScale * (1.1f - 0.3f * currentRoomSize) * mat.highScale;

    // 4. Map to G values: kMinG at Drive=0%, max at Drive=100%
    lowG  = kMinG + curved * effectiveLowScale  * (kMaxLowG  - kMinG);
    midG  = kMinG + curved * effectiveMidScale  * (kMaxMidG  - kMinG);
    highG = kMinG + curved * effectiveHighScale * (kMaxHighG - kMinG);
}

// =============================================================================
// process
// =============================================================================

void ExcitationSection::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 1 || numSamples < 1)
        return;

    // Early exit: fully bypassed and not smoothing
    if (! bypassBlend.isSmoothing() && bypassBlend.getTargetValue() <= 0.0f)
        return;

    // -------------------------------------------------------------------------
    // Compute per-band G values once per block (Drive changes are slow)
    // -------------------------------------------------------------------------
    currentDrive = driveSmoothed.skip (numSamples);
    updateDriveParams();

    // Cache G values for the oversampled loop
    const float blockLowG  = lowG;
    const float blockMidG  = midG;
    const float blockHighG = highG;

    // -------------------------------------------------------------------------
    // Copy dry signal for bypass crossfade
    // -------------------------------------------------------------------------
    for (int ch = 0; ch < juce::jmin (numChannels, 2); ++ch)
        dryCopy.copyFrom (ch, 0, buffer, ch, 0, numSamples);

    // -------------------------------------------------------------------------
    // Upsample
    // -------------------------------------------------------------------------
    juce::dsp::AudioBlock<float> block (buffer);
    auto oversampledBlock = oversampler->processSamplesUp (block);

    const auto osNumSamples  = static_cast<int> (oversampledBlock.getNumSamples());
    const auto osNumChannels = static_cast<int> (oversampledBlock.getNumChannels());

    // -------------------------------------------------------------------------
    // Per-sample processing at oversampled rate:
    // 3-band crossover -> per-band waveshaping -> recombine
    // -------------------------------------------------------------------------
    for (int s = 0; s < osNumSamples; ++s)
    {
        for (int ch = 0; ch < juce::jmin (osNumChannels, 2); ++ch)
        {
            float sample = oversampledBlock.getSample (ch, s);

            // Split into low and midHigh using the two-output processSample
            float lowSample  = 0.0f;
            float midHighSample = 0.0f;
            lowCross.processSample (ch, sample, lowSample, midHighSample);

            // Split midHigh into mid and high
            float midSample  = 0.0f;
            float highSample = 0.0f;
            highCross.processSample (ch, midHighSample, midSample, highSample);

            // Per-band symmetric tanh waveshaping: tanh(x*G)/tanh(G)
            // Guard against near-zero G (linear passthrough)
            if (blockLowG > 0.001f)
                lowSample = std::tanh (lowSample * blockLowG) / std::tanh (blockLowG);

            if (blockMidG > 0.001f)
                midSample = std::tanh (midSample * blockMidG) / std::tanh (blockMidG);

            if (blockHighG > 0.001f)
                highSample = std::tanh (highSample * blockHighG) / std::tanh (blockHighG);

            // Recombine bands
            float output = lowSample + midSample + highSample;

            oversampledBlock.setSample (ch, s, output);
        }
    }

    // -------------------------------------------------------------------------
    // Downsample back to base rate
    // -------------------------------------------------------------------------
    oversampler->processSamplesDown (block);

    // -------------------------------------------------------------------------
    // DC blocker at base rate (post-downsample)
    // -------------------------------------------------------------------------
    if (numChannels >= 1)
    {
        float* dataL = buffer.getWritePointer (0);
        for (int s = 0; s < numSamples; ++s)
            dataL[s] = dcBlockerL.processSample (dataL[s]);
    }

    if (numChannels >= 2)
    {
        float* dataR = buffer.getWritePointer (1);
        for (int s = 0; s < numSamples; ++s)
            dataR[s] = dcBlockerR.processSample (dataR[s]);
    }

    // -------------------------------------------------------------------------
    // Bypass crossfade: dry * (1-blend) + wet * blend
    // -------------------------------------------------------------------------
    float* wetL = buffer.getWritePointer (0);
    const float* dryL = dryCopy.getReadPointer (0);
    float* wetR = (numChannels >= 2) ? buffer.getWritePointer (1) : nullptr;
    const float* dryR = (numChannels >= 2) ? dryCopy.getReadPointer (1) : nullptr;

    for (int s = 0; s < numSamples; ++s)
    {
        float blend = bypassBlend.getNextValue();
        float invBlend = 1.0f - blend;

        wetL[s] = dryL[s] * invBlend + wetL[s] * blend;

        if (wetR != nullptr && dryR != nullptr)
            wetR[s] = dryR[s] * invBlend + wetR[s] * blend;
    }
}
