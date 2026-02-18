---
phase: 03-early-reflections-diffuse-tail
verified: 2026-02-18T19:45:00Z
status: passed
score: 12/12 must-haves verified
human_verification_resolved: "Product owner accepted 50-2000ms decay range as product evolution. REQUIREMENTS.md updated."
---

# Phase 3: Early Reflections & Diffuse Tail Verification Report

**Phase Goal:** Users hear convincing room spatial character -- early reflection patterns that define room shape and a diffuse reverb tail that completes the room impression
**Verified:** 2026-02-18T19:45:00Z
**Status:** human_needed
**Re-verification:** No -- initial verification

---

## Goal Achievement

### Observable Truths (from ROADMAP.md Success Criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|---------|
| 1 | User can hear room size change (small booth to large hall) by sweeping Room Size knob | VERIFIED | `roomSizeToDelayMs()` maps 0-1 to 1-30ms; SmoothedValue with 50ms ramp eliminates zipper noise; all taps scaled by `delayMs / 30.0f` per sample in `ReflectionsSection::process()` |
| 2 | User can hear reflection pattern change by sweeping Shape knob | VERIFIED | 7 `ShapePreset` entries in `kShapes[]` with distinct delay/gain/pan data; `setShape()` triggers 30ms crossfade via `pendingShapeIndex`; human DAW verification confirmed distinct character per shape |
| 3 | User can hear mic proximity shift (near-field to far-field) by sweeping Proximity knob | VERIFIED | Proximity blend: `directGain = 1.0f - proximity * (1.0f - kMinus12dB)`, `reflGain = kMinus18dB + proximity * (1.0f - kMinus18dB)`; human DAW verification confirmed audible direct/reflected ratio shift |
| 4 | Diffuse Tail adds smooth reverb decay without metallic ringing, Diffusion knob controls texture density | VERIFIED (CAVEAT) | 8-line Hadamard FDN with coprime prime delays; 4-stage allpass diffusion; feedback gains clamped to 0.9999f; human approved. CAVEAT: parameter range is 50-2000ms, not 50-500ms as specified in REQUIREMENTS.md -- see human verification below |
| 5 | Pre-delay of Diffuse Tail automatically tracks Room Size setting | VERIFIED | `diffuseTailSection.setPreDelay(roomSizeNorm)` called every `updateStageParams()`; `setPreDelay()` maps Room Size 0-1 to pre-delay 0.5-25ms via `erDelayMs * 0.7f` compressed curve |

**Score:** 5/5 observable truths verified (with one caveat requiring human decision)

---

## Required Artifacts (Plan 03-01, 03-02, 03-03 must_haves)

### Plan 03-01 Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `Source/Parameters.h` | `refl_width` param ID present | VERIFIED | Line 15: `inline constexpr auto reflWidth { "refl_width" };` |
| `Source/Parameters.h` | Shape as `AudioParameterChoice` with 7 Victorian names at version 3 | VERIFIED | Lines 98-104: `AudioParameterChoice` with `{ "The Parlour", "The Gallery", "The Chamber", "The Nave", "The Alcove", "The Crypt", "The Conservatory" }` at `ParameterID { ParamIDs::reflShape, 3 }` |
| `Source/Parameters.h` | `refl_width` float param at version 3 | VERIFIED | Lines 129-138: `AudioParameterFloat` for `reflWidth` at version 3, range 0-1, default 0.7 |
| `Source/Parameters.h` | `tail_decay` extended to 50-2000ms with skew 0.3 at version 3 | VERIFIED (CAVEAT) | Lines 216-228: `NormalisableRange<float>(50.0f, 2000.0f, 1.0f, 0.3f)`. Range is 2000ms, REQUIREMENTS.md specifies 500ms cap -- see human verification section |
| `Source/Parameters.h` | `refl_size` with skew 0.4 at version 3 | VERIFIED | Line 78: `NormalisableRange<float>(0.0f, 1.0f, 0.01f, 0.4f)` at version 3 |
| `Source/PluginProcessor.h` | `reflWidthParam` cached atomic pointer | VERIFIED | Line 58: `std::atomic<float>* reflWidthParam = nullptr;` |
| `Source/PluginProcessor.cpp` | `reflWidthParam` caching + full Stage II forwarding | VERIFIED | Line 20: `reflWidthParam = apvts.getRawParameterValue(ParamIDs::reflWidth)`; Lines 134-138: all 5 reflections params forwarded |
| `Source/dsp/ReflectionsSection.h` | `ShapePreset` struct present | VERIFIED | Lines 13-31: complete `ShapePreset` struct with all fields including `tailDiffusionBias`, `tailModalCharacter` |
| `Source/dsp/ReflectionsSection.cpp` | `kShapes` array with 7 presets | VERIFIED | Lines 11-152: `const ShapePreset ReflectionsSection::kShapes[7]` with all 7 room geometries |

