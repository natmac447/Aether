# Phase 7: Visualization - Research

**Researched:** 2026-02-18
**Domain:** JUCE custom component animation, programmatic 2D vector graphics, audio-thread-to-GUI communication
**Confidence:** HIGH

## Summary

Phase 7 adds a live acoustic visualization to the existing center column placeholder (currently a 340x260px `parchmentDark` rectangle at `(280, 68)` in `PluginEditor.cpp` line 383). The visualization is artistic/perceptual -- inspired by Victorian perception illustrations (psycho-magnetic curves, ripple tank photography, Descartes optics diagrams) -- not a functional room diagram. All rendering uses monochrome ink tones from the existing `AetherColours` palette drawn with JUCE's `juce::Path` and `juce::Graphics` APIs. No external libraries are needed.

The implementation requires: (1) a new `VisualizationComponent` that owns a `juce::Timer` for continuous repaint at ~30Hz, (2) an embedded SVG path for the Victorian head profile, (3) programmatic wavefront ring generation driven by parameter state read from `std::atomic<float>*` pointers, (4) per-stage visual element layers that respond to bypass state, and (5) an `std::atomic<float>` RMS level bridge from the processor for audio-reactive breathing boost. The existing codebase already demonstrates every pattern needed: `AetherKnob` uses `juce::Timer` + `startTimerHz(60)` for animation, `ParchmentElements` shows `juce::Path` construction and transforms, `AetherColours` provides the ink palette, and `PluginProcessor` already caches `std::atomic<float>*` parameter pointers for zero-overhead GUI access.

**Primary recommendation:** Build a single `VisualizationComponent` class (extending `juce::Component` and `juce::Timer`) that reads parameter state via atomic pointers, computes interpolated visual state, and repaints at 30Hz. Keep all geometry in `juce::Path` objects constructed once and transformed per frame. Use the existing `juce::Timer` pattern from `AetherKnob` rather than JUCE 8's animation module, for consistency with the codebase.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Art direction -- Perception over physics:**
- The visualization is artistic/perceptual, NOT a functional room diagram
- Inspired by Victorian perception illustrations: psycho-magnetic curves, anatomical sound studies, concentric wavefronts around the human form
- No wall labels ("Wall A", "Wall B"), no distance annotations (d1, d2, d3), no room boundary rectangle
- Reference images in `~/Projects/Aether/references/` -- key references: psycho-magnetic curves around a head profile, ripple tank wave diffraction photos, Descartes' "La Dioptrique" optics diagram
- Monochrome ink only -- all elements rendered in Ink/Ink Light/Ink Faint at varying opacities. No Accent Warm copper tints. Pure engraving aesthetic.

**Central figure:**
- Head in profile (side view) with concentric wavefronts radiating around it
- Head rendered as embedded SVG path data (pre-drawn Victorian illustration style) baked into code
- Dynamic wave elements rendered programmatically around the static head
- Caption below: "Fig. 1 -- The Listener" in EB Garamond italic

**Shape representation -- Wave patterns, no room boundary:**
- No visible room boundary drawn -- room shape expressed purely through wave pattern character
- Each of 7 room shapes has a distinct visual fingerprint through both symmetry AND density variation:
  - Regular shapes (Hall, Studio) = symmetric concentric rings
  - Irregular shapes (Crypt, Loft) = distorted/asymmetric rings
  - Distinctive shapes (Cathedral = tall/narrow, Tunnel = elongated) have unique character
- Switching shapes triggers a smooth morph transition (~300-500ms)

**Room Size effect:**
- Both wave reach/extent AND spacing change together
- Small room = tightly spaced waves extending only slightly past the head
- Large room = widely spaced waves filling the entire visualization area
- The visualization breathes with the room -- expansive at large sizes, intimate at small

**Proximity effect:**
- Source point position AND wave intensity balance shift together
- Near = source close to head, strong/opaque direct wavefronts, faint reflections
- Far = source distant, flatter/more parallel arriving waves, stronger reflected/scattered patterns
- Creates a visible shift from intimate to ambient

**Per-stage visual elements:**
- Reflections (Stage II): Concentric wavefronts -- direct and reflected wave patterns. The primary visual layer.
- Diffuse Tail (Stage VI): Scattered/diffuse wave patterns -- the ambient wash beyond the direct reflections.
- Air (Stage III): Increasingly dense small particles between the waves -- represents air absorption/diffusion. Density scales with Air amount.
- Excitation (Stage IV): Small spinning star/spark marks scattered across the wave field -- represents room energy/drive. Count/intensity scales with Drive amount.
- Cabinet Resonance (Stage I): No visual representation.
- Room Tone (Stage V): No visual representation.

**Bypass behavior:**
- When a stage is bypassed, its visual elements disappear entirely (smooth fade out)
- Not ghost traces -- clean removal, only active stages are visible
- When all spatial stages are bypassed, the head profile remains as the constant anchor

**Animation:**
- Always breathing with audio boost: subtle constant wavefront pulse (4-second cycle, ~1.0-1.02 scale), with increased amplitude/speed when audio is passing through
- Parameter changes trigger smooth morph transitions (~300-500ms), not snaps or crossfades
- Excitation stars spin continuously when active

