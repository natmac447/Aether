---
phase: quick-3
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - Source/ui/MixLockButton.h
  - Source/ui/MixLockButton.cpp
  - Source/ui/ArrowStepButton.h
  - Source/ui/ArrowStepButton.cpp
  - Source/presets/UserPresetManager.h
  - Source/presets/UserPresetManager.cpp
  - Source/ui/PresetSelector.h
  - Source/ui/PresetSelector.cpp
  - Source/PluginEditor.h
  - Source/PluginEditor.cpp
  - Source/PluginProcessor.h
  - Source/PluginProcessor.cpp
autonomous: true
requirements: [QUICK-3a, QUICK-3b, QUICK-3c]

must_haves:
  truths:
    - "Mix knob value is preserved when a preset is loaded while mix lock is engaged"
    - "Mix lock button toggles visually between locked and unlocked states"
    - "Up/down arrow buttons on all three combo boxes step through items without opening the popup"
    - "Arrow buttons wrap around at list boundaries (last->first, first->last)"
    - "User can save current parameter state to a named preset file"
    - "User presets appear in the preset selector alongside factory presets"
    - "User presets load correctly and restore all parameters"
  artifacts:
    - path: "Source/ui/MixLockButton.h"
      provides: "Lock toggle button component"
    - path: "Source/ui/ArrowStepButton.h"
      provides: "Up/down arrow step button for combo boxes"
    - path: "Source/presets/UserPresetManager.h"
      provides: "Save/load/list user presets as XML files"
    - path: "Source/ui/PresetSelector.h"
      provides: "Extended with user preset entries and save button"
  key_links:
    - from: "Source/ui/MixLockButton.h"
      to: "Source/ui/PresetSelector.cpp"
      via: "PresetSelector checks mixLock state before setting out_mix"
      pattern: "mixLocked"
    - from: "Source/presets/UserPresetManager.h"
      to: "Source/ui/PresetSelector.cpp"
      via: "PresetSelector calls UserPresetManager to save/load/list"
      pattern: "UserPresetManager"
---

<objective>
Add three UI features to the Aether plugin: (1) a mix lock toggle button near the Mix knob, (2) up/down arrow step buttons on all ComboBox dropdowns, and (3) user preset save/load via XML files.

Purpose: Improve workflow for users who want to audition presets without losing their mix ratio, quickly step through material/shape/preset options, and save their own parameter configurations.

Output: MixLockButton, ArrowStepButton components; UserPresetManager utility; extended PresetSelector with user preset support.
</objective>

<execution_context>
@.planning/quick/3-mix-lock-dropdown-arrows-user-preset-sav/3-PLAN.md
</execution_context>

<context>
@Source/ui/PresetSelector.h
@Source/ui/PresetSelector.cpp
@Source/ui/AetherBypassButton.h
@Source/ui/AetherColours.h
@Source/ui/AetherLookAndFeel.h
@Source/ui/AetherKnob.h
@Source/ui/AetherToggle.h
@Source/ui/SectionComponent.h
@Source/PluginEditor.h
@Source/PluginEditor.cpp
@Source/PluginProcessor.h
@Source/PluginProcessor.cpp
@Source/presets/FactoryPresets.h
@Source/Parameters.h
</context>

<tasks>

<task type="auto">
  <name>Task 1: Mix Lock Button + ComboBox Arrow Step Buttons</name>
  <files>
    Source/ui/MixLockButton.h
    Source/ui/MixLockButton.cpp
    Source/ui/ArrowStepButton.h
    Source/ui/ArrowStepButton.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/ui/PresetSelector.h
    Source/ui/PresetSelector.cpp
  </files>
  <action>
**MixLockButton (new files):**

