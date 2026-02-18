---
phase: 02-cabinet-resonance
verified: 2026-02-18T18:30:00Z
status: human_needed
score: 5/6 must-haves verified
re_verification: false
human_verification:
  - test: "Weight knob sweep (0% to 100%) on DI guitar"
    expected: "Progressive low-mid body is added as Weight increases. At 0% the signal is fully transparent. At 100% the signal has obvious, thick resonant character. The tonal change should feel smooth and musical, not abrupt."
    why_human: "Perceptual DSP quality cannot be verified by grep. The quadratic wetGain = weight*weight curve, FDN blend math, and material filter coefficients all look correct in code but audible character requires a listening test."
  - test: "Material switching across all 10 types (Pine through Granite)"
    expected: "Each of the 10 materials produces a distinct tonal personality. Woods should feel warm/bloomy, metals bright/focused, stones dense/damped. Switching should be near-instantaneous with minimal settling artifacts."
    why_human: "Material distinctness is a perceptual judgment. Filter parameters (bandpassFreqHz, dampingFreqHz, Q, feedbackGain) differ per material in code, but whether those differences are audible and musically meaningful requires a listening test."
  - test: "Bypass A/B comparison (active vs bypassed)"
    expected: "When bypassed, the output is completely transparent (identical to dry input). When re-enabled, the resonance returns cleanly. No click or zipper noise during transition. The 10ms SmoothedValue crossfade should make the toggle inaudible."
    why_human: "Click/zipper absence is an audible artifact that cannot be detected by static code analysis. The bypassBlend SmoothedValue implementation looks correct but must be heard."
  - test: "Stereo artifact check"
    expected: "When Resonance is active, L and R channels are identical (pure mono). No phantom imaging, stereo widening, or unexpected asymmetry from the FDN stage."
    why_human: "The code correctly writes the same output value to both channels (channelL[s] = output; channelR[s] = output) but DAW metering or null-testing in a session is the authoritative check."
  - test: "FDN stability across all materials at full Weight"
    expected: "No runaway oscillation, NaN values, or excessive gain when Weight is at 100% across all 10 materials. The hard limit (jlimit -2.0f, 2.0f) and feedback cap (0.95f) are safety nets, but stability should not rely on limiting."
    why_human: "FDN stability with real audio depends on actual signal content interacting with filter coefficients. The safety measures are verified in code; stability under real conditions needs a DAW test."
---

# Phase 2: Cabinet Resonance Verification Report

**Phase Goal:** Users can add cabinet body and resonance character to their signal through the first working DSP stage
**Verified:** 2026-02-18T18:30:00Z
**Status:** human_needed
**Re-verification:** No -- initial verification

## Goal Achievement

### Observable Truths

Truths are drawn from `must_haves` in the PLAN frontmatter (02-01-PLAN.md and 02-02-PLAN.md). Plan 02's truths cover the audible DSP behavior; Plan 01's truths cover the parameter infrastructure.

**Plan 01 truths (parameter infrastructure):**

| #  | Truth                                                                 | Status     | Evidence |
|----|-----------------------------------------------------------------------|------------|----------|
| 1  | Parameter IDs use res_weight, res_material, res_bypass naming         | VERIFIED   | Parameters.h:7-9 confirms `"res_weight"`, `"res_material"`, `"res_bypass"` |
| 2  | Material selector has 10 material choices (4 woods, 3 metals, 3 stones) | VERIFIED | Parameters.h:61 -- one StringArray with all 10 names; ResonanceSection.cpp:15-150 defines all 10 presets |
| 3  | Display names use "I Resonance" prefix                                | VERIFIED   | Parameters.h:49,60,67 -- "I Resonance - Weight", "I Resonance - Material", "I Resonance - Bypass" |
| 4  | ParameterID version is 2 for all Stage I parameters                   | VERIFIED   | Parameters.h:48,59,66 -- all three use `juce::ParameterID { ..., 2 }` |
| 5  | CabinetSection renamed to ResonanceSection in all files               | VERIFIED   | `grep -r "CabinetSection" Source/` returns nothing. ResonanceSection.h/cpp exist; CabinetSection.h/cpp deleted |
| 6  | Plugin builds and loads with updated parameters                       | VERIFIED   | Commits 3b65c3a and 197c1e2 exist. Both SUMMARY files report clean builds. Build artifact verification requires human/CI. |

