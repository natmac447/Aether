# Architecture Research

**Domain:** JUCE audio plugin with multi-stage DSP chain (room/environment simulation)
**Researched:** 2026-02-18
**Confidence:** HIGH

Evidence base: Crucible reference implementation (`~/Projects/Crucible`) provides a battle-tested, production architecture for a multi-stage JUCE plugin within the same brand. Aether follows the same patterns with domain-specific adaptations. JUCE APVTS patterns verified against official documentation.

---

## Standard Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                        PLUGIN HOST (DAW)                            │
│  Calls: prepareToPlay(), processBlock(), getStateInformation()      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌───────────────────────────────────────────────────────────┐      │
│  │              AetherProcessor (AudioProcessor)             │      │
│  │                                                           │      │
│  │  ┌─────────────────────────────────────────────────────┐  │      │
│  │  │         APVTS (AudioProcessorValueTreeState)        │  │      │
│  │  │  - All automatable parameters                       │  │      │
│  │  │  - ValueTree properties for non-automatable state   │  │      │
│  │  │  - Serialization (getState/setState)                │  │      │
│  │  └──────────────────────┬──────────────────────────────┘  │      │
│  │                         │ atomic reads                    │      │
│  │  ┌──────────────────────▼──────────────────────────────┐  │      │
│  │  │              DSP PROCESSING CHAIN                   │  │      │
│  │  │                                                     │  │      │
│  │  │  Input ──► [I. CabinetResonance]                    │  │      │
│  │  │            [II. EarlyReflections ]                   │  │      │
│  │  │            [III. AirDistance     ]                   │  │      │
│  │  │            [IV. Excitation       ]                   │  │      │
│  │  │            [V.  RoomTone         ]                   │  │      │
│  │  │            [VI. DiffuseTail      ]                   │  │      │
│  │  │            [MixOutput           ] ──► Output        │  │      │
│  │  │                                                     │  │      │
│  │  │  Each section: prepare() / process() / reset()      │  │      │
│  │  │  Each section: setBypass() + parameter setters      │  │      │
│  │  └─────────────────────────────────────────────────────┘  │      │
│  │                         │ atomics (meter data)            │      │
│  │  ┌──────────────────────▼──────────────────────────────┐  │      │
│  │  │             METER SOURCES (lock-free)               │  │      │
│  │  │  LevelMeterSource, VisualizationData               │  │      │
│  │  └─────────────────────────────────────────────────────┘  │      │
│  └───────────────────────────────────────────────────────────┘      │
│                         │                                           │
│            ┌────────────┤ (message thread boundary)                 │
│            │            │                                           │
│  ┌─────────▼────────────▼─────────────────────────────────────┐    │
│  │              AetherEditor (AudioProcessorEditor)            │    │
│  │                                                             │    │
│  │  ┌────────────────────────────────────────────────────────┐ │    │
│  │  │  ContentWrapper (AffineTransform scaling at 900x530)   │ │    │
│  │  │                                                        │ │    │
│  │  │  ┌──────────┐ ┌──────────────────┐ ┌──────────────┐   │ │    │
│  │  │  │  LEFT    │ │     CENTER       │ │    RIGHT     │   │ │    │
│  │  │  │  PANEL   │ │     PANEL        │ │    PANEL     │   │ │    │
│  │  │  │  220px   │ │     ~460px       │ │    220px     │   │ │    │
│  │  │  │          │ │                  │ │              │   │ │    │
│  │  │  │ I.Cab    │ │ II.ER Label      │ │ III.Air      │   │ │    │
│  │  │  │ IV.Excit │ │ Visualization    │ │ VI.Tail      │   │ │    │
│  │  │  │ V.Tone   │ │ Room Controls    │ │ Output       │   │ │    │
│  │  │  └──────────┘ └──────────────────┘ └──────────────┘   │ │    │
│  │  │                                                        │ │    │
│  │  │  ┌──────────────────────────────────────────────────┐  │ │    │
│  │  │  │  LookAndFeel + Colour/Font System               │  │ │    │
│  │  │  └──────────────────────────────────────────────────┘  │ │    │
│  │  └────────────────────────────────────────────────────────┘ │    │
│  │                                                             │    │
│  │  ┌──────────────────────────────────────────────────────┐   │    │
│  │  │  PresetManager (file I/O, state serialization)       │   │    │
│  │  └──────────────────────────────────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
```

### Component Responsibilities

| Component | Responsibility | Typical Implementation |
|-----------|----------------|------------------------|
| **AetherProcessor** | Owns APVTS, all DSP sections, meter sources. Orchestrates processBlock signal flow. Manages state serialization. | Single class inheriting `juce::AudioProcessor`. Caches `std::atomic<float>*` pointers to all parameters in constructor. Calls each DSP section's `process()` in series. |
| **APVTS** | Thread-safe parameter storage. Serialization via ValueTree XML. Attachment system for UI binding. | `juce::AudioProcessorValueTreeState` constructed with `createParameterLayout()`. Non-automatable state (e.g., UI scale) stored as ValueTree properties. |
| **Parameters.h** | Central definition of all parameter IDs, defaults, and ranges. | Header-only. `ParamIDs` namespace (string constants), `ParamDefaults` namespace (default values), `ParamRanges` namespace (min/max/step). No code duplication between processor and editor. |
| **DSP Sections (6)** | Each section owns its internal DSP state, exposes `prepare()/process()/reset()` and parameter setters. Independently bypassable. | Plain C++ classes (not inheriting from JUCE base classes). Take `AudioBuffer<float>&` by reference in `process()`. Use `juce::SmoothedValue` for parameter smoothing. Pre-allocate work buffers in `prepare()`. |
| **MixOutputSection** | Global dry/wet mix and output gain trim with auto-gain compensation. | Stores dry signal copy at chain input, crossfades with wet output. Uses smoothed gain values. |
| **Meter Sources** | Lock-free audio-to-UI data transfer for visualization and meters. | Atomic CAS pattern (compare-and-swap). Audio thread writes peaks/levels, UI thread reads and resets. |
| **AetherEditor** | Owns all UI components. Three-column layout. ContentWrapper for AffineTransform scaling. | Inherits `juce::AudioProcessorEditor`. Uses ContentWrapper pattern from Crucible for proportional scaling. Hosts section components as children. |
| **Section UI Components** | Per-section UI panels with knobs, toggles, bypass buttons. | Each inherits `juce::Component`. Receives APVTS reference for `SliderAttachment`/`ButtonAttachment`. Uses `juce::Timer` for meter animation (30Hz). |
| **VisualizationComponent** | Center acoustic ray SVG diagram, reactive to Room Size/Shape/Proximity. | Custom `juce::Component` using `juce::Graphics` path drawing. Receives parameter values via atomics or listener callbacks. 4-second breathing animation via `juce::Timer`. |
| **LookAndFeel** | Parchment theme: custom knob rendering, toggle styles, typography, colour palette. | Single class inheriting `juce::LookAndFeel_V4`. Overrides `drawRotarySlider()`, `drawToggleButton()`, `drawButtonBackground()`. Loads custom fonts from binary data. |
| **PresetManager** | Scans preset directory, loads/saves XML state, builds popup menu. | Owns `std::vector<juce::File>` of preset files. Uses `.aether` file extension. Calls `processor.loadStateFromTree()` for loading. |

---

## Recommended Project Structure

```
Source/
├── PluginProcessor.cpp        # processBlock, state, APVTS setup
├── PluginProcessor.h          # AetherProcessor class
├── PluginEditor.cpp           # Editor layout, ContentWrapper
├── PluginEditor.h             # AetherEditor class
├── Parameters.h               # All ParamIDs, ParamDefaults, ParamRanges
├── PresetManager.cpp          # Preset file I/O
├── PresetManager.h
│
├── dsp/                       # All DSP processing code
│   ├── CabinetResonance.cpp   # Stage I: short feedback delay + resonant filters
│   ├── CabinetResonance.h
│   ├── EarlyReflections.cpp   # Stage II: multi-tap delay network
│   ├── EarlyReflections.h
│   ├── AirDistance.cpp         # Stage III: freq-dependent absorption
│   ├── AirDistance.h
│   ├── Excitation.cpp          # Stage IV: multiband soft saturation
│   ├── Excitation.h
│   ├── RoomTone.cpp            # Stage V: shaped noise generator
│   ├── RoomTone.h
│   ├── DiffuseTail.cpp         # Stage VI: FDN reverb
│   ├── DiffuseTail.h
│   ├── MixOutput.cpp           # Dry/wet mix + output gain
│   ├── MixOutput.h
│   │
│   ├── core/                   # Shared DSP primitives
│   │   ├── DelayLine.h         # Sample-rate-aware delay line (used by I, II, VI)
│   │   ├── AllpassFilter.h     # Allpass diffuser (used by III, VI)
│   │   ├── FeedbackMatrix.h    # Householder/Hadamard matrix (used by II, VI)
│   │   ├── NoiseGenerator.h    # Pink/shaped noise (used by V)
│   │   └── Oversampler.h       # Wrapper around juce::dsp::Oversampling (used by IV)
│   │
│   └── filters/                # Filter primitives
│       ├── BiquadFilter.h      # General biquad (used across stages)
│       ├── OnePoleFilter.h     # Simple one-pole for damping
│       └── CrossoverFilter.h   # Multiband split for Excitation
│
├── ui/                         # All UI components
│   ├── AetherColours.h         # Parchment colour palette constants
│   ├── AetherLookAndFeel.cpp   # Custom LookAndFeel (knobs, toggles, bypasses)
│   ├── AetherLookAndFeel.h
│   ├── AetherFonts.h           # Font loading from binary data
│   │
│   ├── HeaderComponent.cpp     # Title, preset selector, series mark
│   ├── HeaderComponent.h
│   ├── FooterComponent.cpp     # Version, tagline, brand mark
│   ├── FooterComponent.h
│   │
│   ├── CabinetPanel.cpp        # Stage I controls (Body knob, cabinet toggle, bypass)
│   ├── CabinetPanel.h
│   ├── EarlyReflectionsLabel.h # Stage II label (controls are in center)
│   ├── AirDistancePanel.cpp    # Stage III controls (Air knob, character toggle, bypass)
│   ├── AirDistancePanel.h
│   ├── ExcitationPanel.cpp     # Stage IV controls (Drive knob, bypass)
│   ├── ExcitationPanel.h
│   ├── RoomTonePanel.cpp       # Stage V controls (Ambience knob, bypass)
│   ├── RoomTonePanel.h
│   ├── DiffuseTailPanel.cpp    # Stage VI controls (Decay, Diffusion knobs, bypass)
│   ├── DiffuseTailPanel.h
│   ├── OutputPanel.cpp         # Mix and Output knobs
│   ├── OutputPanel.h
│   │
│   ├── VisualizationComponent.cpp  # Center SVG acoustic ray diagram
│   ├── VisualizationComponent.h
│   │
│   ├── AetherKnob.cpp         # Custom rotary slider (parchment gradient)
│   ├── AetherKnob.h
│   ├── AetherToggle.cpp       # Custom toggle switch (ink on parchment)
│   ├── AetherToggle.h
│   ├── AetherBypass.cpp       # Per-section bypass button (In/Out)
│   ├── AetherBypass.h
│   └── Ornaments.h            # Diamond dividers, corner brackets, double-rule
│
Resources/
├── fonts/
│   ├── CormorantGaramond.ttf
│   ├── EBGaramond.ttf
│   └── Spectral.ttf
└── presets/                    # Factory presets
    ├── I-TightBooth.aether
    ├── II-LiveRoom.aether
    ├── III-Garage.aether
    ├── IV-Warehouse.aether
    ├── V-Bedroom.aether
    └── VI-ChurchHall.aether
