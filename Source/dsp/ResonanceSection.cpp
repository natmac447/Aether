#include "ResonanceSection.h"
#include <cmath>

// =============================================================================
// 10 Material Presets
// =============================================================================
// Delay times (ms) chosen so that sample counts at common rates are
// mutually coprime. nearestPrime() enforces this at runtime.
//
// Material families:
//   Woods (0-3):  warm, resonant, varying bloom
//   Metals (4-6): bright, focused, edgy
//   Stones (7-9): deep, dense, diffuse

const MaterialParams ResonanceSection::kMaterials[ResonanceSection::kNumMaterials] =
{
    // --- WOODS ---
    {
        "Pine",
        "A light, open vessel of generous bloom and airy sustain -- "
        "the resonance of sun-warmed timber, alive with harmonic vitality",
        { 1.36f, 1.81f, 2.31f, 2.99f },
        220.0f,     // bandpass center
        1.8f,       // bandpass Q
        4500.0f,    // damping LP cutoff
        0.88f,      // feedback gain
        3.0f,       // low shelf gain dB
        180.0f,     // low shelf freq
        0.85f       // output gain
    },
    {
        "Oak",
        "Dense, structured warmth with controlled low-end authority -- "
        "the steadfast resonance of ancient heartwood",
        { 1.13f, 1.56f, 2.04f, 2.72f },
        180.0f,
        2.2f,
        3800.0f,
        0.82f,
        4.0f,
        150.0f,
        0.90f
    },
    {
        "Walnut",
        "Rich, balanced warmth with musical richness -- "
        "the refined resonance of a craftsman's chosen timber",
        { 1.22f, 1.68f, 2.18f, 2.86f },
        200.0f,
        2.0f,
        4000.0f,
        0.85f,
        3.5f,
        160.0f,
        0.88f
    },
    {
        "Mahogany",
        "Deep, warm resonance with long, dignified sustain -- "
        "the noble vibration of tropical hardwood, full of presence",
        { 1.04f, 1.45f, 1.90f, 2.59f },
        160.0f,
        2.5f,
        3200.0f,
        0.80f,
        4.5f,
        140.0f,
        0.92f
    },
    // --- METALS ---
    {
        "Iron",
        "Forceful, focused resonance with commanding edge -- "
        "the unyielding vibration of forged metal, dense and direct",
        { 0.77f, 1.09f, 1.43f, 1.97f },
        280.0f,
        3.0f,
        5500.0f,
        0.75f,
        2.0f,
        200.0f,
        1.05f
    },
    {
        "Steel",
        "Brilliant, precise resonance with metallic shimmer -- "
        "the bright ring of tempered alloy, cutting and clear",
        { 0.66f, 0.93f, 1.25f, 1.72f },
        320.0f,
        3.5f,
        6500.0f,
        0.70f,
        1.5f,
        220.0f,
        1.10f
    },
    {
        "Copper",
        "Warm brilliance with a singing quality -- "
        "the mellow ring of polished copper, resonant yet refined",
        { 0.86f, 1.20f, 1.59f, 2.13f },
        250.0f,
        2.8f,
        5000.0f,
        0.78f,
        2.5f,
        190.0f,
        1.00f
    },
    // --- STONES ---
    {
        "Limestone",
        "Soft, diffuse warmth with gentle roll -- "
        "the quiet resonance of sedimentary chambers, smoothing all within",
        { 1.50f, 2.04f, 2.63f, 3.45f },
        150.0f,
        1.5f,
        2800.0f,
        0.72f,
        5.0f,
        120.0f,
        0.95f
    },
    {
        "Marble",
        "Polished, even resonance with quiet refinement -- "
        "the smooth vibration of hewn stone, balanced and composed",
        { 1.38f, 1.88f, 2.45f, 3.20f },
        170.0f,
        2.0f,
        3000.0f,
        0.68f,
        4.0f,
        130.0f,
        1.00f
    },
    {
        "Granite",
        "Massive, dense weight with rapid damping -- "
        "the profound stillness of deep earth, absorbing all vibration",
        { 1.63f, 2.22f, 2.86f, 3.74f },
        130.0f,
        1.2f,
        2200.0f,
        0.60f,
        5.0f,
        100.0f,
        1.05f
    }
};

