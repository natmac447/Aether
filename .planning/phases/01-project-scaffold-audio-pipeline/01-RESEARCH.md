# Phase 1: Project Scaffold & Audio Pipeline - Research

**Researched:** 2026-02-18
**Domain:** JUCE 8 C++ audio plugin scaffold, CMake build system, APVTS parameter tree, real-time audio pipeline
**Confidence:** HIGH

## Summary

Phase 1 establishes the complete foundation for Aether: a JUCE 8 audio plugin that builds via CMake, loads in a DAW as VST3, passes audio through, and exposes all parameters (for all 6 stages plus output) in the DAW automation list with smoothing. The Crucible project at `~/Projects/Crucible` provides a proven, production-tested reference for every pattern needed -- CMake FetchContent setup, APVTS parameter layout, DSP section interface design, processBlock flow, parameter smoothing, latency reporting, and state save/restore. Aether adapts these patterns for its fixed 6-stage serial architecture (no reorderable sections, no crossfade routing), which is structurally simpler than Crucible's reorderable 3-section chain.

The key technical decisions for this phase center on (1) defining ~30 parameters across 6 stages plus output, with correct ranges, defaults, and display formatting from the design handoff; (2) building the processBlock skeleton that captures dry input, chains 6 stage placeholders, applies mix/output processing with auto-gain compensation; and (3) establishing real-time safety patterns (allocation-free audio thread, SmoothedValue for all continuous parameters, ScopedNoDenormals, sample-rate awareness) that every subsequent DSP phase inherits. Signalsmith DSP v1.7.1 is included from day one as a FetchContent dependency, providing the delay lines, filters, mixing matrices (Hadamard/Householder), and oversampling infrastructure that later phases will consume.

**Primary recommendation:** Follow Crucible's proven architecture closely -- Parameters.h for centralized IDs/ranges/defaults, DSP section classes with prepare/process/reset interface, cached atomic parameter pointers in the processor -- adapting only where Aether's fixed 6-stage serial flow diverges from Crucible's reorderable chain.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- Claude reviews Crucible's project structure, CMake patterns, and code organization, then decides what to follow vs adapt for Aether's 6-stage architecture
- Dependency versions: Claude evaluates what's worth upgrading (JUCE 8.0.12 is already decided; Catch2 and other deps are Claude's call)
- Crucible patterns (DSP section interface, APVTS usage, etc.): Claude reviews and adopts what works, adapts what doesn't fit
- **Fully standalone** -- no shared code submodules or cross-repo dependencies. Copy patterns, don't link to them.
- Claude decides naming convention for DAW automation list (stage-prefixed, flat, or numbered)
- Claude decides text vs numeric display for text-label parameters (Room Size, Shape, Proximity)
- **Bypass parameters must be automatable** -- they appear in the DAW's automation list, not UI-only
- Claude decides internal parameter ID convention (stage-prefixed vs flat)
- **Manufacturer name:** Cairn
- **Manufacturer code (AU):** Carn
- **Plugin code (AU):** Aeth
- **Plugin category:** Reverb (for DAW browser discoverability)
- **Include Signalsmith DSP v1.7.1 from Phase 1** -- add as CMake FetchContent dependency in the scaffold
- Open to additional DSP libraries in later phases if research recommends them
- Claude decides whether to include test framework (Catch2 + pluginval) from Phase 1 or defer

### Claude's Discretion
- Project directory layout and file organization (informed by Crucible review)
- Dependency version choices beyond JUCE 8.0.12
- Parameter naming convention and display format for DAW
- Internal parameter ID format
- Whether to include Catch2/pluginval in scaffold or defer to Phase 8
- DSP section base class/interface design
- processBlock skeleton structure