```

### Structure Rationale

- **`Source/` root:** Core plugin files (Processor, Editor, Parameters, PresetManager) at root level, matching Crucible convention. These are the files you touch first in any session.
- **`dsp/`:** All audio-thread code isolated. No JUCE UI headers included here. Each stage is a self-contained class with the same `prepare()/process()/reset()` interface. The `core/` subfolder holds shared primitives used by multiple stages (delay lines, matrices, filters) to avoid duplication.
- **`dsp/filters/`:** Reusable filter primitives. Separated from `core/` because filters are the most commonly shared building blocks.
- **`ui/`:** All message-thread UI code. Panel components correspond 1:1 with DSP sections (CabinetPanel talks to CabinetResonance via APVTS). Custom control components (AetherKnob, AetherToggle, AetherBypass) are reused across panels.
- **`Resources/`:** Binary assets embedded via CMake `juce_add_binary_data()`. Fonts and factory presets.

---

## Architectural Patterns

### Pattern 1: Section Interface Contract

**What:** Every DSP section follows the same three-method interface: `prepare()`, `process()`, `reset()`. Parameters are set via individual setters called from `processBlock()` before `process()`.

**When to use:** Always. This is the fundamental contract between the processor and its DSP sections.

**Trade-offs:** Slightly more boilerplate per section (each setter, each cached param). But the gain is massive: sections are independently testable, independently bypassable, and the processor's `processBlock()` reads like a signal flow diagram.

**Example (from Crucible, adapted for Aether):**
```cpp
// In AetherProcessor::processBlock()
void AetherProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Store dry copy for mix
    dryBuffer.makeCopyOf(buffer);

    // Stage I: Cabinet Resonance
    cabinetResonance.setBody(bodyParam->load());
    cabinetResonance.setCabinetType(static_cast<int>(cabinetTypeParam->load()));
    cabinetResonance.setBypass(cabBypassParam->load() > 0.5f);
    cabinetResonance.process(buffer);

    // Stage II: Early Reflections
    earlyReflections.setRoomSize(roomSizeParam->load());
    earlyReflections.setShape(shapeParam->load());
    earlyReflections.setProximity(proximityParam->load());
    earlyReflections.setBypass(erBypassParam->load() > 0.5f);
    earlyReflections.process(buffer);

    // ... Stages III-VI follow the same pattern ...

    // Final mix and output
    mixOutput.process(buffer, dryBuffer,
                      mixParam->load(), outputGainParam->load());
}
```

### Pattern 2: Cached Atomic Parameter Pointers

**What:** Cache `std::atomic<float>*` pointers to APVTS parameters in the processor constructor. Read via `.load()` in `processBlock()`. Never access APVTS by string ID on the audio thread.

**When to use:** Always. This is the Crucible-established pattern for audio-thread parameter access.

**Trade-offs:** More member variables in the processor header. But string-based parameter lookup on the audio thread would be a real-time safety violation (potential allocation).

**Example:**
```cpp
// In AetherProcessor constructor
bodyParam = apvts.getRawParameterValue(ParamIDs::cabinetBody);
cabinetTypeParam = apvts.getRawParameterValue(ParamIDs::cabinetType);
cabBypassParam = apvts.getRawParameterValue(ParamIDs::cabinetBypass);
// ... all ~25-30 parameters cached this way
```

### Pattern 3: Lock-Free Audio-to-UI Metering

**What:** Audio thread writes peak/level data to `std::atomic<float>` members using compare-and-swap (CAS). UI thread reads and resets via `exchange()`. No locks, no allocations.

**When to use:** For any data that needs to flow from audio thread to UI thread -- level meters, visualization parameters, gain reduction displays.

**Trade-offs:** Only supports simple scalar data. For complex data (arrays, structs), need multiple atomics or a lock-free FIFO. For Aether's visualization, the audio thread need only publish a few scalar values (effective room size, current level, per-stage activity) since the visualization is driven primarily by parameter values, not audio data.

**Example:**
```cpp
class LevelMeterSource {
public:
    void update(float newPeak) noexcept {
        auto oldPeak = peakLevel.load(std::memory_order_relaxed);
        while (newPeak > oldPeak &&
               !peakLevel.compare_exchange_weak(oldPeak, newPeak,
                   std::memory_order_release, std::memory_order_relaxed))
            ;
    }
    float readAndReset() noexcept {
        return peakLevel.exchange(0.0f, std::memory_order_acquire);
    }
private:
    std::atomic<float> peakLevel { 0.0f };
};
```

### Pattern 4: ContentWrapper with AffineTransform Scaling

**What:** All UI components are laid out at a fixed base resolution (900x530) inside a `ContentWrapper` component. The wrapper applies `juce::AffineTransform::scale()` so the entire UI scales proportionally when the window is resized.

**When to use:** For fixed-dimension plugin UIs that need optional scaling (accessibility, HiDPI).

**Trade-offs:** All coordinates are in "base" units, simplifying layout. But hit-testing and mouse coordinates must account for the transform. JUCE handles this automatically through the component hierarchy.

**Example:**
```cpp
class ContentWrapper : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        // Paint at base resolution -- transform handles scaling
        g.setColour(AetherColours::parchment);
        g.fillAll();
    }
};