### Plan 03-02 Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `Source/dsp/DiffuseTailSection.h` | `Hadamard` present (8-line FDN class) | VERIFIED | Lines 3-5: includes `mix.h`; line 27: `kNumFDNLines = 8`; `feedbackGains[kNumFDNLines]` member at line 63 |
| `Source/dsp/DiffuseTailSection.cpp` | `feedbackGains` computed from RT60 formula | VERIFIED | Lines 177-180: `feedbackGains[i] = jlimit(0.0f, 0.9999f, pow(10.0f, -3.0f * fdnDelaySamples[i] / (decaySec * sr)))` |

### Plan 03-03 Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `Source/PluginProcessor.cpp` | Cross-stage forwarding with `setPreDelay` | VERIFIED | Lines 151-161: all cross-stage links present -- Room Size -> `setPreDelay`, Air -> `setHFDamping`, Shape -> `setShapeInfluence` |

---

## Key Link Verification

### Plan 03-01 Key Links

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `PluginProcessor.cpp` | `ReflectionsSection.h` | `reflectionsSection.set*` calls in `updateStageParams` | VERIFIED | Lines 134-138: `setRoomSize`, `setShape`, `setProximity`, `setWidth`, `setBypass` all called every block |
| `ReflectionsSection.cpp` | `signalsmith-dsp/delay.h` | `delayLine*.read()` for tapped delay lines | VERIFIED | Lines 293-294: `delayLineL.read(lSamp)`, `delayLineR.read(rSamp)` inside per-tap loop |

### Plan 03-02 Key Links

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `DiffuseTailSection.cpp` | `signalsmith-dsp/mix.h` | `Hadamard<float, 8>::inPlace()` | VERIFIED | Line 295: `signalsmith::mix::Hadamard<float, 8>::inPlace(outputs)` in FDN feedback loop |
| `DiffuseTailSection.cpp` | `signalsmith-dsp/delay.h` | `fdnDelayLines[i].read()` | VERIFIED | Line 288: `outputs[i] = fdnDelayLines[i].read(fdnDelaySamples[i])` inside FDN loop |

### Plan 03-03 Key Links

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `PluginProcessor.cpp` | `DiffuseTailSection.h` | `setPreDelay/setHFDamping/setShapeInfluence/setDecay/setDiffusion` calls | VERIFIED | Lines 145-161: all setters called in `updateStageParams()` |
| `PluginProcessor.cpp` | `ReflectionsSection.h` | `getShapePreset()` for shape influence | NOTE | `getShapePreset()` is defined and accessible but the processor forwards shape influence via shape index directly to `setShapeInfluence(shapeIndex)`. `DiffuseTailSection` has its own internal lookup table `kShapeTailInfluence[]`. The plan key link was an architectural option; the implementation chose a cleaner decoupled approach via index pass-through. Both achieve the same result. This is NOT a gap. |

---

## Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|------------|-------------|--------|---------|
| REFL-01 | 03-01 | Room Size knob scales delay times 1-30ms | SATISFIED | `roomSizeToDelayMs()` maps 0-1 to 1-30ms; skew 0.4 in `NormalisableRange` provides weighted curve |
| REFL-02 | 03-01 | Shape knob with 7 discrete geometry presets | SATISFIED | `AudioParameterChoice` with 7 Victorian names; 7 `ShapePreset` entries in `kShapes[]`; 30ms crossfade on switch |
| REFL-03 | 03-01 | Proximity blends direct/reflected ratio with HF rolloff | SATISFIED | Direct gain fades from 0dB to -12dB; reflected gain rises from -18dB to 0dB across proximity range |
| REFL-04 | 03-01 | 8-16 individually filtered delay taps | SATISFIED | 16 taps total (8 per channel); each tap has individual `BiquadStatic<float>` lowpass filter updated per-sample |
| REFL-05 | 03-01 | L and R channels have different delay times and pan positions | SATISFIED | Each `ShapePreset` has independent `delayMsL[8]`/`delayMsR[8]` and `panL[8]`/`panR[8]`; width interpolation blends mono/stereo |
| REFL-06 | 03-01 | Independent bypass with smooth crossfade | SATISFIED | `bypassBlend` SmoothedValue with 10ms ramp; early exit when fully bypassed |
| TAIL-01 | 03-01/03-02 | Decay knob 50ms-500ms RT60 | PARTIAL - CAVEAT | Decay control wired and functional. RT60 formula correct. BUT range is 50-2000ms (plan extended it), REQUIREMENTS.md and ROADMAP specify 50-500ms cap as deliberate design constraint. See Human Verification. |
| TAIL-02 | 03-02 | Diffusion knob 0-100% controls texture density | SATISFIED | `diffusionSmoothed` 0-1 mapped to allpass coefficient 0.0-0.7; 4-stage cascaded allpass per channel |
| TAIL-03 | 03-03 | Pre-delay linked to Room Size | SATISFIED | `diffuseTailSection.setPreDelay(reflSizeParam->load())` in `updateStageParams()`; compressed 70% curve, 0.5-25ms range |
| TAIL-04 | 03-03 | HF damping linked to Air Amount | SATISFIED | `diffuseTailSection.setHFDamping(airAmountParam->load())` in `updateStageParams()`; logarithmic 12kHz->2kHz mapping; air_amount defaults to 0.4f so damping is non-zero by default |
| TAIL-05 | 03-02 | FDN architecture with stereo implementation | SATISFIED | 8-line Hadamard FDN; even/odd stereo distribution with Hadamard cross-coupling; coprime prime-quantized delay lengths 22-63ms |
| TAIL-06 | 03-02 | Independent bypass with smooth crossfade | SATISFIED | `bypassBlend` SmoothedValue with 10ms ramp in `DiffuseTailSection`; bypass is additive (wet path bypassed, dry always passes) |

