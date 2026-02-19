---
phase: quick-1
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - Source/presets/FactoryPresets.h
  - Source/ui/PresetSelector.h
  - Source/ui/PresetSelector.cpp
  - Source/PluginEditor.h
  - Source/PluginEditor.cpp
  - CMakeLists.txt
autonomous: true
requirements: [PRST-01, PRST-02, PRST-03]

must_haves:
  truths:
    - "User can select any of 6 factory presets from the dropdown and hear a distinct, musically useful room environment"
    - "Selecting a preset immediately changes all DSP parameters to that preset's values"
    - "Preset names display with Roman numeral naming (I. -- Tight Booth, etc.)"
    - "Default (no preset) resets all parameters to their APVTS defaults"
  artifacts:
    - path: "Source/presets/FactoryPresets.h"
      provides: "Preset data definitions with parameter values for all 6 presets"
      contains: "FactoryPresets"
    - path: "Source/ui/PresetSelector.h"
      provides: "PresetSelector with APVTS reference and onChange callback"
    - path: "Source/ui/PresetSelector.cpp"
      provides: "Preset application logic that sets all 19 parameters via APVTS"
  key_links:
    - from: "Source/ui/PresetSelector.cpp"
      to: "Source/presets/FactoryPresets.h"
      via: "include and array access"
      pattern: "FactoryPresets"
    - from: "Source/ui/PresetSelector.cpp"
      to: "APVTS parameters"
      via: "getParameter()->setValueNotifyingHost"
      pattern: "setValueNotifyingHost"
    - from: "Source/PluginEditor.cpp"
      to: "Source/ui/PresetSelector.cpp"
      via: "passes APVTS reference to PresetSelector constructor"
      pattern: "presetSelector.*apvts"
---

<objective>
Implement 6 factory presets and wire the existing PresetSelector dropdown to apply them.

Purpose: Users need musically useful starting points for different room environments. The PresetSelector UI already exists (Phase 6) but is a placeholder -- selecting an item does nothing. This plan creates a FactoryPresets data header and wires the combo box onChange to apply all DSP parameters via APVTS.

Output: Working preset selector that loads 6 distinct room environments. New Source/presets/FactoryPresets.h data file.
</objective>

<execution_context>
@/Users/nathanmcmillan/.claude/get-shit-done/workflows/execute-plan.md
@/Users/nathanmcmillan/.claude/get-shit-done/templates/summary.md
</execution_context>

<context>
@Source/Parameters.h
@Source/ui/PresetSelector.h
@Source/ui/PresetSelector.cpp
@Source/PluginEditor.h
@Source/PluginEditor.cpp
@CMakeLists.txt
</context>

<tasks>

<task type="auto">
  <name>Task 1: Create FactoryPresets data header with 6 preset definitions</name>
  <files>Source/presets/FactoryPresets.h, CMakeLists.txt</files>
  <action>
Create a new directory `Source/presets/` and header-only file `Source/presets/FactoryPresets.h` that defines the 6 factory presets as a constexpr data structure.

Define a `PresetData` struct with:
- `const char* name` -- display name
- Float fields for each continuous parameter (raw values, not normalised)
- Int fields for each choice parameter (index into choice array)

The struct needs fields for all 16 non-bypass parameters: resWeight, resMaterial, reflSize, reflShape, reflProx, reflWidth, airAmount, airChar, excitDrive, toneAmb, toneGate, tailDecay, tailDiff, outMix, outLevel. Bypass params are always set to false (all stages active) when applying a preset.

The 6 presets with their parameter values:

**I. Tight Booth** -- Small iso booth, dead and dry. Minimal reflections, tight and focused.
- res_weight: 0.35, res_material: 0 (Pine -- light wood, small cab)
- refl_size: 0.10 (Small), refl_shape: 4 (The Alcove), refl_prox: 0.15 (Near), refl_width: 0.30
- air_amount: 0.15, air_char: 0 (Warm)
- excit_drive: 0.10
- tone_amb: 0.05, tone_gate: 1 (Signal-Gated)
- tail_decay: 50.0, tail_diff: 0.40
- out_mix: 0.55, out_level: 0.0