Create `Source/ui/MixLockButton.h` and `.cpp`. This is a simple toggle button (extends juce::ToggleButton) that draws a lock/unlock icon programmatically:
- Size: 20x20px. No text -- purely icon-based.
- Locked state: Draw a closed padlock in `AetherColours::accentWarm` -- small rectangle body (8x7px centred) with a closed arc (shackle) on top.
- Unlocked state: Draw an open padlock in `AetherColours::inkGhost` -- same body rectangle, but the arc (shackle) is shifted up and open on the right side.
- Hover: Draw in `AetherColours::accentCopper` regardless of state.
- Override `paintButton()` to draw the padlock path. No background fill.
- Public method: `bool isLocked() const` returns `getToggleState()`.
- Follow the same pattern as AetherBypassButton (simple component, no APVTS attachment needed -- this is purely UI state, not a parameter).

**ArrowStepButton (new files):**

Create `Source/ui/ArrowStepButton.h` and `.cpp`. A small button that draws an up or down triangle arrow:
- Constructor takes `enum Direction { Up, Down }` and a reference to a `juce::ComboBox&`.
- Size: 14x12px (narrow, sits beside a combo box).
- Override `paintButton()` to draw a filled triangle: Up = points up, Down = points down.
- Colour: `AetherColours::inkFaint` normally, `AetherColours::accentCopper` on hover.
- On click: If Up, call `comboBox.setSelectedId(current - 1)` wrapping to last item if at first. If Down, call `comboBox.setSelectedId(current + 1)` wrapping to first item if at last. Use `comboBox.getNumItems()` and `comboBox.getSelectedId()` to determine bounds. Send notification (juce::sendNotificationAsync) so the ComboBoxAttachment picks up the change.

**PluginEditor.h changes:**

Add members:
```cpp
#include "ui/MixLockButton.h"
#include "ui/ArrowStepButton.h"

// In private:
MixLockButton mixLockButton;

// Arrow buttons for combo boxes (6 total: 2 per combo)
ArrowStepButton materialUpArrow;
ArrowStepButton materialDownArrow;
ArrowStepButton shapeUpArrow;
ArrowStepButton shapeDownArrow;
ArrowStepButton presetUpArrow;    // These go on the PresetSelector's comboBox
ArrowStepButton presetDownArrow;
```

Note: The PresetSelector needs to expose its internal comboBox via a public accessor `juce::ComboBox& getComboBox()` so the editor can create ArrowStepButtons targeting it.

**PresetSelector.h changes:**

Add public method:
```cpp
juce::ComboBox& getComboBox() { return comboBox; }
```

Also add a public method to check mix lock state from outside:
```cpp
void setMixLockButton (MixLockButton* button) { mixLockBtn = button; }
```
And a private member: `MixLockButton* mixLockBtn = nullptr;`

In `PresetSelector.cpp`, modify `applyPreset()`: Before the `setParam("out_mix", ...)` line, check `if (mixLockBtn != nullptr && mixLockBtn->isLocked()) return;` -- skip setting out_mix. Same for the Default reset path: if mix is locked, capture the current out_mix value before the reset loop, then restore it after.

**PluginEditor.cpp changes:**

In the constructor (after creating the PresetSelector):
- `presetSelector.setMixLockButton (&mixLockButton);`
- `addAndMakeVisible (mixLockButton);`
- Initialize ArrowStepButton instances in the initializer list, passing the correct comboBox refs:
  - `materialUpArrow { ArrowStepButton::Up, materialCombo }`
  - `materialDownArrow { ArrowStepButton::Down, materialCombo }`
  - `shapeUpArrow { ArrowStepButton::Up, reflShapeCombo }`
  - `shapeDownArrow { ArrowStepButton::Down, reflShapeCombo }`
  - `presetUpArrow { ArrowStepButton::Up, presetSelector.getComboBox() }`
  - `presetDownArrow { ArrowStepButton::Down, presetSelector.getComboBox() }`
- `addAndMakeVisible` for all 6 arrow buttons + mixLockButton.