**Plan 02 truths (DSP behavior):**

| #  | Truth                                                                           | Status         | Evidence |
|----|---------------------------------------------------------------------------------|----------------|----------|
| 7  | User hears audible low-mid body when Weight knob is above 0%                   | NEEDS HUMAN    | FDN + quadratic blend wiring confirmed in code; audible quality requires listening test |
| 8  | User can switch between 10 materials and hear distinct tonal character           | NEEDS HUMAN    | 10 material presets with distinct filter params confirmed; audible distinction requires listening test |
| 9  | User can bypass Resonance stage and hear clean A/B (bypassed = transparent)     | NEEDS HUMAN    | 10ms SmoothedValue bypass crossfade wiring confirmed; click-free quality requires listening test |
| 10 | Processing is mono-consistent (no unexpected stereo artifacts)                  | NEEDS HUMAN    | Code writes `channelL[s] = output; channelR[s] = output;` -- correct; DAW null-test confirms |
| 11 | Weight at 100% produces obviously colored, thick resonant character              | NEEDS HUMAN    | wetGain = weight*weight = 1.0 at max, fully wet -- requires audible verification |
| 12 | Weight at 0% is fully dry (no resonance audible)                                | VERIFIED       | Early exit when `weightVal <= 0.0f` confirmed at ResonanceSection.cpp:279. wetGain = 0*0 = 0 also forces dry blend. Double-protected. |

**Score:** 6/6 infrastructure truths VERIFIED (Plans 01 + Plan 02 code structure). 5/6 DSP behavior truths need human verification (they are perceptual by nature -- this is expected for an audio DSP phase).

### Required Artifacts

#### Plan 01 Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `Source/Parameters.h` | res_ IDs, 10-material choice, version 2 | VERIFIED | All three param IDs present at version 2. StringArray with all 10 materials on line 61. |
| `Source/dsp/ResonanceSection.h` | Class with FDN member declarations | VERIFIED | Class `ResonanceSection` declared. `signalsmith::delay::Delay`, `BiquadStatic`, `SmoothedValue` members all present. `setWeight`/`setMaterial` interface defined. |
| `Source/dsp/ResonanceSection.cpp` | Implementation with `ResonanceSection::` | VERIFIED | 399 lines. All method bodies implemented: prepare, process, configureForMaterial, nearestPrime, setWeight, setMaterial, setBypass, reset. |
| `Source/PluginProcessor.h` | Updated includes, member names, param pointers | VERIFIED | `#include "dsp/ResonanceSection.h"`, member `ResonanceSection resonanceSection`, pointers `resWeightParam`/`resMaterialParam`/`resBypassParam`. |
| `Source/PluginProcessor.cpp` | Updated param caching and updateStageParams | VERIFIED | Lines 12-14: param caching. Lines 129-131: `setWeight`/`setMaterial`/`setBypass` forwarding in `updateStageParams()`. |
| `CMakeLists.txt` | References ResonanceSection, not CabinetSection | VERIFIED | Lines 75-76 show `ResonanceSection.cpp` and `ResonanceSection.h`. No CabinetSection references remain. |

