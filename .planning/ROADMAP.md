# Roadmap: Aether

## Overview

Aether is built in eight phases that progress from a silent plugin scaffold to a fully validated room environment simulator. The first phase establishes the build system, APVTS parameter tree, and audio pipeline with real-time safety patterns baked in from day one. Phases 2 through 5 implement the six DSP stages incrementally -- Cabinet Resonance first (proving the FDN primitives), then Early Reflections and Diffuse Tail together (the core spatial engine with cross-stage linking), then Air & Distance (filter-based processing), then Excitation and Room Tone (completing the chain). Phase 6 builds the Victorian parchment UI with all controls. Phase 7 adds the acoustic ray visualization. Phase 8 tunes factory presets, validates all plugin formats, and ships.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [x] **Phase 1: Project Scaffold & Audio Pipeline** - Build system, APVTS parameter tree, processBlock skeleton with mix/output and real-time safety (completed 2026-02-18)
- [x] **Phase 2: Cabinet Resonance** - First DSP stage (Stage I) with short FDN, ten material types, validates section interface (completed 2026-02-18)
- [x] **Phase 3: Early Reflections & Diffuse Tail** - Core spatial engine (Stages II + VI) with multi-tap delays, FDN reverb, and cross-stage parameter linking (completed 2026-02-18)
- [ ] **Phase 4: Air & Distance** - Frequency-dependent absorption stage (Stage III) with character toggle and HF damping link to Diffuse Tail
- [ ] **Phase 5: Excitation & Room Tone** - Remaining DSP stages (Stages IV + V) completing the full six-stage processing chain
- [ ] **Phase 6: User Interface** - Victorian parchment theme with three-column layout, custom knobs/toggles/bypass, typography, and ornamental elements
- [ ] **Phase 7: Visualization** - Center acoustic ray diagram reactive to Room Size, Shape, and Proximity with breathing animation
- [ ] **Phase 8: Presets, Formats & Validation** - Factory presets, VST3/AU/Standalone builds, pluginval validation, and release readiness

## Phase Details

### Phase 1: Project Scaffold & Audio Pipeline
**Goal**: A building, loading, audio-passing plugin with complete parameter tree, mix/output processing, and real-time safety patterns established
**Depends on**: Nothing (first phase)
**Requirements**: BUILD-04, ENG-01, ENG-02, ENG-03, ENG-04, ENG-05, ENG-06, OUT-01, OUT-02, OUT-03, OUT-04
**Success Criteria** (what must be TRUE):
  1. Plugin builds via CMake and loads in a DAW as VST3 without crashing
  2. Audio passes through the plugin (dry signal audible at output with Mix at 0%)
  3. Mix knob blends dry/wet signal with auto-gain compensation, Output knob trims level
  4. All parameters appear in the DAW's automation list and can be recorded/played back without zipper noise
  5. Plugin reports correct latency to DAW and operates correctly at 44.1, 48, 96, and 192 kHz sample rates
**Plans**: 2 plans

Plans:
- [ ] 01-01-PLAN.md -- CMake build system, complete parameter definitions (20 params), plugin shell with APVTS and state save/restore
- [ ] 01-02-PLAN.md -- DSP stage placeholders (6 stages), Mix/Output sections, processBlock pipeline with serial chain and real-time safety

### Phase 2: Cabinet Resonance
**Goal**: Users can add cabinet body and resonance character to their signal through the first working DSP stage
**Depends on**: Phase 1
**Requirements**: CAB-01, CAB-02, CAB-03, CAB-04, CAB-05
**Success Criteria** (what must be TRUE):
  1. User hears audible low-mid body added to a DI guitar signal when Weight knob is turned up
  2. User can switch between 10 material types (4 woods, 3 metals, 3 stones) and hear distinct tonal character for each
  3. User can bypass Resonance stage independently and hear clean A/B comparison (bypassed = transparent)
  4. Processing is mono-consistent (no unexpected stereo artifacts from resonance stage)
**Plans**: 2 plans

Plans:
- [ ] 02-01-PLAN.md -- Rename CabinetSection to ResonanceSection, refactor parameter IDs (cab_ to res_), expand Material selector to 10 types, version bump to 2
- [ ] 02-02-PLAN.md -- Implement 4-line Householder FDN with 10 material presets, Weight/bypass control, mono processing, DAW verification

### Phase 3: Early Reflections & Diffuse Tail
**Goal**: Users hear convincing room spatial character -- early reflection patterns that define room shape and a diffuse reverb tail that completes the room impression
**Depends on**: Phase 2
**Requirements**: REFL-01, REFL-02, REFL-03, REFL-04, REFL-05, REFL-06, TAIL-01, TAIL-02, TAIL-03, TAIL-04, TAIL-05, TAIL-06
**Success Criteria** (what must be TRUE):
  1. User can hear room size change from small booth to large hall by sweeping the Room Size knob (delay times scale audibly)
  2. User can hear reflection pattern change from regular to irregular by sweeping the Shape knob
  3. User can hear mic proximity change from near-field to far-field by sweeping the Proximity knob (direct/reflected ratio shifts)
  4. Diffuse Tail adds smooth reverb decay (50-500ms) without metallic ringing, with Diffusion knob controlling texture density
  5. Pre-delay of Diffuse Tail automatically tracks Room Size setting (larger room = audibly longer gap before tail)
**Plans**: 3 plans

Plans:
- [ ] 03-01-PLAN.md -- Parameter migration (Shape->Choice, refl_width, tail_decay range) and complete ReflectionsSection with stereo TDL, 7 shape presets, Room Size/Shape/Proximity/Width
- [ ] 03-02-PLAN.md -- Complete DiffuseTailSection with 8-line Hadamard FDN, input allpass diffusion, pre-delay, frequency-dependent feedback damping
- [ ] 03-03-PLAN.md -- Cross-stage parameter linking (Room Size->pre-delay, Air->HF damping, Shape->tail character) and DAW verification

