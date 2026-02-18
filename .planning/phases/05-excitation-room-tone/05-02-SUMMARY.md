---
phase: 05-excitation-room-tone
plan: 02
subsystem: dsp
tags: [kellett-pink-noise, room-tone, noise-shaping, biquad-filters, stereo-decorrelation, gating, auto-gain]

# Dependency graph
requires:
  - phase: 01-project-scaffold
    provides: "Parameter layout with tone_ambience, tone_bypass; stage convention (prepare/process/reset/setBypass)"
  - phase: 02-cabinet-resonance
    provides: "Room Size and Shape parameters for cross-stage coupling to Room Tone spectral shaping"
  - phase: 03-early-reflections-diffuse-tail
    provides: "Shape parameter (7 shapes) mapped to ShapeAmbientCharacter for room tone texture"
  - phase: 04-air-distance
    provides: "AirSection bypass crossfade pattern; cross-stage parameter forwarding pattern"
  - phase: 05-excitation-room-tone
    plan: 01
    provides: "ExcitationSection (Stage IV) completing the 6-stage chain for auto-gain tuning"
provides:
  - "RoomToneSection: Kellett IIR pink noise with biquad spectral shaping, 3 gating modes, stereo decorrelation"
  - "tone_gate parameter: Always On / Signal-Gated / Transport-Only gate modes"
  - "Auto-gain tuned for full 6-stage chain: -3.5 * pow(mix, 1.4)"
  - "Cross-stage Room Size/Shape coupling to room tone spectral character"
  - "Complete 6-stage processing chain fully operational and DAW-verified"
  - "Resonance Weight additive blend fix (was crossfade, now correctly adds resonance energy)"
affects: [06-ui-visualization, 08-polish-optimization]

# Tech tracking
tech-stack:
  added: []
  patterns: [kellett-iir-pink-noise, shape-ambient-character-table, one-pole-asymmetric-gate-envelope, transport-state-gating]

key-files:
  created: []
  modified:
    - Source/Parameters.h
    - Source/dsp/RoomToneSection.h
    - Source/dsp/RoomToneSection.cpp
    - Source/PluginProcessor.h
    - Source/PluginProcessor.cpp
    - Source/dsp/MixSection.cpp
    - Source/dsp/MixSection.h
    - Source/dsp/ResonanceSection.h
    - Source/dsp/ResonanceSection.cpp

key-decisions:
  - "Auto-gain formula -3.5*pow(mix,1.4) tuned for full 6-stage chain with Excitation energy addition"
  - "Resonance Weight uses additive blend (not crossfade) to preserve dry signal while adding resonance energy"
  - "One-pole asymmetric envelope for gate (50ms attack, 500ms release) instead of SmoothedValue"
  - "ShapeAmbientCharacter lookup table maps 7 room shapes to spectral character offsets for room tone"

patterns-established:
  - "Transport state query pattern: getPlayHead()->getPosition()->getIsPlaying() with safe default true"
  - "Cross-stage coupling: Room Size/Shape from Stage II forwarded to Stage V for ambient character"
  - "ShapeAmbientCharacter table: presenceFreqOffset, lpfCutoffOffset, presenceGainDb per shape"

requirements-completed: [TONE-01, TONE-02, TONE-03, TONE-04, TONE-05, TONE-06, TONE-07]

# Metrics
duration: 19min
completed: 2026-02-18
---

# Phase 5 Plan 2: Room Tone Section Summary

**Kellett IIR pink noise with biquad spectral shaping, 3 gating modes (Always On / Signal-Gated / Transport-Only), Room Size/Shape coupling, stereo decorrelation, and auto-gain tuned for complete 6-stage chain**

## Performance

- **Duration:** 19 min (including DAW verification)
- **Started:** 2026-02-18T23:14:10Z
- **Completed:** 2026-02-18T23:33:15Z
- **Tasks:** 3
- **Files modified:** 9

## Accomplishments
- Complete RoomToneSection with Kellett IIR pink noise, HPF/presence/LPF biquad spectral shaping, slow LFO modulation, and stereo decorrelation via independent RNG seeds
- Three gating modes: Always On (constant ambience), Signal-Gated (fades with input silence), Transport-Only (mutes when DAW stopped)
- Auto-gain compensation tuned for full 6-stage chain: formula updated from -2.5*pow(mix,1.5) to -3.5*pow(mix,1.4) accounting for Excitation harmonic energy
- Fixed Resonance Weight crossfade-vs-additive bug discovered during DAW verification
- Full six-stage processing chain DAW-verified: all stages bypass cleanly, auto-gain maintains volume consistency