// In AetherEditor::resized()
float scale = static_cast<float>(getWidth()) / static_cast<float>(baseWidth);
contentWrapper.setTransform(juce::AffineTransform::scale(scale));
contentWrapper.setBounds(0, 0, baseWidth, baseHeight);
```

### Pattern 5: SmoothedValue for Parameter Ramps

**What:** Every parameter that modulates DSP state uses `juce::SmoothedValue` to ramp between values over 10-50ms. Prevents zipper noise (audible stepping when parameters change).

**When to use:** For any parameter that directly affects audio gain, filter frequency, delay time, or mix amount. Not needed for discrete parameters (cabinet type, bypass toggle) which use crossfade instead.

**Trade-offs:** Slight CPU overhead (one multiply per sample per smoothed parameter). Negligible for Aether's parameter count. Essential for professional-quality audio.

**Example:**
```cpp
// In DSP section prepare()
void CabinetResonance::prepare(double sampleRate, int /*samplesPerBlock*/) {
    bodySmoothed.reset(sampleRate, 0.02);  // 20ms ramp
    bodySmoothed.setCurrentAndTargetValue(0.5f);
}

// In process()
void CabinetResonance::process(juce::AudioBuffer<float>& buffer) {
    bodySmoothed.setTargetValue(currentBody);
    for (int s = 0; s < buffer.getNumSamples(); ++s) {
        float body = bodySmoothed.getNextValue();
        // Use smoothed value for this sample
    }
}
```

---

## Data Flow

### Audio Signal Flow

```
Host Audio Input (stereo)
    |
    v