### Claude's Discretion

- Frame rate for visualization repaint (balance smoothness vs CPU)
- Exact wave pattern details per room shape (the "fingerprint" for each of the 7 shapes)
- Star/spark visual design (size, rotation speed, distribution pattern)
- Air particle visual design (size, density mapping curve)
- How audio presence is detected for the breathing boost (RMS level, peak, etc.)
- SVG path data source for the head profile illustration
- Staggered animation delays per wavefront ring

### Deferred Ideas (OUT OF SCOPE)

None -- discussion stayed within phase scope
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| VIZ-01 | Center acoustic ray diagram (340x340px) showing top-down room propagation | VisualizationComponent placed in center column replacing placeholder; note: CONTEXT.md overrides "top-down" -- this is a perceptual diagram with head profile, not a room plan view. Actual viz area is ~340x260px per existing layout. |
| VIZ-02 | Diagram includes: room boundary, source point, listener indicator, concentric wavefronts, direct rays, reflection paths, wall reflection wavefronts (copper-tinted) | CONTEXT.md overrides several elements: NO room boundary, NO copper tints (monochrome ink only). Listener = head profile SVG. Wavefronts = concentric rings. Source point = positioned by Proximity. Reflections expressed through wave pattern character per shape. |
| VIZ-03 | Diagram reacts to Room Size (boundary scale, wavefront spacing), Shape (reflection angles), and Proximity (listener position) | Room Size -> wave extent + spacing. Shape -> wave pattern fingerprint (symmetry, distortion). Proximity -> source position + direct/reflected opacity balance. All via std::atomic parameter reads with smoothed interpolation. |
| VIZ-04 | Subtle breathing animation on wavefront rings (4-second cycle, 1.0->1.02 scale) | juce::Timer at 30Hz drives a sine-based breathing phase. Scale oscillates 1.0-1.02 baseline, boosted when audio RMS > threshold via std::atomic<float> level bridge from processor. |
| VIZ-05 | Bypassed stages fade their corresponding visual elements to ghost opacity | CONTEXT.md overrides "ghost opacity" -- bypassed elements disappear entirely (smooth fade to 0.0 alpha). Each visual layer has an independent opacity float smoothed over ~300ms. Bypass state read from existing std::atomic bypass params. |
</phase_requirements>

## Standard Stack

### Core

| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| JUCE (juce_gui_basics) | 8.0.12 | Component, Graphics, Path, Timer, AffineTransform | Already in project; all rendering via juce::Graphics::strokePath / fillPath |
| JUCE (juce_graphics) | 8.0.12 | Path construction, GlyphArrangement, ColourGradient | Already linked; Path::addEllipse, Path::applyTransform for wavefront rings |

### Supporting

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| AetherColours (internal) | - | Ink/Parchment colour palette | All drawing colours sourced from here: ink, inkLight, inkFaint, inkGhost |
| AetherLookAndFeel (internal) | - | Font access (EB Garamond italic for caption) | getBodyFontItalic(size) for "Fig. 1 -- The Listener" caption |
| ParchmentElements (internal) | - | drawLetterSpacedText for caption rendering | Letter-spaced caption text below visualization |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| juce::Timer at 30Hz | JUCE 8 VBlankAnimatorUpdater | VBlank syncs to display refresh (smoother) but adds complexity. Timer is consistent with existing AetherKnob pattern and sufficient for breathing animation. |
| juce::Timer at 30Hz | JUCE 8 ValueAnimatorBuilder | More declarative API but unfamiliar pattern in this codebase. Stick with Timer for consistency. |
| Software rendering | OpenGL | GPU acceleration helps at higher resolutions but adds build complexity. 340x260px at 30Hz is well within software rendering budget. |

**Installation:**
No new dependencies required. All capabilities exist in the already-linked JUCE modules.

## Architecture Patterns

### Recommended Project Structure

```
Source/
  ui/
    VisualizationComponent.h    # New: visualization widget
    VisualizationComponent.cpp  # New: all rendering + animation logic
    AetherColours.h             # Existing: colour palette
    ParchmentElements.h         # Existing: letter-spaced text
  PluginEditor.h                # Modified: owns VisualizationComponent
  PluginEditor.cpp              # Modified: places viz in center column
  PluginProcessor.h             # Modified: adds std::atomic<float> rmsLevel
  PluginProcessor.cpp           # Modified: computes RMS in processBlock
```

### Pattern 1: Timer-Driven Animation Component

**What:** A juce::Component subclass that also inherits juce::Timer, calls repaint() in timerCallback(), and reads parameter state from std::atomic pointers.

**When to use:** For continuous animation that must react to parameter changes without blocking the audio thread.