// =============================================================================
// nearestPrime -- find the nearest prime >= target
// =============================================================================

int ResonanceSection::nearestPrime (int target)
{
    if (target < 2) return 2;

    auto isPrime = [] (int n) -> bool
    {
        if (n < 2) return false;
        if (n < 4) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;

        for (int i = 5; i * i <= n; i += 6)
            if (n % i == 0 || n % (i + 2) == 0)
                return false;

        return true;
    };

    int candidate = target;
    while (! isPrime (candidate))
        ++candidate;

    return candidate;
}

// =============================================================================
// prepare
// =============================================================================

void ResonanceSection::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Max delay = 5ms (covers all materials at any sample rate)
    int maxDelaySamples = static_cast<int> (std::ceil (5.0 * sampleRate / 1000.0));

    for (int i = 0; i < kNumDelayLines; ++i)
    {
        delayLines[i].resize (maxDelaySamples + 1);
        delayLines[i].reset();
        bandpassFilters[i].reset();
        dampingFilters[i].reset();
    }

    lowShelfFilter.reset();

    // 20ms ramp for Weight changes
    weightSmoothed.reset (sampleRate, 0.020);

    // 10ms bypass crossfade
    bypassBlend.reset (sampleRate, 0.010);
    bypassBlend.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);

    // Configure FDN for current material
    configureForMaterial (kMaterials[currentMaterialIndex]);
}

// =============================================================================
// configureForMaterial
// =============================================================================

void ResonanceSection::configureForMaterial (const MaterialParams& mat)
{
    double sr = currentSampleRate;

    // Compute delay times in samples, quantized to nearest prime for coprimality
    int primeDelays[kNumDelayLines];
    for (int i = 0; i < kNumDelayLines; ++i)
    {
        int targetSamples = static_cast<int> (std::round (mat.delayMs[i] * sr / 1000.0));
        primeDelays[i] = nearestPrime (juce::jmax (2, targetSamples));
    }

    // Ensure all 4 delays are distinct primes (bump duplicates upward)
    for (int i = 1; i < kNumDelayLines; ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            while (primeDelays[i] == primeDelays[j])
                primeDelays[i] = nearestPrime (primeDelays[i] + 1);
        }
    }

    for (int i = 0; i < kNumDelayLines; ++i)
        currentDelayTimeSamples[i] = static_cast<float> (primeDelays[i]);

    // Configure bandpass filters: resonant bandpass at material center freq
    double scaledBP = mat.bandpassFreqHz / sr;
    for (int i = 0; i < kNumDelayLines; ++i)
        bandpassFilters[i].bandpassQ (scaledBP, mat.bandpassQ);

    // Configure damping lowpass per delay line
    double scaledDamp = mat.dampingFreqHz / sr;
    for (int i = 0; i < kNumDelayLines; ++i)
        dampingFilters[i].lowpass (scaledDamp);

    // Output low shelf (outside feedback loop)
    lowShelfFilter.lowShelfDb (mat.lowShelfFreqHz / sr, mat.lowShelfGainDb);

    // Safety clamp: never allow feedback >= 0.95
    currentFeedbackGain = juce::jmin (mat.feedbackGain, 0.95f);
    currentOutputGain = mat.outputGain;
}

// =============================================================================
// process -- THE CORE DSP LOOP
// =============================================================================

