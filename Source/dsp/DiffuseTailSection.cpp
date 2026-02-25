#include "DiffuseTailSection.h"
#include <cmath>

// =============================================================================
// Shape-to-tail influence lookup
// =============================================================================
// These values correspond to the 7 room shapes defined in ReflectionsSection.
// tailDiffusionBias: how much the shape densifies the tail (0.0-1.0)
// tailModalCharacter: 0 = even mode distribution, 1 = clustered modes

struct ShapeTailInfluence
{
    float tailDiffusionBias;
    float tailModalCharacter;
};

static constexpr int kNumShapes = 30;

static constexpr ShapeTailInfluence kShapeTailInfluence[kNumShapes] = {
    // Small Rooms (0-9)
    { 0.30f, 0.20f },   //  0: The Parlour -- regular, even
    { 0.55f, 0.50f },   //  1: The Alcove -- irregular, scattered
    { 0.80f, 0.70f },   //  2: The Crypt -- very dense, dark
    { 0.35f, 0.25f },   //  3: The Vestibule -- hard, clear
    { 0.85f, 0.80f },   //  4: The Closet -- extremely dense, damped
    { 0.45f, 0.30f },   //  5: The Study -- moderate, warm
    { 0.90f, 0.85f },   //  6: The Telephone Box -- maximum density, metallic
    { 0.60f, 0.55f },   //  7: The Pantry -- complex scattering
    { 0.50f, 0.60f },   //  8: The Confessional -- moderate, asymmetric
    { 0.70f, 0.65f },   //  9: The Powder Room -- bright, dense
    // Medium Rooms (10-19)
    { 0.50f, 0.45f },   // 10: The Chamber -- dense, studio-like
    { 0.40f, 0.35f },   // 11: The Gallery -- moderate density
    { 0.35f, 0.15f },   // 12: The Conservatory -- bright, even
    { 0.55f, 0.40f },   // 13: The Scriptorium -- stone, rich
    { 0.30f, 0.20f },   // 14: The Library -- very absorptive
    { 0.40f, 0.30f },   // 15: The Drawing Room -- refined
    { 0.50f, 0.45f },   // 16: The Workshop -- hard, industrial
    { 0.50f, 0.40f },   // 17: The Refectory -- stone+wood
    { 0.40f, 0.25f },   // 18: The Solarium -- glass, airy
    { 0.65f, 0.50f },   // 19: The Apothecary -- complex, scattered
    // Large Rooms (20-29)
    { 0.70f, 0.60f },   // 20: The Nave -- high density, clustered
    { 0.45f, 0.30f },   // 21: The Ballroom -- grand, elegant
    { 0.40f, 0.35f },   // 22: The Atrium -- open, columns
    { 0.65f, 0.55f },   // 23: The Chapel -- reverberant, sacred
    { 0.35f, 0.40f },   // 24: The Warehouse -- harsh, scattered
    { 0.85f, 0.75f },   // 25: The Cistern -- extremely reverberant
    { 0.60f, 0.65f },   // 26: The Observatory -- dome-focused
    { 0.55f, 0.45f },   // 27: The Great Hall -- timber+stone
    { 0.30f, 0.20f },   // 28: The Greenhouse -- bright, even
    { 0.70f, 0.60f },   // 29: The Mausoleum -- marble, resonant
};

// =============================================================================
// nearestPrime -- find the nearest prime >= target
// =============================================================================

int DiffuseTailSection::nearestPrime (int target)
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
// configureDelayLengths
// =============================================================================

void DiffuseTailSection::configureDelayLengths()
{
    double sr = currentSampleRate;

    // --- FDN delay lines: convert ms to samples, quantize to nearest prime ---
    int primeDelays[kNumFDNLines];
    for (int i = 0; i < kNumFDNLines; ++i)
    {
        // Apply shape modal character: slightly modulate delay lengths by +/-5%
        // Higher modal character clusters delays slightly
        float modulation = 1.0f;
        if (shapeModalCharacter > 0.01f)
        {
            // Even indices pulled slightly shorter, odd indices slightly longer
            float direction = (i % 2 == 0) ? -1.0f : 1.0f;
            modulation = 1.0f + direction * shapeModalCharacter * 0.05f;
        }

        int targetSamples = static_cast<int> (std::round (kFDNDelayMs[i] * modulation * sr / 1000.0));
        primeDelays[i] = nearestPrime (juce::jmax (2, targetSamples));
    }

    // Ensure all 8 delays are distinct primes (bump duplicates upward)
    for (int i = 1; i < kNumFDNLines; ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            while (primeDelays[i] == primeDelays[j])
                primeDelays[i] = nearestPrime (primeDelays[i] + 1);
        }
    }

    for (int i = 0; i < kNumFDNLines; ++i)
        fdnDelaySamples[i] = static_cast<float> (primeDelays[i]);

    // --- Input diffusion delays: convert to samples for L and R ---
    for (int i = 0; i < kNumDiffusionStages; ++i)
    {
        int targetL = static_cast<int> (std::round (kDiffusionDelayMs[i] * sr / 1000.0));
        diffDelaySamplesL[i] = static_cast<float> (nearestPrime (juce::jmax (2, targetL)));

        // R channel offset by +12% for stereo decorrelation, prime-quantized separately
        int targetR = static_cast<int> (std::round (kDiffusionDelayMs[i] * 1.12f * sr / 1000.0));
        diffDelaySamplesR[i] = static_cast<float> (nearestPrime (juce::jmax (2, targetR)));
    }
}