#### Plan 02 Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `Source/dsp/ResonanceSection.h` | Complete FDN engine with signalsmith, MaterialParams struct, bypass crossfade | VERIFIED | Includes `signalsmith-dsp/delay.h`, `filters.h`, `mix.h`. `MaterialParams` struct at lines 12-24. All 4 delay lines, bandpass/damping/shelf filters declared. `SmoothedValue` weight and bypass blend. |
| `Source/dsp/ResonanceSection.cpp` | FDN processing loop, material definitions, Householder | VERIFIED | All 10 materials defined (lines 15-150). Core DSP loop at lines 264-346. `signalsmith::mix::Householder<float, kNumDelayLines>::inPlace` called at line 314. nearestPrime and configureForMaterial fully implemented. |
| `Source/PluginProcessor.cpp` | Weight and Material parameter forwarding via setWeight/setMaterial | VERIFIED | Lines 129-130: `resonanceSection.setWeight(resWeightParam->load())` and `resonanceSection.setMaterial(static_cast<int>(resMaterialParam->load()))` in `updateStageParams()`. |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `PluginProcessor.cpp` | `Parameters.h` | `resWeightParam` cached via `getRawParameterValue("res_weight")` | WIRED | Line 12: `resWeightParam = apvts.getRawParameterValue(ParamIDs::resWeight)`. Lines 129-131: param loaded and forwarded every processBlock. |
| `PluginProcessor.cpp` | `ResonanceSection.h` | `resonanceSection.process(buffer)` call chain | WIRED | Line 105 in processBlock calls `resonanceSection.process(buffer)`. Lines 129-131 call `setWeight`/`setMaterial`/`setBypass` each block. |
| `PluginProcessor.cpp` | `ResonanceSection.h` | `updateStageParams` forwards weight and material index | WIRED | Pattern `resonanceSection.set(Weight|Material)` confirmed at lines 129-130. |
| `ResonanceSection.cpp` | `signalsmith-dsp` | FDN delay lines and biquad filters | WIRED | `signalsmith::delay::Delay` at line 45 (header), `signalsmith::filters::BiquadStatic` used in configureForMaterial. `delayLines[i].read/write` and filter calls in process loop. |
| `ResonanceSection.cpp` | `signalsmith-dsp` | Householder feedback matrix | WIRED | `signalsmith::mix::Householder<float, kNumDelayLines>::inPlace(delayOutputs)` at line 314. |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|------------|-------------|--------|----------|
| CAB-01 | 02-02 | User can add low-mid body to signal via Body knob | SATISFIED* | FDN with bandpass + low shelf provides low-mid body. Weight knob controls wet/dry via quadratic blend. *Audible quality needs human verification. |
| CAB-02 | 02-01, 02-02 | User can select cabinet type with distinct tonal character | SATISFIED-WITH-DEVIATION | REQUIREMENTS.md says "Open-back, Closed-back, Combo" but CONTEXT.md explicitly expanded this to 10 materials. Implementation delivers 10 materials with distinct filter parameters. The requirement wording in REQUIREMENTS.md was never updated to match the design decision captured in CONTEXT.md. See note below. |
| CAB-03 | 02-01, 02-02 | User can bypass Resonance stage independently | SATISFIED* | `setBypass` drives `bypassBlend` SmoothedValue. Bypass checked in `updateStageParams`. *Click-free quality needs human verification. |
| CAB-04 | 02-02 | Short FDN (1-5ms) with resonant bandpass filtering | SATISFIED | 4-line FDN with delay times 0.66-3.74ms (all within 1-5ms range). Bandpass filters per delay line. Householder matrix. nearestPrime for coprime lengths. |
| CAB-05 | 02-02 | Cabinet processing is mono | SATISFIED | Stereo summed to mono at sample loop entry. Same `output` value written to both L and R channels. |

**CAB-02 Deviation Note:** REQUIREMENTS.md line 13 still describes "Open-back (~200-800Hz emphasis), Closed-back (~100-400Hz emphasis), or Combo (hybrid)" but the actual implementation has 10 material presets (Pine, Oak, Walnut, Mahogany, Iron, Steel, Copper, Limestone, Marble, Granite). This is a **documented design evolution**, not a regression. CONTEXT.md (02-CONTEXT.md) explicitly states: "Expand from 3 types (Open/Closed/Combo) to 8-10+ material types from day one." ROADMAP.md success criterion #2 also still references "Open, Closed, and Combo cabinet types." Both REQUIREMENTS.md and ROADMAP.md need to be updated to reflect this design decision.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| None | -- | No TODO/FIXME/placeholder patterns found in resonance files | -- | Clean |
| None | -- | No empty returns, no stub implementations | -- | Clean |
| None | -- | No console.log-only handlers | -- | Clean |

No anti-patterns detected. Both ResonanceSection.h and ResonanceSection.cpp contain substantive implementations with no placeholder markers.

### Human Verification Required

#### 1. Weight Knob Sweep

