---
phase: 04-air-distance
verified: 2026-02-18T23:30:00Z
status: human_needed
score: 4/4 automated must-haves verified
re_verification:
  previous_status: human_needed
  previous_score: 4/4
  gaps_closed: []
  gaps_remaining: []
  regressions: []
human_verification:
  - test: "Air Amount sweep: slowly move III Air - Amount from 0% to 100% on a percussive or HF-rich source"
    expected: "Progressive HF softening -- cymbals and pick attack soften as Air increases. At 100%, sounds like 'next room over'. Must NOT sound like a blanket/lowpass tone knob."
    why_human: "HF rolloff shape and depth is perceptual -- cannot verify audio quality programmatically"
  - test: "Character comparison: set Air Amount to 60-70%, switch between Warm, Neutral, and Cold"
    expected: "Warm = gentle, thick with subtle low-end warmth; Neutral = balanced moderate HF reduction; Cold = brighter, crisper with slight thinness. All three noticeably different."
    why_human: "Perceptual character differentiation requires listening"
  - test: "Transient softening: use snare or picked guitar at Air 0%, 50%, 100%"
    expected: "0% = minimal change from bypass. 50% = subtle reduction in immediacy. 100% = clearly less 'sharp' -- the allpass smearing produces a washy/diffuse quality."
    why_human: "Allpass phase smearing effect on transient perception cannot be verified by code analysis"
  - test: "Bypass A/B: toggle III Air - Bypass on and off rapidly at Air 100%"
    expected: "Clean 10ms crossfade transition -- no clicks or pops. When bypassed, full brightness returns."
    why_human: "Audio artifact detection requires listening"
  - test: "Cross-stage coupling: set Air 80% Warm, listen to reflections and tail, then switch to Cold"
    expected: "Reflections become slightly brighter (Cold has lower reflDarkeningScale=0.7 vs Warm 1.2). Tail feels slightly tighter (Cold tailDecayBias=-0.1). At Air 0%, reflections and tail return to uncoupled state."
    why_human: "Cross-stage coupling subtlety and cohesion requires perceptual judgment"
  - test: "Automation smoothness: automate Air Amount from 0% to 100% over 2-4 bars"
    expected: "No zipper noise, clicks, or stepping artifacts. Coefficients update every 16 samples during smoothing."
    why_human: "Audio artifact detection requires listening"
---

# Phase 4: Air & Distance Verification Report

**Phase Goal:** Users can simulate high-frequency absorption and air diffusion that makes the signal sound like it traveled through a real room
**Verified:** 2026-02-18T23:30:00Z
**Status:** human_needed
**Re-verification:** Yes -- regression check after initial human_needed verdict

## Re-verification Summary

Previous verification (2026-02-18T22:00:00Z) found `status: human_needed` with all automated checks passing (4/4). No gaps were recorded. This re-verification confirms:

- No new commits since initial verification (codebase unchanged)
- All regression checks pass -- artifacts exist, are substantive, are wired
- Build still compiles cleanly (Aether, Aether_vst3_helper, Aether_VST3)
- Both plan commits verified in git log (867e06f, 32c90df)
- 04-02-SUMMARY.md attests human DAW verification passed all 6 checks

The human checkpoint (Plan 04-02, Task 2) is documented as "approved" in the SUMMARY. This verifier cannot independently confirm perceptual audio quality -- that attestation remains on the human who ran the DAW session. No regressions found. Status remains `human_needed` because the verifier cannot programmatically re-confirm audio quality; the SUMMARY's claim of approval is the human record of that confirmation.

---

## Goal Achievement

### Observable Truths (from ROADMAP Success Criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | User hears progressive HF rolloff as Air knob increases (cymbals and pick attack soften) | ? HUMAN | HF shelf confirmed in AirSection.cpp:149 -- `hfShelfL.highShelfDb(scaledFreq, shelfDb, ch.shelfOctaves)`. Shelf dB interpolates from minShelfDb to maxShelfDb as airAmount 0->1. Uses high shelf (NOT lowpass) -- design rule preserved. Perceptual quality requires listening. |
| 2 | User can hear difference between Warm/Neutral/Cold characters | ? HUMAN | Three presets verified at distinct values in AirSection.h lines 56-131: Warm (6kHz shelf, 5 allpass, +LF boost), Neutral (8kHz shelf, 4 allpass, no LF), Cold (10kHz shelf, 4 allpass, -LF cut). Character crossfade over 30ms via cheapEnergyCrossfade confirmed at AirSection.cpp:315-320. Perceptual distinction requires listening. |
| 3 | Higher Air values produce subtle transient softening (not just EQ) | ? HUMAN | Allpass cascade confirmed at AirSection.cpp:173-175: 4-5 stage allpass with Q scaling from allpassMinQ to allpassMaxQ as airAmount increases (Warm: Q 0.15->0.8, 5 stages). L/R 12% frequency offset (`sf * 1.12f`) for stereo decorrelation. Perceptual smearing quality requires listening. |
| 4 | Air setting automatically influences Diffuse Tail HF damping (more air = darker tail) | ✓ VERIFIED | PluginProcessor.cpp:162: `diffuseTailSection.setHFDamping(airAmount)`. DiffuseTailSection.h:25 declares `setHFDamping()`. Cross-stage link is unconditional and not multiplied by character scaling. |