### Phase 4: Air & Distance
**Goal**: Users can simulate high-frequency absorption and air diffusion that makes the signal sound like it traveled through a real room
**Depends on**: Phase 3
**Requirements**: AIR-01, AIR-02, AIR-03, AIR-04, AIR-05
**Success Criteria** (what must be TRUE):
  1. User hears progressive high-frequency rolloff as Air knob increases (cymbals and pick attack soften)
  2. User can hear difference between Warm character (gentle, carpeted room) and Neutral character (steeper, hard surfaces)
  3. Higher Air values produce subtle transient softening (not just EQ -- the sound becomes less "immediate")
  4. Air setting automatically influences Diffuse Tail HF damping (more air = darker tail)
**Plans**: TBD

Plans:
- [ ] 04-01: TBD

### Phase 5: Excitation & Room Tone
**Goal**: The full six-stage processing chain is operational -- users can drive room excitation for liveliness and add ambient noise floor for realism
**Depends on**: Phase 4
**Requirements**: EXCIT-01, EXCIT-02, EXCIT-03, EXCIT-04, EXCIT-05, TONE-01, TONE-02, TONE-03, TONE-04, TONE-05, TONE-06, TONE-07
**Success Criteria** (what must be TRUE):
  1. User hears increased density and liveliness when Drive knob is turned up, without audible distortion at 50% setting
  2. Excitation does not produce aliasing artifacts (verified with high-frequency test tones)
  3. User hears subtle ambient room tone when Ambience knob is turned up, and it sounds like a real room (not white noise)
  4. Room Tone character changes with Room Size setting (larger rooms produce lower-frequency ambient character)
  5. All six DSP stages can be individually bypassed and the full chain processes audio without artifacts at all supported sample rates
**Plans**: TBD

Plans:
- [ ] 05-01: TBD
- [ ] 05-02: TBD

### Phase 6: User Interface
**Goal**: Users interact with a polished Victorian parchment UI that matches the design handoff -- all controls functional, typography correct, ornamental elements in place
**Depends on**: Phase 5
**Requirements**: UI-01, UI-02, UI-03, UI-04, UI-05, UI-06, UI-07, UI-08, UI-09, UI-10, UI-11, UI-12
**Success Criteria** (what must be TRUE):
  1. Plugin displays 900x530px window with three-column layout, parchment background, and paper texture
  2. All six DSP stages have their knobs, toggles, and bypass buttons functional and connected to parameters
  3. Custom knob components render with parchment gradient body and ink indicator (not default JUCE sliders)
  4. Header shows "AETHER" title, preset selector, and "Cosmos Series" mark; footer shows version and brand info
  5. Typography uses embedded Cormorant Garamond/EB Garamond/Spectral fonts with Roman numeral section labels
**Plans**: TBD

Plans:
- [ ] 06-01: TBD
- [ ] 06-02: TBD
- [ ] 06-03: TBD

### Phase 7: Visualization
**Goal**: Users see a live acoustic ray diagram in the center column that visually represents the room environment and reacts to parameter changes
**Depends on**: Phase 6
**Requirements**: VIZ-01, VIZ-02, VIZ-03, VIZ-04, VIZ-05
**Success Criteria** (what must be TRUE):
  1. Center column displays 340x340px acoustic ray diagram showing room boundary, source, listener, wavefronts, and reflection paths
  2. Diagram reacts in real-time to Room Size (boundary scale), Shape (reflection angles), and Proximity (listener position)
  3. Wavefront rings animate with a subtle breathing cycle (visible slow pulse)
  4. Bypassed stages fade their corresponding visual elements to ghost opacity
**Plans**: TBD

Plans:
- [ ] 07-01: TBD

### Phase 8: Presets, Formats & Validation
**Goal**: Plugin ships with factory presets, builds in all target formats, passes validation, and reliably saves/restores state across DAW sessions
**Depends on**: Phase 7
**Requirements**: PRST-01, PRST-02, PRST-03, PRST-04, BUILD-01, BUILD-02, BUILD-03, BUILD-05
**Success Criteria** (what must be TRUE):
  1. User can select any of 6 factory presets (Tight Booth through Church Hall) from a dropdown and hear a cohesive, musically useful starting point
  2. Preset selector displays Roman numeral naming (I. -- Tight Booth, II. -- Live Room, etc.)
  3. Plugin state saves and restores correctly across DAW sessions (close project, reopen, all parameters intact)
  4. Plugin builds and loads as VST3, AU, and Standalone on macOS without errors
  5. Plugin passes pluginval validation (strictness 5+)
**Plans**: TBD

Plans:
- [ ] 08-01: TBD
- [ ] 08-02: TBD

## Progress

**Execution Order:**
Phases execute in numeric order: 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Project Scaffold & Audio Pipeline | 0/0 | Complete    | 2026-02-18 |
| 2. Cabinet Resonance | 2/2 | Complete    | 2026-02-18 |
| 3. Early Reflections & Diffuse Tail | 3/3 | Complete    | 2026-02-18 |
| 4. Air & Distance | 0/0 | Not started | - |
| 5. Excitation & Room Tone | 0/0 | Not started | - |
| 6. User Interface | 0/0 | Not started | - |
| 7. Visualization | 0/0 | Not started | - |
| 8. Presets, Formats & Validation | 0/0 | Not started | - |

---
*Roadmap created: 2026-02-18*
*Depth: comprehensive (8 phases)*
*Coverage: 70/70 v1 requirements mapped*
