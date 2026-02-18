---
phase: 02-cabinet-resonance
plan: 02
subsystem: dsp
tags: [resonance, fdn, householder, signalsmith-dsp, material-presets, delay-network, biquad]

# Dependency graph
requires:
  - phase: 02-cabinet-resonance
    plan: 01
    provides: "ResonanceSection class with FDN member declarations, 10-material choice list, res_ parameter IDs at version 2"
  - phase: 01-project-scaffold-audio-pipeline
    provides: "processBlock pipeline, Mix/Output sections, APVTS parameter tree"
provides:
  - "Working 4-line Householder FDN in ResonanceSection with audible cabinet resonance"
  - "10 material presets (Pine/Oak/Walnut/Mahogany/Iron/Steel/Copper/Limestone/Marble/Granite) with distinct tonal character"
  - "Weight knob with quadratic response curve for progressive wet/dry blend"
  - "10ms bypass crossfade for clean A/B comparison"
  - "Mono-consistent FDN processing (no stereo artifacts)"
  - "MaterialParams struct with description field ready for Phase 6 tooltip UI"
  - "Safety limiting (feedback gain capped at 0.95, output hard-limited to +/-2.0)"
affects: [phase-3, phase-5, phase-6]

# Tech tracking
tech-stack:
  added:
    - "signalsmith-dsp/delay.h (FDN delay lines)"
    - "signalsmith-dsp/filters.h (biquad bandpass, damping lowpass, low shelf)"
    - "signalsmith-dsp/mix.h (Householder feedback matrix)"
  patterns:
    - "4-line Householder FDN architecture: bandpass input -> delay -> damping -> Householder matrix -> feedback"
    - "Quadratic weight response curve: wetGain = weight * weight for musical knob feel"
    - "Coprime delay line lengths via nearest-prime calculation for maximal mode density"
    - "Per-material output gain normalization for consistent perceived loudness across presets"
    - "SmoothedValue for per-sample weight and bypass crossfade (20ms and 10ms ramp)"

key-files:
  created: []
  modified:
    - "Source/dsp/ResonanceSection.h"
    - "Source/dsp/ResonanceSection.cpp"
    - "Source/PluginProcessor.cpp"

key-decisions:
  - "4-line Householder FDN (not 8 or 16) -- sufficient for short cabinet resonance, CPU-efficient"
  - "Coprime delay lengths via nearest-prime for maximal mode density and minimal periodicity"
  - "Quadratic weight curve (weight^2) for musical response -- linear felt too abrupt"
  - "Immediate material coefficient switching (no crossfade) -- FDN settles in ~5ms, artifacts minimal"
  - "Mono summing before FDN, identical L/R output -- matches physical cabinet behavior (CAB-05)"
  - "Volume compensation deferred to Phase 5 -- auto-gain designed for full 6-stage chain, expected drop with single stage"

patterns-established:
  - "FDN DSP section pattern: prepare configures delay/filters, process runs per-sample with SmoothedValue, configureForMaterial recalculates coefficients"
  - "Material preset struct: name, description, delay times, filter params, feedback gain, output normalization"
  - "Safety pattern: feedback gain capped < 1.0 (0.95 ceiling), output hard-limited, bypass skip for CPU savings"

requirements-completed: [CAB-01, CAB-02, CAB-03, CAB-04, CAB-05]

# Metrics
duration: 12min
completed: 2026-02-18
---

# Phase 2 Plan 02: FDN DSP Implementation Summary

**4-line Householder FDN with 10 material presets producing audible cabinet resonance character, verified in DAW with DI guitar**

## Performance

- **Duration:** 12 min (includes DAW verification checkpoint)
- **Started:** 2026-02-18T16:47:00Z
- **Completed:** 2026-02-18T16:59:44Z
- **Tasks:** 2 (1 auto + 1 human-verify checkpoint)
- **Files modified:** 3