### Deferred Ideas (OUT OF SCOPE)
None -- discussion stayed within phase scope
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| BUILD-04 | CMake build system (not Projucer) with JUCE 8 | Crucible CMakeLists.txt provides exact pattern: FetchContent for JUCE 8.0.12, juce_add_plugin with correct metadata, juce_generate_juce_header, target_sources, compile definitions, link libraries. Adapt directly. |
| ENG-01 | All knob parameters smoothed (10-50ms ramp time) to prevent zipper noise | Crucible uses `juce::SmoothedValue<float, ValueSmoothingTypes::Multiplicative>` with 20ms ramp for gain params. Pattern: reset(sampleRate, timeInSeconds) in prepareToPlay, setTargetValue + getNextValue per sample in processBlock. |
| ENG-02 | All delay/filter frequencies sample-rate aware (44.1-192 kHz) | DSP section prepare() receives sampleRate; all time-based calculations use it. Signalsmith delay/filter classes accept sample rate. Crucible pattern: store currentSampleRate, recalculate coefficients when it changes. |
| ENG-03 | Plugin reports latency via setLatencySamples() for DAW PDC | Crucible calls setLatencySamples() in prepareToPlay and updates it in processBlock when oversampling changes. Phase 1 reports 0 latency (no DSP yet); later phases update when DSP introduces latency. |
| ENG-04 | Processing works with any buffer size (no buffer size assumptions) | Crucible processes sample-by-sample within whatever buffer the host provides. No hardcoded buffer sizes. Pre-allocate work buffers in prepareToPlay using samplesPerBlock as max. |
| ENG-05 | Audio thread is allocation-free (no malloc/new in processBlock) | Crucible pattern: pre-allocate all buffers in prepareToPlay, use stack arrays and SmoothedValue (no heap ops). ScopedNoDenormals at processBlock top. No juce::String construction on audio thread. |
| ENG-06 | Signal flow: Input -> I -> II -> III -> IV -> V -> VI -> Mix -> Output | processBlock skeleton chains 6 stage.process(buffer) calls in fixed order. Simpler than Crucible's reorderable sections -- no lookup table or crossfading needed. |
| OUT-01 | Mix knob blends dry/wet (0-100%, default 70%) | Use JUCE's dsp::DryWetMixer with sin3dB or balanced mixing rule. pushDrySamples() at input capture, mixWetSamples() after all 6 stages. Handles latency compensation internally. |
| OUT-02 | Output knob trims level (-24dB to +6dB, default 0.0dB) | Direct adaptation of Crucible's OutputSection: SmoothedValue<Multiplicative> with 20ms ramp, Decibels::decibelsToGain conversion. Process after mix. |
| OUT-03 | Mix auto-gain compensation (reduce output as Mix increases) | Implement as a gain curve applied after dry/wet mix: at Mix=0% no compensation, at Mix=100% apply -2 to -3dB reduction. Use a simple polynomial or lookup. SmoothedValue to avoid clicks. |
| OUT-04 | Dry signal tapped at input (pre all processing) | DryWetMixer.pushDrySamples() called immediately after reading input buffer, before any stage processing. This is the standard JUCE DryWetMixer workflow. |
</phase_requirements>

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| JUCE | 8.0.12 | Audio plugin framework (VST3/AU/Standalone, APVTS, DSP module) | Industry standard for C++ audio plugins. Locked decision. |
| Signalsmith DSP | 1.7.1 | Delay lines, filters, mixing matrices (Hadamard/Householder), oversampling | Header-only, MIT licensed, provides exactly the primitives Aether's 6-stage architecture needs (delay networks, orthogonal matrices for FDN, quality interpolators). Locked decision. |
| CMake | >= 3.22 | Build system | Required by JUCE's CMake API. Crucible uses 3.22 minimum. |

### Supporting (Recommendation: Include from Phase 1)
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| Catch2 | 3.8.1 | Unit testing framework | **Recommend including from Phase 1.** Cost is negligible (FetchContent, builds only when tests are built). Having the test infrastructure from day one means parameter range tests, sample-rate sweep tests, and allocation-free verification can be written alongside the scaffold rather than retrofitted in Phase 8. Crucible already uses Catch2. |

### Supporting (Defer to Phase 8)
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| pluginval | latest | Plugin host compatibility validation | Defer to Phase 8. Requires a built plugin binary, run as an external tool via script. Include validate.sh script from Phase 1 (copied from Crucible pattern) but actual validation runs in Phase 8. |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Signalsmith delay/filter | JUCE dsp:: module delays/filters | JUCE's dsp:: module is adequate but Signalsmith provides higher-quality interpolation (Kaiser-sinc), Hadamard/Householder matrices for FDN, and StereoMultiMixer -- all critical for Aether's reverb architecture. Use Signalsmith for DSP primitives, JUCE dsp:: for DryWetMixer and SmoothedValue. |
| Catch2 | JUCE built-in UnitTest | JUCE UnitTest is simpler but less feature-rich. Catch2 is the C++ ecosystem standard and Crucible already uses it. Keep consistency. |

### Installation / CMake Setup

```cmake
cmake_minimum_required(VERSION 3.22)
project(Aether VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version")

include(FetchContent)

# JUCE 8.0.12
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Signalsmith DSP v1.7.1
FetchContent_Declare(
    signalsmith-dsp
    GIT_REPOSITORY https://github.com/Signalsmith-Audio/dsp.git
    GIT_TAG v1.7.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(signalsmith-dsp)

# Catch2 (test framework)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)
```

## Architecture Patterns

### Recommended Project Structure

```
Aether/
├── CMakeLists.txt                    # Main build file
├── Resources/
│   └── fonts/                        # Embedded fonts (Phase 6, but create dir now)
├── Source/
│   ├── PluginProcessor.h             # Main processor with APVTS
│   ├── PluginProcessor.cpp           # processBlock, parameter caching, state
│   ├── PluginEditor.h                # Editor shell (minimal Phase 1)
│   ├── PluginEditor.cpp              # Just sets window size
│   ├── Parameters.h                  # All param IDs, defaults, ranges (central)
│   └── dsp/                          # DSP section classes
│       ├── StageSection.h            # Base interface (prepare/process/reset)
│       ├── CabinetSection.h/.cpp     # Stage I placeholder
│       ├── ReflectionsSection.h/.cpp # Stage II placeholder
│       ├── AirSection.h/.cpp         # Stage III placeholder
│       ├── ExcitationSection.h/.cpp  # Stage IV placeholder
│       ├── RoomToneSection.h/.cpp    # Stage V placeholder
│       ├── DiffuseTailSection.h/.cpp # Stage VI placeholder
│       ├── MixSection.h/.cpp         # Dry/wet mix with auto-gain
│       └── OutputSection.h/.cpp      # Output trim (from Crucible pattern)
├── Tests/                            # Catch2 test files
│   ├── CMakeLists.txt                # Test target
│   ├── ParameterTests.cpp            # Verify all params registered, ranges correct
│   └── AudioPipelineTests.cpp        # Verify passthrough, mix behavior
└── scripts/
    └── validate.sh                   # pluginval runner (from Crucible)
```