void ResonanceSection::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 1 || numSamples < 0)
        return;

    // Early exit: if fully bypassed AND smoothing is done AND weight is 0,
    // skip all processing for CPU savings
    if (! bypassBlend.isSmoothing() && ! weightSmoothed.isSmoothing())
    {
        float blendVal = bypassBlend.getTargetValue();
        float weightVal = weightSmoothed.getTargetValue();

        if (blendVal <= 0.0f || weightVal <= 0.0f)
            return;  // Fully bypassed or weight is zero -- passthrough
    }

    float* channelL = buffer.getWritePointer (0);
    float* channelR = (numChannels >= 2) ? buffer.getWritePointer (1) : nullptr;

    for (int s = 0; s < numSamples; ++s)
    {
        // a. Sum stereo to mono
        float mono;
        if (channelR != nullptr)
            mono = (channelL[s] + channelR[s]) * 0.5f;
        else
            mono = channelL[s];

        // b. Get smoothed values
        float weight = weightSmoothed.getNextValue();
        float blend  = bypassBlend.getNextValue();

        // c. Quadratic response curve for Weight
        float wetGain = weight * weight;

        // d. Compute FDN
        std::array<float, kNumDelayLines> delayOutputs;

        // Read from each delay line
        for (int i = 0; i < kNumDelayLines; ++i)
            delayOutputs[i] = delayLines[i].read (currentDelayTimeSamples[i]);

        // Apply damping lowpass to each output
        for (int i = 0; i < kNumDelayLines; ++i)
            delayOutputs[i] = dampingFilters[i] (delayOutputs[i]);

        // Apply Householder feedback matrix (in-place, orthogonal)
        signalsmith::mix::Householder<float, kNumDelayLines>::inPlace (delayOutputs);

        // Write new input to each delay line (bandpass-filtered input + feedback)
        float fdnOut = 0.0f;
        for (int i = 0; i < kNumDelayLines; ++i)
        {
            float input = bandpassFilters[i] (mono) + delayOutputs[i] * currentFeedbackGain;
            delayLines[i].write (input);
            fdnOut += delayOutputs[i];
        }

        // Normalize by number of delay lines
        fdnOut *= (1.0f / static_cast<float> (kNumDelayLines));

        // e. Apply output low shelf (outside feedback loop)
        fdnOut = lowShelfFilter (fdnOut);

        // f. Apply per-material output gain
        fdnOut *= currentOutputGain;

        // Safety hard limit to prevent runaway
        fdnOut = juce::jlimit (-2.0f, 2.0f, fdnOut);

        // g. Additive blend: dry signal always passes through, resonance layered on top
        float effectiveWet = blend * wetGain;
        float output = mono + fdnOut * effectiveWet;

        // h. Write mono output to both channels (CAB-05: mono processing)
        channelL[s] = output;
        if (channelR != nullptr)
            channelR[s] = output;
    }
}

// =============================================================================
// setWeight
// =============================================================================

void ResonanceSection::setWeight (float weight)
{
    weightSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, weight));
}

// =============================================================================
// setMaterial
// =============================================================================

void ResonanceSection::setMaterial (int materialIndex)
{
    materialIndex = juce::jlimit (0, kNumMaterials - 1, materialIndex);

    if (materialIndex != currentMaterialIndex)
    {
        currentMaterialIndex = materialIndex;
        configureForMaterial (kMaterials[currentMaterialIndex]);
    }
}

// =============================================================================
// setBypass
// =============================================================================

void ResonanceSection::setBypass (bool shouldBypass)
{
    bypassed = shouldBypass;
    bypassBlend.setTargetValue (shouldBypass ? 0.0f : 1.0f);
}

// =============================================================================
// reset
// =============================================================================

void ResonanceSection::reset()
{
    for (int i = 0; i < kNumDelayLines; ++i)
    {
        delayLines[i].reset();
        bandpassFilters[i].reset();
        dampingFilters[i].reset();
    }

    lowShelfFilter.reset();
    weightSmoothed.setCurrentAndTargetValue (0.5f);
    bypassBlend.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);
}
