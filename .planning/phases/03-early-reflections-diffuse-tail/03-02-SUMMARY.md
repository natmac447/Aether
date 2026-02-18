---
phase: 03-early-reflections-diffuse-tail
plan: 02
subsystem: dsp
tags: [fdn, hadamard, reverb, allpass, diffusion, delay, signalsmith-dsp]

# Dependency graph
requires:
  - phase: 02-cabinet-resonance
    provides: "ResonanceSection pattern (FDN, nearestPrime, SmoothedValue bypass blend)"
provides:
  - "Complete DiffuseTailSection with 8-line Hadamard FDN, allpass diffusion, pre-delay"
  - "Setter methods for Decay, Diffusion, PreDelay, HFDamping, ShapeInfluence"
  - "Shape-to-tail influence lookup table (7 shapes)"
affects: [03-early-reflections-diffuse-tail, 04-air-excitation, 05-mix-output]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "8-line Hadamard FDN with coprime delay lengths for reverb tail"
    - "Explicit delay+feedback allpass diffusion (not biquad) for 1.5-8ms delays"
    - "Additive wet signal (tail adds reverb on top, does not replace input)"
    - "Cross-stage parameter forwarding via setter methods"
    - "Shape modal character modulating FDN delay lengths by +/-5%"

key-files:
  created: []
  modified:
    - "Source/dsp/DiffuseTailSection.h"
    - "Source/dsp/DiffuseTailSection.cpp"

key-decisions:
  - "Even/odd stereo distribution (even lines=L, odd=R) instead of StereoMultiMixer -- simpler, sufficient with Hadamard cross-coupling"
  - "Shape influence via lookup table rather than coupling to ReflectionsSection presets directly -- keeps stages orthogonal"
  - "Decay smoothing recalculates feedback gains per-sample only while smoothing, skips when stable"

patterns-established:
  - "8-line Hadamard FDN: delay -> damp -> Hadamard -> feedback*gain + input -> write"
  - "Explicit allpass cascade: output = -g*input + delayed, write = input + g*delayed"
  - "Additive tail: output = input + wet (not replacing input)"

requirements-completed: [TAIL-01, TAIL-02, TAIL-05, TAIL-06]

# Metrics
duration: 5min
completed: 2026-02-18
---

# Phase 3 Plan 2: Diffuse Tail Summary

**8-line Hadamard FDN reverb tail with 4-stage allpass input diffusion, automatic pre-delay linked to Room Size, and frequency-dependent feedback damping linked to Air**

## Performance

- **Duration:** 5 min
- **Started:** 2026-02-18T18:53:23Z
- **Completed:** 2026-02-18T18:58:29Z
- **Tasks:** 1
- **Files modified:** 2

## Accomplishments
- Complete 8-line Hadamard FDN with coprime delay lengths (22-63ms range, nearest-prime quantized at runtime)
- 4-stage allpass input diffusion network per stereo channel with 12% R-offset for decorrelation
- Pre-delay buffer (0.5-25ms) with compressed curve linked to Room Size
- Frequency-dependent feedback damping (lowpass in feedback loop, 2kHz-12kHz linked to Air parameter)
- Shape influence on tail character: diffusion bias and modal character modulation across 7 room shapes
- RT60-to-gain feedback calculation supporting 50ms-2000ms decay range with 0.9999 gain ceiling
- Safety hard limiting (+/-4.0f) and NaN debug assertions

## Task Commits

Each task was committed atomically:

1. **Task 1: Implement DiffuseTailSection with 8-line Hadamard FDN** - `87c2a74` (feat)

**Plan metadata:** (pending)

## Files Created/Modified
- `Source/dsp/DiffuseTailSection.h` - Complete class with 8-line FDN, allpass diffusion, pre-delay, damping, all setter methods
- `Source/dsp/DiffuseTailSection.cpp` - Full DSP implementation: nearestPrime, configureDelayLengths, prepare, process (per-sample stereo FDN loop), updateDecay, updateDampingFilters, all setters, reset

## Decisions Made
- Even/odd stereo distribution (even FDN lines get L, odd get R) instead of StereoMultiMixer -- simpler implementation, sufficient since Hadamard matrix provides dense cross-coupling between all 8 lines
- Shape-to-tail influence via static lookup table rather than coupling directly to ReflectionsSection presets -- maintains stage orthogonality per ARCHITECTURE.md anti-pattern avoidance
- Decay smoothing only recalculates all 8 feedback gains while SmoothedValue is actively smoothing -- skips per-sample updateDecay when stable to save CPU
- Diffusion bias from shape adds up to +0.15 to base diffusion coefficient (clamped to 0.7 max)
- Modal character modulates FDN delay lengths by up to +/-5% (even indices shorter, odd longer) to create shape-specific mode clustering

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- DiffuseTailSection is structurally complete with all DSP internals implemented
- All parameters received via setter methods (setDecay, setDiffusion, setPreDelay, setHFDamping, setShapeInfluence, setBypass)
- Ready for processor wiring in Plan 03 (parameter forwarding from PluginProcessor::updateStageParams)
- Cross-stage links (Room Size -> pre-delay, Air -> HF damping, Shape -> tail character) implemented as setters, awaiting processor forwarding

## Self-Check: PASSED

- FOUND: Source/dsp/DiffuseTailSection.h
- FOUND: Source/dsp/DiffuseTailSection.cpp
- FOUND: commit 87c2a74
- FOUND: 03-02-SUMMARY.md

---
*Phase: 03-early-reflections-diffuse-tail*
*Completed: 2026-02-18*