**Key differences from Crucible:**
- Crucible has reorderable sections (EQ/Sat/Comp with lookup table and crossfade). Aether has a fixed serial chain (I through VI) -- simpler processBlock, no crossfade machinery.
- Crucible has ~60 parameters across 4 main sections. Aether has ~30 parameters across 6 stages + output -- more sections but fewer params per section.
- Crucible has no DryWetMixer (output gain + clipper are final stages). Aether needs DryWetMixer for the global Mix knob that blends pre-processing dry with post-processing wet.

### Pattern 1: Centralized Parameter Definitions (Parameters.h)

**What:** All parameter IDs, default values, and ranges in a single header file.
**When to use:** Always. This is the single source of truth for parameter metadata.
**Why:** Crucible uses this pattern successfully -- both PluginProcessor and PluginEditor reference the same constants. Prevents ID typos, default drift, and range mismatches.

```cpp
// Source: Crucible/Source/Parameters.h (adapted for Aether)
#pragma once

namespace ParamIDs
{
    // Stage I: Cabinet Resonance
    inline constexpr auto cabBody     { "cab_body" };
    inline constexpr auto cabType     { "cab_type" };
    inline constexpr auto cabBypass   { "cab_bypass" };

    // Stage II: Early Reflections
    inline constexpr auto reflSize    { "refl_size" };
    inline constexpr auto reflShape   { "refl_shape" };
    inline constexpr auto reflProx    { "refl_prox" };
    inline constexpr auto reflBypass  { "refl_bypass" };

    // Stage III: Air & Distance
    inline constexpr auto airAmount   { "air_amount" };
    inline constexpr auto airChar     { "air_char" };
    inline constexpr auto airBypass   { "air_bypass" };

    // Stage IV: Excitation
    inline constexpr auto excitDrive  { "excit_drive" };
    inline constexpr auto excitBypass { "excit_bypass" };

    // Stage V: Room Tone
    inline constexpr auto toneAmb     { "tone_amb" };
    inline constexpr auto toneBypass  { "tone_bypass" };

    // Stage VI: Diffuse Tail
    inline constexpr auto tailDecay   { "tail_decay" };
    inline constexpr auto tailDiff    { "tail_diff" };
    inline constexpr auto tailBypass  { "tail_bypass" };

    // Output
    inline constexpr auto outMix      { "out_mix" };
    inline constexpr auto outLevel    { "out_level" };
}
```

**Parameter ID Naming Convention (RECOMMENDATION):**

Use **stage-prefixed snake_case** IDs (e.g., `cab_body`, `refl_size`, `air_amount`). Rationale:
- Stage prefix groups related parameters in sorted lists (DAW automation menus often sort alphabetically)
- Short prefixes (3-5 chars) keep IDs readable: `cab_`, `refl_`, `air_`, `excit_`, `tone_`, `tail_`, `out_`
- Consistent with Crucible's convention (`eq_`, `clip_`, `sat_`, `comp_`)
- Internal IDs are the same as automation IDs (no mapping layer needed)

**DAW Automation Display Names (RECOMMENDATION):**

Use **"Stage - Parameter"** format for the display name passed to `AudioParameterFloat`/`AudioParameterBool`/`AudioParameterChoice`:
- "I Cabinet - Body"
- "II Reflections - Room Size"
- "III Air - Amount"
- "IV Excitation - Drive"
- "V Room Tone - Ambience"
- "VI Tail - Decay"
- "Output - Mix"
- "Output - Level"

This gives a clear, scannable automation list. The Roman numeral prefix matches the plugin's visual hierarchy and sorts correctly (I, II, III, IV, V, VI).

### Pattern 2: DSP Section Interface

**What:** Each DSP stage implements prepare/process/reset with setter methods for parameters.
**When to use:** Every DSP stage in Aether.

```cpp
// Source: Crucible DSP sections (OutputSection, EQSection, SaturationSection pattern)
class StageSection
{
public:
    virtual ~StageSection() = default;

    virtual void prepare(double sampleRate, int samplesPerBlock) = 0;
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;
    virtual void reset() = 0;
    virtual void setBypass(bool bypassed) = 0;
};
```

**Note:** Crucible does NOT use a virtual base class -- each section is a concrete class with the same interface by convention. For Aether, **use concrete classes (no virtual base)** to match Crucible and avoid virtual dispatch overhead on the audio thread. The interface is enforced by convention and compilation, not inheritance.