// =============================================================================
// prepare
// =============================================================================

void DiffuseTailSection::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Pre-delay: max 35ms (covers max Room Size ER delay * 0.7 + margin)
    int maxPreDelay = static_cast<int> (std::ceil (35.0 * sampleRate / 1000.0));
    preDelayL.resize (maxPreDelay + 1);
    preDelayR.resize (maxPreDelay + 1);
    preDelayL.reset();
    preDelayR.reset();

    // FDN delay lines: max 70ms (max delay + margin)
    int maxFDN = static_cast<int> (std::ceil (70.0 * sampleRate / 1000.0));
    for (int i = 0; i < kNumFDNLines; ++i)
    {
        fdnDelayLines[i].resize (maxFDN + 1);
        fdnDelayLines[i].reset();
        dampingFilters[i].reset();
    }

    // Diffusion delays: max 10ms
    int maxDiff = static_cast<int> (std::ceil (10.0 * sampleRate / 1000.0));
    for (int i = 0; i < kNumDiffusionStages; ++i)
    {
        diffDelayL[i].resize (maxDiff + 1);
        diffDelayR[i].resize (maxDiff + 1);
        diffDelayL[i].reset();
        diffDelayR[i].reset();
    }

    // Configure delay lengths (prime-quantized)
    configureDelayLengths();

    // SmoothedValue initialization
    decaySmoothed.reset (sampleRate, 0.020);       // 20ms ramp
    diffusionSmoothed.reset (sampleRate, 0.020);    // 20ms ramp
    preDelaySmoothed.reset (sampleRate, 0.050);     // 50ms ramp
    bypassBlend.reset (sampleRate, 0.010);          // 10ms ramp

    // Initial values
    decaySmoothed.setCurrentAndTargetValue (150.0f);
    diffusionSmoothed.setCurrentAndTargetValue (0.6f);
    bypassBlend.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);

    // Initial pre-delay: corresponds to default room size
    float defaultPreDelaySamples = 5.0f * static_cast<float> (sampleRate) / 1000.0f;
    preDelaySmoothed.setCurrentAndTargetValue (defaultPreDelaySamples);

    // Initialize feedback gains and damping
    lastDecayMs = 150.0f;
    updateDecay (150.0f);
    updateDampingFilters();
}

// =============================================================================
// updateDecay
// =============================================================================

void DiffuseTailSection::updateDecay (float decayMs)
{
    // Apply character decay bias: +/-10% modulation on decay time
    float biasedDecayMs = decayMs * (1.0f + characterDecayBias);
    biasedDecayMs = juce::jlimit (50.0f, 2200.0f, biasedDecayMs);

    float decaySec = biasedDecayMs / 1000.0f;
    float sr = static_cast<float> (currentSampleRate);

    for (int i = 0; i < kNumFDNLines; ++i)
    {
        float gain = std::pow (10.0f, -3.0f * fdnDelaySamples[i] / (decaySec * sr));
        feedbackGains[i] = juce::jlimit (0.0f, 0.9999f, gain);
    }
}

// =============================================================================
// updateDampingFilters
// =============================================================================

void DiffuseTailSection::updateDampingFilters()
{
    float cutoff = juce::jlimit (1000.0f, 16000.0f, dampingCutoffHz);
    float scaledFreq = cutoff / static_cast<float> (currentSampleRate);

    for (int i = 0; i < kNumFDNLines; ++i)
        dampingFilters[i].lowpass (scaledFreq);
}

// =============================================================================
// process -- THE CORE DSP LOOP
// =============================================================================

