# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-02-18)

**Core value:** Make digital amp sims and DI recordings sound like they were recorded in a real room with a real microphone -- through physically-modeled environmental layers, not reverb effects.
**Current focus:** Phase 7 in progress. Visualization component built, awaiting editor integration (Plan 02).

## Current Position

Phase: 7 of 8 (Visualization)
Plan: 1 of 2 in current phase (07-01 complete)
Status: Executing Phase 7 -- Plan 01 complete, Plan 02 next
Last activity: 2026-02-19 - Completed quick task 3: Mix lock button, arrow step buttons, user preset save/load

Progress: [████████░░] 88%

## Performance Metrics

**Velocity:**
- Total plans completed: 15
- Average duration: 7min
- Total execution time: 1.7 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 1 - Project Scaffold | 2 | 11min | 6min |
| 2 - Cabinet Resonance | 2 | 17min | 9min |
| 3 - Early Reflections & Diffuse Tail | 3 | 20min | 7min |
| 4 - Air & Distance | 2 | 10min | 5min |
| 5 - Excitation & Room Tone | 2/2 | 23min | 12min |
| 6 - User Interface | 4/4 | 23min | 6min |
| 7 - Visualization | 1/2 | 4min | 4min |

**Recent Trend:**
- Last 5 plans: 06-01 (6min), 06-02 (4min), 06-03 (2min), 06-04 (6min), 07-01 (4min)
- Trend: Consistent fast execution (2-6min) -- well-established patterns and clear plan specs

*Updated after each plan completion*
| Phase 07 P01 | 4min | 2 tasks | 4 files |

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table.
Recent decisions affecting current work:

- Roadmap: 8 phases derived from 70 requirements at comprehensive depth
- Roadmap: FDN stages grouped -- Cabinet alone (Phase 2) to prove primitives, then ER + Tail together (Phase 3)
- Roadmap: UI and Visualization split into separate phases (6 and 7) due to distinct deliverables
- 01-01: 19 parameters registered (plan stated 20 but detailed spec lists 19)
- 01-01: Stage-prefixed snake_case parameter IDs for sorted DAW automation lists
- 01-01: Text-label float params for Room Size/Shape/Proximity (continuous 0-1 with descriptive display)
- 01-01: GenericAudioProcessorEditor for parameter testing until Phase 6 custom UI
- 01-02: Auto-gain compensation (-2.5dB * pow(mix, 1.5)) causes expected volume drop with passthrough stages -- self-corrects when DSP stages add energy
- 01-02: sin3dB crossfade creates +3dB constructive sum at 50% with correlated signals -- normalizes with decorrelated wet signal
- 01-02: Stage convention established: concrete class with prepare/process/reset/setBypass (no virtual base)
- 01-02: processBlock order: ScopedNoDenormals -> clear unused channels -> updateStageParams -> dry capture -> 6 stages -> mix -> auto-gain -> output trim
- 02-01: CabinetSection renamed to ResonanceSection; param IDs cab_ -> res_ with version 2 bump
- 02-01: 10 materials across 3 families: woods (Pine/Oak/Walnut/Mahogany), metals (Iron/Steel/Copper), stones (Limestone/Marble/Granite)
- 02-01: Mahogany (index 3) as default material -- warm, musical, period-appropriate
- 02-02: 4-line Householder FDN -- sufficient for short cabinet resonance, CPU-efficient
- 02-02: Coprime delay lengths via nearest-prime for maximal mode density
- 02-02: Quadratic weight curve (weight^2) for musical response
- 02-02: Immediate material coefficient switching (no crossfade) -- FDN settles in ~5ms
- 02-02: Volume compensation deferred to Phase 5 -- auto-gain designed for full 6-stage chain
- 03-01: Shape and Room Size independent -- any shape works at any size for maximum flexibility
- 03-01: No Stage I material coupling to reflections -- stages remain orthogonal
- 03-01: Subtle room size darkening via per-tap filter cutoff (12kHz at min, 6kHz at max)
- 03-01: Width via tap pattern interpolation (mono->stereo), not mid-side -- avoids phase cancellation on mono sum
- 03-01: Proximity Far extreme: -12dB direct floor, never fully removes direct signal
- 03-01: Shape crossfade uses 30ms linear ramp, both shape tap patterns read simultaneously during transition
- 03-02: 8-line Hadamard FDN for diffuse tail -- higher mixing density than Householder for 22-63ms delay lines
- 03-02: Even/odd stereo distribution (L=even, R=odd) instead of StereoMultiMixer -- simpler, Hadamard provides cross-coupling
- 03-02: Shape-to-tail influence via lookup table -- keeps stages orthogonal per ARCHITECTURE.md
- 03-02: Explicit delay+feedback allpass for input diffusion (not biquad) -- 1.5-8ms delays too long for biquad allpass
- 03-03: Cross-stage links always automatic (not user-toggleable) -- larger room always means longer pre-delay
- 03-03: getTailLengthSeconds returns 2.0s (maximum decay) for conservative DAW PDC reporting
- 04-01: air_char upgraded from 2-choice to 3-choice (Warm/Neutral/Cold) with version 2 bump, default changed to Neutral
- 04-01: AirCharacterPreset struct with physics-derived values per character (shelf freq, allpass stages, coupling factors)
- 04-01: BiquadStatic full object assignment for crossfade completion -- preserves filter state continuity
- 04-01: cheapEnergyCrossfade x = (1 - charCrossfade) convention for current->pending transition
- 04-01: Coefficient recalculation every 16 samples during smoothing -- balances CPU vs smoothness at 2756Hz update rate
- 04-02: 2kHz floor on Reflections baseCutoff prevents Air darkening from making reflections inaudible
- 04-02: +/-10% clamp on character decay bias keeps tail modification subtle and musical
- 04-02: All cross-stage coupling multiplied by airAmount ensures zero propagation at Air 0%
- 04-02: kCharacterPresets accessed directly from processor via file-scope static const array in AirSection.h
- 05-01: Single oversampler with crossover at oversampled rate (not 3 per-band oversamplers) for CPU efficiency
- 05-01: LR filter allpass mode with two-output processSample for clean band splitting
- 05-01: Drive smoothing once per block (not per oversampled sample) since knob changes are slow
- 05-01: kMinG=0.15 at Drive 0% ensures subtle room nonlinearity when not bypassed
- 05-01: MaterialExcitationBias lookup table: 10 entries with per-band (low/mid/high) scale modifiers per material
- 05-02: Auto-gain formula updated to -3.5*pow(mix,1.4) for full 6-stage chain (was -2.5*pow(mix,1.5))
- 05-02: Resonance Weight changed from crossfade to additive blend -- preserves dry signal, adds resonance energy on top
- 05-02: One-pole asymmetric gate envelope (50ms attack, 500ms release) for natural room tone gating behavior
- 05-02: ShapeAmbientCharacter lookup table maps 7 room shapes to spectral offsets for room tone character
- 06-01: Font helper getSpectralFont(float) instead of getLabelFont(float) to avoid JUCE override name clash
- 06-01: EB Garamond as variable-weight TTF (Google Fonts format); JUCE handles via createSystemTypefaceFor
- 06-01: Bypass buttons identified via "isBypass" component property for distinct outline-only LookAndFeel styling
- 06-01: Indicator line length adapts to knob radius via jmin(16px, radius*0.55) for 56px/64px knob sizes
- 06-02: GlyphArrangement for spaced label drawing (non-deprecated API) instead of Font::getStringWidthFloat
- 06-02: Slider alpha 0.0 (invisible) with all rendering in paint() using displayValue -- prevents visual desync
- 06-02: Rich value formatter takes normalised value + parameter text for composite displays like "62% (Medium)"
- 06-03: GlyphArrangement offset for letter-spacing (addLineOfText + moveRangeOfGlyphs per glyph) -- simpler than character-by-character
- 06-03: Radial vignette uses corner distance endpoint with 0.6 colour stop for natural 60% onset darkening
- 07-01: 30Hz timer for visualization -- sufficient for 4s breathing cycle, halves CPU vs 60Hz
- 07-01: Embedded cubic Bezier head profile path (~20 control points in 0-100 space) -- no SVG parsing needed
- 07-01: Compare-and-swap RMS bridge retains peak between GUI reads -- prevents missing transients
- 07-01: Asymmetric RMS smoothing (0.3 attack / 0.05 release) for natural audio-reactive breathing
- quick-2: 1ms delay floor on reflection taps (sr * 0.001) prevents near-zero-delay copies at small Room Size
- quick-2: 3-stage allpass decorrelation on dry signal (318/145/94 Hz coprime frequencies) before dry/wet mix reduces comb notches
- quick-2: R-channel 1.12x allpass frequency offset matches AirSection stereo decorrelation convention
- quick-3: Mix lock is purely UI state (not APVTS parameter) -- no need to persist lock across sessions
- quick-3: ArrowStepButton navigates by item index (not ID) to handle non-contiguous IDs after separators
- quick-3: User preset IDs start at 100 (kUserPresetIdOffset) to avoid collision with factory preset IDs 1-7
- quick-3: User presets stored at ~/Library/Audio/Presets/Cairn/Aether/ per macOS audio preset conventions