In `resized()`:
- Mix lock button: Position 20x20px to the RIGHT of the mixKnob. The mixKnob is at `(outCtrl.getX(), outCtrl.getY(), twoKnobW, knobH)`. Place mixLockButton at `(mixKnob.getRight() - 2, mixKnob.getY() + 2, 20, 20)` -- top-right corner of the mix knob area.
- Material arrows: Place the Up arrow at `(materialCombo.getRight() + 2, materialCombo.getY(), 14, 12)` and Down arrow at `(materialCombo.getRight() + 2, materialCombo.getY() + 12, 14, 12)`.
- Shape arrows: Same pattern relative to `reflShapeCombo`.
- Preset arrows: Same pattern relative to `presetSelector`. Place at `(presetSelector.getRight() + 2, presetSelector.getY(), 14, 11)` and `(presetSelector.getRight() + 2, presetSelector.getY() + 11, 14, 11)`.
  </action>
  <verify>
Build with `cmake --build` and confirm no compilation errors. Visually verify in the plugin host that:
- Lock button appears near Mix knob and toggles between locked/unlocked icon states on click
- Arrow buttons appear beside each ComboBox and step through items correctly
- When mix lock is engaged, loading a preset preserves the current Mix value
  </verify>
  <done>
Mix lock button toggles and prevents Mix value changes during preset load. Arrow buttons step through all three combo boxes (Material, Shape, Preset) with wrapping. All drawn in parchment/ink aesthetic.
  </done>
</task>

<task type="auto">
  <name>Task 2: User Preset Save/Load Manager</name>
  <files>
    Source/presets/UserPresetManager.h
    Source/presets/UserPresetManager.cpp
  </files>
  <action>
**UserPresetManager (new files):**

Create `Source/presets/UserPresetManager.h` and `.cpp`. A stateless utility class with static methods for user preset management.

**Header:**
```cpp
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class UserPresetManager
{
public:
    /** Get the user preset directory, creating it if it does not exist.
     *  macOS: ~/Library/Audio/Presets/Cairn/Aether/ */
    static juce::File getPresetDirectory();

    /** Save current APVTS state to a named XML file in the preset directory.
     *  Returns true on success. Filename is sanitised (remove path separators, limit length).
     *  File extension: .xml */
    static bool savePreset (const juce::String& name,
                            juce::AudioProcessorValueTreeState& apvts);

    /** Load a user preset XML file and apply it to the APVTS.
     *  Returns true on success. Validates XML tag matches APVTS state type before applying. */
    static bool loadPreset (const juce::File& presetFile,
                            juce::AudioProcessorValueTreeState& apvts);

    /** Return an array of available user preset files (sorted alphabetically).
     *  Only returns .xml files from the preset directory. */
    static juce::Array<juce::File> getAvailablePresets();

    /** Delete a user preset file. Returns true on success. */
    static bool deletePreset (const juce::File& presetFile);
};
```

**Implementation (.cpp):**

`getPresetDirectory()`:
- Use `juce::File::getSpecialLocation(juce::File::userHomeDirectory)` and append `/Library/Audio/Presets/Cairn/Aether/`.
- Call `dir.createDirectory()` to ensure it exists.
- Return the directory File.