## Task Commits

Each task was committed atomically:

1. **Task 1: Add tone_gate parameter and implement RoomToneSection** - `ebd5e52` (feat)
2. **Task 2: Wire Room Tone parameters, tune auto-gain, and report latency** - `fcdaf7b` (feat)
3. **Task 3: DAW verification of Stages IV + V** - checkpoint (human-verify, approved)

**Bug fix during verification:** `e95b5ab` (fix) - Resonance Weight crossfade to additive blend

## Files Created/Modified
- `Source/Parameters.h` - Added tone_gate AudioParameterChoice (Always On, Signal-Gated, Transport-Only), total params now 20
- `Source/dsp/RoomToneSection.h` - Complete class with KellettPinkNoise struct, ShapeAmbientCharacter table, GateMode enum, BiquadStatic spectral shaping, LFO modulation, stereo decorrelation
- `Source/dsp/RoomToneSection.cpp` - Full DSP: prepare (filter setup, envelope coefficients), process (white noise -> Kellett pink -> HPF/presence/LPF shaping -> LFO modulation -> gating -> level control), updateShapingFilters (Room Size/Shape coupling), reset, parameter setters
- `Source/PluginProcessor.h` - Added toneGateParam cached pointer
- `Source/PluginProcessor.cpp` - Forward Ambience, Gate mode, Room Size (cross-stage), Shape (cross-stage), transport state, bypass to RoomToneSection; transport state query from AudioPlayHead
- `Source/dsp/MixSection.cpp` - Auto-gain formula updated to -3.5*pow(mix,1.4) for full 6-stage chain
- `Source/dsp/MixSection.h` - Minor update for auto-gain documentation
- `Source/dsp/ResonanceSection.h` - Changed Weight application from crossfade to additive blend
- `Source/dsp/ResonanceSection.cpp` - Changed Weight application from crossfade to additive blend

## Decisions Made
- Auto-gain formula -3.5*pow(mix,1.4) provides better compensation for the energy added by Excitation saturation and Room Tone noise across the Mix sweep
- Resonance Weight should use additive blend (dry + wet*weight) not crossfade (dry*(1-weight) + wet*weight) -- crossfade was replacing dry signal with resonance instead of adding resonance energy on top
- One-pole asymmetric envelope for gating (50ms attack, 500ms release) provides natural-sounding gate behavior without SmoothedValue limitations
- ShapeAmbientCharacter lookup table keeps room tone texture changes orthogonal from reflections while still being shape-aware

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Resonance Weight crossfade changed to additive blend**
- **Found during:** Task 3 (DAW verification)
- **Issue:** Resonance Weight parameter was using crossfade formula (replacing dry with wet) instead of additive blend. This caused the dry signal to be attenuated as Weight increased, rather than adding resonance energy on top.
- **Fix:** Changed ResonanceSection from `dry*(1-weight) + wet*weight` to `dry + wet*weight` so resonance energy is added to the signal rather than replacing it.
- **Files modified:** Source/dsp/ResonanceSection.h, Source/dsp/ResonanceSection.cpp
- **Verification:** DAW re-test confirmed Weight now adds body without attenuating dry signal
- **Committed in:** `e95b5ab`

---

**Total deviations:** 1 auto-fixed (1 bug fix)
**Impact on plan:** Bug fix was necessary for correct Resonance behavior. No scope creep.

## Issues Encountered
None beyond the Resonance Weight bug discovered during DAW verification (documented above as deviation).

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- All six DSP stages are fully operational and DAW-verified
- Phase 5 (Excitation & Room Tone) is complete -- all 12 requirements fulfilled (EXCIT-01 through EXCIT-05, TONE-01 through TONE-07)
- Phase 6 (User Interface) can proceed: all parameters and DSP stages exist for UI binding
- Auto-gain compensation is tuned and verified for the complete processing chain
- Volume drop blocker from Phase 2 is resolved (Resonance Weight additive blend fix)

## Self-Check: PASSED

All 9 modified files verified present. All 3 commit hashes (ebd5e52, fcdaf7b, e95b5ab) found in git log.

---
*Phase: 05-excitation-room-tone*
*Completed: 2026-02-18*