processBlock() entry
    |
    |-- Store dry copy for Mix stage
    |
    v
[I. Cabinet Resonance] ---- Body, CabinetType, Bypass
    |                        Short FDN (1-5ms) + resonant BPF
    |                        Mono processing (real cab is mono source)
    v
[II. Early Reflections] --- RoomSize, Shape, Proximity, Bypass
    |                        Multi-tap delay (8-16 taps, 1-30ms)
    |                        Different L/R tap times (stereo decorrelation)
    |                        Hadamard/Householder feedback matrix
    v
[III. Air & Distance] ----- Air, Character, Bypass
    |                        Frequency-dependent LP filter
    |                        Allpass phase smearing
    |                        Transient softening at high Air values
    v
[IV. Excitation] ---------- Drive, Bypass
    |                        3-band split (low/mid/high)
    |                        Per-band soft saturation (more in low-mids)
    |                        2x/4x oversampling via juce::dsp::Oversampling
    v
[V. Room Tone] ------------ Ambience, Bypass
    |                        Shaped noise (pink base, BPF around 200-500Hz)
    |                        Decorrelated L/R noise generators
    |                        Level linked to Room Size
    |                        Added to signal (not replacing)
    v
[VI. Diffuse Tail] -------- Decay, Diffusion, Bypass
    |                        FDN (4-8 delay lines, Hadamard mixing)
    |                        Pre-delay auto-linked to Room Size
    |                        HF damping linked to Air
    |                        Short RT60: 50-500ms max
    v
