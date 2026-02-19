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

    // Allpass frequencies derived from delay times: f = 1/(2*pi*delay)
    // 0.5ms -> ~318Hz, 1.1ms -> ~145Hz, 1.7ms -> ~94Hz
    // Coprime-ish delay times avoid reinforcing any single frequency
    const double decorrelationFreqs[3] = { 318.0, 145.0, 94.0 };
    for (int i = 0; i < kDryDecorrelationStages; ++i)
    {
        double scaledFreq = decorrelationFreqs[i] / sampleRate;
        dryDecorrelationL[i].allpassQ (scaledFreq, 0.707);
        // Slightly offset R channel frequencies for stereo decorrelation
        dryDecorrelationR[i].allpassQ (scaledFreq * 1.12, 0.707);
    }
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

void MixSection::applyAutoGainCompensation (juce::AudioBuffer<float>& buffer, float mixValue)
{
    // Auto-gain curve: compensationDb = -3.5 * pow(mix, 1.4)
    // Tuned for full 6-stage chain (Phase 5). Excitation adds ~1-2dB harmonic energy.
    // At Mix=0%:   0.0 dB (no compensation)
    // At Mix=50%: -1.33 dB
    // At Mix=100%: -3.5 dB
    const float compensationDb = -3.5f * std::pow (mixValue, 1.4f);
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
}
