---
phase: 06-user-interface
plan: 02
subsystem: ui
tags: [juce, knob, toggle, bypass-button, slider-attachment, combobox-attachment, button-attachment, animation, velocity-mode]

requires:
  - phase: 06-user-interface
    plan: 01
    provides: AetherLookAndFeel with drawRotarySlider, AetherColours palette, embedded fonts
provides:
  - AetherKnob component with full interaction behavior (drag, scroll, double-click, click-to-type, animation)
  - AetherToggle multi-option component with hidden ComboBox attachment bridge
  - AetherBypassButton In/Out toggle with ButtonAttachment
affects: [06-03, 06-04]

tech-stack:
  added: []
  patterns: [velocity-mode drag sensitivity, hidden ComboBox attachment bridge for AudioParameterChoice, isBypass component property, exponential smoothing animation, GlyphArrangement char-by-char spaced label drawing]

key-files:
  created:
    - Source/ui/AetherKnob.h
    - Source/ui/AetherKnob.cpp
    - Source/ui/AetherToggle.h
    - Source/ui/AetherToggle.cpp
    - Source/ui/AetherBypassButton.h
    - Source/ui/AetherBypassButton.cpp
  modified:
    - CMakeLists.txt

key-decisions:
  - "GlyphArrangement for spaced label drawing instead of deprecated getStringWidthFloat -- future-proofs against JUCE API removal"
  - "Slider alpha set to 0.0 (invisible) with all rendering in paint() using displayValue -- prevents visual desync between animation and JUCE's built-in slider drawing"
  - "Rich value formatter receives both normalised value and parameter text -- enables combined displays like '62% (Medium)' for Room Size"

patterns-established:
  - "attachToParameter(APVTS&, paramID) pattern on all custom controls -- consistent API across AetherKnob, AetherToggle, AetherBypassButton"
  - "setBypassed(bool) on knob/toggle draws 40% parchment overlay; controls remain interactive (LOCKED DECISION)"
  - "Hidden ComboBox bridge: AetherToggle owns invisible ComboBox + ComboBoxAttachment to sync TextButton group with AudioParameterChoice"
  - "Animation: displayValue/targetValue split with 60Hz timer exponential smoothing (0.15 factor) for visual-only animation; parameter value jumps immediately"

requirements-completed: [UI-03, UI-04, UI-05]

duration: 4min
completed: 2026-02-18
---

# Phase 6 Plan 2: Custom Control Components Summary

**Three reusable UI components (AetherKnob, AetherToggle, AetherBypassButton) with velocity-mode drag, click-to-type editing, animated indicator sweep, and APVTS attachment patterns for all 19 parameters**

## Performance

- **Duration:** 4 min
- **Started:** 2026-02-19T00:35:16Z
- **Completed:** 2026-02-19T00:39:32Z
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments
- AetherKnob implements all locked interaction decisions: ~300px drag sensitivity, shift-drag 10x fine control, scroll wheel, double-click to parameter default, click-to-type value editing, smooth animated indicator sweep, real-time value readout, bypassed visual dimming
- AetherToggle displays N mutually exclusive options with ink-inverted active state, synced to AudioParameterChoice via hidden ComboBox + ComboBoxAttachment bridge pattern
- AetherBypassButton toggles In/Out with opacity states, synced to AudioParameterBool via ButtonAttachment, uses isBypass property for distinct LookAndFeel styling
- All 6 new source files compile cleanly with zero warnings

## Task Commits

Each task was committed atomically:

1. **Task 1: Implement AetherKnob with full interaction behavior** - `36e74db` (feat)
2. **Task 2: Implement AetherToggle and AetherBypassButton** - `1bca8c6` (feat)

## Files Created/Modified
- `Source/ui/AetherKnob.h` - Custom knob component declaration with attachToParameter, setBypassed, setValueFormatter
- `Source/ui/AetherKnob.cpp` - 375-line implementation: velocity drag, scroll, double-click reset, click-to-type, 60Hz animation, GlyphArrangement spaced labels
- `Source/ui/AetherToggle.h` - Multi-option toggle switch declaration with attachToParameter, setBypassed
- `Source/ui/AetherToggle.cpp` - 110-line implementation: hidden ComboBox bridge, button state sync, bypassed overlay
- `Source/ui/AetherBypassButton.h` - Bypass button declaration with attachToParameter
- `Source/ui/AetherBypassButton.cpp` - 50-line implementation: In/Out text switching via ButtonAttachment state
- `CMakeLists.txt` - Added 6 new source files to target_sources

## Decisions Made
- Used GlyphArrangement for char-by-char spaced label measurement instead of deprecated Font::getStringWidthFloat -- JUCE 8 marks this API deprecated with recommendation to use GlyphArrangement or TextLayout
- Made the Slider completely invisible (alpha 0.0) rather than just hiding the text box -- ensures no visual artifacts from JUCE's built-in slider rendering while paint() draws the animated indicator at displayValue position
- Rich value formatter signature takes both normalised value (0-1) and parameter's text representation -- enables composite displays like "62% (Medium)" for Room Size parameter that has text labels

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Replaced deprecated getStringWidthFloat with GlyphArrangement**
- **Found during:** Task 1 (AetherKnob spaced label drawing)
- **Issue:** JUCE 8 deprecates Font::getStringWidthFloat, generating compiler warnings and risk of removal in future JUCE versions
- **Fix:** Created GlyphArrangement-based measureChar lambda for per-character width measurement
- **Files modified:** Source/ui/AetherKnob.cpp (drawSpacedLabel method)
- **Verification:** Build compiles with zero warnings
- **Committed in:** 36e74db (Task 1 commit)

---

**Total deviations:** 1 auto-fixed (1 bug)
**Impact on plan:** Trivial API modernization. No scope impact.

## Issues Encountered
None.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- All three custom control types ready for Plan 03 (Section Panel) and Plan 04 (Layout) to instantiate
- attachToParameter API consistent across all controls -- layout code calls the same pattern for knobs, toggles, and bypass buttons
- setBypassed API ready for section bypass state propagation
- AetherKnob's setValueFormatter enables per-parameter display customization (Room Size, Decay, Output Level)

## Self-Check: PASSED

All 6 created files verified on disk. Both task commits (36e74db, 1bca8c6) found in git log.

---
*Phase: 06-user-interface*
*Completed: 2026-02-18*