[Mix/Output] -------------- Mix (dry/wet), Output (gain trim)
    |                        Auto-gain compensation
    |                        Crossfade dry copy with wet signal
    v
Safety Limiter (brick-wall at 0dBFS)
    |
    v
Level Meter Source (atomic update)
    |
    v
Host Audio Output (stereo)
```

### Parameter Flow (Message Thread to Audio Thread)

```
UI Control (knob, toggle, bypass button)
    |
    v (SliderAttachment / ButtonAttachment)
APVTS Parameter
    |
    v (atomic float, written by message thread)
std::atomic<float>* cached pointer
    |
    v (read in processBlock via .load())
DSP Section setter (e.g., setRoomSize())
    |
    v (SmoothedValue target update)
Per-sample smoothed parameter value
```

### Visualization Data Flow

```
APVTS Parameters (Room Size, Shape, Proximity)
    |
    v (ParameterAttachment callbacks on message thread)
VisualizationComponent member variables
    |
    v (repaint() triggered)
paint() draws:
  - Room boundary scaled to Room Size
  - Reflection paths angled by Shape
  - Listener position from Proximity
  - Wavefront breathing animation (Timer-driven, 4s cycle)
  - Bypass-state opacity per stage

Note: The visualization is parameter-driven, NOT audio-driven.
No audio data crosses to the visualization. This keeps it simple
and avoids the complexity of lock-free audio data streaming.
```

### State Serialization Flow

```
Save (DAW session save or preset save):
  apvts.copyState() --> ValueTree --> XML --> binary blob (DAW)
                                         --> .aether file (preset)

