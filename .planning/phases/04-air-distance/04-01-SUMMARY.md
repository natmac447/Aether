---
phase: 04-air-distance
plan: 01
subsystem: dsp
tags: [biquad, high-shelf, allpass, phase-smearing, crossfade, signalsmith-dsp, air-absorption]

# Dependency graph
requires:
  - phase: 01-project-scaffold
    provides: "Parameter infrastructure (APVTS, ParamIDs, AudioParameterChoice)"
  - phase: 02-cabinet-resonance
    provides: "BiquadStatic filter usage patterns, bypass crossfade pattern"
  - phase: 03-early-reflections-diffuse-tail
    provides: "Shape crossfade pattern (30ms dual-chain blend), cross-stage coupling via updateStageParams"
provides:
  - "Complete AirSection DSP: HF shelf absorption, LF shelf, allpass phase smearing cascade"
  - "AirCharacterPreset struct with 3 physics-derived presets (Warm/Neutral/Cold)"
  - "Character crossfade (30ms) via cheapEnergyCrossfade with dual filter chains"
  - "air_char parameter upgraded to 3-choice at version 2"
  - "Cross-stage coupling values: tailDecayBias, reflDarkeningScale per character"
affects: [04-air-distance, 05-mixing-integration, 06-ui-visualization]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Dual filter chain crossfade for character switching (current + pending filter sets)"
    - "cheapEnergyCrossfade for per-sample energy-preserving blend between filter outputs"
    - "Coefficient recalculation throttling (every 16 samples during smoothing)"
    - "L/R allpass frequency offset (12%) for stereo decorrelation"

key-files:
  created: []
  modified:
    - Source/Parameters.h
    - Source/dsp/AirSection.h
    - Source/dsp/AirSection.cpp

key-decisions:
  - "BiquadStatic assignment (not copyFrom) for crossfade completion to preserve filter state continuity"
  - "cheapEnergyCrossfade x parameter convention: x = (1 - charCrossfade) maps current->pending transition"

patterns-established:
  - "Character preset struct: compile-time struct defining complete filter parameter set per environmental condition"
  - "Dual filter chain crossfade: process input through both current and pending filter chains during 30ms transition"

requirements-completed: [AIR-01, AIR-02, AIR-03, AIR-04, AIR-05]

# Metrics
duration: 4min
completed: 2026-02-18
---

# Phase 4 Plan 01: Air & Distance DSP Summary

**Physics-driven HF shelf absorption with 3 character presets (Warm/Neutral/Cold), allpass phase smearing cascade for transient softening, and 30ms energy-preserving character crossfade**

## Performance

- **Duration:** 4 min
- **Started:** 2026-02-18T21:10:35Z
- **Completed:** 2026-02-18T21:14:12Z
- **Tasks:** 1
- **Files modified:** 3

## Accomplishments
- Upgraded air_char parameter from 2-choice (Warm/Neutral) to 3-choice (Warm/Neutral/Cold) with version bump to 2 and new default of Neutral
- Implemented complete AirSection DSP with HF high-shelf absorption (NOT lowpass), optional LF shelf per character, and 4-6 stage allpass cascade for physics-driven phase smearing
- Defined 3 character presets with physics-derived values: Warm (6kHz, 5 allpass, +1.5dB LF boost), Neutral (8kHz, 4 allpass, no LF), Cold (10kHz, 4 allpass, -1dB LF cut)
- Implemented character crossfade (30ms) via cheapEnergyCrossfade with dual filter chain architecture
- Implemented bypass crossfade (10ms) and Air Amount smoothing (25ms) with coefficient throttling every 16 samples
- L/R allpass stereo decorrelation via 12% frequency offset

## Task Commits

Each task was committed atomically:

1. **Task 1: Update air_char parameter to 3 choices and implement AirSection DSP** - `867e06f` (feat)

**Plan metadata:** (pending)

## Files Created/Modified
- `Source/Parameters.h` - air_char upgraded to version 2 with 3 choices, default Neutral
- `Source/dsp/AirSection.h` - AirCharacterPreset struct, kCharacterPresets[3], complete AirSection class with dual filter chains
- `Source/dsp/AirSection.cpp` - Full DSP: prepare, process (per-sample loop with shelf/allpass/crossfade), updateFilters, setters

## Decisions Made
- Used BiquadStatic full object assignment (not copyFrom) when completing character crossfade, preserving both coefficients and filter state for seamless transition
- cheapEnergyCrossfade x parameter mapped as (1 - charCrossfade) so x=0 at crossfade start (all current) and x=1 at crossfade end (all pending)
- Allpass stage count differentiated per character (5 for Warm, 4 for Neutral/Cold) providing additional diffusion variation beyond Q alone

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered
None

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- AirSection DSP is complete and compiles successfully
- Plan 04-02 will wire cross-stage coupling (Air -> Reflections darkening, Air -> Tail decay bias) through updateStageParams
- Character preset coupling values (tailDecayBias, reflDarkeningScale) are defined and ready for forwarding

## Self-Check: PASSED

- All 3 modified files verified on disk
- Commit 867e06f verified in git log
- Build compiles without errors

---
*Phase: 04-air-distance*
*Completed: 2026-02-18*