**Score:** 1/4 truths fully verifiable by automated code analysis (Truth 4). Truths 1-3 have full code support verified but require human perceptual confirmation.

---

## Required Artifacts

### Plan 04-01 Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `Source/Parameters.h` | air_char at version 2 with 3 choices (Warm/Neutral/Cold), default Neutral | ✓ VERIFIED | Line 161: `ParameterID { ParamIDs::airChar, 2 }`, StringArray `{"Warm", "Neutral", "Cold"}`, default index 1 (Neutral) |
| `Source/dsp/AirSection.h` | AirCharacterPreset struct + kCharacterPresets[3] + complete class | ✓ VERIFIED | Lines 18-43: `AirCharacterPreset` struct. Line 56: `kCharacterPresets[kNumCharacters]`. Complete class with HF/LF shelf, allpass arrays, pending sets, SmoothedValues, crossfade state (lines 137-182). |
| `Source/dsp/AirSection.cpp` | Full DSP: shelf filters, allpass, bypass crossfade, character crossfade, Air smoothing | ✓ VERIFIED | 368 lines. `updateFilters()` at line 139 uses `highShelfDb`. Process loop with per-sample bypass blend (line 247), character crossfade via `cheapEnergyCrossfade` (line 320), coefficient throttle every 16 samples (line 257). |

### Plan 04-02 Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `Source/dsp/ReflectionsSection.h` | `setAirDarkening(float)` public method | ✓ VERIFIED | Line 44: `void setAirDarkening (float darkening);` -- private member `airDarkeningFactor = 0.0f` at line 82 |
| `Source/dsp/ReflectionsSection.cpp` | Air darkening integrated into baseCutoff | ✓ VERIFIED | Lines 239-241: `airCutoffReduction = airDarkeningFactor * 3000.0f; baseCutoff = 12000.0f - roomSize * 6000.0f - airCutoffReduction; baseCutoff = juce::jmax(2000.0f, baseCutoff)` |
| `Source/dsp/DiffuseTailSection.h` | `setCharacterDecayBias(float)` public method | ✓ VERIFIED | Line 25: `void setCharacterDecayBias (float bias);` -- private member `characterDecayBias = 0.0f` at line 92 |
| `Source/dsp/DiffuseTailSection.cpp` | Character bias applied to decay feedback gains | ✓ VERIFIED | Lines 174-177: `biasedDecayMs = decayMs * (1.0f + characterDecayBias); biasedDecayMs = juce::jlimit(50.0f, 2200.0f, biasedDecayMs)` -- `biasedDecayMs` feeds into `decaySec` used in feedback gain calculation |
| `Source/PluginProcessor.cpp` | Complete Air forwarding + cross-stage coupling in updateStageParams | ✓ VERIFIED | Lines 141-178: setAmount, setCharacter, setBypass, setHFDamping, setCharacterDecayBias (via kCharacterPresets lookup), setAirDarkening all present |

---

## Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `Source/dsp/AirSection.h` | `signalsmith-dsp/filters.h` | `BiquadStatic<float>` for shelf/allpass | ✓ WIRED | AirSection.h line 3: `#include <signalsmith-dsp/filters.h>`. BiquadStatic used at lines 153-166 (8 filter instances + 8 pending). |
| `Source/dsp/AirSection.cpp` | `Source/dsp/AirSection.h` | `kCharacterPresets` drives all filter coefficient calculations | ✓ WIRED | `kCharacterPresets[currentCharIndex]` at AirSection.cpp:141, `kCharacterPresets[pendingCharIndex]` at line 188, `kCharacterPresets[currentCharIndex].lfShelfFreqHz` at line 276, `kCharacterPresets[currentCharIndex].allpassStages` at line 283. |
| `Source/PluginProcessor.cpp` | `Source/dsp/AirSection.h` | `setAmount` and `setCharacter` in updateStageParams | ✓ WIRED | Lines 143-144: `airSection.setAmount(airAmount)`, `airSection.setCharacter(airCharIndex)`. Both called every processBlock via updateStageParams. |
| `Source/PluginProcessor.cpp` | `Source/dsp/ReflectionsSection.h` | `setAirDarkening` with character-scaled value | ✓ WIRED | Line 178: `reflectionsSection.setAirDarkening(airAmount * reflDarkeningScale)`. `reflDarkeningScale` resolved from `kCharacterPresets[airCharIndex].reflDarkeningScale` at line 170-171. |
| `Source/PluginProcessor.cpp` | `Source/dsp/DiffuseTailSection.h` | `setCharacterDecayBias` with air-scaled character bias | ✓ WIRED | Line 173: `diffuseTailSection.setCharacterDecayBias(tailDecayBias * airAmount)`. `tailDecayBias` from `kCharacterPresets[airCharIndex].tailDecayBias` at line 170. |

