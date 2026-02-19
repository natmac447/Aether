---
phase: quick-3
plan: 01
subsystem: ui
tags: [juce, toggle-button, combo-box, preset-manager, xml]

# Dependency graph
requires:
  - phase: 06-user-interface
    provides: "PresetSelector, AetherBypassButton pattern, AetherColours, PluginEditor layout"
  - phase: quick-1
    provides: "Factory presets wired into PresetSelector"
provides:
  - "MixLockButton: padlock toggle protecting mix value during preset changes"
  - "ArrowStepButton: combo box step navigation with wrap-around"
  - "UserPresetManager: XML-based user preset save/load/list/delete"
  - "PresetSelector extended with user presets, save button, mix lock support"
affects: []

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "UI-only toggle state (not APVTS parameter) for mix lock"
    - "Static utility class for file-based preset management"
    - "AlertWindow async modal for user input"

key-files:
  created:
    - Source/ui/MixLockButton.h
    - Source/ui/MixLockButton.cpp
    - Source/ui/ArrowStepButton.h
    - Source/ui/ArrowStepButton.cpp
    - Source/presets/UserPresetManager.h
    - Source/presets/UserPresetManager.cpp
  modified:
    - Source/ui/PresetSelector.h
    - Source/ui/PresetSelector.cpp
    - Source/PluginEditor.h
    - Source/PluginEditor.cpp
    - CMakeLists.txt

key-decisions:
  - "Mix lock is purely UI state (not an APVTS parameter) -- no need to persist lock across sessions"
  - "ArrowStepButton navigates by item index (not ID) to handle non-contiguous IDs after separators"
  - "User preset IDs start at 100 to avoid collision with factory preset IDs 1-7"
  - "User presets stored at ~/Library/Audio/Presets/Cairn/Aether/ following macOS audio preset conventions"

patterns-established:
  - "MixLockButton pointer injection: PresetSelector checks lock via setMixLockButton() before applying parameters"
  - "rebuildPresetList() pattern: clear + factory + separator + user, callable after save/delete"

requirements-completed: [QUICK-3a, QUICK-3b, QUICK-3c]

# Metrics
duration: 5min
completed: 2026-02-19
---

# Quick Task 3: Mix Lock, Arrow Steps, User Presets Summary

**Padlock mix-lock toggle, combo box arrow step buttons with wrap-around, and XML user preset save/load via ~/Library/Audio/Presets/Cairn/Aether/**

## Performance

- **Duration:** 5 min
- **Started:** 2026-02-19T17:42:49Z
- **Completed:** 2026-02-19T17:47:31Z
- **Tasks:** 3
- **Files modified:** 11

## Accomplishments
- MixLockButton draws a padlock icon (closed/open) in the parchment aesthetic, preserves mix value during any preset load
- ArrowStepButton draws filled triangles for stepping through all three combo boxes (Material, Shape, Preset) with wrap-around at boundaries
- UserPresetManager saves APVTS state as XML files to the standard macOS audio presets directory
- PresetSelector extended with save button (AlertWindow dialog), user presets below separator, and mix lock support for all preset types (Default, factory, user)

## Task Commits

Each task was committed atomically:

1. **Task 1: Mix Lock Button + ComboBox Arrow Step Buttons** - `f145f17` (feat)
2. **Task 2: User Preset Save/Load Manager** - `6d9face` (feat)
3. **Task 3: Wire User Presets into PresetSelector UI** - included in `f145f17` (merged into Task 1 due to shared files)

## Files Created/Modified
- `Source/ui/MixLockButton.h` - Padlock toggle button component (20x20px, programmatic drawing)
- `Source/ui/MixLockButton.cpp` - Padlock icon rendering (closed/open shackle, keyhole dot)
- `Source/ui/ArrowStepButton.h` - Triangle step button for ComboBox navigation (14x12px)
- `Source/ui/ArrowStepButton.cpp` - Arrow drawing and click handler with index-based wrap-around
- `Source/presets/UserPresetManager.h` - Static utility: getPresetDirectory, savePreset, loadPreset, getAvailablePresets, deletePreset
- `Source/presets/UserPresetManager.cpp` - XML serialisation, filename sanitisation, directory creation
- `Source/ui/PresetSelector.h` - Added getComboBox(), setMixLockButton(), rebuildPresetList(), save button, user preset members
- `Source/ui/PresetSelector.cpp` - Extended with user preset loading, mix lock in all paths (Default/factory/user), save dialog
- `Source/PluginEditor.h` - Added MixLockButton, 6 ArrowStepButton members
- `Source/PluginEditor.cpp` - Initializer list, addAndMakeVisible, resized() positioning for all new buttons
- `CMakeLists.txt` - Added 6 new source files to target_sources

## Decisions Made
- Mix lock is UI-only state (not APVTS parameter) -- no persistence needed, follows AetherBypassButton pattern of simple component
- ArrowStepButton steps by index rather than ID to correctly handle the separator gap between factory (1-7) and user (100+) preset IDs
- User preset ID offset is 100 (kUserPresetIdOffset) to give room for future factory preset expansion
- User presets stored at ~/Library/Audio/Presets/Cairn/Aether/ per macOS audio preset conventions

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Task 3 work merged into Task 1 commit**
- **Found during:** Task 1 (Mix Lock + Arrows)
- **Issue:** PresetSelector.cpp includes UserPresetManager.h and uses user preset logic. Tasks 1 and 3 modify the same files (PresetSelector.h/cpp, PluginEditor.h/cpp). Cannot compile Task 1 changes without Task 3 wiring.
- **Fix:** Implemented all PresetSelector changes (user presets, save button, mix lock, rebuildPresetList) in Task 1 along with MixLockButton and ArrowStepButton.
- **Files modified:** All PresetSelector and PluginEditor files
- **Verification:** Build succeeds with no errors
- **Committed in:** f145f17 (Task 1 commit)

---

**Total deviations:** 1 auto-fixed (1 blocking)
**Impact on plan:** Task ordering merged for compile-ability. All planned functionality delivered, just in 2 commits instead of 3.

## Issues Encountered
None

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- All three features (mix lock, arrows, user presets) are fully wired and functional
- Plugin builds and installs correctly as VST3 and Standalone
- Ready for Phase 7 Plan 02 (visualization editor integration) or further quick tasks

## Self-Check: PASSED

All 6 created files verified present. Both task commits (f145f17, 6d9face) verified in git log.

---
*Quick Task: 3*
*Completed: 2026-02-19*