**Example:**
```cpp
// Follows existing AetherKnob timer pattern
class VisualizationComponent : public juce::Component,
                                private juce::Timer
{
public:
    VisualizationComponent();
    ~VisualizationComponent() override { stopTimer(); }

    void startAnimation() { startTimerHz (30); }
    void stopAnimation()  { stopTimer(); }

    // Called by editor to provide atomic param pointers
    void setParameterSources (std::atomic<float>* roomSize,
                              std::atomic<float>* shape,
                              std::atomic<float>* proximity,
                              std::atomic<float>* airAmount,
                              std::atomic<float>* excitDrive,
                              std::atomic<float>* reflBypass,
                              std::atomic<float>* airBypass,
                              std::atomic<float>* excitBypass,
                              std::atomic<float>* tailBypass,
                              std::atomic<float>* rmsLevel);

    void paint (juce::Graphics& g) override;

private:
    void timerCallback() override
    {
        updateSmoothedState();
        repaint();
    }

    void updateSmoothedState();

    // Smoothed visual state (interpolated each frame)
    float smoothedSize     = 0.4f;
    float smoothedProx     = 0.3f;
    float smoothedAir      = 0.4f;
    float smoothedDrive    = 0.25f;
    float smoothedReflAlpha = 1.0f;  // bypass fade
    float smoothedAirAlpha  = 1.0f;
    float smoothedExcitAlpha = 1.0f;
    float smoothedTailAlpha  = 1.0f;

    // Animation phase
    float breathPhase = 0.0f;  // 0..2pi, advances each frame
    float starRotation = 0.0f; // continuous spin for excitation stars

    // Atomic param pointers (set once, read every frame)
    std::atomic<float>* pRoomSize  = nullptr;
    std::atomic<float>* pShape     = nullptr;
    // ... etc
};
```

### Pattern 2: Smoothed Parameter Interpolation

**What:** Each visual parameter maintains a "current" float that exponentially approaches its "target" (read from std::atomic) each frame. This creates the smooth morph transitions (~300-500ms) required by the design.

**When to use:** Every parameter that affects the visualization.

**Example:**
```cpp
void VisualizationComponent::updateSmoothedState()
{
    // Smoothing coefficient: at 30Hz, 0.12 gives ~300ms settle time
    constexpr float kSmooth = 0.12f;
    constexpr float kBypassSmooth = 0.15f;  // slightly faster for bypass fade

    if (pRoomSize)  smoothedSize  += kSmooth * (pRoomSize->load()  - smoothedSize);
    if (pProximity) smoothedProx  += kSmooth * (pProximity->load() - smoothedProx);
    if (pAirAmount) smoothedAir   += kSmooth * (pAirAmount->load() - smoothedAir);
    if (pExcitDrive) smoothedDrive += kSmooth * (pExcitDrive->load() - smoothedDrive);

    // Bypass: target 0.0 when bypassed, 1.0 when active
    float reflTarget = (pReflBypass && pReflBypass->load() >= 0.5f) ? 0.0f : 1.0f;
    smoothedReflAlpha += kBypassSmooth * (reflTarget - smoothedReflAlpha);
    // ... repeat for air, excit, tail

    // Breathing phase: 4-second cycle = 2*pi / (4.0 * 30Hz) per frame
    constexpr float kBreathIncrement = juce::MathConstants<float>::twoPi / (4.0f * 30.0f);
    float rms = pRmsLevel ? pRmsLevel->load() : 0.0f;
    float speedBoost = 1.0f + rms * 0.5f;  // audio makes breathing faster
    breathPhase += kBreathIncrement * speedBoost;
    if (breathPhase > juce::MathConstants<float>::twoPi)
        breathPhase -= juce::MathConstants<float>::twoPi;

    // Star rotation: continuous spin
    starRotation += 0.02f;  // ~0.6 rad/sec at 30Hz
}
```

### Pattern 3: Layered Drawing with Per-Layer Alpha

**What:** The paint() method draws visual layers in order, each with its own opacity controlled by bypass state. The head profile is the base layer (always visible), then reflections wavefronts, then diffuse scatter, then air particles, then excitation stars.

**When to use:** To implement the "bypassed elements disappear entirely" requirement with smooth fade transitions.

**Example:**
```cpp
void VisualizationComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Layer 1: Head profile (always visible, constant anchor)
    drawHeadProfile (g, bounds);

    // Layer 2: Reflection wavefronts (Stage II)
    if (smoothedReflAlpha > 0.001f)
    {
        g.saveState();
        g.reduceClipRegion (getLocalBounds());
        drawReflectionWavefronts (g, bounds, smoothedReflAlpha);
        g.restoreState();
    }

    // Layer 3: Diffuse tail scatter (Stage VI)
    if (smoothedTailAlpha > 0.001f)
        drawDiffuseScatter (g, bounds, smoothedTailAlpha);

    // Layer 4: Air particles (Stage III)
    if (smoothedAirAlpha > 0.001f && smoothedAir > 0.01f)
        drawAirParticles (g, bounds, smoothedAirAlpha);

    // Layer 5: Excitation stars (Stage IV)
    if (smoothedExcitAlpha > 0.001f && smoothedDrive > 0.01f)
        drawExcitationStars (g, bounds, smoothedExcitAlpha);

    // Caption: "Fig. 1 -- The Listener" (always visible)
    drawCaption (g, bounds);
}
```

### Pattern 4: Audio Level Bridge (Processor -> GUI)

**What:** The processor computes a simple RMS level in processBlock and stores it in an std::atomic<float>. The visualization component reads it each frame to modulate breathing amplitude/speed. Uses compare-and-swap to keep the peak value between GUI reads.