`savePreset()`:
- Sanitise the name: replace `/`, `\`, `:` with `_`, trim to 64 chars max, trim whitespace.
- If name is empty after sanitisation, return false.
- Get the APVTS state via `apvts.copyState()`, create XML via `state.createXml()`.
- Write to `getPresetDirectory().getChildFile(sanitisedName + ".xml")`.
- Use `xml->writeTo(file)` (JUCE's XmlElement::writeTo returns bool).
- Return success.

`loadPreset()`:
- Parse the file via `juce::XmlDocument::parse(presetFile)`.
- If null or tagName does not match `apvts.state.getType()`, return false.
- Call `apvts.replaceState(juce::ValueTree::fromXml(*xml))`.
- Return true.

`getAvailablePresets()`:
- Get preset directory, find child files matching `*.xml` wildcard.
- Use `dir.findChildFiles(results, juce::File::findFiles, false, "*.xml")`.
- Sort alphabetically by filename: `results.sort()` (juce::Array<File> sorts by full path, which works since same directory).
- Return results.

`deletePreset()`:
- Call `presetFile.deleteFile()` and return result.
  </action>
  <verify>
Build with `cmake --build` and confirm no compilation errors. The UserPresetManager compiles as a standalone utility with no UI dependencies.
  </verify>
  <done>
UserPresetManager provides static save/load/list/delete operations for XML preset files in ~/Library/Audio/Presets/Cairn/Aether/.
  </done>
</task>

<task type="auto">
  <name>Task 3: Wire User Presets into PresetSelector UI</name>
  <files>
    Source/ui/PresetSelector.h
    Source/ui/PresetSelector.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
  </files>
  <action>
**PresetSelector changes:**

Extend PresetSelector to support user presets alongside factory presets.

**Header additions:**
```cpp
#include "../presets/UserPresetManager.h"

// Add public methods:
/** Rebuild the combo box item list (factory + user presets). Call after saving/deleting. */
void rebuildPresetList();

