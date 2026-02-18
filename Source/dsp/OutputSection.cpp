#include "OutputSection.h"

void OutputSection::prepare (double sampleRate, int /*samplesPerBlock*/)
{
    gainSmoothed.reset (sampleRate, 0.020);  // 20ms ramp (ENG-01)
    gainSmoothed.setCurrentAndTargetValue (1.0f);
}

void OutputSection::process (juce::AudioBuffer<float>& buffer, float gainDb)
{
    const float targetGain = juce::Decibels::decibelsToGain (gainDb, -100.0f);
    gainSmoothed.setTargetValue (targetGain);

    if (gainSmoothed.isSmoothing())
    {
        // Sample-by-sample processing during parameter transitions (no zipper noise)
        const int numChannels = buffer.getNumChannels();
        const int numSamples  = buffer.getNumSamples();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float gain = gainSmoothed.getNextValue();
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.getWritePointer (ch)[sample] *= gain;
        }
    }
    else
    {
        // Settled -- apply uniform gain to entire buffer (more efficient)
        buffer.applyGain (gainSmoothed.getCurrentValue());
    }
}

void OutputSection::reset()
{
    gainSmoothed.setCurrentAndTargetValue (1.0f);
}