**When to use:** For the audio-reactive breathing boost.

**Example:**
```cpp
// In PluginProcessor.h -- add:
std::atomic<float> visualizationRmsLevel { 0.0f };

// In PluginProcessor.cpp -- processBlock, after output trim:
{
    float rms = buffer.getRMSLevel (0, 0, buffer.getNumSamples());
    if (getTotalNumInputChannels() > 1)
        rms = std::max (rms, buffer.getRMSLevel (1, 0, buffer.getNumSamples()));

    // Keep peak since last GUI read (compare-and-swap)
    float expected = visualizationRmsLevel.load();
    while (rms > expected &&
           !visualizationRmsLevel.compare_exchange_weak (expected, rms))
    {}
}

// In VisualizationComponent::updateSmoothedState:
float rms = pRmsLevel ? pRmsLevel->exchange (0.0f) : 0.0f;
// Use rms to boost breathing amplitude and speed
```

### Pattern 5: Shape Fingerprint System

**What:** A data-driven approach where each of the 7 room shapes is defined by a struct containing wave distortion parameters. The current shape struct interpolates toward the target during transitions.

**When to use:** To give each room shape its distinctive visual character without 7 separate drawing functions.

**Example:**
```cpp
struct ShapeFingerprint
{
    float xStretch;      // horizontal stretch factor (1.0 = circular)
    float yStretch;      // vertical stretch factor
    float asymmetry;     // 0.0 = symmetric, 1.0 = maximum distortion
    float densityBias;   // ring spacing modifier (1.0 = even, <1 = compressed inner)
    float angularWarp;   // angular distortion amount for irregular shapes
    int   ringCount;     // base number of wavefront rings
};

// 7 shapes mapped to the combo box indices:
static constexpr ShapeFingerprint kShapeFingerprints[7] = {
    // The Parlour:  Small, regular, symmetric
    { 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 5 },
    // The Gallery:  Wide, slightly elongated
    { 1.3f, 0.85f, 0.1f, 0.9f, 0.05f, 6 },
    // The Chamber:  Medium, balanced
    { 1.0f, 1.0f, 0.05f, 1.0f, 0.02f, 6 },
    // The Nave:     Tall, narrow (cathedral-like)
    { 0.7f, 1.4f, 0.15f, 0.85f, 0.1f, 7 },
    // The Alcove:   Compact, slightly irregular
    { 1.1f, 0.9f, 0.2f, 1.1f, 0.15f, 4 },
    // The Crypt:    Irregular, compressed, distorted
    { 1.0f, 0.8f, 0.4f, 1.2f, 0.3f, 5 },
    // The Conservatory: Wide, bright, open
    { 1.2f, 1.1f, 0.1f, 0.8f, 0.05f, 7 },
};
```

### Anti-Patterns to Avoid

- **Constructing Path objects in paint():** Path construction allocates memory. Build base paths once (in constructor or when shape changes) and transform them per-frame with `applyTransform()`.
- **Reading APVTS parameters by string ID in paint():** Use cached `std::atomic<float>*` pointers, not `apvts.getRawParameterValue(id)` calls every frame.
- **Timer at 60Hz for the visualization:** 60Hz is overkill for a breathing animation. 30Hz is smooth enough and halves CPU cost. The existing knob animation uses 60Hz but only runs during brief transitions; the visualization runs continuously.
- **Drawing to a cached Image and recompositing:** The visualization changes every frame (breathing), so caching gains nothing. Draw directly with `juce::Graphics`.
- **Using JUCE 8 animation module (ValueAnimatorBuilder):** While more modern, this pattern is not used elsewhere in the Aether codebase. Mixing animation paradigms creates maintenance burden. Stick with `juce::Timer`.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| SVG path parsing | Custom SVG parser | `juce::Drawable::createFromSVG()` or raw `juce::Path` data | JUCE handles SVG path `d` attributes; or use path coordinates directly as float arrays |
| Letter-spaced caption | Manual char positioning | `ParchmentElements::drawLetterSpacedText()` | Already implemented, tested, matches existing typography |
| Exponential smoothing | Custom smoothing class | Inline `current += coeff * (target - current)` | Pattern already used in `AetherKnob::timerCallback()`, trivial inline math |
| Colour management | Hardcoded hex values | `AetherColours::ink`, `inkLight`, `inkFaint`, `inkGhost` | Palette consistency; all existing components use this |
| Font access | Direct typeface loading | `AetherLookAndFeel::getBodyFontItalic()` etc. | Fonts already embedded and loaded in LookAndFeel |

**Key insight:** The visualization needs zero external dependencies. Every building block exists in JUCE's `juce_gui_basics` and `juce_graphics` modules, and the project's existing UI infrastructure provides colours, fonts, and rendering helpers.

## Common Pitfalls

### Pitfall 1: Timer Left Running When Editor Destroyed