### Pattern 3: processBlock Skeleton (Fixed Serial Chain)

**What:** The processBlock flow for Aether's fixed I-through-VI stage chain.
**When to use:** The main audio processing entry point.

```cpp
// Source: Adapted from Crucible processBlock pattern
void AetherProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Clear unused output channels
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Capture dry signal for mix (pre all processing)
    dryWetMixer.pushDrySamples(
        juce::dsp::AudioBlock<float>(buffer));

    // Fixed serial chain: I -> II -> III -> IV -> V -> VI
    updateStageParams();  // Read all atomic params, set on sections
    cabinetSection.process(buffer);
    reflectionsSection.process(buffer);
    airSection.process(buffer);
    excitationSection.process(buffer);
    roomToneSection.process(buffer);
    diffuseTailSection.process(buffer);

    // Mix dry/wet
    dryWetMixer.mixWetSamples(
        juce::dsp::AudioBlock<float>(buffer));

    // Auto-gain compensation for mix
    applyMixAutoGain(buffer);

    // Output trim
    outputSection.process(buffer, outLevelParam->load());

    // Report latency
    setLatencySamples(computeTotalLatency());
}
```

### Pattern 4: Parameter Caching with Atomic Pointers

**What:** Cache `std::atomic<float>*` pointers in the constructor for zero-overhead audio-thread parameter access.
**When to use:** Every parameter read in processBlock.

```cpp
// Source: Crucible PluginProcessor constructor pattern
AetherProcessor::AetherProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache parameter pointers (done once, used every processBlock)
    cabBodyParam   = apvts.getRawParameterValue(ParamIDs::cabBody);
    cabTypeParam   = apvts.getRawParameterValue(ParamIDs::cabType);
    cabBypassParam = apvts.getRawParameterValue(ParamIDs::cabBypass);
    // ... all parameters cached similarly
}
```

### Pattern 5: Text-Label Parameters (Room Size, Shape, Proximity)

**What:** Continuous float parameters (0.0-1.0) that display as text labels in the DAW.
**When to use:** Room Size (Small->Large), Shape (Regular->Irregular), Proximity (Near->Far).
**Recommendation:** Use `AudioParameterFloat` with `withStringFromValueFunction` that maps the normalized 0.0-1.0 range to descriptive text.

```cpp
// Room Size: continuous 0.0-1.0, displayed as text labels
params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID { ParamIDs::reflSize, 1 },
    "II Reflections - Room Size",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
    0.4f,  // default: "Medium" region
    juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float value, int) {
            if (value < 0.2f)  return juce::String("Small");
            if (value < 0.4f)  return juce::String("Sm-Med");
            if (value < 0.6f)  return juce::String("Medium");
            if (value < 0.8f)  return juce::String("Med-Lg");
            return juce::String("Large");
        })
        .withValueFromStringFunction([](const juce::String& text) {
            if (text == "Small")  return 0.1f;
            if (text == "Medium") return 0.5f;
            if (text == "Large")  return 0.9f;
            return text.getFloatValue();
        })
));
```

**Rationale for float + text over AudioParameterChoice:** The DSP treats these as continuous values (Room Size smoothly scales delay times from 1-30ms). `AudioParameterChoice` would force discrete steps and complicate the DSP mapping. `AudioParameterFloat` with text labels gives smooth DAW automation while showing meaningful text in the UI and automation list.

### Anti-Patterns to Avoid

- **Heap allocation in processBlock:** No `new`, `std::vector::push_back`, `juce::String` construction, `std::map` lookups. Pre-allocate everything in prepareToPlay.
- **Virtual dispatch in the audio loop:** Use concrete section classes, not virtual base class pointers. Virtual dispatch adds indirection and cache misses on the hot path.
- **Blocking operations on audio thread:** No mutex locks, file I/O, or GUI operations. Use atomics for parameter transfer (already handled by APVTS).
- **Assuming buffer size:** Never use a fixed buffer size. Always iterate `buffer.getNumSamples()`. Pre-allocate work buffers to `samplesPerBlock` from prepareToPlay.
- **Ignoring denormals:** Always use `juce::ScopedNoDenormals` at the top of processBlock. Denormal numbers cause severe CPU spikes on x86 when processing near-silence.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Dry/wet mixing with latency compensation | Custom delay line + crossfade math | `juce::dsp::DryWetMixer` | Handles latency compensation, multiple mixing rules (sin3dB, balanced, etc.), and smoothing internally. Battle-tested in JUCE's own examples. |
| Parameter smoothing | Custom one-pole filter or linear ramp | `juce::SmoothedValue<float, Multiplicative>` | Handles reset, ramp time, isSmoothing() optimization. Crucible uses it everywhere. |
| Gain in dB | Manual pow(10, db/20) conversion | `juce::Decibels::decibelsToGain()` / `gainToDecibels()` | Handles -infinity (mute), consistent across codebase. |
| Plugin state save/restore | Custom XML serialization | `apvts.copyState()` / `replaceState()` + `createXml()` / `fromXml()` | Standard JUCE pattern, handles parameter versioning via ParameterID version number. |
| Delay lines for DSP | Custom circular buffer | `signalsmith::delay::Delay<float>` | Provides multiple interpolation qualities (linear, cubic, Kaiser-sinc), tested performance, correct wraparound. |
| Orthogonal mixing matrices | Manual Hadamard matrix code | `signalsmith::mix::Hadamard<N>` / `Householder<N>` | Correct, tested, in-place operation. Critical for FDN in later phases. |
| Oversampling | Custom FIR upsampling/downsampling | `signalsmith::rates::Oversampler2xFIR` or `juce::dsp::Oversampling` | Complex to get right (aliasing, latency reporting). Both libraries handle it correctly. |