Load (DAW session restore or preset load):
  binary blob / .aether file --> XML --> ValueTree
    --> processor.loadStateFromTree(tree)
        --> apvts.replaceState(tree)
        --> re-add ValueTree listeners
        --> sync non-automatable state (UI scale, etc.)
```

### Key Data Flows

1. **Audio processing:** Linear signal chain, each stage modifies the buffer in-place. Dry copy stored once at input for final mix. No branching or parallel paths (fixed order, unlike Crucible's reorderable sections).

2. **Parameter communication:** APVTS atomic reads in processBlock, SmoothedValue for continuous params, crossfade for discrete mode switches. UI binds via Attachment objects.

3. **Meter/visualization:** Audio thread publishes scalar atomics. UI thread reads on Timer callback (30Hz for meters, 15Hz adequate for visualization breathing). No complex data structures cross the thread boundary.

4. **Preset/state:** Full state round-trips through ValueTree XML. PresetManager handles file I/O on message thread. loadStateFromTree() is the single entry point for both DAW recall and preset loading.

---

## Scaling Considerations

Aether is a desktop audio plugin, not a web service. "Scaling" means CPU budget at different buffer sizes and sample rates.

| Concern | 44.1kHz / 512 buffer | 96kHz / 64 buffer | 192kHz / 32 buffer |
|---------|---------------------|--------------------|--------------------|
| **CPU budget** | Comfortable. ~5ms per block. All stages run easily. | Tight. ~0.67ms per block. Oversampling in Excitation is the bottleneck. | Very tight. Consider bypassing 4x oversampling, falling back to 2x. |
| **Delay line memory** | ~100KB total for all delay lines | ~200KB (doubled sample rate) | ~400KB (still fine) |
| **FDN (Diffuse Tail)** | 4-8 delay lines, no issues | Same structure, longer buffers | Same, but damping filters cost more |
| **Oversampling (Excitation)** | 4x oversampling feasible | 2x oversampling recommended | 2x or bypass oversampling |
| **Noise generator (Room Tone)** | Trivial CPU | Trivial | Trivial |

### Scaling Priorities

1. **First bottleneck: Excitation oversampling.** At 96kHz+, 4x oversampling means processing at 384kHz+. Mitigate by making oversampling factor sample-rate-aware (4x at 44.1kHz, 2x at 96kHz, off at 192kHz) or by offering a quality setting.

2. **Second bottleneck: Early Reflections tap count.** 16 taps with per-tap filtering at high sample rates. Mitigate by keeping filters simple (one-pole damping, not biquad chains per tap).

3. **Third bottleneck: FDN matrix operations.** Hadamard matrix multiply for 8 delay lines = 8x8 operations per sample. This is inherently efficient (Hadamard is just additions and subtractions, no multiplies). Not a real concern.

---

## Anti-Patterns

### Anti-Pattern 1: String-Based Parameter Lookup on Audio Thread

**What people do:** Call `apvts.getParameter("room_size")->getValue()` inside `processBlock()`.
**Why it's wrong:** String comparison and potential map lookup on the audio thread. May allocate. Violates real-time safety.
**Do this instead:** Cache `std::atomic<float>*` pointers in the constructor via `getRawParameterValue()`. Read with `.load()` in processBlock.

### Anti-Pattern 2: Allocating in processBlock

**What people do:** Create temporary `AudioBuffer`, `std::vector`, or use `new` inside `processBlock()`.
**Why it's wrong:** Memory allocation is not real-time safe. Can cause audio dropouts (glitches).
**Do this instead:** Pre-allocate all work buffers in `prepare()`. Store them as class members. Use `juce::AudioBuffer::setSize()` only in `prepare()`, never in `process()`.

### Anti-Pattern 3: Using dsp::ProcessorChain for Bypassable Stages

**What people do:** Use `juce::dsp::ProcessorChain` to wire all stages together.
**Why it's wrong:** ProcessorChain is a compile-time fixed chain with no runtime bypass support. You cannot skip a stage or reorder stages. It also hides the signal flow in template metaprogramming, making debugging harder.
**Do this instead:** Call each section's `process()` manually in processBlock. Each section handles its own bypass internally (early return when bypassed). This is exactly what Crucible does and it is clear, debuggable, and flexible.

### Anti-Pattern 4: Audio-Driven Visualization

**What people do:** Stream audio data (waveforms, spectra) to the visualization component via lock-free FIFOs.
**Why it's wrong:** For Aether specifically, the visualization is a parametric room diagram, not an oscilloscope. Streaming audio data adds complexity with no benefit. The visualization should react to parameter values (Room Size, Shape, Proximity), not audio content.
**Do this instead:** Use `juce::ParameterAttachment` callbacks to update visualization state on the message thread. Only the level meter needs audio-thread data (via atomic scalars).

### Anti-Pattern 5: Shared Mutable State Between DSP Sections

**What people do:** Have Stage VI (Diffuse Tail) read Room Size directly from Stage II (Early Reflections) via a shared pointer or reference.
**Why it's wrong:** Creates hidden coupling. Sections should be independently testable. Makes parameter linking fragile.
**Do this instead:** The processor reads parameter values from APVTS and passes them to each section via setters. If Diffuse Tail needs Room Size for its pre-delay, the processor reads `roomSizeParam->load()` and calls `diffuseTail.setPreDelayFromRoomSize(roomSize)`. All inter-stage data flows through the processor, which is the single coordinator.

### Anti-Pattern 6: Per-Stage Output Gain

**What people do:** Add individual output gain controls to each of the 6 stages.
**Why it's wrong:** Creates a gain-staging nightmare for the user. 6 stages with individual gains means 6 places where signal level can accumulate or drop unexpectedly. Aether's design intentionally omits per-stage gain to keep the UX simple.
**Do this instead:** Each stage's controls affect character/intensity, not volume. Use auto-gain compensation within stages where level changes are inherent (Excitation saturation, Mix blend). Only expose global Mix and Output controls.

---

## Integration Points

### External Services

| Service | Integration Pattern | Notes |
|---------|---------------------|-------|
| DAW Host | `AudioProcessor` virtual methods | processBlock, getState/setState, bus layout, latency reporting |
| Plugin Formats (VST3/AU) | JUCE wrappers (automatic) | CMake `juce_add_plugin()` generates format wrappers. Verify AU validation with `auval`. |
| File System | Preset directory | `~/Documents/Aether/Presets/` on macOS. Factory presets embedded or copied on first run. |

### Internal Boundaries

| Boundary | Communication | Notes |
|----------|---------------|-------|
| Processor <-> DSP Sections | Direct method calls (same thread) | Processor calls setters then process(). No indirection needed -- all on audio thread. |
| Processor <-> Editor | APVTS (thread-safe parameter bridge) | SliderAttachment, ButtonAttachment, ParameterAttachment for UI binding. Meter data via atomics. |
| Processor <-> PresetManager | loadStateFromTree() | PresetManager calls back into processor for state restoration. Lives on message thread. |
| DSP Sections <-> DSP Core Primitives | Direct composition (owns instances) | Each section owns its filters, delay lines, etc. No shared instances between sections. |
| Editor <-> Section Panels | Component parent-child hierarchy | Editor owns panels, passes APVTS reference. Panels create their own attachments. |
| Editor <-> Visualization | ParameterAttachment + Timer | Visualization registers parameter listeners. Timer drives breathing animation at 15Hz. repaint() on parameter change. |
| Editor <-> LookAndFeel | setLookAndFeel() on contentWrapper | Single LookAndFeel instance owned by editor. Applied to contentWrapper, inherited by all children. |

---

## Build Order Implications

The architecture has clear dependency layers that dictate build order:

```
Layer 0: Foundation
  Parameters.h (no dependencies, defines all IDs/defaults/ranges)
  dsp/core/* (DelayLine, AllpassFilter, FeedbackMatrix, NoiseGenerator)
  dsp/filters/* (BiquadFilter, OnePoleFilter, CrossoverFilter)

Layer 1: DSP Sections (depend on Layer 0)
  dsp/CabinetResonance (uses DelayLine, BiquadFilter)
  dsp/EarlyReflections (uses DelayLine, FeedbackMatrix, OnePoleFilter)
  dsp/AirDistance (uses BiquadFilter, AllpassFilter)
  dsp/Excitation (uses CrossoverFilter, Oversampler)
  dsp/RoomTone (uses NoiseGenerator, BiquadFilter)
  dsp/DiffuseTail (uses DelayLine, FeedbackMatrix, AllpassFilter)
  dsp/MixOutput (standalone, uses SmoothedValue)

Layer 2: Processor (depends on Layer 1)
  PluginProcessor (owns all DSP sections, APVTS, orchestrates processBlock)

Layer 3: UI Foundation (depends on Layer 0 for ParamIDs)
  ui/AetherColours.h, ui/AetherFonts.h, ui/AetherLookAndFeel
  ui/AetherKnob, ui/AetherToggle, ui/AetherBypass, ui/Ornaments

Layer 4: UI Sections (depend on Layer 3)
  ui/*Panel components (one per DSP section)
  ui/VisualizationComponent
  ui/HeaderComponent, ui/FooterComponent, ui/OutputPanel

Layer 5: Editor (depends on Layer 2, Layer 4)
  PluginEditor (owns ContentWrapper, all panels, PresetManager)

Layer 6: Preset System (depends on Layer 2)
  PresetManager (file I/O, state serialization)
```

**What this means for phasing:**
- Parameters.h and DSP core primitives must come first (Phase 1 setup).
- DSP sections can be built independently and in any order within Phase 2 (but Early Reflections is the "heart" -- build it first for validation).
- The processor shell can be built in Phase 1 with stub sections, then sections plug in as completed.
- UI components are independent of DSP logic (communicate only via APVTS), so UI and DSP development can run in parallel after Phase 1 establishes the APVTS contract.
- Visualization is entirely UI-side and has no DSP dependency -- can be built whenever the center panel layout is ready.
- Presets require both processor state management and at least one complete DSP section to be meaningful.

---

## Sources

- Crucible reference implementation at `~/Projects/Crucible` -- **PRIMARY SOURCE** (HIGH confidence). Production-proven architecture within the same brand, same developer, same build system. All architectural patterns directly observed from source code.
- Aether Design Handoff at `/Users/nathanmcmillan/Downloads/Aether Files/AETHER-DESIGN-HANDOFF.md` -- **DEFINITIVE SPEC** (HIGH confidence). Complete DSP specifications, UI layout, color system, typography, and implementation phases.
- [JUCE AudioProcessorValueTreeState documentation](https://docs.juce.com/master/classAudioProcessorValueTreeState.html) -- Official APVTS API reference (HIGH confidence).
- [JUCE dsp::ProcessorChain documentation](https://docs.juce.com/master/classdsp_1_1ProcessorChain.html) -- Official ProcessorChain reference; confirmed limitation of no runtime bypass (HIGH confidence).
- [JUCE Cascading Effects tutorial](https://juce.com/tutorials/tutorial_audio_processor_graph/) -- AudioProcessorGraph tutorial showing multi-stage processing approaches (MEDIUM confidence -- shows patterns but we use direct calls per Crucible convention).
- [Jatin Chowdhury's FDN implementation](https://github.com/jatinchowdhury18/Feedback-Delay-Networks) -- Open-source FDN reverb in JUCE (MEDIUM confidence -- confirms FDN patterns, though specific implementation details vary).
- [Room Reverberation Simulation using Parametrised FDNs](https://projekter.aau.dk/projekter/files/334638099/dstrub18_Room_Reverberation_Simulation_using_Parametrised_FDNs.pdf) -- Academic paper on parametrized FDN for room simulation (MEDIUM confidence -- academic reference for early reflection + FDN architecture).
- [JUCE Forum: dsp::ProcessorChain vs AudioProcessorGraph](https://forum.juce.com/t/advantages-of-dsp-processorchain-vs-audioprocessorgraph/51445) -- Community discussion confirming ProcessorChain is for static chains only (MEDIUM confidence).
- [JUCE Forum: FDN software design considerations](https://forum.juce.com/t/feedback-delay-network-software-design-considerations/22510) -- Community discussion on FDN architecture in JUCE (LOW confidence -- forum post, but aligns with academic sources).

---
*Architecture research for: Aether (JUCE audio plugin -- room environment simulation)*
*Researched: 2026-02-18*