// Add private members:
juce::Array<juce::File> userPresetFiles;  // Cached list of user preset files
static constexpr int kUserPresetIdOffset = 100;  // User preset IDs start at 100
```

Also add a save button member (juce::TextButton):
```cpp
juce::TextButton saveButton { "Save" };
```

**Implementation changes (.cpp):**

In the constructor:
- Replace the hardcoded `comboBox.addItem(...)` calls with a call to `rebuildPresetList()`.
- Set up the save button:
  - `saveButton.setColour(juce::TextButton::textColourOffId, juce::Colour(AetherColours::inkFaint));`
  - `saveButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);`
  - Set the "isBypass" property to false (so LookAndFeel draws it as a standard small button).
  - Wire `saveButton.onClick`: Show a JUCE AlertWindow with a text input field asking for a preset name. On OK, call `UserPresetManager::savePreset(name, apvts)`, then `rebuildPresetList()`. Use `juce::AlertWindow::showAsync` pattern:
    ```cpp
    auto* aw = new juce::AlertWindow ("Save Preset", "Enter a name for your preset:",
                                       juce::MessageBoxIconType::NoIcon);
    aw->addTextEditor ("name", "", "Preset name:");
    aw->addButton ("Save", 1);
    aw->addButton ("Cancel", 0);
    aw->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, aw] (int result) {
            if (result == 1)
            {
                auto name = aw->getTextEditorContents ("name");
                if (name.isNotEmpty())
                {
                    UserPresetManager::savePreset (name, apvts);
                    rebuildPresetList();
                }
            }
            delete aw;
        }), false);
    ```
  - `addAndMakeVisible(saveButton);`

`rebuildPresetList()`:
- Clear the combo box: `comboBox.clear(juce::dontSendNotification)`.
- Add factory items (same as before): Default (ID=1), then 6 factory presets (IDs 2-7).
- Add a separator if user presets exist: `comboBox.addSeparator()`.
- Load user presets: `userPresetFiles = UserPresetManager::getAvailablePresets()`.
- For each user preset file, add item: `comboBox.addItem(file.getFileNameWithoutExtension(), kUserPresetIdOffset + i)` where i is the 0-based index.
- Restore selection to ID 1 (Default) if no previous selection.

Modify the `comboBox.onChange` lambda:
- After the existing factory preset handling (selectedId 1-7), add:
  ```cpp
  else if (selectedId >= kUserPresetIdOffset)
  {
      int userIndex = selectedId - kUserPresetIdOffset;
      if (userIndex >= 0 && userIndex < userPresetFiles.size())
      {
          // If mix is locked, capture current mix value
          float savedMix = 0.0f;
          bool lockMix = (mixLockBtn != nullptr && mixLockBtn->isLocked());
          if (lockMix)
          {
              if (auto* mixParam = apvts.getParameter(ParamIDs::outMix))
                  savedMix = mixParam->getValue();  // normalised 0-1
          }

          UserPresetManager::loadPreset(userPresetFiles[userIndex], apvts);

          // Restore mix if locked
          if (lockMix)
          {
              if (auto* mixParam = apvts.getParameter(ParamIDs::outMix))
              {
                  mixParam->beginChangeGesture();
                  mixParam->setValueNotifyingHost(savedMix);
                  mixParam->endChangeGesture();
              }
          }
      }
  }
  ```

Update `resized()`:
- The save button goes to the right of the comboBox. Allocate bounds as: comboBox gets the left portion, save button gets 36px on the right.
  ```cpp
  auto bounds = getLocalBounds();
  saveButton.setBounds (bounds.removeFromRight (36));
  comboBox.setBounds (bounds);
  ```

**PluginEditor.cpp changes:**

In `resized()`, widen the presetSelector bounds to accommodate the save button. Change:
```cpp
presetSelector.setBounds ((kWidth - 180) / 2, 10, 180, 22);
```
to:
```cpp
presetSelector.setBounds ((kWidth - 220) / 2, 10, 220, 22);
```

This gives 220px total: ~184px for the combo, 36px for the save button.

Also ensure the preset arrow buttons are positioned relative to the updated presetSelector bounds.

**CMakeLists.txt:** Add the 4 new source files (MixLockButton.h/.cpp, ArrowStepButton.h/.cpp, UserPresetManager.h/.cpp) to the target_sources list.
  </action>
  <verify>
Build with `cmake --build` and confirm no compilation errors. In the plugin:
1. Click "Save" button -- dialog appears, enter a name, confirm -- preset file appears in ~/Library/Audio/Presets/Cairn/Aether/
2. The saved preset appears in the dropdown below a separator after factory presets
3. Selecting the user preset loads its parameter values correctly
4. With Mix Lock engaged, selecting any preset (factory or user) preserves the current Mix value
5. Arrow buttons on the preset combo step through all entries including user presets
  </verify>
  <done>
User presets save to ~/Library/Audio/Presets/Cairn/Aether/ as XML files. They appear in PresetSelector below factory presets. Save button shows a name dialog. User presets load correctly, respecting mix lock state. All three features (mix lock, arrows, user presets) work together as a cohesive system.
  </done>
</task>

</tasks>

<verification>
1. Build: `cmake --build build --config Debug` compiles without errors or warnings
2. Mix Lock: Load plugin, set Mix to 50%, engage lock, load any factory preset -- Mix stays at 50%
3. Mix Lock (off): Disengage lock, load a preset -- Mix changes to preset value
4. Arrows: Click up/down arrows on Material, Shape, Preset -- items cycle correctly with wrapping
5. Save: Click Save, enter "My Test Preset", confirm -- file exists at ~/Library/Audio/Presets/Cairn/Aether/My Test Preset.xml
6. Load: Select "My Test Preset" from dropdown -- all parameters restored
7. Mix Lock + User Preset: Lock Mix, load user preset -- Mix preserved
</verification>

<success_criteria>
- MixLockButton drawn as padlock icon in parchment/ink aesthetic, toggles correctly
- ArrowStepButton draws triangles, steps through combo items with wrap-around on all 3 combos
- UserPresetManager saves/loads XML files to ~/Library/Audio/Presets/Cairn/Aether/
- PresetSelector shows factory presets + separator + user presets in dropdown
- Save button triggers name dialog and persists preset
- Mix lock prevents out_mix from changing during any preset load (factory or user)
- All new source files added to CMakeLists.txt
</success_criteria>

<output>
After completion, create `.planning/quick/3-mix-lock-dropdown-arrows-user-preset-sav/3-SUMMARY.md`
</output>