**Key insight:** Audio DSP has extreme correctness requirements -- a tiny smoothing bug causes audible clicks heard by every user. Use battle-tested library components for infrastructure, reserve custom code for Aether's unique DSP algorithms (room modeling, cabinet resonance).

## Common Pitfalls

### Pitfall 1: Parameter Version Mismatch
**What goes wrong:** Changing parameter IDs or ranges after users have saved sessions causes state recall failures -- parameters reset to defaults or silently map wrong values.
**Why it happens:** `ParameterID` in JUCE includes a version number. If you change ranges without incrementing the version, old saved states apply old values to new ranges incorrectly.
**How to avoid:** Use version 1 for all parameters from day one (as Crucible does: `juce::ParameterID { "param_id", 1 }`). Only increment when you intentionally change parameter semantics.
**Warning signs:** Parameters snapping to wrong values when loading old sessions; DAW reporting parameter mismatch warnings.

### Pitfall 2: DryWetMixer Latency Mismatch
**What goes wrong:** The Mix knob causes phasing/comb filtering artifacts because the dry path isn't delay-compensated to match the wet path's latency.
**Why it happens:** If any DSP stage introduces latency (oversampling, lookahead), the dry signal arrives early. DryWetMixer compensates, but only if `setWetLatency()` is called correctly.
**How to avoid:** Call `dryWetMixer.setWetLatency(totalWetPathLatency)` in prepareToPlay and whenever latency changes. In Phase 1, wet latency is 0 (no DSP). Update in later phases.
**Warning signs:** Audible comb filtering or hollow sound when Mix is around 50%. Mono-summing test reveals cancellation.

### Pitfall 3: SmoothedValue Not Reset on prepareToPlay
**What goes wrong:** When the DAW changes sample rate or buffer size, smoothed values retain stale state, causing parameter jumps or extra-long ramps.
**Why it happens:** prepareToPlay is called with new sample rate, but SmoothedValue still has the old ramp rate.
**How to avoid:** In prepareToPlay, call `reset(newSampleRate, rampTimeInSeconds)` then `setCurrentAndTargetValue(currentValue)` for every SmoothedValue. Crucible does this consistently.
**Warning signs:** Clicking when changing sample rate in DAW, or parameters taking unusually long to reach target after sample rate change.

### Pitfall 4: Forgetting ScopedNoDenormals
**What goes wrong:** CPU spikes during quiet passages or reverb tails, causing audio dropouts.
**Why it happens:** IEEE floating-point denormal numbers (very small values near zero) are orders of magnitude slower to process than normal floats. Reverb and delay tails naturally produce denormals as they decay.
**How to avoid:** `juce::ScopedNoDenormals noDenormals;` as the first line of processBlock. This is a RAII guard -- it sets the CPU flush-to-zero flag for the scope.
**Warning signs:** CPU meter spikes during silence or decay tails; users report occasional dropouts.

### Pitfall 5: AU Manufacturer/Plugin Code Constraints
**What goes wrong:** AU validation fails or plugin doesn't appear in Logic Pro's plugin list.
**Why it happens:** AU manufacturer code must have at least one uppercase letter. Plugin code must have exactly one uppercase letter. GarageBand requires first letter uppercase, rest lowercase.
**How to avoid:** Manufacturer code "Carn" (C uppercase, rest lowercase) and plugin code "Aeth" (A uppercase, rest lowercase) already satisfy these constraints. Verify with AU validation tool after build.
**Warning signs:** Plugin missing from DAW scanner, auval returning errors.

### Pitfall 6: Allocating Work Buffers on Audio Thread
**What goes wrong:** Occasional audio glitches (clicks, dropouts) especially at high sample rates or small buffer sizes.
**Why it happens:** `juce::AudioBuffer<float>` constructor allocates heap memory. Creating temporary buffers in processBlock triggers malloc on the audio thread.
**How to avoid:** Pre-allocate ALL work buffers as member variables, size them in prepareToPlay with `setSize(channels, samplesPerBlock)`. Only call `getWritePointer()` / `getReadPointer()` in processBlock.
**Warning signs:** Intermittent clicks that worsen under CPU load; profiler showing malloc in audio callback.

## Code Examples

### Complete Plugin CMake Configuration