**II. Live Room** -- Professional studio live room. Balanced and musical "default" preset.
- res_weight: 0.50, res_material: 3 (Mahogany -- warm, musical)
- refl_size: 0.40 (Medium), refl_shape: 2 (The Chamber), refl_prox: 0.35 (Nr-Mid), refl_width: 0.65
- air_amount: 0.40, air_char: 1 (Neutral)
- excit_drive: 0.25
- tone_amb: 0.10, tone_gate: 1 (Signal-Gated)
- tail_decay: 150.0, tail_diff: 0.60
- out_mix: 0.70, out_level: 0.0

**III. Recording Studio** -- Controlled acoustic environment, balanced absorption, moderate diffusion.
- res_weight: 0.45, res_material: 1 (Oak -- balanced wood)
- refl_size: 0.35 (Sm-Med), refl_shape: 0 (The Parlour), refl_prox: 0.30 (Nr-Mid), refl_width: 0.55
- air_amount: 0.30, air_char: 1 (Neutral)
- excit_drive: 0.15
- tone_amb: 0.08, tone_gate: 1 (Signal-Gated)
- tail_decay: 120.0, tail_diff: 0.55
- out_mix: 0.65, out_level: 0.0

**IV. Concert Hall** -- Large performance space, spacious and enveloping.
- res_weight: 0.55, res_material: 2 (Walnut -- rich resonance)
- refl_size: 0.80 (Med-Lg), refl_shape: 3 (The Nave), refl_prox: 0.70 (Mid-Far), refl_width: 0.85
- air_amount: 0.60, air_char: 1 (Neutral)
- excit_drive: 0.30
- tone_amb: 0.15, tone_gate: 0 (Always On)
- tail_decay: 600.0, tail_diff: 0.75
- out_mix: 0.70, out_level: 0.0

**V. Church Hall** -- Stone walls, high ceiling, bright reflections.
- res_weight: 0.60, res_material: 7 (Limestone -- stone, bright)
- refl_size: 0.75 (Med-Lg), refl_shape: 5 (The Crypt), refl_prox: 0.65 (Mid-Far), refl_width: 0.80
- air_amount: 0.55, air_char: 2 (Cold)
- excit_drive: 0.20
- tone_amb: 0.12, tone_gate: 0 (Always On)
- tail_decay: 500.0, tail_diff: 0.70
- out_mix: 0.70, out_level: 0.0

**VI. Cathedral** -- Massive stone space, long decay, maximum grandeur.
- res_weight: 0.65, res_material: 8 (Marble -- dense stone, long resonance)
- refl_size: 0.95 (Large), refl_shape: 3 (The Nave), refl_prox: 0.85 (Far), refl_width: 0.95
- air_amount: 0.70, air_char: 2 (Cold)
- excit_drive: 0.35
- tone_amb: 0.20, tone_gate: 0 (Always On)
- tail_decay: 1200.0, tail_diff: 0.85
- out_mix: 0.65, out_level: 0.0

Store as `static constexpr std::array<PresetData, 6> kFactoryPresets`. Include `static constexpr int kNumPresets = 6`.

Important: For tail_decay, store the ACTUAL value in ms (50.0, 150.0, etc.) not normalised -- the application code will convert using `param->convertTo0to1(rawValue)` before calling `setValueNotifyingHost`.

Also update CMakeLists.txt: add `Source/presets/FactoryPresets.h` to the `target_sources(Aether PRIVATE ...)` list, after the existing `Source/Parameters.h` line.
  </action>
  <verify>Build compiles: `cmake --build build --target Aether 2>&1 | tail -20` (the header is included in CMakeLists but not yet used -- just validates syntax)</verify>
  <done>FactoryPresets.h exists with 6 complete preset definitions, each specifying all 16 non-bypass parameter values. CMakeLists.txt updated.</done>
</task>

<task type="auto">
  <name>Task 2: Wire PresetSelector to apply presets via APVTS</name>
  <files>Source/ui/PresetSelector.h, Source/ui/PresetSelector.cpp, Source/PluginEditor.h, Source/PluginEditor.cpp</files>
  <action>
