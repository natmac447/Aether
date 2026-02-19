#include "MixSection.h"

void MixSection::prepare (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = 2;

    dryWetMixer.prepare (spec);
    dryWetMixer.setMixingRule (juce::dsp::DryWetMixingRule::sin3dB);  // Equal-power crossfade (OUT-01)
    dryWetMixer.setWetLatency (0.0f);  // No DSP latency in Phase 1
    dryWetMixer.setWetMixProportion (currentMix);

    compensationSmoothed.reset (sampleRate, 0.020);  // 20ms ramp
    compensationSmoothed.setCurrentAndTargetValue (1.0f);

    storedSampleRate = sampleRate;

    // Two allpass stages in the upper-mid range (1.5kHz-3.5kHz) to decorrelate
    // comb-prone frequencies.  Fewer stages = less cumulative phase rotation,
    // keeping the low end intact while still softening comb-filter notches.
    const double decorrelationFreqs[2] = { 2800.0, 1800.0 };
    for (int i = 0; i < kDryDecorrelationStages; ++i)
    {
        double scaledFreq = decorrelationFreqs[i] / sampleRate;
        dryDecorrelationL[i].allpassQ (scaledFreq, 0.707);
        // Slightly offset R channel frequencies for stereo decorrelation
        dryDecorrelationR[i].allpassQ (scaledFreq * 1.12, 0.707);
    }

    // Gentle low shelf on wet signal to restore low-end body (100-250Hz)
    // lost to phase cancellation between dry and wet paths during mixing.
    double shelfFreq = 200.0 / sampleRate;
    wetLowShelfL.lowShelfDb (shelfFreq, 3.0, 2.0);
    wetLowShelfR.lowShelfDb (shelfFreq, 3.0, 2.0);
}

void MixSection::pushDrySamples (const juce::dsp::AudioBlock<float>& dryBlock)
{
    // Apply allpass decorrelation to dry signal before mixing
    // This reduces coherence with the phase-shifted wet signal,
    // softening comb-filter notches in the 800Hz-3kHz range
    const auto numSamples = dryBlock.getNumSamples();
    const auto numChannels = dryBlock.getNumChannels();

    // Copy dry block into mutable buffer for allpass processing
    juce::AudioBuffer<float> decorrelated (static_cast<int> (numChannels),
                                            static_cast<int> (numSamples));

    for (int ch = 0; ch < static_cast<int> (numChannels); ++ch)
    {
        auto* src = dryBlock.getChannelPointer (static_cast<size_t> (ch));
        auto* dst = decorrelated.getWritePointer (ch);
        std::memcpy (dst, src, sizeof (float) * numSamples);
    }

    // Process through allpass chain
    auto* dataL = decorrelated.getWritePointer (0);
    auto* dataR = (numChannels >= 2) ? decorrelated.getWritePointer (1) : nullptr;

    for (size_t s = 0; s < numSamples; ++s)
    {
        float sL = dataL[s];
        for (int i = 0; i < kDryDecorrelationStages; ++i)
            sL = dryDecorrelationL[i] (sL);
        dataL[s] = sL;

        if (dataR != nullptr)
        {
            float sR = dataR[s];
            for (int i = 0; i < kDryDecorrelationStages; ++i)
                sR = dryDecorrelationR[i] (sR);
            dataR[s] = sR;
        }
    }

    juce::dsp::AudioBlock<float> decorrelatedBlock (decorrelated);
    dryWetMixer.pushDrySamples (decorrelatedBlock);
}

void MixSection::mixWetSamples (juce::dsp::AudioBlock<float>& wetBlock)
{
    // Restore low-end body on wet signal before mixing with dry
    const auto numSamples = wetBlock.getNumSamples();

    auto* dataL = wetBlock.getChannelPointer (0);
    for (size_t s = 0; s < numSamples; ++s)
        dataL[s] = wetLowShelfL (dataL[s]);

    if (wetBlock.getNumChannels() >= 2)
    {
        auto* dataR = wetBlock.getChannelPointer (1);
        for (size_t s = 0; s < numSamples; ++s)
            dataR[s] = wetLowShelfR (dataR[s]);
    }

    dryWetMixer.mixWetSamples (wetBlock);
}

void MixSection::setMixLevel (float mixValue)
{
    currentMix = mixValue;
    dryWetMixer.setWetMixProportion (mixValue);
}

void MixSection::setWetLatency (float samples)
{
    dryWetMixer.setWetLatency (samples);
}

void MixSection::applyAutoGainCompensation (juce::AudioBuffer<float>& buffer, float mixValue,
                                             float driveValue, float decayNorm,
                                             float diffusionValue,
                                             float roomSizeNorm, float proximity)
{
    // Base auto-gain: compensationDb = -3.5 * pow(mix, 1.4)
    // Additional compensation for energy-adding parameters:
    //   Drive: saturator adds harmonic energy (up to ~2.5dB at full drive)
    //   Decay: longer tail accumulates reverberant energy (up to ~2dB at max)
    //   Diffusion: handled internally by DiffuseTailSection (not needed here)
    //   Room Size + Proximity: small room with high proximity causes coherent
    //     tap summation in reflections and short pre-delay in tail, cascading
    //     through all downstream stages (+8-9dB at extremes).
    // Each term is scaled by mix since at Mix=0% wet signal is inaudible.
    float compensationDb = -3.5f * std::pow (mixValue, 1.4f);
    compensationDb -= 4.0f * std::pow (driveValue, 2.0f) * mixValue;
    compensationDb -= 3.5f * std::pow (decayNorm, 1.5f) * mixValue;
    float smallRoom = (1.0f - roomSizeNorm) * (1.0f - roomSizeNorm);
    float proxCurve = std::pow (proximity, 1.5f);   // softer onset, full at extremes
    compensationDb -= 15.0f * smallRoom * proxCurve * mixValue;
    const float targetGain = juce::Decibels::decibelsToGain (compensationDb, -100.0f);
    compensationSmoothed.setTargetValue (targetGain);

    if (compensationSmoothed.isSmoothing())
    {
        // Sample-by-sample processing during parameter transitions (no zipper noise)
        const int numChannels = buffer.getNumChannels();
        const int numSamples  = buffer.getNumSamples();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float gain = compensationSmoothed.getNextValue();
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.getWritePointer (ch)[sample] *= gain;
        }
    }
    else
    {
        // Settled -- apply uniform gain (more efficient)
        buffer.applyGain (compensationSmoothed.getCurrentValue());
    }
}

void MixSection::reset()
{
    dryWetMixer.reset();
    compensationSmoothed.setCurrentAndTargetValue (1.0f);

    for (int i = 0; i < kDryDecorrelationStages; ++i)
    {
        dryDecorrelationL[i].reset();
        dryDecorrelationR[i].reset();
    }

    wetLowShelfL.reset();
    wetLowShelfR.reset();
}