```cmake
# Source: Adapted from Crucible/CMakeLists.txt for Aether
juce_add_plugin(Aether
    COMPANY_NAME "Cairn"
    PLUGIN_MANUFACTURER_CODE Carn
    PLUGIN_CODE Aeth

    FORMATS VST3        # VST3 only for Phase 1; AU/Standalone added Phase 8

    PRODUCT_NAME "Aether"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    VST3_CATEGORIES Reverb
    AU_MAIN_TYPE kAudioUnitType_Effect

    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(Aether)

target_sources(Aether
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Parameters.h
        Source/dsp/CabinetSection.cpp
        Source/dsp/CabinetSection.h
        Source/dsp/ReflectionsSection.cpp
        Source/dsp/ReflectionsSection.h
        Source/dsp/AirSection.cpp
        Source/dsp/AirSection.h
        Source/dsp/ExcitationSection.cpp
        Source/dsp/ExcitationSection.h
        Source/dsp/RoomToneSection.cpp
        Source/dsp/RoomToneSection.h
        Source/dsp/DiffuseTailSection.cpp
        Source/dsp/DiffuseTailSection.h
        Source/dsp/MixSection.cpp
        Source/dsp/MixSection.h
        Source/dsp/OutputSection.cpp
        Source/dsp/OutputSection.h
)

target_compile_definitions(Aether
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Aether
    PRIVATE
        signalsmith-dsp
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
```

### Output Section with Smoothing (Proven Crucible Pattern)

```cpp
// Source: Crucible/Source/dsp/OutputSection.cpp (direct adaptation)
void OutputSection::prepare(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    gainSmoothed.reset(sampleRate, 0.020);  // 20ms ramp
    gainSmoothed.setCurrentAndTargetValue(1.0f);
}

void OutputSection::process(juce::AudioBuffer<float>& buffer, float gainDb)
{
    float targetGain = juce::Decibels::decibelsToGain(gainDb, -100.0f);
    gainSmoothed.setTargetValue(targetGain);

    if (gainSmoothed.isSmoothing())
    {
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float gain = gainSmoothed.getNextValue();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample(ch, s, buffer.getSample(ch, s) * gain);
        }
    }
    else
    {
        buffer.applyGain(gainSmoothed.getCurrentValue());
    }
}
```

### State Save/Restore (Proven Crucible Pattern)

```cpp
// Source: Crucible/Source/PluginProcessor.cpp
void AetherProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AetherProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
```

### Mix Auto-Gain Compensation Approach

```cpp
// Custom implementation (not from a library -- this is Aether-specific)
// As mix increases from 0% to 100%, apply a small gain reduction
// to compensate for the reverb adding energy (perceived loudness increase).
// Curve: 0% mix = 0dB, 50% mix = -1dB, 100% mix = -2.5dB
// Uses a simple quadratic: compensation_dB = -2.5 * mix^1.5
void MixSection::process(juce::AudioBuffer<float>& buffer, float mixValue)
{
    float compensationDb = -2.5f * std::pow(mixValue, 1.5f);
    float compensationGain = juce::Decibels::decibelsToGain(compensationDb);

    compensationSmoothed.setTargetValue(compensationGain);

    // Apply smoothed compensation
    if (compensationSmoothed.isSmoothing())
    {
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float g = compensationSmoothed.getNextValue();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample(ch, s, buffer.getSample(ch, s) * g);
        }
    }
    else
    {
        buffer.applyGain(compensationSmoothed.getCurrentValue());
    }
}
```

## Complete Parameter Inventory

All parameters that must be registered in Phase 1's APVTS, with ranges from the design handoff:

### Stage I: Cabinet Resonance
| Parameter | ID | Type | Range | Default | Display |
|-----------|----|------|-------|---------|---------|
| Body | `cab_body` | Float | 0.0-1.0 | 0.5 | Percentage (0-100%) |
| Cabinet Type | `cab_type` | Choice | Open/Closed/Combo | 1 (Closed) | Text |
| Bypass | `cab_bypass` | Bool | In/Out | false (In) | In/Out |

### Stage II: Early Reflections
| Parameter | ID | Type | Range | Default | Display |
|-----------|----|------|-------|---------|---------|
| Room Size | `refl_size` | Float | 0.0-1.0 | 0.4 | Text label (Small->Large) |
| Shape | `refl_shape` | Float | 0.0-1.0 | 0.4 | Text label (Regular->Irregular) |
| Proximity | `refl_prox` | Float | 0.0-1.0 | 0.3 | Text label (Near->Far) |
| Bypass | `refl_bypass` | Bool | In/Out | false (In) | In/Out |

### Stage III: Air & Distance
| Parameter | ID | Type | Range | Default | Display |
|-----------|----|------|-------|---------|---------|
| Air | `air_amount` | Float | 0.0-1.0 | 0.4 | Percentage (0-100%) |
| Character | `air_char` | Choice | Warm/Neutral | 0 (Warm) | Text |
| Bypass | `air_bypass` | Bool | In/Out | false (In) | In/Out |

