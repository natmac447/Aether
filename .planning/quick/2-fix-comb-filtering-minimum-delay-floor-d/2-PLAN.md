---
phase: quick
plan: 2
type: execute
wave: 1
depends_on: []
files_modified:
  - Source/dsp/ReflectionsSection.cpp
  - Source/dsp/MixSection.h
  - Source/dsp/MixSection.cpp
autonomous: true
requirements: []
must_haves:
  truths:
    - "Reflection taps never produce near-zero-delay copies at small Room Size values"
    - "Dry/wet mixing produces reduced comb-filter artifacts in 800Hz-3kHz range"
    - "No audible new artifacts introduced by either fix"
  artifacts:
    - path: "Source/dsp/ReflectionsSection.cpp"
      provides: "Minimum 1ms delay floor on all reflection tap reads"
      contains: "minDelaySamples"
    - path: "Source/dsp/MixSection.h"
      provides: "Allpass decorrelation filter members for dry path"
      contains: "dryDecorrelation"
    - path: "Source/dsp/MixSection.cpp"
      provides: "Allpass chain applied to dry signal before mixing"
      contains: "dryDecorrelation"
  key_links:
    - from: "Source/dsp/ReflectionsSection.cpp"
      to: "delayLineL.read / delayLineR.read"
      via: "jmax clamp before read call"
      pattern: "jmax.*minDelaySamples"
    - from: "Source/dsp/MixSection.cpp"
      to: "dryWetMixer.pushDrySamples"
      via: "allpass processing before push"
      pattern: "dryDecorrelation.*push"
---

<objective>
Fix comb filtering artifacts when mixing dry/wet signal via the Mix knob. Two independent fixes:
(1) Add minimum 1ms delay floor on reflection taps to prevent near-zero-delay copies at small Room Size.
(2) Add short allpass decorrelation chain to dry signal in MixSection to reduce coherence causing sharp comb notches.

Purpose: Eliminate upper-mid nasal buildup (800Hz-3kHz) when using the Mix knob, especially at small Room Size values.
Output: Modified ReflectionsSection.cpp and MixSection.h/.cpp
</objective>

<execution_context>
@/Users/nathanmcmillan/.claude/get-shit-done/workflows/execute-plan.md
@/Users/nathanmcmillan/.claude/get-shit-done/templates/summary.md
</execution_context>

<context>
@.planning/PROJECT.md
@.planning/STATE.md
@Source/dsp/ReflectionsSection.h
@Source/dsp/ReflectionsSection.cpp
@Source/dsp/MixSection.h
@Source/dsp/MixSection.cpp
@Source/PluginProcessor.cpp
</context>

<tasks>

<task type="auto">
  <name>Task 1: Add minimum 1ms delay floor to reflection taps</name>
  <files>Source/dsp/ReflectionsSection.cpp</files>
  <action>
In ReflectionsSection::process(), add a minimum delay floor so no reflection tap ever reads at near-zero delay (which creates a near-duplicate of the input causing comb filtering).

1. At the top of the per-sample loop (after `float sr = ...`), compute the minimum delay in samples once:
   ```
   const float minDelaySamples = sr * 0.001f;  // 1ms floor at any sample rate
   ```

2. In the current-shape tap accumulation loop (lines ~293-294), change the lSamp/rSamp clamp from:
   ```
   float lSamp = juce::jmax (0.0f, lDMs * sr / 1000.0f);
   float rSamp = juce::jmax (0.0f, rDMs * sr / 1000.0f);
   ```
   to:
   ```
   float lSamp = juce::jmax (minDelaySamples, lDMs * sr / 1000.0f);
   float rSamp = juce::jmax (minDelaySamples, rDMs * sr / 1000.0f);
   ```

3. Apply the same change in the pending-shape tap accumulation loop (lines ~351-352 in the crossfade block). Change the identical jmax(0.0f, ...) calls to jmax(minDelaySamples, ...).

This ensures The Crypt's first tap (0.8ms base) at Room Size ~0.1 gets clamped to 1ms (~44 samples at 44.1kHz) instead of ~0.027ms (~1.2 samples), preventing near-zero-delay copies.

Note: The minDelaySamples variable can be computed outside the per-sample loop since sr does not change within process(). Move it to just after the `const float sr = ...` line before the sample loop for efficiency.
  </action>
  <verify>Build with `cmake --build build --target Aether 2>&1 | tail -5` -- must compile cleanly with no errors or warnings related to ReflectionsSection.</verify>
  <done>Both tap accumulation loops (current shape and pending shape) clamp delay reads to minimum 1ms floor. No reflection tap can produce a delay shorter than 1ms regardless of Room Size setting.</done>