**What goes wrong:** If the visualization timer keeps running after the editor is destroyed, timerCallback fires on a dangling pointer.
**Why it happens:** JUCE editors are created/destroyed when the DAW plugin window opens/closes. The timer must stop before the component is deleted.
**How to avoid:** Stop the timer in the destructor (as `AetherKnob` does: `stopTimer()` in `~AetherKnob`). Also stop when the component is removed from the parent (`~VisualizationComponent` calls `stopTimer()`).
**Warning signs:** Crash on closing plugin window, specifically in `timerCallback()`.

### Pitfall 2: Audio Thread Contention on Parameter Reads

**What goes wrong:** Using `apvts.getParameter(id)->getValue()` in paint() involves virtual dispatch and potential locking.
**Why it happens:** `getParameter()` returns a `RangedAudioParameter*` which may involve tree traversal.
**How to avoid:** Cache `std::atomic<float>*` pointers (via `apvts.getRawParameterValue()`) once at construction and `.load()` them in the timer callback. This is the exact pattern used by `PluginProcessor::updateStageParams()`.
**Warning signs:** UI stutter when audio thread is under high load.

### Pitfall 3: Repainting Entire Editor Instead of Just the Visualization

**What goes wrong:** If `VisualizationComponent` is a child component but `repaint()` is called on the parent editor, it repaints the entire 900x530px window at 30Hz.
**Why it happens:** JUCE repaints the component and all its children when `repaint()` is called.
**How to avoid:** Make `VisualizationComponent` a separate child component and call `repaint()` only on itself. JUCE will only repaint its bounds (~340x260px).
**Warning signs:** High CPU usage (30%+) even when the visualization appears simple.

### Pitfall 4: Shape Transition Artifacts

**What goes wrong:** When switching shapes, wavefront rings jump to new positions instantly, creating visual discontinuity.
**Why it happens:** Shape index is a discrete parameter (0-6). If the visualization reads the integer and switches immediately, there's no interpolation.
**How to avoid:** Maintain two ShapeFingerprint structs (`currentShape` and `targetShape`) and interpolate between them over ~300-500ms when the shape index changes. Detect shape change by comparing `int(pShape->load())` to `lastShapeIndex`.
**Warning signs:** Jarring visual pop when switching room shapes.

### Pitfall 5: SVG Head Path Scaling Issues

**What goes wrong:** The embedded SVG path data renders at the wrong size or position, or appears distorted at different DPI scales.
**Why it happens:** SVG paths have their own coordinate system. If the original artboard is 200x200 and the component is 340x260, direct drawing will be mispositioned.
**How to avoid:** Define the head path in a normalized coordinate system (e.g., 0-100), then apply `AffineTransform::scale()` and `AffineTransform::translation()` to map to the component's actual bounds. Compute the transform once in `resized()`, not per frame.
**Warning signs:** Head appears in the wrong position, too large, or too small.

### Pitfall 6: Excessive Path Complexity Causing Stutter

**What goes wrong:** Drawing 15+ concentric ellipses with varying stroke widths and opacities at 30Hz causes frame drops.
**Why it happens:** Each `strokePath()` call is relatively expensive in software rendering. The cost scales linearly with path complexity and number of draw calls.
**How to avoid:** Limit total wavefront rings to 8-10 maximum. Use thin (0.5-1.0px) stroke widths. Combine rings into a single path where possible (multiple sub-paths in one `juce::Path`). Skip drawing rings with alpha < 0.02 (invisible anyway).
**Warning signs:** Frame drops visible as stuttery breathing, measurable via JUCE performance monitor.

## Code Examples

### Drawing Concentric Wavefront Rings

```cpp
// Construct wavefront ring paths with shape distortion
void VisualizationComponent::drawReflectionWavefronts (
    juce::Graphics& g, juce::Rectangle<float> bounds, float layerAlpha)
{
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY() - 10.0f;  // head center, slightly above visual center

    // Breathing scale: 1.0 to 1.02 baseline, boosted by audio
    float breathScale = 1.0f + 0.02f * (0.5f + 0.5f * std::sin (breathPhase));
    if (currentRmsSmoothed > 0.01f)
        breathScale += 0.01f * currentRmsSmoothed;  // audio boost

    // Room size controls extent and spacing
    float maxRadius = 20.0f + smoothedSize * 140.0f;  // 20-160px range
    float ringSpacing = 8.0f + smoothedSize * 12.0f;   // 8-20px spacing
    int ringCount = currentFingerprint.ringCount;

    // Source position from proximity
    float sourceDistance = 30.0f + (1.0f - smoothedProx) * 80.0f;
    float sourceX = cx - sourceDistance;  // source to the left of head

    for (int i = 0; i < ringCount; ++i)
    {
        float t = static_cast<float> (i) / static_cast<float> (ringCount - 1);
        float radius = ringSpacing * (i + 1) * breathScale;
        if (radius > maxRadius) break;

        // Shape distortion: stretch and warp
        float xR = radius * currentFingerprint.xStretch;
        float yR = radius * currentFingerprint.yStretch;

        // Opacity: outer rings fainter, proximity affects direct vs reflected balance
        float directAlpha = (1.0f - t * 0.6f) * smoothedProx;
        float reflectedAlpha = t * 0.5f * (1.0f - smoothedProx);
        float ringAlpha = juce::jlimit (0.0f, 1.0f, (directAlpha + reflectedAlpha)) * layerAlpha;

        if (ringAlpha < 0.02f) continue;

        // Draw as ellipse path
        juce::Path ring;
        ring.addEllipse (-xR, -yR, xR * 2.0f, yR * 2.0f);

        // Apply angular warp for irregular shapes
        // ... (shape-specific distortion applied via path transform)

        auto transform = juce::AffineTransform::translation (sourceX, cy);
        g.setColour (juce::Colour (AetherColours::inkLight).withAlpha (ringAlpha));
        g.strokePath (ring, juce::PathStrokeType (0.8f), transform);
    }
}
```

