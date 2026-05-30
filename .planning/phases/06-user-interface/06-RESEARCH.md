# Phase 6: User Interface - Research

**Researched:** 2026-02-18
**Domain:** JUCE custom GUI components, LookAndFeel, font embedding, Victorian parchment theme
**Confidence:** HIGH

## Summary

Phase 6 builds the complete Victorian parchment UI for Aether. The existing codebase provides an empty editor shell (`AetherEditor`) already sized to 900x530px with a parchment background fill, and 21 APVTS parameters ready for attachment. The Crucible plugin provides a proven reference implementation of the Cairn design patterns: custom `LookAndFeel_V4` subclass for knob/button rendering, `juce_add_binary_data` for font embedding, `ContentWrapper` with `AffineTransform` scaling, and decorative element helpers drawn via `juce::Path` in `paint()` overrides.

The UI requires three custom component types (knob, toggle, bypass button), three embedded fonts (Cormorant Garamond, EB Garamond, Spectral), a three-column grid layout, and decorative Victorian ornamental elements. The critical distinction from Crucible is that Aether uses a light parchment/ink theme instead of the dark Cairn palette, and replaces arc-style knobs with filled-body radial-gradient knobs with an ink indicator line. Additionally, the parameter definitions in `Parameters.h` have evolved beyond the original design handoff (10 materials instead of 3 cabinet types, 3 air characters instead of 2, 7 room shapes as named presets) -- the UI must display controls matching the actual APVTS, not the handoff spec.

**Primary recommendation:** Follow the Crucible architecture pattern exactly: `AetherLookAndFeel` (LookAndFeel_V4 subclass), `AetherColours` namespace, custom `AetherKnob` component wrapping `juce::Slider`, custom `AetherToggle` and `AetherBypassButton` wrapping `juce::TextButton`, and a `ParchmentElements` namespace for decorative drawing helpers. Font embedding via `juce_add_binary_data` in CMakeLists.txt.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Knob interaction:**
- Scroll wheel supported with fine increments (~1% per tick)
- Click-to-type on value readout for exact value entry; out-of-range values clamp silently
- Slower/more precise drag sensitivity (~300px for full sweep vs default ~200px)
- Shift+drag for fine control (10x slower); no Cmd+snap behavior
- Double-click resets to parameter's saved default value (not noon/50%)
- Smooth animated indicator sweep (~100-200ms) on preset load, double-click reset, and value jumps
- Real-time value readout update while dragging

**Section bypass treatment:**
- Controls remain fully interactive when section is bypassed (users can pre-dial settings before toggling In)

**Value display & readout:**
- Text-label params (Room Size, Shape, Proximity) show percentage + nearest descriptor: "62% (Medium)"
- Decay knob switches display from ms to seconds above 1000ms: "150ms" below, "1.2s" above
- No min/max range labels at knob sweep endpoints -- value readout only

**Ornamental weight:**
- Overall feel: moderate/balanced -- ornaments clearly present but don't compete with controls
- Paper texture: visible on close inspection (opacity ~0.015-0.02), faint ruled horizontal lines every 4px
- Edge vignette: noticeable warm darkening, visible ~30px inward
- Diamond dividers: functional separators -- clean horizontal lines with small diamond
- Double-rule header/footer borders: two distinct visible lines with 3px gap
- Section Roman numerals and section name at equal visual weight

### Claude's Discretion

- Hover feedback and right-click context menu
- Center knobs (64px) vs side panel knobs (56px) differentiation
- Visual dimming scope and bypass transition animation
- Additional bypass indicator beyond In/Out text
- Percentage precision (whole numbers vs one decimal)
- Corner L-brackets: tuned to match moderate/balanced weight

### Deferred Ideas (OUT OF SCOPE)