**Orphaned requirements check:** All 12 requirement IDs (REFL-01 through REFL-06, TAIL-01 through TAIL-06) are claimed by plans 03-01, 03-02, and 03-03. No orphans.

---

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `PluginProcessor.cpp` | 56 | `return {}` in `getProgramName()` | Info | Standard JUCE idiom for empty String return. Not a stub -- expected behavior for a plugin with no program names. |

No blockers or warnings found. No TODO/FIXME/PLACEHOLDER comments in any phase 3 files.

---

## DSP Integrity Checks

The following implementation details were verified to confirm correct, non-stub DSP:

**ReflectionsSection:**
- Per-sample processing loop present (lines 225-396 in ReflectionsSection.cpp)
- `delayLineL.write(inputL)` before `delayLineL.read(lSamp)` -- correct write-before-read pattern
- Per-tap lowpass filter coefficients updated per-sample: `tapFiltersL[t].lowpass(cutL / sr)`
- Proximity blend uses precomputed constants (`kMinus12dB`, `kMinus18dB`) -- no per-sample `pow()` call
- Shape crossfade runs both tap patterns simultaneously during 30ms transition

**DiffuseTailSection:**
- All 4 allpass diffusion stages cascade correctly: `sigL = outL` after each stage
- Hadamard applied to the `outputs[]` array AFTER damping, BEFORE feedback write -- correct order
- Tail is additive: `outputL = inputL + wetL` (line 324) -- not replacing input
- Safety limiting: `jlimit(-4.0f, 4.0f, wetL)` at line 316
- NaN debug assertions: `jassert(!std::isnan(wetL))` at line 320

**Build verification:**
```
[ 19%] Built target Aether
[ 20%] Built target Aether_vst3_helper
[ 29%] Built target Aether_VST3
[100%] Built target Catch2WithMain
```
All targets built without errors.

**Commits verified:**
- `fe826ea` -- feat(03-01): update parameters for Phase 3 and add reflections method stubs
- `ee0bf07` -- feat(03-01): implement ReflectionsSection with stereo TDL and 7 shape presets
- `87c2a74` -- feat(03-02): implement DiffuseTailSection with 8-line Hadamard FDN
- `3cac9f0` -- feat(03-03): wire cross-stage parameter linking and tail forwarding

---

## Human Verification Required

### 1. Confirm 2000ms Decay Range Extension (TAIL-01 Spec Divergence)

**Test:** Review the tail_decay parameter range decision.

**What to verify:**
- REQUIREMENTS.md line 147 (Out-of-Scope section) explicitly states: `Long reverb (>500ms decay) | Aether is environment simulator, not creative reverb. 500ms cap is deliberate constraint.`
- ROADMAP.md Phase 3 Success Criterion #4 states: `Diffuse Tail adds smooth reverb decay (50-500ms)...`
- REQUIREMENTS.md TAIL-01 states: `User can control reverb decay time via Decay knob (50ms-500ms RT60)`
- The 03-01-PLAN.md extended `tail_decay` to `NormalisableRange<float>(50.0f, 2000.0f, 1.0f, 0.3f)` (version 3)
- The implementation and human DAW verification both use this 2000ms range

**Decision required:** Accept this as a product decision (update REQUIREMENTS.md to reflect the 2000ms range and remove the Out-of-Scope cap) OR revert `tail_decay` back to `NormalisableRange<float>(50.0f, 500.0f, 1.0f, 0.3f)`.

**Why human:** This is a product scope decision -- whether "environment simulator" product identity allows 2000ms reverb tails. The DSP implementation is functionally correct either way. Only a product owner can resolve the intentional-vs-accidental nature of this spec divergence.

**Note:** The 03-03 human DAW verification was marked "approved" with all 10 verification points passing, including Decay sweep testing (50ms to 2000ms). The user described results as "really solid" with "a ton of useable territory."

---

## Gaps Summary

No functional gaps were found. All DSP is substantive and wired. The one open item is a product-scope decision about the decay range extension (50-500ms per spec vs. 50-2000ms as implemented), which requires human judgment rather than code change. The implementation is otherwise complete and fully functional.

---

_Verified: 2026-02-18T19:45:00Z_
_Verifier: Claude (gsd-verifier)_