### Stage IV: Excitation
| Parameter | ID | Type | Range | Default | Display |
|-----------|----|------|-------|---------|---------|
| Drive | `excit_drive` | Float | 0.0-1.0 | 0.25 | Percentage (0-100%) |
| Bypass | `excit_bypass` | Bool | In/Out | false (In) | In/Out |

### Stage V: Room Tone
| Parameter | ID | Type | Range | Default | Display |
|-----------|----|------|-------|---------|---------|
| Ambience | `tone_amb` | Float | 0.0-1.0 | 0.1 | Percentage (0-100%) |
| Bypass | `tone_bypass` | Bool | In/Out | true (Out) | In/Out |

### Stage VI: Diffuse Tail
| Parameter | ID | Type | Range | Default | Display |
|-----------|----|------|-------|---------|---------|
| Decay | `tail_decay` | Float | 50.0-500.0 | 150.0 | Milliseconds |
| Diffusion | `tail_diff` | Float | 0.0-1.0 | 0.6 | Percentage (0-100%) |
| Bypass | `tail_bypass` | Bool | In/Out | false (In) | In/Out |

### Output
| Parameter | ID | Type | Range | Default | Display |
|-----------|----|------|-------|---------|---------|
| Mix | `out_mix` | Float | 0.0-1.0 | 0.7 | Percentage (0-100%) |
| Output | `out_level` | Float | -24.0-6.0 | 0.0 | Decibels |

**Total parameters: 20** (12 float + 2 choice + 6 bool)
All 6 bypass parameters are `AudioParameterBool` and automatable (locked decision).

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Projucer project management | CMake with FetchContent | JUCE 6+ (standardized in 7-8) | No Projucer dependency, CI/CD friendly, version-pinned deps |
| AudioParameterFloat without ParameterID version | ParameterID with version number | JUCE 7 | Enables safe parameter evolution without breaking saved sessions |
| Manual dB gain smoothing | SmoothedValue<Multiplicative> | JUCE 5+ | Exponential smoothing is more perceptually correct for gain than linear |
| Custom dry/wet with manual delay compensation | dsp::DryWetMixer with setWetLatency() | JUCE 6+ | Built-in latency compensation, multiple mixing rules |
| JUCE dsp::DelayLine for all delays | Signalsmith delay::Delay with Kaiser-sinc interpolation | Available since Signalsmith DSP 1.4+ | Higher quality interpolation for modulated delays, critical for reverb quality |

**Deprecated/outdated:**
- Projucer: Still maintained but CMake is the recommended path for new projects
- JUCE_FORCE_USE_LEGACY_PARAM_IDS: Only needed for VST2->VST3 migration (Aether is new, use modern ParameterID from start)
- `createEditor()` returning `GenericAudioProcessorEditor` for parameter testing: Still useful for Phase 1 before custom UI exists

## Open Questions

1. **Mix auto-gain compensation curve**
   - What we know: Design handoff says "slightly reduces output as Mix increases." Crucible doesn't have a global mix, so no reference pattern.
   - What's unclear: The exact dB reduction curve. The example uses -2.5dB at 100% mix with a power curve, but the optimal value depends on the actual DSP content (how much energy the reverb adds).
   - Recommendation: Start with -2.5dB at 100% mix using a smooth power curve. This is a tuning parameter -- revisit in Phase 5 when all DSP stages are active and real audio can be measured. Make the compensation curve a named constant, not a magic number.

2. **DryWetMixer mixing rule selection**
   - What we know: JUCE offers linear, balanced, sin3dB, sin4p5dB, sin6dB, squareRoot3dB, squareRoot4p5dB.
   - What's unclear: Which rule sounds best for a reverb plugin's global mix. Reverb plugins typically use equal-power (sin3dB or squareRoot3dB) to maintain perceived loudness across the mix range.
   - Recommendation: Use `DryWetMixingRule::sin3dB` (standard equal-power crossfade for reverb). Can be changed later if listening tests prefer different rule.

3. **Whether DSP stage placeholders should process or passthrough**
   - What we know: Phase 1 creates the section classes but implements no actual DSP. processBlock calls all 6 stages.
   - What's unclear: Should placeholder process() methods be completely empty (no-op) or explicitly copy input to output? An empty process() with in-place buffer is already a passthrough -- the buffer is unchanged.
   - Recommendation: Placeholder process() methods should be empty no-ops when bypassed=false (buffer passes through unchanged since processing is in-place). When bypassed=true, still no-op (bypass skips the section's DSP, but there's no DSP yet). Add a comment in each explaining what the section will do in its target phase.

## Signalsmith DSP Library Deep Dive

**Version:** 1.7.1 (released Jan 22, 2026 -- fix for sqrt call in fft.h)
**License:** MIT
**Integration:** Header-only, CMake target `signalsmith-dsp` via FetchContent

### Modules Available for Aether