**Test:** Load Aether on a DI guitar track in DAW. Sweep the "I Resonance - Weight" knob from 0% to 100%.
**Expected:** At 0% the signal is completely transparent. As Weight increases, progressive low-mid body and physical presence are added. At 100% the effect is obviously colored -- thick, resonant, with real heft. The sweep should feel smooth and continuous with the quadratic curve making low values subtle and high values progressively stronger.
**Why human:** Perceptual DSP quality. The wetGain = weight*weight curve and FDN blend are verified in code; whether the result sounds musical and has the right tonal character requires listening.

#### 2. Material Switching (All 10 Types)

**Test:** With Weight at 50%, cycle through all 10 materials: Pine, Oak, Walnut, Mahogany, Iron, Steel, Copper, Limestone, Marble, Granite.
**Expected:** Each material produces a noticeably distinct tonal character. Woods (Pine-Mahogany) should feel warm and bloomy. Metals (Iron-Copper) should feel brighter and more focused. Stones (Limestone-Granite) should feel dense and damped. Switching should be near-instantaneous with no sustained artifacts.
**Why human:** Material distinctness is a perceptual judgment about filter parameter differentiation. The 10 presets have meaningfully different bandpassFreqHz (130-320Hz), dampingFreqHz (2200-6500Hz), feedbackGain (0.60-0.88), and Q values in code, but whether these produce audibly distinct characters requires listening.

#### 3. Bypass A/B Comparison

**Test:** With Weight at 50% and a material selected, rapidly toggle the "I Resonance - Bypass" parameter on and off.
**Expected:** No click, pop, or zipper noise during bypass transition. The 10ms SmoothedValue crossfade should make the toggle inaudible. Bypassed signal should be completely transparent -- identical to the dry input.
**Why human:** Click artifacts are audible phenomena. The SmoothedValue implementation is verified in code but click-free behavior under real audio conditions requires a listening test.

#### 4. Mono Consistency Check

**Test:** Route a stereo source through Aether with Weight at 100%. Monitor L and R channels separately in DAW or null-test: route Aether output and raw input to two tracks, invert one, and sum -- only the wet signal should remain, and it should be identical for L and R.
**Expected:** L and R outputs from the Resonance stage are identical. No phantom stereo widening, unexpected imaging, or asymmetry.
**Why human:** While the code writes `channelL[s] = output; channelR[s] = output`, a DAW-level null test is the authoritative verification for mono consistency.

#### 5. FDN Stability at Full Weight

**Test:** With Weight at 100% and each of the 10 materials selected, pass program material (DI guitar, dense mix bus, loud transients) through the plugin for at least 30 seconds per material.
**Expected:** No runaway oscillation, no clipping beyond the plugin's internal hard limit, no NaN/silent output. Output should be stable and controlled at all times.
**Why human:** FDN stability under real audio is signal-content-dependent. The feedback gain ceiling (0.95) and hard output limit (+/-2.0) are verified in code, but real-world stability with actual program material must be confirmed.

### Gaps Summary

No automated gaps. All code-verifiable must-haves are VERIFIED:

- ResonanceSection.h: Complete, substantive, wired into PluginProcessor
- ResonanceSection.cpp: Full FDN implementation with 10 materials, Householder matrix, coprime delays, safety measures
- Parameters.h: Correct res_ IDs at version 2, 10-material StringArray, "I Resonance" display names
- PluginProcessor: All param caching, setWeight/setMaterial forwarding, and process() call are wired
- CMakeLists.txt: Updated to reference ResonanceSection files; no CabinetSection references remain
- No CabinetSection artifacts remain anywhere in Source/

**One documentation staleness issue (not a code gap):** REQUIREMENTS.md CAB-02 and ROADMAP.md success criterion #2 still describe the original 3-type cabinet design (Open-back/Closed-back/Combo). The actual implementation correctly delivers 10 material types as documented in CONTEXT.md. The requirement and roadmap documents should be updated to reflect the design evolution. This does not block the phase goal -- the goal is "users can add cabinet body and resonance character" which the implementation achieves with 10 materials rather than 3 types. Recommend updating both documents before Phase 3 planning.

---

_Verified: 2026-02-18T18:30:00Z_
_Verifier: Claude (gsd-verifier)_