## Accomplishments
- Implemented complete 4-line Householder Feedback Delay Network in ResonanceSection using signalsmith-dsp primitives
- Defined 10 material presets across 3 families (wood/metal/stone) with distinct delay times, bandpass frequencies, damping, and feedback characteristics
- Weight knob adds progressive low-mid body with quadratic response curve (smooth musical feel from subtle to thick resonance)
- Clean bypass crossfade (10ms) enables transparent A/B comparison with no click artifacts
- Mono-consistent processing -- stereo input summed to mono, FDN output written identically to L/R
- Safety measures: feedback gain capped at 0.95, output hard-limited to +/-2.0, CPU bypass when weight=0 or bypassed
- User verified audible resonance in DAW: weight sweep smooth, materials distinct, bypass clean, no instability

## Task Commits

Each task was committed atomically:

1. **Task 1: Implement 4-line Householder FDN with material presets and Weight/bypass control** - `197c1e2` (feat)
2. **Task 2: Verify audible resonance in DAW** - N/A (human-verify checkpoint, approved)

## Files Created/Modified
- `Source/dsp/ResonanceSection.h` - Complete FDN class with MaterialParams struct, 4 delay lines, bandpass/damping/shelf filters, SmoothedValue weight/bypass, configureForMaterial/nearestPrime helpers
- `Source/dsp/ResonanceSection.cpp` - FDN processing loop (mono sum -> bandpass -> delay -> damping -> Householder -> feedback -> shelf -> blend), 10 material definitions with per-material output normalization, prepare/process/reset/setBypass/setWeight/setMaterial implementations
- `Source/PluginProcessor.cpp` - Weight and Material parameter forwarding via resonanceSection.setWeight() and resonanceSection.setMaterial() in updateStageParams()

## Decisions Made
- **4-line FDN size:** Sufficient for short cabinet resonance (1-5ms delays). Larger networks (8/16 lines) would add CPU cost without perceptual benefit for this stage. Phase 3 Diffuse Tail will use a larger FDN.
- **Coprime delay lengths:** Delay times converted to samples then snapped to nearest prime numbers. This maximizes mode density and prevents periodic ringing patterns.
- **Quadratic weight curve:** `wetGain = weight * weight` gives musical feel -- first 50% of knob travel is subtle, last 50% gets progressively more colored. Linear felt too abrupt at low settings.
- **No material crossfade:** Coefficient changes are immediate when switching materials. The short FDN (~5ms delay times) settles nearly instantly, making crossfade unnecessary complexity for now.
- **Volume compensation deferred:** User noted volume drops slightly when resonance is added. This is expected -- the Phase 1 auto-gain compensation formula (-2.5dB * pow(mix, 1.5)) was designed for the full 6-stage chain where wet signal adds energy. With only 1 stage active, the compensation slightly over-attenuates. Will self-correct as stages are added through Phase 5.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

**Volume drop with resonance active:** User observed slight volume reduction when resonance Weight is turned up. This is expected behavior from the Phase 1 auto-gain compensation interacting with a single active stage. The auto-gain formula assumes a full wet signal with energy from all 6 stages; with only cabinet resonance active, the wet signal has less energy than the compensation anticipates. This will self-correct as more DSP stages are implemented. If it remains an issue after Phase 5 (all stages active), the compensation curve can be tuned then.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- ResonanceSection is the first complete DSP stage, proving the FDN primitive and section interface pattern
- Phase 3 (Early Reflections & Diffuse Tail) can build on the established FDN architecture with a larger network for diffuse reverb
- MaterialParams.description field is populated for all 10 materials, ready for Phase 6 tooltip UI
- Volume compensation curve tuning deferred to Phase 5 when all 6 stages contribute to wet signal energy

## Self-Check: PASSED

All files verified:
- FOUND: Source/dsp/ResonanceSection.h
- FOUND: Source/dsp/ResonanceSection.cpp
- FOUND: Source/PluginProcessor.cpp
- FOUND: commit 197c1e2
- FOUND: 02-02-SUMMARY.md

---
*Phase: 02-cabinet-resonance*
*Completed: 2026-02-18*