### Embedded Head Profile as Path Data

```cpp
// Head profile path: Victorian illustration style, side view
// Coordinates in normalized 0-100 space, scaled to component bounds
static juce::Path createHeadProfilePath()
{
    juce::Path head;
    // Example: simplified profile silhouette
    // In practice, trace from reference illustration or use SVG path d="" string
    head.startNewSubPath (50.0f, 10.0f);   // crown
    head.cubicTo (65.0f, 10.0f, 75.0f, 20.0f, 78.0f, 35.0f);  // forehead
    head.cubicTo (80.0f, 42.0f, 78.0f, 50.0f, 72.0f, 55.0f);  // brow to nose
    head.lineTo (75.0f, 58.0f);   // nose tip
    head.cubicTo (73.0f, 62.0f, 70.0f, 63.0f, 68.0f, 62.0f);  // nose to lip
    head.cubicTo (68.0f, 65.0f, 65.0f, 68.0f, 62.0f, 70.0f);  // chin
    head.cubicTo (55.0f, 78.0f, 48.0f, 80.0f, 45.0f, 85.0f);  // jawline
    head.cubicTo (42.0f, 88.0f, 40.0f, 92.0f, 40.0f, 95.0f);  // neck
    head.lineTo (38.0f, 95.0f);   // neck back
    head.cubicTo (38.0f, 88.0f, 35.0f, 80.0f, 32.0f, 72.0f);  // back of head
    head.cubicTo (28.0f, 55.0f, 30.0f, 35.0f, 38.0f, 20.0f);  // skull
    head.cubicTo (42.0f, 14.0f, 46.0f, 10.0f, 50.0f, 10.0f);  // back to crown
    head.closeSubPath();

    // Ear detail (small arc on the left side)
    head.startNewSubPath (34.0f, 48.0f);
    head.cubicTo (30.0f, 50.0f, 30.0f, 56.0f, 34.0f, 58.0f);

    return head;
}
```

### Excitation Star/Spark Drawing

```cpp
// Draw a small star shape at a given position
static void drawStar (juce::Graphics& g, float cx, float cy,
                       float outerRadius, float innerRadius,
                       int numPoints, float rotation, float alpha)
{
    if (alpha < 0.02f) return;

    juce::Path star;
    for (int i = 0; i < numPoints * 2; ++i)
    {
        float angle = rotation + static_cast<float> (i) * juce::MathConstants<float>::pi
                      / static_cast<float> (numPoints);
        float r = (i % 2 == 0) ? outerRadius : innerRadius;
        float px = cx + r * std::cos (angle);
        float py = cy + r * std::sin (angle);

        if (i == 0) star.startNewSubPath (px, py);
        else        star.lineTo (px, py);
    }
    star.closeSubPath();

    g.setColour (juce::Colour (AetherColours::ink).withAlpha (alpha));
    g.strokePath (star, juce::PathStrokeType (0.6f));
}

// In drawExcitationStars: scatter N stars across wave field
void VisualizationComponent::drawExcitationStars (
    juce::Graphics& g, juce::Rectangle<float> bounds, float layerAlpha)
{
    int starCount = static_cast<int> (3.0f + smoothedDrive * 8.0f);  // 3-11 stars
    float starSize = 2.0f + smoothedDrive * 2.0f;  // 2-4px radius

    // Deterministic positions based on index (not random per frame!)
    for (int i = 0; i < starCount; ++i)
    {
        float angle = static_cast<float> (i) * 2.39996f;  // golden angle
        float dist = 30.0f + static_cast<float> (i * 13 % 7) * 15.0f;
        float sx = bounds.getCentreX() + dist * std::cos (angle);
        float sy = bounds.getCentreY() + dist * std::sin (angle);
        float starAlpha = layerAlpha * (0.4f + smoothedDrive * 0.6f);

        drawStar (g, sx, sy, starSize, starSize * 0.4f, 4,
                  starRotation + static_cast<float> (i) * 0.5f, starAlpha);
    }
}
```

### Air Particle Drawing

