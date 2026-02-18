# Requirements: Aether

**Defined:** 2026-02-18
**Core Value:** Make digital amp sims and DI recordings sound like they were recorded in a real room with a real microphone -- through physically-modeled environmental layers, not reverb effects.

## v1 Requirements

Requirements for initial release. Each maps to roadmap phases.

### Cabinet Resonance (Stage I)

- [x] **CAB-01**: User can add low-mid body and resonance character to signal via Weight knob (0-100%)
- [x] **CAB-02**: User can select from 10 material types across 3 families: Woods (Pine, Oak, Walnut, Mahogany), Metals (Iron, Steel, Copper), Stones (Limestone, Marble, Granite)
- [x] **CAB-03**: User can bypass Resonance stage independently
- [x] **CAB-04**: Resonance uses short 4-line Householder FDN (1-5ms) with resonant bandpass filtering
- [x] **CAB-05**: Resonance processing is mono (single source, consistent with real cab behavior)

### Early Reflections (Stage II)

- [x] **REFL-01**: User can control room size via Room Size knob (Small -> Large), scaling delay times from 1ms-30ms
- [x] **REFL-02**: User can control reflection distribution via Shape knob (Regular -> Irregular), adjusting tap spacing from even to randomized
- [x] **REFL-03**: User can control mic distance via Proximity knob (Near -> Far), adjusting direct/reflected ratio and HF rolloff
- [x] **REFL-04**: Early Reflections use 8-16 individually filtered delay taps
- [x] **REFL-05**: Left and right channels have different delay tap times and pan positions for stereo decorrelation
- [x] **REFL-06**: User can bypass Early Reflections stage independently

### Air & Distance (Stage III)

- [x] **AIR-01**: User can control high-frequency absorption via Air knob (0-100%), from no filtering to significant HF rolloff
- [x] **AIR-02**: User can select air character: Warm (gentle slope from ~6kHz, carpeted room feel) or Neutral (steeper slope from ~10kHz, hard-surfaced room feel)
- [x] **AIR-03**: Air stage includes subtle allpass filtering for phase smearing (air diffusion simulation)
- [x] **AIR-04**: Higher Air values produce subtle transient softening
- [x] **AIR-05**: User can bypass Air & Distance stage independently

### Excitation (Stage IV)

- [x] **EXCIT-01**: User can add nonlinear room excitation via Drive knob (0-100%), producing density and liveliness without audible distortion
- [x] **EXCIT-02**: Excitation uses frequency-dependent multiband soft saturation (3 bands: low, mid, high with different curves)
- [x] **EXCIT-03**: Excitation uses 2x-4x oversampling to prevent aliasing artifacts
- [x] **EXCIT-04**: User can bypass Excitation stage independently
- [x] **EXCIT-05**: At 50% drive, the effect sounds like "louder/more alive" not "distorted"

### Room Tone (Stage V)

- [ ] **TONE-01**: User can add shaped ambient noise floor via Ambience knob (0-100%)
- [ ] **TONE-02**: Noise is shaped to sound like actual room tone: rolled off below ~80Hz, presence bump 200-500Hz, rolled off above ~8kHz
- [ ] **TONE-03**: Noise character subtly changes based on Room Size setting (larger rooms = lower resonant character)
- [ ] **TONE-04**: Left and right channels use decorrelated noise generators
- [ ] **TONE-05**: At 100%, noise is very quiet relative to signal (-40dB to -30dB below signal)
- [ ] **TONE-06**: Room Tone stage defaults to bypassed (Out)
- [ ] **TONE-07**: User can bypass Room Tone stage independently

### Diffuse Tail (Stage VI)

- [x] **TAIL-01**: User can control reverb decay time via Decay knob (50ms-2000ms RT60)
- [x] **TAIL-02**: User can control reverb texture density via Diffusion knob (0-100%)
- [x] **TAIL-03**: Pre-delay is automatically linked to Room Size from Stage II (larger room = longer pre-delay)
- [x] **TAIL-04**: HF damping is automatically linked to Air setting from Stage III (more air = more HF damping)
- [x] **TAIL-05**: Diffuse Tail uses FDN (Feedback Delay Network) architecture with stereo implementation
- [x] **TAIL-06**: User can bypass Diffuse Tail stage independently

### Output

- [x] **OUT-01**: User can blend dry/wet signal via Mix knob (0-100%, default 70%)
- [x] **OUT-02**: User can trim output level via Output knob (-24dB to +6dB, default 0.0dB)
- [x] **OUT-03**: Mix knob applies auto-gain compensation (slightly reduces output as Mix increases to maintain perceived loudness)
- [x] **OUT-04**: Dry signal for Mix is tapped at input (pre all processing)

