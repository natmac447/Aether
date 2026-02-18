---
phase: 05-excitation-room-tone
plan: 01
subsystem: dsp
tags: [multiband-saturation, oversampling, linkwitz-riley, tanh-waveshaping, crossover, latency-compensation]

# Dependency graph
requires:
  - phase: 01-project-scaffold
    provides: "Parameter layout with excit_drive, excit_bypass; stage convention (prepare/process/reset/setBypass)"
  - phase: 02-cabinet-resonance
    provides: "Material parameter (10 materials) for cross-stage coupling"
  - phase: 03-early-reflections-diffuse-tail
    provides: "Room Size parameter for cross-stage band balance coupling"
  - phase: 04-air-distance
    provides: "AirSection bypass crossfade pattern; cross-stage parameter forwarding pattern"
provides:
  - "ExcitationSection: 3-band multiband saturation with adaptive oversampling"
  - "Material-coupled excitation (per-band drive scaling based on Stage I material)"
  - "Room-size-coupled excitation (band balance adjusted by Stage II room size)"
  - "DAW latency reporting via setLatencySamples for plugin delay compensation"
  - "MixSection::setWetLatency() for DryWetMixer internal latency compensation"
affects: [05-excitation-room-tone, 06-ui-visualization, 08-polish-optimization]

# Tech tracking
tech-stack:
  added: [juce::dsp::Oversampling, juce::dsp::LinkwitzRileyFilter, juce::dsp::IIR::Filter]
  patterns: [adaptive-oversampling, multiband-crossover-waveshaping, material-excitation-coupling, room-size-band-balance]

key-files:
  created: []
  modified:
    - Source/dsp/ExcitationSection.h
    - Source/dsp/ExcitationSection.cpp
    - Source/PluginProcessor.cpp
    - Source/dsp/MixSection.h
    - Source/dsp/MixSection.cpp

key-decisions:
  - "Single oversampler with crossover at oversampled rate (not 3 per-band oversamplers) for CPU efficiency"
  - "LR filter allpass mode with two-output processSample for clean band splitting"
  - "Drive smoothing once per block (not per oversampled sample) since knob changes are slow"
  - "kMinG=0.15 ensures non-zero excitation at Drive 0% (real room always has acoustic effect)"

patterns-established:
  - "Adaptive oversampling: 4x/2x/1x based on sample rate thresholds (50kHz, 100kHz)"
  - "MaterialExcitationBias lookup table for cross-stage material coupling"
  - "Processor latency reporting pattern: section.getLatencySamples() -> setLatencySamples()"
  - "MixSection wet latency forwarding: mixSection.setWetLatency() after all sections prepared"

requirements-completed: [EXCIT-01, EXCIT-02, EXCIT-03, EXCIT-04, EXCIT-05]

# Metrics
duration: 4min
completed: 2026-02-18
---

# Phase 5 Plan 1: Excitation Section Summary

**3-band multiband tanh saturation with adaptive oversampling (4x/2x/1x), material-coupled drive scaling, room-size band balance, and DAW latency reporting**

## Performance

- **Duration:** 4 min
- **Started:** 2026-02-18T23:03:43Z
- **Completed:** 2026-02-18T23:07:43Z
- **Tasks:** 2
- **Files modified:** 5

## Accomplishments
- Complete ExcitationSection with 3-band LR4 crossover, per-band symmetric tanh waveshaping, and adaptive oversampling
- Cross-stage coupling: Material from Stage I adjusts per-band drive scaling; Room Size from Stage II adjusts band balance
- DAW plugin delay compensation: oversampling latency reported to host and DryWetMixer

## Task Commits

Each task was committed atomically:

1. **Task 1: Implement ExcitationSection with multiband saturation and oversampling** - `34b840d` (feat)
2. **Task 2: Wire Excitation parameter forwarding and latency reporting in processor** - `2f09865` (feat)

## Files Created/Modified
- `Source/dsp/ExcitationSection.h` - Complete class with LR crossover, oversampling, material bias table, drive constants
- `Source/dsp/ExcitationSection.cpp` - Full DSP: prepare (adaptive oversampling, filter setup, buffer alloc), process (upsample, 3-band split, per-band tanh waveshape, recombine, downsample, DC block, bypass crossfade), reset, parameter setters
- `Source/PluginProcessor.cpp` - Forward Drive/Material/RoomSize to ExcitationSection; report oversampling latency; set MixSection wet latency
- `Source/dsp/MixSection.h` - Added setWetLatency() declaration
- `Source/dsp/MixSection.cpp` - Added setWetLatency() implementation forwarding to dryWetMixer

## Decisions Made
- Used single oversampler with crossover at oversampled rate rather than 3 per-band oversamplers -- more CPU efficient and avoids phase issues from independent oversampling paths
- Used LinkwitzRileyFilter in allpass mode with the two-output processSample overload to extract both LP and HP outputs from one filter instance per crossover point
- Drive SmoothedValue read once per block (skip to end), not per oversampled sample -- Drive knob changes are slow, per-block granularity is sufficient
- kMinG=0.15 at Drive 0% provides subtle room nonlinearity per user constraint ("a real room always has some acoustic effect")

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered
None

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- ExcitationSection is fully operational in the processing chain
- Room Tone (Stage V, Plan 05-02) can proceed independently
- Auto-gain compensation tuning deferred to Plan 05-02 per research recommendation (tune after all stages operational)
- All cross-stage couplings wired: Material -> Excitation band scaling, Room Size -> Excitation band balance

## Self-Check: PASSED

All files verified present. All commit hashes found in git log.

---
*Phase: 05-excitation-room-tone*
*Completed: 2026-02-18*