```cpp
void VisualizationComponent::drawAirParticles (
    juce::Graphics& g, juce::Rectangle<float> bounds, float layerAlpha)
{
    // Particle count scales with Air amount: 0 at Air=0%, ~40 at Air=100%
    int particleCount = static_cast<int> (smoothedAir * 40.0f);
    float particleSize = 1.0f;  // 1px dots

    auto inkFaint = juce::Colour (AetherColours::inkFaint);

    for (int i = 0; i < particleCount; ++i)
    {
        // Deterministic scatter using hash-like distribution
        float hash1 = std::sin (static_cast<float> (i) * 127.1f) * 43758.5453f;
        float hash2 = std::sin (static_cast<float> (i) * 269.5f) * 43758.5453f;
        float px = bounds.getX() + (hash1 - std::floor (hash1)) * bounds.getWidth();
        float py = bounds.getY() + (hash2 - std::floor (hash2)) * bounds.getHeight();

        // Slight drift with breathing phase
        px += 1.5f * std::sin (breathPhase + static_cast<float> (i) * 0.3f);
        py += 1.0f * std::cos (breathPhase + static_cast<float> (i) * 0.7f);

        float alpha = layerAlpha * (0.15f + smoothedAir * 0.35f);
        g.setColour (inkFaint.withAlpha (alpha));
        g.fillEllipse (px, py, particleSize, particleSize);
    }
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| juce::Timer + repaint() | JUCE 8 VBlankAnimatorUpdater | JUCE 8 (2024) | Smoother animations synced to display; however Timer remains fully supported |
| ComponentAnimator (affine transforms only) | ValueAnimatorBuilder + AnimatorSetBuilder | JUCE 8 (2024) | More flexible, declarative animation composition |
| OpenGL for plugin UI | Software rendering (improved in JUCE 8) | JUCE 8 (2024) | Direct2D on Windows, Metal hints on macOS; software path is faster than before |

**Deprecated/outdated:**
- `ComponentAnimator` for custom property animations -- still works but `juce_animation` module is the modern replacement
- `AnimatedAppComponent` as base class -- unnecessary weight; a plain `Component` + `Timer` is cleaner

## Discretion Recommendations

### Frame Rate: 30Hz

**Recommendation:** 30Hz (startTimerHz(30)).

**Rationale:** The breathing animation is a 4-second sinusoidal pulse with only 2% scale range. At 30Hz, each frame advances the breathing phase by ~3 degrees -- imperceptibly smooth for a slow oscillation. Parameter morph transitions at 300-500ms get 9-15 interpolation frames at 30Hz, which is more than enough for smooth visual transitions. 60Hz would double CPU cost with no perceptible benefit. The AetherKnob uses 60Hz but only during brief animation bursts, not continuously.

### Wave Pattern Details Per Room Shape

**Recommendation:** Use a `ShapeFingerprint` data struct with 6 parameters (xStretch, yStretch, asymmetry, densityBias, angularWarp, ringCount) that create unique visual character per shape. See the Pattern 5 example above for specific values.

**Rationale:** Data-driven approach means adding or tuning shapes requires only changing numbers, not code. The 6 parameters cover the full range from symmetric (Parlour) to heavily distorted (Crypt) to elongated (Nave). Interpolation between shapes is trivial -- lerp each float independently.

### Star/Spark Visual Design

**Recommendation:** 4-pointed stars (8 vertices), outer radius 2-4px scaled by Drive, inner radius at 40% of outer. Continuous rotation at ~0.02 rad/frame (~0.6 rad/sec). Positions distributed using the golden angle (2.39996 rad) for uniform scatter without clustering. Count: 3 at Drive=0% (minimum visible), 11 at Drive=100%.

**Rationale:** 4-pointed stars read as "sparks" at small sizes and are fast to draw (8 line segments). Golden angle distribution prevents visual clustering regardless of count. The ink-stroke aesthetic means outlined stars, not filled.

### Air Particle Visual Design

**Recommendation:** 1px filled circles scattered deterministically across the wave field. Count scales linearly: 0 particles at Air=0%, 40 at Air=100%. Opacity: inkFaint at 15-50% alpha. Subtle position drift synced to breathing phase for organic movement.

**Rationale:** 1px dots at low opacity evoke dust motes / atmospheric particles without competing with the wavefront lines. Deterministic positioning (hash function of index) prevents the "snowfall" effect of random repositioning per frame. Drift with breathing creates subtle organic motion.

### Audio Presence Detection

**Recommendation:** Per-block RMS level computed from the output buffer in processBlock, stored via compare-and-swap (keep peak between GUI reads). The visualization reads with `exchange(0.0f)` once per frame. Smooth the RMS value with a one-pole filter in the visualization's updateSmoothedState (attack ~50ms, release ~500ms).

**Rationale:** RMS is simple and meaningful for this purpose -- we only need "is audio flowing?" not precise level measurement. Compare-and-swap prevents the GUI from missing brief transients between reads. Asymmetric smoothing (fast attack, slow release) makes the breathing respond quickly to audio onset but decay gracefully during silence.

### SVG Head Profile Source

**Recommendation:** Hand-trace a simplified Victorian profile silhouette as raw `juce::Path` cubic Bezier commands. Keep it to ~15-20 path points for a clean, engraving-style outline. Draw as 1px ink stroke, not filled. The profile should face right (matching the psycho-magnetic curves reference image). Position centered vertically, offset slightly left of center horizontally to leave room for wavefronts expanding to the right.

**Rationale:** Embedding raw path coordinates (not SVG XML) avoids XML parsing overhead and keeps the implementation self-contained. A simplified silhouette (forehead, nose, lips, chin, jawline, back of skull, ear) is sufficient to read as "head in profile" at 340x260px. The Victorian references show clean outlines, not detailed anatomical rendering.

### Staggered Animation Delays

**Recommendation:** Each wavefront ring's breathing phase is offset by `ringIndex * 0.3` radians, creating a ripple-out effect where inner rings pulse slightly before outer rings.

**Rationale:** This creates the visual impression of waves emanating outward from the source, consistent with the psycho-magnetic curves reference. The 0.3 radian offset means consecutive rings are ~17 degrees out of phase -- visible but subtle. This is trivially implemented by adding `i * 0.3f` to the breathing phase when computing each ring's scale.

## Open Questions

1. **Exact visualization bounds after CONTEXT.md update**
   - What we know: The current placeholder is at `(280, 68, 340, 260)` in PluginEditor.cpp. The CONTEXT.md says ~340x340px. The current editor layout leaves 260px vertical height between the section label and the shape dropdown.
   - What's unclear: Whether the visualization should expand to fill more vertical space, or remain at the existing 340x260px footprint.
   - Recommendation: Use the existing 340x260px area. The "340x340" in the requirements may have been aspirational. The actual layout constraint is set by the center column's content. Verify during implementation and adjust if needed.

2. **Head profile path data fidelity**
   - What we know: The head should be Victorian illustration style, profile facing right, rendered as ink stroke.
   - What's unclear: Exact level of detail -- simple silhouette (10 points) vs detailed illustration (50+ points).
   - Recommendation: Start with a clean ~20-point silhouette. If it looks too simple, add detail iteratively. The psycho-magnetic curves reference shows a moderately detailed face with flowing contour lines -- aim for that level of recognizability.

3. **Caption text exact placement**
   - What we know: "Fig. 1 -- The Listener" in EB Garamond italic below the visualization. The current placeholder draws "Fig. 1" at y=340.
   - What's unclear: Whether the caption should include "The Listener" subtitle or just "Fig. 1".
   - Recommendation: Use "Fig. 1 -- The Listener" per CONTEXT.md. Place below visualization area using existing drawLetterSpacedText.

## Sources

### Primary (HIGH confidence)

- Aether codebase exploration: `PluginEditor.cpp`, `AetherKnob.cpp`, `ParchmentElements.cpp`, `PluginProcessor.cpp`, `AetherColours.h`, `Parameters.h`, `CMakeLists.txt` -- all read directly
- JUCE 8.0.12 API: `Path`, `Graphics`, `Timer`, `AffineTransform`, `Component` -- verified against project's linked version
- Reference images in `/Users/nathanmcmillan/Projects/Aether/References/` -- psycho-magnetic curves (1507e51b), ripple tank diffraction (a816536f, 7881c997), Descartes La Dioptrique (e6fa5749), Victorian head profile (a7a1a4c8), harmonic curves (c572c150)

### Secondary (MEDIUM confidence)

- [JUCE Animation Module overview](https://juce.com/blog/juce-8-feature-overview-animation-module/) -- VBlankAnimatorUpdater, ValueAnimatorBuilder patterns
- [JUCE Path class reference](https://docs.juce.com/master/classPath.html) -- addEllipse, applyTransform, cubicTo
- [JUCE AnimatedAppComponent](https://docs.juce.com/master/classAnimatedAppComponent.html) -- setFramesPerSecond pattern
- [JUCE Drawable::createFromSVG](https://docs.juce.com/master/classDrawable.html) -- SVG parsing capability
- [Melatonin: Dealing with jank in JUCE](https://melatonin.dev/blog/dealing-with-jank-in-juce/) -- repaint performance analysis
- [JUCE Forum: Timer-based repaint](https://forum.juce.com/t/using-a-timer-to-repaint-gui/23626) -- patterns and pitfalls
- [JUCE Forum: Repaint performance](https://forum.juce.com/t/repaint-terrible-performance-why/59808) -- CPU cost of frequent repaints
- [JUCE Forum: VBlankAttachment](https://forum.juce.com/t/vblankattachment/55026) -- modern alternative to Timer

### Tertiary (LOW confidence)

- [Rendering performance optimization](https://medium.com/@akaztp/journey-into-audio-programming-12-optimizing-rendering-performance-fd44d286588c) -- general guidance, not JUCE 8 specific
- [KVR: Real-time audio display](https://www.kvraudio.com/forum/viewtopic.php?t=335974) -- community patterns for visualization

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH -- no external dependencies, all JUCE built-in, verified against project's JUCE 8.0.12
- Architecture: HIGH -- all patterns derived from existing codebase patterns (AetherKnob timer, ParchmentElements path drawing, PluginProcessor atomic params)
- Pitfalls: HIGH -- based on direct codebase analysis and JUCE forum documentation of common issues
- Art direction: HIGH -- locked decisions from CONTEXT.md with reference images examined
- Discretion recommendations: MEDIUM -- reasonable recommendations based on reference analysis and performance considerations, but tuning values will need iteration during implementation

**Research date:** 2026-02-18
**Valid until:** 2026-03-18 (stable domain -- JUCE version pinned, art direction locked)