| Module | Header | Aether Use Case | Phase |
|--------|--------|-----------------|-------|
| Delay utilities | `dsp/delay.h` | Early Reflections multi-tap delays, Cabinet short delays, Diffuse Tail FDN delays | 2, 3 |
| Basic filters | `dsp/filters.h` | BiquadStatic for all filtering (lowpass, highpass, bandpass, allpass, shelving) | 2, 3, 4 |
| Multichannel mixing | `dsp/mix.h` | Hadamard/Householder matrices for FDN, StereoMultiMixer, cheapEnergyCrossfade | 2, 3 |
| Multi-rate processing | `dsp/rates.h` | Oversampler2xFIR for Excitation stage oversampling | 5 |
| Envelopes | `dsp/envelopes.h` | Envelope followers if needed for transient detection in Air stage | 4 |
| FFT | `dsp/fft.h` | Spectral processing if needed (likely not in v1) | - |

### Key Classes That Will Be Used

- `signalsmith::delay::Delay<float, InterpolatorCubic>` -- Primary delay line for early reflections and cabinet. Cubic interpolation is good default (clean, low-cost).
- `signalsmith::delay::Delay<float, InterpolatorKaiserSincN<8>>` -- High-quality delay for modulated parameters where interpolation artifacts matter.
- `signalsmith::mix::Hadamard<N>::inPlace(data)` -- Orthogonal matrix for FDN feedback, provides dense reflections.
- `signalsmith::mix::Householder<N>::inPlace(data)` -- Alternative orthogonal matrix (simpler, slightly different sonic character).
- `signalsmith::filters::BiquadStatic<float>` -- All filtering needs (lowpass for air, bandpass for cabinet, allpass for diffusion).
- `signalsmith::mix::cheapEnergyCrossfade` -- For smooth stage bypass transitions.

## Sources

### Primary (HIGH confidence)
- Crucible source code at `~/Projects/Crucible/` -- CMakeLists.txt, Parameters.h, PluginProcessor.h/.cpp, OutputSection.h/.cpp, EQSection.h, SaturationSection.h (directly examined, production-proven patterns)
- Design handoff at `/Users/nathanmcmillan/Downloads/Aether Files/AETHER-DESIGN-HANDOFF.md` -- All parameter ranges, defaults, stage specifications, signal flow (directly examined)
- [JUCE CMake API](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md) -- juce_add_plugin parameters, format options, compile definitions
- [JUCE 8.0.12 Release](https://github.com/juce-framework/JUCE/releases/tag/8.0.12) -- Confirmed release Dec 16, 2025, minor fixes
- [Signalsmith DSP GitHub](https://github.com/Signalsmith-Audio/dsp) -- v1.7.1 tag confirmed (Jan 22, 2026), header-only, MIT licensed
- [JUCE DryWetMixer API](https://docs.juce.com/master/classdsp_1_1DryWetMixer.html) -- DryWetMixingRule enum values, pushDrySamples/mixWetSamples interface
- [JUCE SmoothedValue API](https://docs.juce.com/master/classSmoothedValue.html) -- Multiplicative smoothing for gain, reset/setTargetValue/getNextValue

### Secondary (MEDIUM confidence)
- [Signalsmith DSP mix.h source](https://github.com/Signalsmith-Audio/dsp/blob/main/mix.h) -- Hadamard, Householder, StereoMultiMixer, cheapEnergyCrossfade (confirmed from source)
- [Signalsmith DSP delay.h source](https://github.com/Signalsmith-Audio/dsp/blob/main/delay.h) -- Delay, MultiDelay, Buffer, interpolator classes (confirmed from source)
- [Signalsmith DSP filters.h source](https://github.com/Signalsmith-Audio/dsp/blob/main/filters.h) -- BiquadStatic with multiple design methods (confirmed from source)
- [Signalsmith DSP rates.h source](https://github.com/Signalsmith-Audio/dsp/blob/main/rates.h) -- Oversampler2xFIR for 2x oversampling (confirmed from source)
- [Catch2 CMake integration](https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md) -- FetchContent pattern, v3.8.1 confirmed
- [JUCE Forum: VST3 categories with CMake](https://forum.juce.com/t/solved-vst3-plugin-category-with-cmake/67258) -- VST3_CATEGORIES Reverb, AU_MAIN_TYPE kAudioUnitType_Effect

### Tertiary (LOW confidence)
- None -- all findings verified with primary or secondary sources.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH -- JUCE 8.0.12 and Signalsmith DSP 1.7.1 are locked decisions with verified version tags. Catch2 3.8.1 is a well-established recommendation.
- Architecture: HIGH -- Patterns are directly extracted from the working Crucible codebase, not theoretical. Aether's fixed chain is structurally simpler than Crucible's reorderable sections.
- Pitfalls: HIGH -- All pitfalls are documented in Crucible's code (ScopedNoDenormals, SmoothedValue reset, allocation-free processBlock) or verified from JUCE documentation (AU code constraints, DryWetMixer latency).
- Parameter inventory: HIGH -- All 20 parameters extracted directly from the design handoff document with exact ranges and defaults.

**Research date:** 2026-02-18
**Valid until:** 2026-03-18 (stable domain -- JUCE and Signalsmith DSP release cycles are months apart)