void DiffuseTailSection::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 1 || numSamples < 0)
        return;

    // Early exit: if fully bypassed AND smoothing is done, skip processing
    if (! bypassBlend.isSmoothing())
    {
        float blendVal = bypassBlend.getTargetValue();
        if (blendVal <= 0.0f)
            return;  // Fully bypassed -- passthrough
    }

    float* channelL = buffer.getWritePointer (0);
    float* channelR = (numChannels >= 2) ? buffer.getWritePointer (1) : nullptr;

    // Compute effective diffusion coefficient: base + shape bias (up to +0.15)
    // This is evaluated once per block since shape changes are infrequent
    float diffBiasAdd = shapeDiffusionBias * 0.15f;

    for (int s = 0; s < numSamples; ++s)
    {
        // a. Read smoothed values
        float preDelaySamp = preDelaySmoothed.getNextValue();
        float blend = bypassBlend.getNextValue();
        float currentDecayMs = decaySmoothed.getNextValue();
        float diffNorm = diffusionSmoothed.getNextValue();

        // Map diffusion 0-1 to allpass coefficient 0.0-0.7, plus shape bias
        float diffCoeff = juce::jlimit (0.0f, 0.7f, diffNorm * 0.7f + diffBiasAdd);

        // Update decay if smoothing (avoid recalculating every sample when stable)
        if (decaySmoothed.isSmoothing())
        {
            lastDecayMs = currentDecayMs;
            updateDecay (currentDecayMs);
        }

        // b. Read stereo input (preserve original for bypass crossfade)
        float inputL = channelL[s];
        float inputR = (channelR != nullptr) ? channelR[s] : inputL;
        float origL = inputL;
        float origR = inputR;

        // c. Pre-delay: write input, read delayed
        preDelayL.write (inputL);
        preDelayR.write (inputR);
        float delayedL = preDelayL.read (preDelaySamp);
        float delayedR = preDelayR.read (preDelaySamp);

        // d. Input allpass diffusion (4 stages, per channel)
        // Explicit allpass: output = -g*input + delayed + g*delayed_feedback
        // Write to delay: input + g * delayed
        float sigL = delayedL;
        float sigR = delayedR;

        for (int stage = 0; stage < kNumDiffusionStages; ++stage)
        {
            // Left channel
            float dL = diffDelayL[stage].read (diffDelaySamplesL[stage]);
            float outL = -diffCoeff * sigL + dL;
            diffDelayL[stage].write (sigL + diffCoeff * dL);
            sigL = outL;

            // Right channel
            float dR = diffDelayR[stage].read (diffDelaySamplesR[stage]);
            float outR = -diffCoeff * sigR + dR;
            diffDelayR[stage].write (sigR + diffCoeff * dR);
            sigR = outR;
        }

        // e. Distribute stereo to 8 FDN channels
        // Even lines (0,2,4,6) get L, odd lines (1,3,5,7) get R
        // Scale by 1/sqrt(4) = 0.5 for energy preservation
        float fdnInput[kNumFDNLines];
        for (int i = 0; i < kNumFDNLines; i += 2)
            fdnInput[i] = sigL * 0.5f;
        for (int i = 1; i < kNumFDNLines; i += 2)
            fdnInput[i] = sigR * 0.5f;

        // f. FDN loop: read -> damp -> Hadamard -> feedback -> write
        std::array<float, kNumFDNLines> outputs;

        // Read from delay lines
        for (int i = 0; i < kNumFDNLines; ++i)
            outputs[i] = fdnDelayLines[i].read (fdnDelaySamples[i]);

        // Apply damping lowpass
        for (int i = 0; i < kNumFDNLines; ++i)
            outputs[i] = dampingFilters[i] (outputs[i]);

        // Apply Hadamard mixing matrix (orthogonal, energy-preserving)
        signalsmith::mix::Hadamard<float, 8>::inPlace (outputs);

        // Apply feedback gain and add input, write back to delays
        for (int i = 0; i < kNumFDNLines; ++i)
        {
            float in = outputs[i] * feedbackGains[i] + fdnInput[i];
            fdnDelayLines[i].write (in);
        }

        // g. Downmix 8 channels back to stereo
        // Even lines -> L, odd lines -> R
        float wetL = 0.0f;
        float wetR = 0.0f;
        for (int i = 0; i < kNumFDNLines; i += 2)
            wetL += outputs[i];
        for (int i = 1; i < kNumFDNLines; i += 2)
            wetR += outputs[i];
        wetL *= 0.5f;  // normalize (4 lines per channel)
        wetR *= 0.5f;

        // h. Diffusion-dependent output compensation
        // Higher diffusion feeds sustained energy into the FDN, building up
        // steady-state level. Quadratic curve: gentle at low values, steeper at high.
        float diffCompDb = -14.0f * diffNorm * diffNorm;
        float diffCompGain = juce::Decibels::decibelsToGain (diffCompDb, -100.0f);
        wetL *= diffCompGain;
        wetR *= diffCompGain;

        // Safety hard limit to prevent runaway
        wetL = juce::jlimit (-4.0f, 4.0f, wetL);
        wetR = juce::jlimit (-4.0f, 4.0f, wetR);

        // Debug: NaN check
        jassert (! std::isnan (wetL));
        jassert (! std::isnan (wetR));

        // i. Diffuse Tail is ADDITIVE -- adds reverb on top of signal
        float outputL = inputL + wetL;
        float outputR = inputR + wetR;

        // j. Bypass crossfade: blend between original (bypassed) and processed
        float finalL = origL * (1.0f - blend) + outputL * blend;
        float finalR = origR * (1.0f - blend) + outputR * blend;

        // k. Write output
        channelL[s] = finalL;
        if (channelR != nullptr)
            channelR[s] = finalR;
    }
}