### Pending Todos

None yet.

### Quick Tasks Completed

| # | Description | Date | Commit | Directory |
|---|-------------|------|--------|-----------|
| 1 | Implement 6 factory presets and wire PresetSelector | 2026-02-19 | 5a0ddac | [1-implement-6-factory-presets-and-wire-pre](./quick/1-implement-6-factory-presets-and-wire-pre/) |
| 2 | Fix comb filtering: 1ms delay floor + dry allpass decorrelation | 2026-02-19 | 6008809 | [2-fix-comb-filtering-minimum-delay-floor-d](./quick/2-fix-comb-filtering-minimum-delay-floor-d/) |
| 3 | Mix lock button, arrow step buttons, user preset save/load | 2026-02-19 | 6d9face | [3-mix-lock-dropdown-arrows-user-preset-sav](./quick/3-mix-lock-dropdown-arrows-user-preset-sav/) |

### Blockers/Concerns

- ~~Volume drops slightly when Resonance Weight is turned up~~ -- RESOLVED in 05-02: Root cause was crossfade formula replacing dry signal instead of adding resonance energy. Fixed to additive blend (commit e95b5ab). Auto-gain also retuned to -3.5*pow(mix,1.4).
- ~~Room Size sweep produces comb-filter artifacts at intermediate positions~~ -- ADDRESSED in quick-2: Two-pronged fix: (1) 1ms minimum delay floor on reflection taps prevents near-zero-delay copies at small Room Size (commit 6b01571). (2) 3-stage allpass decorrelation on dry signal reduces phase coherence at dry/wet mix point (commit 6008809).

## Session Continuity

Last session: 2026-02-20
Stopped at: Ad-hoc DSP tuning (auto-gain K=15, decorrelation fix, low shelf), display image, GitHub push
Resume file: .planning/phases/07-visualization/.continue-here.md
