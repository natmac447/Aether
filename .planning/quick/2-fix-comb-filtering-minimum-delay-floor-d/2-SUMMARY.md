---
phase: quick
plan: 2
subsystem: dsp
tags: [allpass, comb-filter, delay-floor, dry-wet-mixing, decorrelation]

# Dependency graph
requires:
  - phase: 03-early-reflections-diffuse-tail
    provides: "ReflectionsSection tapped delay line"
  - phase: 01-project-scaffold
    provides: "MixSection dry/wet mixer"
provides:
  - "1ms minimum delay floor on reflection taps preventing near-zero-delay copies"
  - "3-stage allpass decorrelation on dry signal reducing comb notches at mix point"
affects: []

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Allpass decorrelation chain before dry/wet mixing"
    - "Minimum delay floor clamp on tapped delay reads"

key-files:
  created: []
  modified:
    - Source/dsp/ReflectionsSection.cpp
    - Source/dsp/MixSection.h
    - Source/dsp/MixSection.cpp

key-decisions:
  - "1ms delay floor chosen as minimum -- 44 samples at 44.1kHz, sufficient to prevent comb artifacts while preserving short room character"
  - "3 allpass stages at coprime frequencies (318/145/94 Hz) derived from 0.5/1.1/1.7ms delay equivalents"
  - "R-channel 1.12x frequency offset matches AirSection stereo decorrelation convention"

patterns-established:
  - "Dry signal decorrelation before mix: allpass chain applied in pushDrySamples() before DryWetMixer"

requirements-completed: []

# Metrics
duration: 2min
completed: 2026-02-19
---

# Quick Task 2: Fix Comb Filtering Summary

**1ms delay floor on reflection taps plus 3-stage allpass dry decorrelation to eliminate upper-mid comb artifacts at mix point**

## Performance

- **Duration:** 2 min
- **Started:** 2026-02-19T17:26:01Z
- **Completed:** 2026-02-19T17:28:07Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments
- Reflection taps now clamp to minimum 1ms delay (44 samples at 44.1kHz), preventing near-zero-delay copies at small Room Size values
- Dry signal processed through 3-stage allpass decorrelation chain before dry/wet mixing, reducing coherence that causes sharp comb notches in 800Hz-3kHz range
- R-channel allpass frequencies offset by 1.12x for stereo decorrelation matching AirSection convention
- MixSection public API unchanged -- no PluginProcessor modifications needed

## Task Commits

Each task was committed atomically:

1. **Task 1: Add minimum 1ms delay floor to reflection taps** - `6b01571` (fix)
2. **Task 2: Add allpass decorrelation chain to dry signal path in MixSection** - `6008809` (feat)

## Files Created/Modified
- `Source/dsp/ReflectionsSection.cpp` - Added minDelaySamples floor (sr * 0.001) clamping both current and pending shape tap reads
- `Source/dsp/MixSection.h` - Added kDryDecorrelationStages, BiquadStatic allpass filter arrays (L/R), storedSampleRate member
- `Source/dsp/MixSection.cpp` - Allpass configuration in prepare(), decorrelation processing in pushDrySamples(), filter reset in reset()

## Decisions Made
- 1ms delay floor chosen as minimum -- at 44.1kHz this gives 44 samples, well above sub-sample territory while preserving short room character like The Crypt
- Coprime-ish allpass frequencies (318Hz, 145Hz, 94Hz) derived from delay-time equivalents (0.5ms, 1.1ms, 1.7ms) to avoid reinforcing any single comb frequency
- R-channel 1.12x frequency offset follows AirSection established convention for stereo decorrelation
- AudioBuffer copy in pushDrySamples for mutable processing -- const input AudioBlock cannot be modified in place

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Context

This fix addresses the comb-filter artifacts noted in STATE.md blockers section ("Room Size sweep produces comb-filter artifacts at intermediate positions"). The two independent fixes target different aspects: the delay floor prevents the most extreme case (near-zero-delay copies at small Room Size), while the allpass decorrelation reduces the general case (phase coherence at the dry/wet mix point).

## Self-Check: PASSED

- All 3 modified files verified on disk
- Both task commits (6b01571, 6008809) verified in git log
- Build succeeds with no errors or new warnings

---
*Quick Task: 2*
*Completed: 2026-02-19*