None -- discussion stayed within phase scope
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| UI-01 | Plugin window 900x530px, three-column layout (220px \| flex \| 220px) | Editor shell already has `setSize(900, 530)`. Layout via `setBounds()` in `resized()` with grid columns. Crucible uses explicit coordinate layout, not FlexBox. |
| UI-02 | Parchment background (#f0e6d3) with paper texture and edge vignette | Background fill already in place. Texture via `repeating-linear-gradient` equivalent (horizontal lines every 4px at ~0.015-0.02 opacity). Vignette via radial gradient overlay in `paint()`. |
| UI-03 | Custom knob components: 56px/64px, parchment gradient body, ink indicator | Custom `AetherKnob` component wrapping `juce::Slider` with `drawRotarySlider` override in `AetherLookAndFeel`. Radial gradient body + ink indicator line. |
| UI-04 | Custom toggle switches with ink-inverted active state | Custom `AetherToggle` component using `juce::TextButton` array with attachment to `AudioParameterChoice`. Ink fill on active option, parchment text. |
| UI-05 | Custom bypass buttons per section (In/Out with opacity) | `AetherBypassButton` wrapping `juce::TextButton` with `ButtonAttachment` to `AudioParameterBool`. Spectral 7px text, opacity shift on state change. |
| UI-06 | Section labels with Roman numerals (I.-VI.) in Spectral font | Drawn in section component `paint()` methods. Spectral loaded from binary data. Cormorant Garamond italic for numerals. |
| UI-07 | Diamond ornament dividers between sections in side panels | `ParchmentElements::drawDiamondDivider()` -- horizontal lines + 5x5px rotated square stroke in Ink Ghost. |
| UI-08 | Corner L-shaped ornaments at plugin frame corners | `ParchmentElements::drawCornerBracket()` -- 20x20px L-brackets at 6px from edges, 1px Ink Ghost. |
| UI-09 | Header: "AETHER" title, subtitle, preset selector, "Cosmos Series" mark | Header zone drawn in editor `paint()`. Letter-spaced Cormorant Garamond title. Preset selector via `juce::ComboBox` or custom dropdown. |
| UI-10 | Footer: version, tagline, brand mark | Footer zone drawn in `paint()` with Cairn triangle mark (same pattern as Crucible footer). |
| UI-11 | Double-rule header/footer borders | `::after` pseudo-element equivalent: second 1px line drawn 3px below/above primary border at 40% opacity. |
| UI-12 | Custom typography: Cormorant Garamond, EB Garamond, Spectral (fallback Georgia) | Three TTF/OTF files embedded via `juce_add_binary_data`. Three `Typeface::Ptr` members in `AetherLookAndFeel`. `FontOptions(typeface).withHeight(size)` pattern from Crucible. |
</phase_requirements>

## Standard Stack

### Core

| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| JUCE | 8.0.12 | GUI framework | Already in project via FetchContent. Provides LookAndFeel, Component, Slider, TextButton, Graphics, Path, Font, Typeface, BinaryData |
| juce_gui_basics | (part of JUCE 8) | Widgets, LookAndFeel, Component hierarchy | Standard JUCE module for plugin GUIs |
| juce_graphics | (part of JUCE 8) | 2D rendering, fonts, paths, gradients | Standard JUCE module for custom drawing |

### Supporting

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| juce_add_binary_data (CMake) | JUCE 8 | Embed TTF font files as BinaryData | Font embedding -- proven pattern from Crucible |
| Google Fonts (Cormorant Garamond, EB Garamond, Spectral) | Latest TTF | Victorian serif typography | Download TTF files, embed as binary data |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Custom component classes | Pure LookAndFeel overrides | LookAndFeel alone doesn't handle layout, value display, click-to-type, animation. Custom components needed for knob interaction behavior. |
| Manual letter-spacing drawing | JUCE's built-in font rendering | JUCE has no letter-spacing API. Must draw character-by-character with custom spacing (Crucible already does this). |
| juce::FlexBox for layout | Manual setBounds() | FlexBox adds complexity for a fixed-size layout. Manual coordinates match Crucible's proven approach and give pixel-perfect control. |

**Font Installation:**

Download from Google Fonts:
- Cormorant Garamond (Regular 400, Light 300, Italic 400i)
- EB Garamond (Regular 400, Italic 400i)
- Spectral (Regular 400, Italic 400i)

Place in `Resources/fonts/`. Add to CMakeLists.txt via `juce_add_binary_data`.

## Architecture Patterns

### Recommended Project Structure

```
Source/
├── PluginProcessor.h/cpp      # Existing (createEditor switches to AetherEditor)
├── PluginEditor.h/cpp         # Existing shell -- expand with full layout
├── Parameters.h               # Existing 21-parameter APVTS layout
└── ui/
    ├── AetherColours.h         # Colour palette namespace (like CrucibleColours)
    ├── AetherLookAndFeel.h/cpp # LookAndFeel_V4 subclass (3 fonts, drawRotarySlider)
    ├── AetherKnob.h/cpp        # Custom knob: wraps Slider, handles value display,
    │                           #   click-to-type, animation, size variants
    ├── AetherToggle.h/cpp      # Toggle switch: wraps TextButton group for choices
    ├── AetherBypassButton.h/cpp# Per-section bypass: In/Out text button
    ├── SectionComponent.h/cpp  # Base/shared section layout (label + bypass + controls)
    ├── ParchmentElements.h/cpp # Decorative drawing helpers (dividers, corners, texture)
    └── PresetSelector.h/cpp    # Header preset dropdown (ComboBox or custom)
```

### Pattern 1: AetherLookAndFeel (LookAndFeel_V4 Subclass)

**What:** Central styling class that loads all three embedded fonts and overrides `drawRotarySlider`, `drawButtonBackground`, `drawButtonText`, `drawLabel`, `drawPopupMenuItem`, `drawPopupMenuBackground`. Sets colour IDs for all JUCE component colour slots.

**When to use:** Set on the editor in constructor, applied to all child components automatically via JUCE's LookAndFeel cascade.

**Example (from Crucible, adapted for Aether):**

```cpp
// AetherLookAndFeel constructor - font loading pattern
AetherLookAndFeel::AetherLookAndFeel()
{
    // Load embedded fonts from binary data
    displayTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::CormorantGaramondRegular_ttf,
        BinaryData::CormorantGaramondRegular_ttfSize);

    bodyTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::EBGaramondRegular_ttf,
        BinaryData::EBGaramondRegular_ttfSize);

    labelTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::SpectralRegular_ttf,
        BinaryData::SpectralRegular_ttfSize);

    // Font helper methods
    // getDisplayFont(size)  -> Cormorant Garamond, fallback Georgia
    // getBodyFont(size)     -> EB Garamond, fallback Georgia
    // getLabelFont(size)    -> Spectral, fallback Georgia
}
```

### Pattern 2: Custom Knob Component (AetherKnob)

**What:** A `juce::Component` that owns a `juce::Slider` (RotaryHorizontalVerticalDrag style) and a value display `juce::Label`. Handles all the custom interaction behavior: drag sensitivity, shift-drag fine control, scroll wheel, double-click reset, click-to-type on value readout, animated indicator sweep.

**When to use:** Every knob in the plugin.

**Key JUCE APIs for knob behavior:**

```cpp
// Slower drag sensitivity (~300px full sweep vs ~200px default)
// JUCE velocity mode: sensitivity < 1.0 = slower
slider.setVelocityModeParameters(0.7, 1, 0.0, true,
    juce::ModifierKeys::shiftModifier);  // Shift for fine control

// Alternatively, use non-velocity mode with custom sensitivity:
slider.setSliderSnapsToMousePosition(false);
// The velocity sensitivity parameter controls effective pixels-per-range.
// sensitivity=0.7 means the user needs ~43% more pixels to traverse full range.

// Double-click reset to default (parameter default, not 50%)
slider.setDoubleClickReturnValue(true, defaultValue);

// Scroll wheel: enabled by default in JUCE
slider.setScrollWheelEnabled(true);
// For fine increments (~1% per tick), this is the default JUCE behavior for
// NormalisableRange with step of 0.01

// Click-to-type on value readout
// Implemented by making the value Label editable on click, then parsing
// the text back into a value via Label::Listener::labelTextChanged

// Rotary parameters (270-degree arc, 7 o'clock to 5 o'clock)
slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,  // 7 o'clock
                           juce::MathConstants<float>::pi * 2.75f,  // 5 o'clock
                           true);                                     // clockwise
```

### Pattern 3: APVTS Parameter Attachment

**What:** JUCE's `SliderAttachment` and `ButtonAttachment` classes automatically synchronize UI controls with APVTS parameters, handling thread safety (UI thread <-> audio thread).

**When to use:** Every control-to-parameter connection.

**Example:**

```cpp
// In AetherKnob constructor or parent component
using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
std::unique_ptr<SliderAttachment> attachment;

// Attach slider to parameter
attachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::resWeight, slider);

// For bypass buttons
using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
std::unique_ptr<ButtonAttachment> bypassAttachment;
bypassAttachment = std::make_unique<ButtonAttachment>(apvts, ParamIDs::resBypass, bypassButton);

// For toggles (AudioParameterChoice with 2-3 options)
// Use ComboBoxAttachment with a hidden ComboBox, or
// manually sync via Listener + parameter.setValueNotifyingHost()
using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
```

### Pattern 4: Parchment Gradient Knob Rendering

**What:** Custom `drawRotarySlider` override that draws a filled circle with radial gradient (Parchment Light highlight at top-left to Parchment Dark shadow at bottom-right), 1.5px Ink Light border, and a rotating Ink indicator line.

**Key difference from Crucible:** Crucible draws arc-style knobs (background arc + active arc + center body). Aether draws filled-body knobs with no arc -- the entire circle is the knob body.

```cpp
void AetherLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y,
                                          int width, int height,
                                          float sliderPos, float startAngle,
                                          float endAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    auto centre = bounds.getCentre();
    float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

    // Body: radial gradient (highlight top-left, shadow bottom-right)
    juce::ColourGradient bodyGrad(
        juce::Colour(AetherColours::parchmentLight),
        centre.x - radius * 0.4f, centre.y - radius * 0.4f,
        juce::Colour(AetherColours::parchmentDark),
        centre.x + radius * 0.4f, centre.y + radius * 0.4f,
        true);  // radial
    g.setGradientFill(bodyGrad);
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    // Border: 1.5px Ink Light
    g.setColour(juce::Colour(AetherColours::inkLight));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

    // Shadow
    g.setColour(juce::Colour(AetherColours::shadow));
    // Use drop shadow offset or subtle outer glow

    // Indicator line: 1.5px wide, 16px long, from center outward
    float currentAngle = startAngle + sliderPos * (endAngle - startAngle);
    float indicatorLen = 16.0f;
    float innerR = radius - indicatorLen;
    float outerR = radius - 2.0f;

    float lineX1 = centre.x + innerR * std::sin(currentAngle);
    float lineY1 = centre.y - innerR * std::cos(currentAngle);
    float lineX2 = centre.x + outerR * std::sin(currentAngle);
    float lineY2 = centre.y - outerR * std::cos(currentAngle);

    g.setColour(juce::Colour(AetherColours::ink));
    g.drawLine(lineX1, lineY1, lineX2, lineY2, 1.5f);
}
```

### Pattern 5: Animated Value Changes

**What:** When a preset loads, or the user double-click resets, the knob indicator should sweep smoothly to the new position over 100-200ms instead of jumping.

**Implementation approach:**

```cpp
// In AetherKnob, detect value jumps and animate
class AetherKnob : public juce::Component, private juce::Slider::Listener, private juce::Timer
{
    float displayValue = 0.0f;   // Animated display position (0-1)
    float targetValue = 0.0f;    // Actual parameter value

    void sliderValueChanged(juce::Slider* s) override
    {
        float newTarget = static_cast<float>(s->getValue());
        float normalised = s->valueToProportionOfLength(newTarget);

        // If jump > threshold (e.g., 5% of range), animate
        if (std::abs(normalised - displayValue) > 0.05f && !isDragging)
        {
            targetValue = normalised;
            startTimerHz(60);  // 60fps animation
        }
        else
        {
            displayValue = normalised;
            targetValue = normalised;
            repaint();
        }
    }

    void timerCallback() override
    {
        // Ease toward target (exponential smoothing)
        float alpha = 0.15f;  // ~100-200ms settling at 60fps
        displayValue += alpha * (targetValue - displayValue);
        if (std::abs(displayValue - targetValue) < 0.001f)
        {
            displayValue = targetValue;
            stopTimer();
        }
        repaint();
    }
};
```

### Pattern 6: Three-Column Layout

**What:** Fixed-coordinate layout in `resized()` method. No FlexBox needed for a fixed-size plugin.

```cpp
void AetherEditor::resized()
{
    auto bounds = getLocalBounds();

    // Header zone (~50px)
    auto header = bounds.removeFromTop(50);

    // Footer zone (~40px)
    auto footer = bounds.removeFromBottom(40);

    // Primary zone: three columns
    auto leftPanel  = bounds.removeFromLeft(220);
    auto rightPanel = bounds.removeFromRight(220);
    auto centerPanel = bounds;  // flex, remaining ~460px

    // Left panel sections: Stage I, diamond, Stage IV, diamond, Stage V
    // Right panel sections: Stage III, diamond, Stage VI, diamond, Output
    // Center panel: Stage II label, visualization placeholder, room controls
}
```

### Pattern 7: Parameter-to-UI Mapping (CRITICAL)

**What:** The actual `Parameters.h` has diverged from the design handoff. The UI must display controls matching the real APVTS, not the original handoff spec.

**Divergences from design handoff that affect UI:**

| Stage | Handoff Says | Parameters.h Actually Has | UI Impact |
|-------|-------------|---------------------------|-----------|
| I. Cabinet Resonance | "Body" knob + 3-way toggle (Open/Closed/Combo) | `res_weight` knob + `res_material` 10-choice (Pine/Oak/Walnut/Mahogany/Iron/Steel/Copper/Limestone/Marble/Granite) | Need a 10-option selector, not a 3-way toggle. Dropdown or scrollable list instead of toggle buttons. |
| II. Early Reflections | Room Size/Shape/Proximity (3 knobs) | Same 3 knobs + `refl_width` (Width knob, 0-100%) | Width is a 4th knob in Stage II not shown in handoff. Place below the 3 center knobs or in center panel. |
| III. Air & Distance | 2-way toggle (Warm/Neutral) | 3-choice (Warm/Neutral/Cold) | 3-way toggle instead of 2-way. |
| V. Room Tone | "Ambience" knob only | Ambience knob + `tone_gate` 3-choice (Always On/Signal-Gated/Transport-Only) | Needs a 3-way toggle for gate mode not in handoff. |
| II. Room Shape | Knob (Regular-Irregular continuum) | 7-choice (The Parlour/The Gallery/The Chamber/The Nave/The Alcove/The Crypt/The Conservatory) | Dropdown selector or scrollable list, not a continuum knob. |
| II. Room Size | Knob with text labels | Float 0-1 with 5-label display (Small/Sm-Med/Medium/Med-Lg/Large) | Knob with percentage + descriptor as decided. |

**Naming adaptation for the UI:**
- `res_weight` displays as "Body" (matching handoff label)
- `res_material` displays as "Material" (new control, not in handoff)
- `refl_width` displays as "Width" (new control, not in handoff)
- `tone_gate` displays as "Gate" (new control, not in handoff)

### Anti-Patterns to Avoid

- **Don't use `GenericAudioProcessorEditor` as a fallback.** The current `createEditor()` returns it. Switch to `return new AetherEditor(*this);` once the custom editor is functional.
- **Don't draw controls in `paint()` manually.** Use JUCE's component tree with `addAndMakeVisible()` and let `LookAndFeel` handle rendering. Custom drawing belongs in `drawRotarySlider`/`drawButtonBackground` overrides.
- **Don't create parameter attachments before components are visible.** Attachment objects must be created after `addAndMakeVisible()` or the initial value sync can fail silently.
- **Don't forget to null-check typefaces.** Font loading can fail. Always provide Georgia fallback.
- **Don't use `setDefaultLookAndFeel` without clearing it in destructor.** Crucible pattern: set in constructor, `setDefaultLookAndFeel(nullptr)` in destructor.
- **Don't modify APVTS parameters on the audio thread from UI code.** Attachments handle thread safety automatically. Direct `setValueNotifyingHost()` is safe from UI thread.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Knob-to-parameter sync | Manual listener + setValue loop | `juce::AudioProcessorValueTreeState::SliderAttachment` | Thread-safe, handles undo, handles automation, handles preset recall |
| Toggle-to-parameter sync | Manual button callback | `ComboBoxAttachment` (via hidden ComboBox) or manual `setValueNotifyingHost` | Attachment handles undo integration |
| Font rendering | Custom text engine | JUCE's `Font` + `Typeface::createSystemTypefaceFor` + `GlyphArrangement` for letter-spacing | Handles all OS-specific font rendering |
| Gradient fills | Pixel-by-pixel rendering | `juce::ColourGradient` with `g.setGradientFill()` | Hardware-accelerated, handles radial/linear correctly |
| Timer-based animation | Raw `std::thread` or manual frame counting | `juce::Timer::startTimerHz(60)` | Message-thread safe, automatic stop/start |
| Popup menus | Custom windowed dropdown | `juce::PopupMenu` with LookAndFeel override | Handles OS-level windowing, positioning, keyboard nav |

**Key insight:** JUCE's component and LookAndFeel system is designed for exactly this kind of custom-themed plugin UI. Fighting it (e.g., drawing everything in a single `paint()` call) leads to broken hit-testing, missing keyboard access, and attachment failures. Work WITH the component tree.

## Common Pitfalls

### Pitfall 1: Font Loading Produces Null Typeface

**What goes wrong:** `Typeface::createSystemTypefaceFor()` returns nullptr if the binary data is corrupted, wrong format, or the font file has issues. All subsequent `Font(FontOptions(nullptr))` calls crash or produce invisible text.

**Why it happens:** Wrong file added to binary data, or CMake build doesn't regenerate BinaryData after adding fonts.

**How to avoid:** Always null-check typeface pointers. Provide fallback `juce::Font(juce::FontOptions("Georgia", size, juce::Font::plain))`. Test font loading early.

**Warning signs:** Text doesn't appear, or all text renders in system default sans-serif.

### Pitfall 2: Letter Spacing Not Built Into JUCE

**What goes wrong:** JUCE's `Font` class has no `letterSpacing` property. Text renders with default spacing, making tracked uppercase labels look wrong.

**Why it happens:** JUCE delegates to OS text rendering which doesn't expose per-character spacing control.

**How to avoid:** Use character-by-character drawing with `GlyphArrangement::addLineOfText` to measure each character's width, then draw with added spacing. Crucible already implements this in `ForgeElements::drawEngravedText()` and the wordmark drawing code.

**Warning signs:** Uppercase labels look cramped compared to the HTML mockup.

### Pitfall 3: SliderAttachment Created Before Component Hierarchy Is Ready

**What goes wrong:** Attachment sets the slider's initial value from the parameter, but if the slider isn't added to the component hierarchy yet, the `repaint()` call does nothing, and the initial visual position can be wrong.

**Why it happens:** Creating attachment in member initializer list before `addAndMakeVisible()`.

**How to avoid:** Create attachments after all components are added to the hierarchy. In Crucible, attachments are created in the constructor body after `addAndMakeVisible()`.

**Warning signs:** Knobs display wrong initial positions that correct on first drag.

### Pitfall 4: Bypassed Section Dimming Interferes With Interaction

**What goes wrong:** If bypass dimming is implemented by setting `setAlpha()` on the section component, it dims everything including hover feedback and makes the controls feel broken.

**Why it happens:** `setAlpha()` applies to the entire component tree.

**How to avoid:** Implement dimming via a translucent overlay drawn in `paint()` AFTER child components render, or by adjusting individual control colours via a "bypassed" flag that changes paint behaviour without affecting mouse interaction. The user explicitly decided controls remain fully interactive when bypassed.

**Warning signs:** Bypassed sections feel dead/unresponsive even though controls should work.

### Pitfall 5: AudioParameterChoice Requires ComboBox for Attachment

**What goes wrong:** `juce::AudioProcessorValueTreeState::ComboBoxAttachment` only works with `juce::ComboBox`, not with `juce::TextButton`. Custom toggle controls backed by `AudioParameterChoice` need manual sync.

**Why it happens:** JUCE's attachment classes are tied to specific widget types.

**How to avoid:** For toggle switches backed by `AudioParameterChoice`:
1. Create a hidden `juce::ComboBox` and attach it via `ComboBoxAttachment`
2. Listen to the ComboBox for changes and update your custom toggle visually
3. When the user clicks a toggle option, set the ComboBox selected index
4. OR skip attachments entirely and use `parameter->setValueNotifyingHost()` + `ParameterAttachment` (lower-level API)

**Warning signs:** Toggle state doesn't sync with DAW automation or preset recall.

### Pitfall 6: Animated Knob Creates Feedback Loop With Attachment

**What goes wrong:** Animating the display value triggers slider value changes, which trigger the attachment, which notifies the parameter, which triggers the listener again.

**Why it happens:** The animation interpolates the display value but the slider's actual value is already at the target.

**How to avoid:** Separate "display value" (what's drawn) from "parameter value" (what the attachment controls). The animation only affects the display value used in `drawRotarySlider()`. The `Slider`'s actual value jumps immediately; only the visual representation animates.

**Warning signs:** Knobs oscillate or stutter on preset changes.

### Pitfall 7: Paper Texture Overdraw Kills Performance

**What goes wrong:** Drawing repeating horizontal lines every 4px across 900x530px in `paint()` on every repaint is expensive, especially with alpha blending.

**Why it happens:** Naive implementation redraws the texture every frame.

**How to avoid:** Pre-render the paper texture + vignette to a `juce::Image` once (in the constructor or on resize), then draw that cached image in `paint()`. The texture never changes, so it only needs to be generated once.

**Warning signs:** Plugin UI stutters when knobs are being dragged (continuous repainting).

### Pitfall 8: Value Readout for Text-Label Parameters

**What goes wrong:** Parameters like Room Size show raw float (0.0-1.0) instead of "62% (Medium)" format.

**Why it happens:** `stringFromValueFunction` on the parameter already returns "Medium" text, but the UI decision requires "62% (Medium)" format which combines percentage AND descriptor.

**How to avoid:** In the knob's value display, compute the percentage from the normalised value AND call the parameter's string conversion to get the descriptor. Combine them: `juce::String(roundToInt(norm * 100)) + "% (" + param->getCurrentValueAsText() + ")"`.

**Warning signs:** Value readout shows only percentage or only descriptor, not both.

## Code Examples

### Font Embedding in CMakeLists.txt

```cmake
# After juce_generate_juce_header(Aether)

# Embed fonts as binary data
juce_add_binary_data(AetherBinaryData
    SOURCES
        Resources/fonts/CormorantGaramond-Regular.ttf
        Resources/fonts/CormorantGaramond-Light.ttf
        Resources/fonts/CormorantGaramond-LightItalic.ttf
        Resources/fonts/EBGaramond-Regular.ttf
        Resources/fonts/EBGaramond-Italic.ttf
        Resources/fonts/Spectral-Regular.ttf
        Resources/fonts/Spectral-Italic.ttf
)

# Add to target_link_libraries
target_link_libraries(Aether
    PRIVATE
        AetherBinaryData
        # ... existing libraries
)
```

### AetherColours Namespace

```cpp
// Source: Design handoff spec, verified against HTML mockup CSS variables
namespace AetherColours
{
    inline constexpr juce::uint32 parchment      = 0xfff0e6d3;
    inline constexpr juce::uint32 parchmentLight  = 0xfff5edd9;
    inline constexpr juce::uint32 parchmentDark   = 0xffe0d2b8;
    inline constexpr juce::uint32 paperEdge       = 0xffd4c4a8;
    inline constexpr juce::uint32 ink             = 0xff2a2118;
    inline constexpr juce::uint32 inkLight        = 0xff5a4a38;
    inline constexpr juce::uint32 inkFaint        = 0xff9a8a72;
    inline constexpr juce::uint32 inkGhost        = 0xffc8b89a;
    inline constexpr juce::uint32 accentWarm      = 0xff8b6b3d;
    inline constexpr juce::uint32 accentCopper    = 0xffa67c52;
    inline constexpr juce::uint32 sepia           = 0xff704214;
    inline constexpr juce::uint32 shadow          = 0x262a2118;  // rgba(42,33,24,0.15)

    // Bypass dimming
    inline constexpr float bypassedOpacity = 0.5f;
}
```

### Toggle Switch for AudioParameterChoice

```cpp
// Pattern for 3-way toggle (e.g., Warm/Neutral/Cold)
class AetherToggle : public juce::Component
{
public:
    AetherToggle(juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& paramID,
                 const juce::StringArray& options)
    {
        // Hidden ComboBox for APVTS attachment
        hiddenCombo.addItemList(options, 1);
        addChildComponent(hiddenCombo);  // invisible
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, paramID, hiddenCombo);

        // Create visible toggle buttons
        for (int i = 0; i < options.size(); ++i)
        {
            auto* btn = buttons.add(new juce::TextButton(options[i]));
            btn->setClickingTogglesState(false);
            btn->onClick = [this, i]() {
                hiddenCombo.setSelectedItemIndex(i, juce::sendNotification);
            };
            addAndMakeVisible(btn);
        }

        // Listen for external changes (automation, preset recall)
        hiddenCombo.onChange = [this]() {
            updateButtonStates();
        };
        updateButtonStates();
    }

private:
    void updateButtonStates()
    {
        int selected = hiddenCombo.getSelectedItemIndex();
        for (int i = 0; i < buttons.size(); ++i)
            buttons[i]->setToggleState(i == selected, juce::dontSendNotification);
        repaint();
    }

    juce::ComboBox hiddenCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    juce::OwnedArray<juce::TextButton> buttons;
};
```

### Cached Paper Texture + Vignette

```cpp
// In AetherEditor or main background component
void regenerateBackground()
{
    backgroundImage = juce::Image(juce::Image::ARGB, 900, 530, true);
    juce::Graphics g(backgroundImage);

    // Base parchment fill
    g.setColour(juce::Colour(AetherColours::parchment));
    g.fillAll();

    // Paper texture: horizontal lines every 4px
    g.setColour(juce::Colour(0xff704214).withAlpha(0.018f));  // ~0.015-0.02
    for (int y = 0; y < 530; y += 4)
        g.fillRect(0, y, 900, 1);

    // Edge vignette: radial gradient overlay
    juce::ColourGradient vignette(
        juce::Colours::transparentBlack, 450.0f, 265.0f,  // center
        juce::Colour(0xff704214).withAlpha(0.08f), 0.0f, 0.0f,  // edge
        true);  // radial
    vignette.addColour(0.6, juce::Colours::transparentBlack);  // start darkening at 60% from center
    g.setGradientFill(vignette);
    g.fillRect(0, 0, 900, 530);
}

void paint(juce::Graphics& g) override
{
    g.drawImageAt(backgroundImage, 0, 0);
}
```

### Diamond Divider Drawing

```cpp
// ParchmentElements namespace
void drawDiamondDivider(juce::Graphics& g, float y, float left, float right)
{
    float midX = (left + right) / 2.0f;

    g.setColour(juce::Colour(AetherColours::inkGhost));

    // Left line
    g.drawLine(left, y, midX - 8.0f, y, 1.0f);

    // Diamond (5x5px rotated square, stroke only)
    juce::Path diamond;
    diamond.addRectangle(-2.5f, -2.5f, 5.0f, 5.0f);
    diamond.applyTransform(juce::AffineTransform::rotation(
        juce::MathConstants<float>::pi / 4.0f).translated(midX, y));
    g.strokePath(diamond, juce::PathStrokeType(1.0f));

    // Right line
    g.drawLine(midX + 8.0f, y, right, y, 1.0f);
}
```

### Switching createEditor to Custom UI

```cpp
// In PluginProcessor.cpp -- the one-line change to activate the custom editor
juce::AudioProcessorEditor* AetherProcessor::createEditor()
{
    return new AetherEditor(*this);
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| JUCE Font constructor `Font(name, size, style)` | `Font(FontOptions(typeface).withHeight(size))` | JUCE 8.x | FontOptions is the modern API. Old constructors still work but deprecated. |
| `setDefaultSansSerifTypefaceName()` | `FontOptions` with explicit typeface pointer | JUCE 8.x | Per-font typeface pointers are more precise than global default override. |
| Projucer for project setup | CMake with FetchContent | JUCE 7+ | Project already uses CMake. Projucer is legacy. |
| `Typeface::createSystemTypefaceFor(data, size)` | Same API, unchanged | JUCE 7+ | Still the correct way to load embedded fonts in JUCE 8.0.12. |
| `juce::Component::setAlpha()` for dimming | Custom paint with alpha overlay | Always | `setAlpha()` dims mouse interaction too. Paint-based dimming preserves interactivity. |

**Deprecated/outdated:**
- `Font(String, float, int)` constructor: Still works in JUCE 8 but FontOptions is preferred
- `LookAndFeel_V2::drawRotarySlider` default rendering: Replaced by V4 in JUCE 6+

## Open Questions

1. **Font weight variants: how many TTF files to embed?**
   - What we know: Design handoff specifies Light (300), Regular (400), and Italic variants across three font families. That's potentially 6-9 TTF files.
   - What's unclear: Whether all weight/style combinations are actually needed for the typography table, or if Regular + Italic per family (6 files) is sufficient.
   - Recommendation: Start with the minimum set that covers the typography table: Cormorant Garamond Light (title), EB Garamond Regular + Italic (values), Spectral Regular + Italic (labels, body). Add weight variants only if visual testing shows they're needed. Bold variants are NOT used anywhere in the spec.

2. **Stage I Material selector UI treatment**
   - What we know: `res_material` is a 10-choice `AudioParameterChoice` (Pine, Oak, Walnut, Mahogany, Iron, Steel, Copper, Limestone, Marble, Granite). The design handoff shows a 3-way toggle (Open/Closed/Combo).
   - What's unclear: 10 options is too many for inline toggle buttons in a 220px panel. A dropdown/ComboBox is the natural fit, but the handoff's visual language uses inline toggles.
   - Recommendation: Use a styled `juce::ComboBox` dropdown for Material, matching the preset selector styling (EB Garamond 11px, Ink Ghost border). This keeps the panel compact while showing all 10 options.

3. **Stage II Width knob placement**
   - What we know: `refl_width` (Width, 0-100%) exists in APVTS but is not in the design handoff center panel layout.
   - What's unclear: Should Width go below the 3 center knobs, or be moved to a side panel?
   - Recommendation: Add Width as a 4th knob in the center controls row below the visualization. Space it to the right of Proximity. At 64px each with 28px gap, 4 knobs need ~340px which fits in the ~460px center panel.

4. **Stage V Gate mode selector placement**
   - What we know: `tone_gate` (3-choice: Always On / Signal-Gated / Transport-Only) exists but isn't in handoff.
   - What's unclear: The left panel Stage V section currently only has an Ambience knob.
   - Recommendation: Place a 3-way toggle below the Ambience knob, similar to the Cabinet Type toggle treatment.

5. **Stage II Shape as dropdown vs knob**
   - What we know: `refl_shape` is a 7-choice `AudioParameterChoice` with named room shapes (The Parlour, The Gallery, etc.), not a continuous Regular-Irregular knob.
   - What's unclear: Should this be a 64px knob that snaps to 7 positions, or a dropdown selector?
   - Recommendation: Use a styled dropdown (`ComboBox`) since the 7 named shapes are discrete choices with evocative names. Display as "64% (The Nave)" format per the user's decision on text-label params.

## Discretion Recommendations

Based on research, here are recommendations for areas marked as Claude's discretion:

### Hover Feedback
**Recommendation:** Subtle cursor change to pointer on knobs. No visual hover glow -- maintains the "quiet confidence" design principle. Knob body border lightens from Ink Light to Ink on hover (barely noticeable but provides feedback). Bypass buttons increase to full opacity on hover (already specified in handoff).

### Right-click Context Menu
**Recommendation:** Simple right-click menu on knobs with: "Reset to Default", "Copy Value", "Paste Value". Uses JUCE PopupMenu with LookAndFeel override for parchment styling. Minimal -- most users never right-click plugin knobs.

### Center vs Side Knob Differentiation (64px vs 56px)
**Recommendation:** Yes, maintain the size difference. 64px for center panel knobs (Room Size, Shape, Proximity, Width) and 56px for side panel knobs. The size difference is subtle but reinforces the center panel as the "heart" of the plugin. Both use identical gradient/indicator styling, just scaled.

### Bypass Dimming Scope and Transition
**Recommendation:** When a section is bypassed, draw a translucent Parchment overlay at 40% opacity over the entire section area (below the section label row, which stays at full opacity). This creates visible but gentle dimming. Controls remain fully interactive underneath. Transition: 150ms ease-out opacity animation using `juce::Timer`.

### Additional Bypass Indicator
**Recommendation:** None beyond In/Out text. The dimming overlay provides sufficient visual signal. Adding more indicators (colored dots, icons) would conflict with the "moderate/balanced ornamental weight" decision.

### Percentage Precision
**Recommendation:** Whole numbers (no decimal places) for all percentage displays. "62%" not "62.0%". The Decay parameter shows "150ms" or "1.2s" (one decimal for seconds). Output Level shows one decimal: "0.0 dB", "-3.5 dB".

### Corner L-brackets
**Recommendation:** Include them at the handoff specification: 20x20px, 1px Ink Ghost stroke, 6px from edges. They're small enough to stay within "moderate/balanced" ornamental weight. They frame the parchment like a scientific document without being heavy.

## Sources

### Primary (HIGH confidence)
- `/Users/nathanmcmillan/Projects/Aether/Source/Parameters.h` -- actual 21-parameter APVTS layout (authoritative for parameter types, ranges, defaults)
- `/Users/nathanmcmillan/Projects/Aether/Source/PluginEditor.h/cpp` -- current editor shell (empty, 900x530, parchment fill)
- `/Users/nathanmcmillan/Projects/Aether/Source/PluginProcessor.h/cpp` -- processor with `apvts` member, currently returns GenericEditor
- `/Users/nathanmcmillan/Projects/Aether/CMakeLists.txt` -- build config, no binary data target yet, JUCE 8.0.12
- `/Users/nathanmcmillan/Downloads/Aether Files/AETHER-DESIGN-HANDOFF.md` -- definitive visual spec
- `/Users/nathanmcmillan/Downloads/Aether Files/aether-concept.html` -- HTML mockup reference
- `/Users/nathanmcmillan/Projects/Crucible/Source/ui/CrucibleLookAndFeel.h/cpp` -- proven Cairn LookAndFeel pattern
- `/Users/nathanmcmillan/Projects/Crucible/Source/ui/CrucibleColours.h` -- proven colour namespace pattern
- `/Users/nathanmcmillan/Projects/Crucible/Source/ui/ForgeElements.h/cpp` -- decorative helper pattern
- `/Users/nathanmcmillan/Projects/Crucible/Source/PluginEditor.h/cpp` -- proven editor layout pattern
- `/Users/nathanmcmillan/Projects/Crucible/CMakeLists.txt` -- `juce_add_binary_data` font embedding pattern
- `/Users/nathanmcmillan/Projects/brand/Docs/CAIRN-BRAND-REFERENCE.md` -- brand typography and colour guidelines
- `/Users/nathanmcmillan/Projects/brand/Docs/CAIRN-PLUGIN-DESIGN-SYSTEM.md` -- four-zone layout, control standards
- JUCE 8.0.12 source (local): `juce_Slider.h`, `juce_FontOptions.h`, `juce_LookAndFeel_V4.h` -- API verification

### Secondary (MEDIUM confidence)
- Design handoff control specifications (from original concept phase -- mostly aligned with Parameters.h but with some divergences documented above)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH -- JUCE 8.0.12 already in project, all APIs verified against local source
- Architecture: HIGH -- Crucible provides a complete, working reference implementation of the same patterns
- Pitfalls: HIGH -- Identified from direct code inspection of JUCE source and Crucible implementation experience
- Parameter mapping: HIGH -- Parameters.h read directly, divergences from handoff documented

**Research date:** 2026-02-18
**Valid until:** 2026-03-18 (stable -- JUCE 8 APIs are mature, font embedding hasn't changed)