// =============================================================================
// setDecay
// =============================================================================

void DiffuseTailSection::setDecay (float decayMs)
{
    decayMs = juce::jlimit (50.0f, 2000.0f, decayMs);
    decaySmoothed.setTargetValue (decayMs);

    // If not currently smoothing, update immediately
    if (! decaySmoothed.isSmoothing())
    {
        lastDecayMs = decayMs;
        updateDecay (decayMs);
    }
}

// =============================================================================
// setDiffusion
// =============================================================================

void DiffuseTailSection::setDiffusion (float diffNorm)
{
    diffusionSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, diffNorm));
}

// =============================================================================
// setPreDelay
// =============================================================================

void DiffuseTailSection::setPreDelay (float roomSizeNorm)
{
    // Compressed curve: 70% of ER delay time, capped at 25ms
    float erDelayMs = 1.0f + roomSizeNorm * 29.0f;  // Same mapping as ReflectionsSection
    float preDelayMs = erDelayMs * 0.7f;
    preDelayMs = juce::jlimit (0.5f, 25.0f, preDelayMs);
    float preDelaySamples = preDelayMs * static_cast<float> (currentSampleRate) / 1000.0f;
    preDelaySmoothed.setTargetValue (preDelaySamples);
}

// =============================================================================
// setHFDamping
// =============================================================================

void DiffuseTailSection::setHFDamping (float airAmount)
{
    // Air 0%: bright (12kHz), Air 100%: dark (2kHz)
    // Logarithmic mapping for perceptual linearity
    dampingCutoffHz = 12000.0f * std::pow (2000.0f / 12000.0f, airAmount);
    dampingCutoffHz = juce::jlimit (1000.0f, 16000.0f, dampingCutoffHz);
    updateDampingFilters();
}

// =============================================================================
// setCharacterDecayBias -- Cross-stage coupling from Air Character
// =============================================================================

void DiffuseTailSection::setCharacterDecayBias (float bias)
{
    characterDecayBias = juce::jlimit (-0.10f, 0.10f, bias);
}

// =============================================================================
// setShapeInfluence
// =============================================================================

void DiffuseTailSection::setShapeInfluence (int shapeIndex)
{
    shapeIndex = juce::jlimit (0, kNumShapes - 1, shapeIndex);

    shapeDiffusionBias = kShapeTailInfluence[shapeIndex].tailDiffusionBias;
    shapeModalCharacter = kShapeTailInfluence[shapeIndex].tailModalCharacter;

    // Reconfigure delay lengths with new modal character
    configureDelayLengths();

    // Recalculate feedback gains for new delay lengths
    updateDecay (lastDecayMs);
}

// =============================================================================
// setBypass
// =============================================================================

void DiffuseTailSection::setBypass (bool shouldBypass)
{
    bypassed = shouldBypass;
    bypassBlend.setTargetValue (shouldBypass ? 0.0f : 1.0f);
}

// =============================================================================
// reset
// =============================================================================

void DiffuseTailSection::reset()
{
    preDelayL.reset();
    preDelayR.reset();

    for (int i = 0; i < kNumDiffusionStages; ++i)
    {
        diffDelayL[i].reset();
        diffDelayR[i].reset();
    }

    for (int i = 0; i < kNumFDNLines; ++i)
    {
        fdnDelayLines[i].reset();
        dampingFilters[i].reset();
    }

    decaySmoothed.setCurrentAndTargetValue (150.0f);
    diffusionSmoothed.setCurrentAndTargetValue (0.6f);
    preDelaySmoothed.setCurrentAndTargetValue (5.0f * static_cast<float> (currentSampleRate) / 1000.0f);
    bypassBlend.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);

    lastDecayMs = 150.0f;
}
