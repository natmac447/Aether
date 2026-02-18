# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-02-18)

**Core value:** Make digital amp sims and DI recordings sound like they were recorded in a real room with a real microphone -- through physically-modeled environmental layers, not reverb effects.
**Current focus:** Phase 4 complete. All Air & Distance plans done. Next: Phase 5 (Excitation & Room Tone)

## Current Position

Phase: 4 of 8 (Air & Distance) -- COMPLETE
Plan: 2 of 2 in current phase (all done)
Status: Phase Complete
Last activity: 2026-02-18 -- Completed 04-02-PLAN.md (cross-stage Air coupling + DAW verification)

Progress: [██████░░░░] 56%

## Performance Metrics

**Velocity:**
- Total plans completed: 9
- Average duration: 6min
- Total execution time: 1.0 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 1 - Project Scaffold | 2 | 11min | 6min |
| 2 - Cabinet Resonance | 2 | 17min | 9min |
| 3 - Early Reflections & Diffuse Tail | 3 | 20min | 7min |
| 4 - Air & Distance | 2 | 10min | 5min |

**Recent Trend:**
- Last 5 plans: 03-01 (7min), 03-03 (8min), 04-01 (4min), 04-02 (6min)
- Trend: Consistent (averaging 6min across Phase 4)

*Updated after each plan completion*

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

### Pending Todos

None yet.

### Blockers/Concerns

- Volume drops slightly when Resonance Weight is turned up (user-reported). Expected: auto-gain compensation formula was designed for full 6-stage chain. Should self-correct as stages are added. Revisit in Phase 5 if compensation curve needs tuning.
- Room Size sweep produces comb-filter artifacts at intermediate positions (user-reported during 03-03 DAW verification). Tapped delay line delay time combinations create constructive/destructive interference at certain ratios. Not a blocker -- classified as tuning refinement. Potential mitigations: delay time jitter during sweeps, non-linear room size curves to skip problematic ratios, per-tap detuning at intermediate sizes.

## Session Continuity

Last session: 2026-02-18
Stopped at: Phase 5 context gathered
Resume file: .planning/phases/05-excitation-room-tone/05-CONTEXT.md
