---
phase: 04-air-distance
plan: 02
subsystem: dsp
tags: [cross-stage-coupling, air-absorption, reflections-darkening, decay-bias, processor-forwarding, daw-verification]

# Dependency graph
requires:
  - phase: 04-air-distance
    provides: "AirSection DSP with HF shelf, allpass smearing, 3 character presets (Warm/Neutral/Cold), kCharacterPresets array"
  - phase: 03-early-reflections-diffuse-tail
    provides: "ReflectionsSection with baseCutoff filtering, DiffuseTailSection with updateDecay, cross-stage coupling pattern in updateStageParams"
provides:
  - "Cross-stage Air -> Reflections darkening via setAirDarkening() (reduces baseCutoff by up to 3600Hz)"
  - "Cross-stage Character -> Tail decay bias via setCharacterDecayBias() (+/-10% decay modification)"
  - "Complete Air parameter forwarding in updateStageParams (amount, character, coupling)"
  - "Human-verified Air stage in DAW: spatial HF softening, distinct character variants, smooth automation"
affects: [05-excitation-room-tone, 06-ui-visualization]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Character-scaled cross-stage coupling: coupling strength = airAmount * characterPreset.couplingFactor"
    - "Zero-coupling baseline: all cross-stage links multiply by airAmount, guaranteeing zero propagation at Air 0%"
    - "Clamped bias pattern: jlimit on coupling values to bound cross-stage influence (2kHz cutoff floor, +/-10% decay)"

key-files:
  created: []
  modified:
    - Source/dsp/ReflectionsSection.h
    - Source/dsp/ReflectionsSection.cpp
    - Source/dsp/DiffuseTailSection.h
    - Source/dsp/DiffuseTailSection.cpp
    - Source/PluginProcessor.cpp

key-decisions:
  - "2kHz floor on Reflections baseCutoff prevents Air darkening from making reflections inaudible"
  - "+/-10% clamp on character decay bias keeps tail modification subtle and musical"
  - "All cross-stage coupling multiplied by airAmount ensures zero propagation at Air 0%"

patterns-established:
  - "Multi-target cross-stage coupling: single source parameter (Air) drives multiple downstream stages with per-target scaling"
  - "Character preset lookup in processor: kCharacterPresets[airCharIndex] accessed from updateStageParams for coupling factor dispatch"

requirements-completed: [AIR-01, AIR-02, AIR-05]

# Metrics
duration: 6min
completed: 2026-02-18
---

# Phase 4 Plan 02: Cross-Stage Air Coupling Summary

**Cross-stage Air coupling to Reflections (HF darkening up to 3600Hz) and Diffuse Tail (character-biased decay +/-10%), with complete processor forwarding and human-verified DAW performance across all 6 verification points**

## Performance

- **Duration:** 6 min
- **Started:** 2026-02-18T21:15:00Z
- **Completed:** 2026-02-18T21:21:00Z
- **Tasks:** 2 (1 auto + 1 human-verify checkpoint)
- **Files modified:** 5

## Accomplishments
- Added setAirDarkening() to ReflectionsSection that reduces baseCutoff by up to 3600Hz with a 2kHz safety floor, creating progressive HF darkening of early reflections proportional to Air Amount and character scaling
- Added setCharacterDecayBias() to DiffuseTailSection that modifies decay time by +/-10% based on character preset (Warm extends, Cold tightens)
- Wired complete Air parameter forwarding in PluginProcessor::updateStageParams() including amount, character, and all cross-stage coupling links
- Ensured zero-coupling baseline: at Air 0%, all cross-stage links produce zero effect (airAmount multiplied into every coupling path)
- Human DAW verification passed all 6 checks: Air Amount sweep, character comparison, transient softening, bypass A/B, cross-stage coupling audibility, and automation smoothness

## Task Commits

Each task was committed atomically:

1. **Task 1: Add cross-stage coupling setters and wire processor forwarding** - `32c90df` (feat)
2. **Task 2: DAW verification of Air & Distance stage** - Human checkpoint (approved)

**Plan metadata:** `ed34f8b` (docs: complete plan)

## Files Created/Modified
- `Source/dsp/ReflectionsSection.h` - Added setAirDarkening() public method and airDarkeningFactor private member
- `Source/dsp/ReflectionsSection.cpp` - Implemented setAirDarkening() with jlimit(0,2), integrated airCutoffReduction into baseCutoff with 2kHz floor
- `Source/dsp/DiffuseTailSection.h` - Added setCharacterDecayBias() public method and characterDecayBias private member
- `Source/dsp/DiffuseTailSection.cpp` - Implemented setCharacterDecayBias() with jlimit(-0.10,0.10), applied biasedDecayMs to feedback gain calculation
- `Source/PluginProcessor.cpp` - Added airSection.setAmount/setCharacter forwarding, reflectionsSection.setAirDarkening coupling, diffuseTailSection.setCharacterDecayBias coupling via kCharacterPresets lookup

## Decisions Made
- baseCutoff floor of 2kHz prevents Air darkening from making early reflections inaudible even at extreme settings
- Character decay bias clamped to +/-10% keeps the tail modification subtle -- it should shift perception, not dramatically alter decay length
- All coupling paths multiply by airAmount ensuring zero propagation at Air 0% (baseline does not leak into downstream stages)
- kCharacterPresets accessed directly from processor via file-scope static const array in AirSection.h

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered
None

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Air & Distance stage (Phase 4) is complete: both AirSection DSP (Plan 01) and cross-stage coupling (Plan 02) are implemented and human-verified
- All 6 DSP stages in the signal chain now have their core processing: Resonance (Stage I), Reflections (Stage II), Air (Stage III), Excitation (Stage IV -- placeholder), Room Tone (Stage V -- placeholder), Diffuse Tail (Stage VI)
- Phase 5 will implement Excitation and Room Tone to complete the full six-stage processing chain
- Cross-stage coupling pattern is established and can be extended for any future stage-to-stage links

## Self-Check: PASSED

- All 5 modified source files verified on disk
- Commit 32c90df verified in git log
- SUMMARY.md created at .planning/phases/04-air-distance/04-02-SUMMARY.md

---
*Phase: 04-air-distance*
*Completed: 2026-02-18*