</task>

<task type="auto">
  <name>Task 2: Add allpass decorrelation chain to dry signal path in MixSection</name>
  <files>Source/dsp/MixSection.h, Source/dsp/MixSection.cpp</files>
  <action>
Add 3 short allpass filters to the dry signal path inside MixSection. These decorrelate the dry signal slightly before mixing with wet, reducing the coherence that causes sharp comb notches when the wet signal (which has accumulated phase shifts through 6 DSP stages) mixes with the clean dry.

**MixSection.h changes:**

1. Add `#include <signalsmith-dsp/filters.h>` at the top (after JuceHeader.h).

2. Add a static constexpr for the number of allpass stages:
   ```
   static constexpr int kDryDecorrelationStages = 3;
   ```

3. Add private members -- two arrays (L/R) of BiquadStatic allpass filters:
   ```
   signalsmith::filters::BiquadStatic<float> dryDecorrelationL[kDryDecorrelationStages];
   signalsmith::filters::BiquadStatic<float> dryDecorrelationR[kDryDecorrelationStages];
   double storedSampleRate = 44100.0;
   ```

**MixSection.cpp changes:**

1. In prepare(): After the existing setup, store sampleRate and configure the allpass filters. Use coprime-ish delay times to avoid creating their own comb artifacts:
   ```
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
   ```
   The 1.12x R-channel offset follows the same pattern used in AirSection for stereo decorrelation.

2. In pushDrySamples(): Process the dry audio block through the allpass chain BEFORE passing to dryWetMixer. The input is a const AudioBlock, so we need to process into a temporary buffer:
   ```
   void MixSection::pushDrySamples (const juce::dsp::AudioBlock<float>& dryBlock)
   {
       // Apply allpass decorrelation to dry signal before mixing
       // This reduces coherence with the phase-shifted wet signal,
       // softening comb-filter notches in the 800Hz-3kHz range
       const auto numSamples = dryBlock.getNumSamples();
       const auto numChannels = dryBlock.getNumChannels();

       // Use a small heap-free stack buffer for typical block sizes
       // For the decorrelated copy we need to modify the samples
       juce::AudioBuffer<float> decorrelated (static_cast<int> (numChannels),
                                               static_cast<int> (numSamples));

       // Copy dry block into mutable buffer
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
   ```

3. In reset(): Add filter resets:
   ```
   for (int i = 0; i < kDryDecorrelationStages; ++i)
   {
       dryDecorrelationL[i].reset();
       dryDecorrelationR[i].reset();
   }
   ```

Note: Allpass filters are unity gain (no level change), so auto-gain compensation needs no adjustment. The phase smearing is subtle (sub-2ms equivalent delays) and will not be perceptible as a separate effect -- it only reduces the sharpness of comb notches when dry and wet signals are summed.
  </action>
  <verify>Build with `cmake --build build --target Aether 2>&1 | tail -5` -- must compile cleanly. Grep for "dryDecorrelation" in MixSection files to confirm all 3 integration points (prepare, pushDrySamples, reset).</verify>
  <done>MixSection applies a 3-stage allpass decorrelation chain to the dry signal before mixing with wet. Filters are configured in prepare(), applied in pushDrySamples(), and cleared in reset(). R-channel uses 1.12x frequency offset for stereo decorrelation matching AirSection convention.</done>
</task>

</tasks>

<verification>
1. `cmake --build build --target Aether` compiles without errors
2. Grep confirms minDelaySamples used in both tap loops in ReflectionsSection.cpp
3. Grep confirms dryDecorrelation members in MixSection.h and all three methods in MixSection.cpp
4. No other files modified (PluginProcessor unchanged -- MixSection API is the same)
</verification>

<success_criteria>
- ReflectionsSection enforces 1ms minimum delay on all 16 tap reads (both current and pending shape loops)
- MixSection decorrelates dry signal through 3 allpass stages before pushing to DryWetMixer
- Full build succeeds with no new warnings
- Public API of MixSection unchanged (PluginProcessor needs no modifications)
</success_criteria>

<output>
After completion, create `.planning/quick/2-fix-comb-filtering-minimum-delay-floor-d/2-SUMMARY.md`
</output>