---

## Requirements Coverage

| Requirement | Source Plan | REQUIREMENTS.md Description | Status | Evidence |
|-------------|-------------|------------------------------|--------|----------|
| AIR-01 | 04-01, 04-02 | User can control HF absorption via Air knob (0-100%) | ✓ SATISFIED | HF shelf dB interpolates from minShelfDb (baseline) to maxShelfDb at Air=100% via `airSmoothed`. Parameter exists at Parameters.h:150, forwarded via `setAmount()`. |
| AIR-02 | 04-01, 04-02 | User can select air character: Warm or Neutral (original text) | ✓ SATISFIED (SUPERSEDED) | REQUIREMENTS.md text predates Phase 4 plan. Implementation delivers 3 choices (Warm/Neutral/Cold) with parameter version bump to 2. Traceability table marks AIR-02 Complete. Deliberate design evolution per research notes. |
| AIR-03 | 04-01 | Air stage includes subtle allpass filtering for phase smearing | ✓ SATISFIED | 4-6 stage allpass cascade per character. Q scales from allpassMinQ to allpassMaxQ as Air increases. `allpassQ()` called in both `updateFilters()` and `updatePendingFilters()`. |
| AIR-04 | 04-01 | Higher Air values produce subtle transient softening | ✓ SATISFIED | Allpass Q scales with Air Amount (e.g. Neutral: 0.1->0.6). Phase smearing increases with Air. Perceptual verification documented as approved in 04-02-SUMMARY. |
| AIR-05 | 04-01, 04-02 | User can bypass Air & Distance stage independently | ✓ SATISFIED | `bypassBlend` SmoothedValue (10ms ramp). `setBypass()` sets target. Bypass blend applied per-sample: `finalL = inL * (1.0f - blend) + outL * blend` at AirSection.cpp. Early exit when fully bypassed and not smoothing. |

**Orphaned requirements check:** All AIR-01 through AIR-05 are claimed by the Phase 4 plans (04-01 claims all 5; 04-02 claims AIR-01, AIR-02, AIR-05). No additional AIR requirements appear in REQUIREMENTS.md that are unmapped. No orphaned requirements found.

---

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `Source/dsp/ExcitationSection.cpp` | 15 | `TODO Phase 5: Implement multiband soft saturation` | Info | Phase 5 stub, expected -- not part of Phase 4 scope |
| `Source/dsp/RoomToneSection.cpp` | 15 | `TODO Phase 5: Implement shaped noise generation` | Info | Phase 5 stub, expected -- not part of Phase 4 scope |

No anti-patterns found in Phase 4 modified files:
- `Source/dsp/AirSection.h` / `Source/dsp/AirSection.cpp` -- no TODOs, no stub returns, no empty handlers
- `Source/dsp/ReflectionsSection.h` / `Source/dsp/ReflectionsSection.cpp` -- no TODOs in modified sections
- `Source/dsp/DiffuseTailSection.h` / `Source/dsp/DiffuseTailSection.cpp` -- no TODOs in modified sections
- `Source/PluginProcessor.cpp` -- no TODOs in Air forwarding section, all coupling paths implemented

---

## Build Verification

- **Build status:** PASSED -- `cmake --build build --config Debug` produces no errors. Targets built: `Aether`, `Aether_vst3_helper`, `Aether_VST3`.
- **Test target:** No `AetherTests` target exists yet (Catch2 compiled but test executable deferred). No regression to check.
- **Commits verified:** `867e06f` (feat: Air DSP) and `32c90df` (feat: cross-stage coupling) both present in git log.
- **No new commits** since initial verification at 2026-02-18T22:00:00Z.

## Zero-Coupling Baseline Verification

Plan 04-02 required that at Air 0%, all cross-stage coupling is zero:
- `airAmount * reflDarkeningScale = 0 * anything = 0` -- setAirDarkening(0) -- no cutoff reduction. VERIFIED at PluginProcessor.cpp:178.
- `tailDecayBias * airAmount = anything * 0 = 0` -- setCharacterDecayBias(0) -- no decay change. VERIFIED at PluginProcessor.cpp:173.
- `setHFDamping(0)` -- produces 12kHz (bright, no damping). VERIFIED at PluginProcessor.cpp:162.