**PresetSelector.h** -- Update the class:
1. Constructor takes APVTS reference: `explicit PresetSelector (juce::AudioProcessorValueTreeState& apvts);`
2. Add private member: `juce::AudioProcessorValueTreeState& apvts;`
3. Add private method: `void applyPreset (int presetIndex);`

**PresetSelector.cpp** -- Update implementation:
1. Constructor accepts and stores APVTS reference in initializer list
2. Replace placeholder item names with final preset names using em-dash (Unicode \u2014):
   - Item 1: "-- Default --"
   - Item 2: "I. \u2014 Tight Booth"
   - Item 3: "II. \u2014 Live Room"
   - Item 4: "III. \u2014 Recording Studio"
   - Item 5: "IV. \u2014 Concert Hall"
   - Item 6: "V. \u2014 Church Hall"
   - Item 7: "VI. \u2014 Cathedral"
3. Add `comboBox.onChange` lambda in constructor:
   - Get selected ID (1-7)
   - If ID == 1 ("Default"): iterate all parameters in the APVTS tree using `apvts.processor.getParameters()`, for each `RangedAudioParameter*`, call `param->setValueNotifyingHost(param->getDefaultValue())`
   - If ID 2-7: call `applyPreset(selectedId - 2)` (maps to index 0-5)
4. Implement `applyPreset(int index)`:
   - `#include "../presets/FactoryPresets.h"` at top of .cpp
   - Access `kFactoryPresets[index]`
   - Helper lambda to set a parameter: get `RangedAudioParameter*` via `apvts.getParameter(paramId)`, call `param->beginChangeGesture()`, `param->setValueNotifyingHost(param->convertTo0to1(rawValue))`, `param->endChangeGesture()`
   - Apply all 15 continuous/choice values from the preset struct using this helper
   - Apply all 6 bypass parameters: set to `0.0f` (false/not bypassed) -- bypass bool params have 0.0=false, 1.0=true normalised
   - Note: choice params raw value is the float cast of the index (e.g., material index 3 -> raw value 3.0f), convertTo0to1 handles it

**PluginEditor.h** -- Change `PresetSelector presetSelector;` member declaration. Since PresetSelector now requires an APVTS reference in its constructor, it cannot be default-constructed. The member will be initialized in the AetherEditor constructor's initializer list.

**PluginEditor.cpp** -- In the AetherEditor constructor's member initializer list, add `presetSelector (processorRef.apvts)` before any member that comes after it in declaration order (check PluginEditor.h declaration order). The initializer list order must match declaration order to avoid warnings.
  </action>
  <verify>
1. Build: `cmake --build build --target Aether 2>&1 | tail -30` -- must compile without errors or warnings
2. Functional test: launch Standalone (`open ./build/Aether_artefacts/Standalone/Aether.app`), select "VI. -- Cathedral" from dropdown, verify knobs visually jump to new positions (Room Size near max, Decay long). Select "I. -- Tight Booth", verify knobs move to small/tight values. Select "-- Default --", verify knobs return to original defaults.
  </verify>
  <done>Selecting any preset from the dropdown immediately applies all DSP parameter values and knobs visually update. Selecting "Default" resets to APVTS defaults. All 6 presets produce distinct room environments. Build succeeds.</done>
</task>

</tasks>

<verification>
1. Build succeeds: `cmake --build build --target Aether` completes without errors
2. All 6 presets selectable from dropdown, each applies distinct parameter values
3. "Default" option resets all parameters to their APVTS default values
4. Knobs/toggles visually update when a preset is selected (APVTS listener propagation)
5. Parameter changes from preset selection are audible in audio output
</verification>

<success_criteria>
- 6 factory presets defined with musically appropriate parameter values for each room type
- PresetSelector dropdown wired to APVTS -- selecting a preset sets all 19 parameters (16 values + 6 bypasses off, minus out_level shared)
- "Default" option restores original parameter defaults
- Plugin builds and runs without errors
- Each preset sounds distinctly different (Tight Booth is small/dry, Cathedral is massive/reverberant)
</success_criteria>

<output>
After completion, create `.planning/quick/1-implement-6-factory-presets-and-wire-pre/1-SUMMARY.md`
</output>
