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
}

void MixSection::pushDrySamples (const juce::dsp::AudioBlock<float>& dryBlock)
{
    dryWetMixer.pushDrySamples (dryBlock);
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

void MixSection::applyAutoGainCompensation (juce::AudioBuffer<float>& buffer, float mixValue)
{
    // Auto-gain curve: compensationDb = -2.5 * pow(mix, 1.5)
    // At Mix=0%:   0.0 dB (no compensation)
    // At Mix=50%: -0.88 dB
    // At Mix=100%: -2.5 dB
    const float compensationDb = -2.5f * std::pow (mixValue, 1.5f);
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
}