### Audio Engine

- [x] **ENG-01**: All knob parameters are smoothed (10-50ms ramp time) to prevent zipper noise
- [x] **ENG-02**: All delay times and filter frequencies are sample-rate aware (correct at 44.1, 48, 88.2, 96, 176.4, 192 kHz)
- [x] **ENG-03**: Plugin reports latency via setLatencySamples() for DAW PDC
- [x] **ENG-04**: Processing works with any buffer size (no buffer size assumptions)
- [x] **ENG-05**: Audio thread is allocation-free (no malloc/new in processBlock)
- [x] **ENG-06**: Signal flow follows specified order: Input -> I -> II -> III -> IV -> V -> VI -> Mix -> Output

### User Interface

- [ ] **UI-01**: Plugin window is 900x530px with three-column layout (220px | flex | 220px)
- [ ] **UI-02**: Parchment background (#f0e6d3) with subtle paper texture and edge vignette
- [ ] **UI-03**: Custom knob components: 56px standard, 64px for center room controls, parchment gradient body with ink indicator
- [ ] **UI-04**: Custom toggle switches with ink-inverted active state (Open/Closed/Combo, Warm/Neutral)
- [ ] **UI-05**: Custom bypass buttons per section (In/Out states with opacity changes)
- [ ] **UI-06**: Section labels with Roman numerals (I. through VI.) in Spectral font
- [ ] **UI-07**: Diamond ornament dividers between sections in side panels
- [ ] **UI-08**: Corner L-shaped ornaments at plugin frame corners
- [ ] **UI-09**: Header zone with "AETHER" title, "Environment Simulator" subtitle, preset selector, and "Cosmos Series" mark
- [ ] **UI-10**: Footer zone with version, tagline, and brand mark
- [ ] **UI-11**: Double-rule header/footer borders (Victorian publication style)
- [ ] **UI-12**: Custom typography embedded: Cormorant Garamond, EB Garamond, Spectral (fallback to Georgia)

### Visualization

- [ ] **VIZ-01**: Center acoustic ray diagram (340x340px) showing top-down room propagation
- [ ] **VIZ-02**: Diagram includes: room boundary, source point, listener indicator, concentric wavefronts, direct rays, reflection paths, wall reflection wavefronts (copper-tinted)
- [ ] **VIZ-03**: Diagram reacts to Room Size (boundary scale, wavefront spacing), Shape (reflection angles), and Proximity (listener position)
- [ ] **VIZ-04**: Subtle breathing animation on wavefront rings (4-second cycle, 1.0->1.02 scale)
- [ ] **VIZ-05**: Bypassed stages fade their corresponding visual elements to ghost opacity

### Presets

- [ ] **PRST-01**: 6 factory presets: Tight Booth, Live Room, Garage, Warehouse, Bedroom, Church Hall
- [ ] **PRST-02**: Each preset configures all six stages to a cohesive starting point
- [ ] **PRST-03**: Preset selector dropdown with Roman numeral naming (I. -- Tight Booth, etc.)
- [ ] **PRST-04**: User can save and recall plugin state via DAW's built-in preset/state management

### Build & Platform

- [ ] **BUILD-01**: Plugin builds as VST3 format on macOS
- [ ] **BUILD-02**: Plugin builds as AU format on macOS
- [ ] **BUILD-03**: Plugin builds as Standalone application on macOS
- [x] **BUILD-04**: CMake build system (not Projucer) with JUCE 8
- [ ] **BUILD-05**: Plugin passes pluginval validation

## v2 Requirements

Deferred to future release. Tracked but not in current roadmap.

### Presets

- **PRST-05**: Expanded factory preset library (10-12 presets covering more room varieties)
- **PRST-06**: User preset management UI (save/load/rename/delete with file browser)

### Platform

- **BUILD-06**: Windows build (VST3, Standalone)
- **BUILD-07**: macOS installer (.pkg)
- **BUILD-08**: Windows installer (.exe)
- **BUILD-09**: AAX format for Pro Tools

### Features

- **FEAT-01**: Mono-Safe mode that reduces L/R decorrelation for mono compatibility
- **FEAT-02**: A/B comparison mode (toggle between two parameter states)
- **FEAT-03**: Undo/redo for parameter changes

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature | Reason |
|---------|--------|
| Convolution / IR loading | Undermines core value of interactive, parameter-driven room modeling. IRs are static snapshots. |
| Microphone modeling | Aether simulates room environment, not recording chain. Use dedicated mic sim plugins. |
| Surround / Atmos support | Target audience works in stereo. Massive DSP complexity for non-primary market. |
| Long reverb (>2000ms decay) | Aether is environment simulator, not creative reverb. 2000ms cap covers large halls. |
| Real-time spectrum analyzer | Adds CPU overhead and visual clutter. Acoustic ray visualization provides more relevant feedback. |
| Room browser with 50+ spaces | Continuous parameters give infinite rooms. Fixed library implies convolution accuracy. |
| MIDI learn UI | DAW-level MIDI mapping already works with APVTS parameters. |
| Built-in tuner/noise gate | Feature creep. Users already have these in their signal chain. |
| Video posts / mobile app | N/A |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| CAB-01 | Phase 2 | Complete |
| CAB-02 | Phase 2 | Complete |
| CAB-03 | Phase 2 | Complete |
| CAB-04 | Phase 2 | Complete |
| CAB-05 | Phase 2 | Complete |
| REFL-01 | Phase 3 | Complete |
| REFL-02 | Phase 3 | Complete |
| REFL-03 | Phase 3 | Complete |
| REFL-04 | Phase 3 | Complete |
| REFL-05 | Phase 3 | Complete |
| REFL-06 | Phase 3 | Complete |
| AIR-01 | Phase 4 | Complete |
| AIR-02 | Phase 4 | Complete |
| AIR-03 | Phase 4 | Complete |
| AIR-04 | Phase 4 | Complete |
| AIR-05 | Phase 4 | Complete |
| EXCIT-01 | Phase 5 | Complete |
| EXCIT-02 | Phase 5 | Complete |
| EXCIT-03 | Phase 5 | Complete |
| EXCIT-04 | Phase 5 | Complete |
| EXCIT-05 | Phase 5 | Complete |
| TONE-01 | Phase 5 | Pending |
| TONE-02 | Phase 5 | Pending |
| TONE-03 | Phase 5 | Pending |
| TONE-04 | Phase 5 | Pending |
| TONE-05 | Phase 5 | Pending |
| TONE-06 | Phase 5 | Pending |
| TONE-07 | Phase 5 | Pending |
| TAIL-01 | Phase 3 | Complete |
| TAIL-02 | Phase 3 | Complete |
| TAIL-03 | Phase 3 | Complete |
| TAIL-04 | Phase 3 | Complete |
| TAIL-05 | Phase 3 | Complete |
| TAIL-06 | Phase 3 | Complete |
| OUT-01 | Phase 1 | Complete |
| OUT-02 | Phase 1 | Complete |
| OUT-03 | Phase 1 | Complete |
| OUT-04 | Phase 1 | Complete |
| ENG-01 | Phase 1 | Complete |
| ENG-02 | Phase 1 | Complete |
| ENG-03 | Phase 1 | Complete |
| ENG-04 | Phase 1 | Complete |
| ENG-05 | Phase 1 | Complete |
| ENG-06 | Phase 1 | Complete |
| UI-01 | Phase 6 | Pending |
| UI-02 | Phase 6 | Pending |
| UI-03 | Phase 6 | Pending |
| UI-04 | Phase 6 | Pending |
| UI-05 | Phase 6 | Pending |
| UI-06 | Phase 6 | Pending |
| UI-07 | Phase 6 | Pending |
| UI-08 | Phase 6 | Pending |
| UI-09 | Phase 6 | Pending |
| UI-10 | Phase 6 | Pending |
| UI-11 | Phase 6 | Pending |
| UI-12 | Phase 6 | Pending |
| VIZ-01 | Phase 7 | Pending |
| VIZ-02 | Phase 7 | Pending |
| VIZ-03 | Phase 7 | Pending |
| VIZ-04 | Phase 7 | Pending |
| VIZ-05 | Phase 7 | Pending |
| PRST-01 | Phase 8 | Pending |
| PRST-02 | Phase 8 | Pending |
| PRST-03 | Phase 8 | Pending |
| PRST-04 | Phase 8 | Pending |
| BUILD-01 | Phase 8 | Pending |
| BUILD-02 | Phase 8 | Pending |
| BUILD-03 | Phase 8 | Pending |
| BUILD-04 | Phase 1 | Complete |
| BUILD-05 | Phase 8 | Pending |

**Coverage:**
- v1 requirements: 70 total
- Mapped to phases: 70
- Unmapped: 0

---
*Requirements defined: 2026-02-18*
*Last updated: 2026-02-18 after roadmap creation*