---

## Human Verification Required

The following 6 points require loading Aether in a DAW. The 04-02-SUMMARY.md documents that these were approved during the Plan 02 human checkpoint. A verifier cannot confirm audio quality programmatically.

### 1. HF Rolloff Progression

**Test:** Slowly sweep III Air - Amount from 0% to 100% on a HF-rich source (cymbals, acoustic guitar, hi-hat)
**Expected:** Progressive HF softening proportional to Air Amount. At 100%, signal sounds like "next room over" -- noticeably softer but still intelligible. Must not sound like a blanket or lowpass tone knob (the difference is: a high shelf preserves harmonic structure above the transition, lowpass kills it).
**Why human:** Perceptual quality of HF rolloff shape cannot be confirmed by grep

### 2. Character Distinction

**Test:** Set Air Amount to 60-70%. Cycle through Warm, Neutral, and Cold characters. Listen for 30ms crossfade during switching.
**Expected:** Three audibly distinct signatures. Warm: gentle, carpeted-room feel with subtle +1.5dB LF warmth at 250Hz. Neutral: balanced, moderate. Cold: brighter, slightly thin (-1dB LF at 250Hz). Transitions are smooth (no clicks).
**Why human:** Character differentiation is perceptual. Crossfade audibility requires listening.

### 3. Transient Softening

**Test:** Use a percussive source (snare, click track). Compare Air at 0%, 50%, and 100%.
**Expected:** 0% -- nearly transparent (only -0.5 to -1.0dB baseline HF shelf). 50% -- subtle reduction in "immediacy" and "snap". 100% -- clear phase smearing from allpass cascade, signal feels more diffuse and "washy".
**Why human:** Allpass phase smearing is a transient artifact, not measurable by frequency analysis

### 4. Bypass A/B

**Test:** Rapidly toggle III Air - Bypass on/off at Air Amount 70-100%.
**Expected:** Clean 10ms crossfade -- no clicks, pops, or discontinuities. When bypassed, full brightness and immediacy return immediately.
**Why human:** Audio click detection requires listening

### 5. Cross-Stage Coupling Cohesion

**Test:** Set Air 80%, Warm character. Listen to tail and reflections. Switch to Cold. Then set Air to 0%.
**Expected:** Warm: reflections darker (reflDarkeningScale 1.2), tail slightly extended (tailDecayBias +0.15). Cold: reflections brighter (reflDarkeningScale 0.7), tail slightly tighter (tailDecayBias -0.1). Air 0%: all coupling collapses to zero -- reflections and tail return to their uncoupled defaults.
**Why human:** Subtle coupling changes require perceptual judgment across multiple listening passes

### 6. Automation Smoothness

**Test:** Record Air Amount automation moving from 0% to 100% over 2-4 bars.
**Expected:** Perfectly smooth sweep -- no zipper noise, steps, or clicking. The 25ms SmoothedValue ramp + every-16-sample coefficient update should produce inaudible coefficient transitions.
**Why human:** Zipper noise detection requires listening

---

## Summary

Re-verification confirms no regressions. The Phase 4 Air & Distance DSP is unchanged since the initial verification:

- `AirSection` is a complete 368-line DSP processor with HF high shelf (not lowpass), optional LF shelf per character, 4-6 stage allpass cascade, dual filter chains for 30ms character crossfade, 10ms bypass crossfade, and 25ms Air Amount smoothing.
- Three character presets (Warm/Neutral/Cold) have physics-derived values with distinct frequency targets, allpass counts, and cross-stage coupling factors. All values verified in `Source/dsp/AirSection.h` lines 56-131.
- All five AIR requirements are implemented. AIR-02 is superseded by a deliberate 3-choice upgrade.
- Cross-stage coupling (Air -> Reflections darkening, Air/Character -> Tail decay bias, Air -> Tail HF damping) is fully wired through `PluginProcessor::updateStageParams()`.
- Zero-coupling baseline at Air 0% is mathematically guaranteed by multiplying all cross-stage values by airAmount.
- Build compiles cleanly. Both plan commits (867e06f, 32c90df) verified in git log.

Goal achievement is blocked only on human audio verification. The 04-02-SUMMARY.md documents human approval of all 6 DAW verification points. The code delivers the correct architecture and DSP design for "sounds like it traveled through a real room" -- the perceptual quality of that effect can only be confirmed by ears.

---

_Verified: 2026-02-18T23:30:00Z_
_Verifier: Claude (gsd-verifier)_
_Mode: Re-verification (initial: 2026-02-18T22:00:00Z, status: human_needed, score: 4/4)_
