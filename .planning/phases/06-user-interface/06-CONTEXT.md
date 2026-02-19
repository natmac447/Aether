# Phase 6: User Interface - Context

**Gathered:** 2026-02-18
**Status:** Ready for planning

<domain>
## Phase Boundary

Victorian parchment-themed plugin UI with three-column layout (220px | flex | 220px), custom knob/toggle/bypass components, embedded typography, and ornamental elements. All controls connected to the existing 19-parameter APVTS. 900x530px fixed window. Visualization is Phase 7 — this phase builds the layout, controls, and decorative elements only.

</domain>

<decisions>
## Implementation Decisions

### Knob interaction
- Scroll wheel supported with fine increments (~1% per tick)
- Click-to-type on value readout for exact value entry; out-of-range values clamp silently
- Slower/more precise drag sensitivity (~300px for full sweep vs default ~200px)
- Shift+drag for fine control (10x slower); no Cmd+snap behavior
- Double-click resets to parameter's saved default value (not noon/50%)
- Smooth animated indicator sweep (~100-200ms) on preset load, double-click reset, and value jumps
- Real-time value readout update while dragging
- Hover feedback and right-click context menu: Claude's discretion
- Center knobs (64px) vs side panel knobs (56px) differentiation: Claude's discretion

### Section bypass treatment
- Controls remain fully interactive when section is bypassed (users can pre-dial settings before toggling In)
- Visual dimming scope and bypass transition animation: Claude's discretion (user trusts recommendation)
- Additional bypass indicator beyond In/Out text: Claude's discretion

### Value display & readout
- Text-label params (Room Size, Shape, Proximity) show percentage + nearest descriptor: "62% (Medium)"
- Decay knob switches display from ms to seconds above 1000ms: "150ms" below, "1.2s" above
- Percentage precision (whole numbers vs one decimal): Claude's discretion
- No min/max range labels at knob sweep endpoints — value readout only

### Ornamental weight
- Overall feel: moderate/balanced — ornaments clearly present but don't compete with controls, like a well-typeset Victorian publication page
- Paper texture: visible on close inspection (opacity ~0.015-0.02), faint ruled horizontal lines every 4px
- Edge vignette: noticeable warm darkening, visible ~30px inward, adds depth and framing
- Diamond dividers: functional separators — clean horizontal lines with small diamond, structural like a typographic rule
- Double-rule header/footer borders: traditional Victorian publication style — two distinct visible lines with 3px gap
- Section Roman numerals and section name at equal visual weight — "I. Cabinet Resonance" reads as one balanced line
- Corner L-brackets: Claude's discretion, tuned to match moderate/balanced weight

</decisions>

<specifics>
## Specific Ideas

- Design handoff at `/Users/nathanmcmillan/Downloads/Aether Files/AETHER-DESIGN-HANDOFF.md` is the definitive visual spec — colors, typography sizes, layout dimensions, control specs all defined there
- HTML mockup at `/Users/nathanmcmillan/Downloads/Aether Files/aether-concept.html` for visual reference
- Crucible plugin at `~/Projects/Crucible` is the most complete Cairn reference implementation
- Brand guidelines at `~/Projects/brand`
- Knob interaction should feel deliberate and precise — slower drag, smooth animations, click-to-type — consistent with the "crafted instrument" aesthetic
- Ornaments should evoke a Victorian acoustic science publication — functional typographic elements, not decorative clip art

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 06-user-interface*
*Context gathered: 2026-02-18*
