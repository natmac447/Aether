# Phase 7: Visualization - Context

**Gathered:** 2026-02-18
**Status:** Ready for planning

<domain>
## Phase Boundary

Live acoustic ray diagram in the center column that visually represents the room environment and reacts to parameter changes. Fills the existing visualization placeholder area (~340x340px) in the editor's center panel. Includes breathing animation and bypass-reactive visual elements.

</domain>

<decisions>
## Implementation Decisions

### Art direction — Perception over physics
- The visualization is artistic/perceptual, NOT a functional room diagram
- Inspired by Victorian perception illustrations: psycho-magnetic curves, anatomical sound studies, concentric wavefronts around the human form
- No wall labels ("Wall A", "Wall B"), no distance annotations (d1, d2, d3), no room boundary rectangle
- Reference images in `~/Projects/Aether/references/` — key references: psycho-magnetic curves around a head profile, ripple tank wave diffraction photos, Descartes' "La Dioptrique" optics diagram
- Monochrome ink only — all elements rendered in Ink/Ink Light/Ink Faint at varying opacities. No Accent Warm copper tints. Pure engraving aesthetic.

### Central figure
- Head in profile (side view) with concentric wavefronts radiating around it
- Head rendered as embedded SVG path data (pre-drawn Victorian illustration style) baked into code
- Dynamic wave elements rendered programmatically around the static head
- Caption below: "Fig. 1 — The Listener" in EB Garamond italic

### Shape representation — Wave patterns, no room boundary
- No visible room boundary drawn — room shape expressed purely through wave pattern character
- Each of 7 room shapes has a distinct visual fingerprint through both symmetry AND density variation:
  - Regular shapes (Hall, Studio) = symmetric concentric rings
  - Irregular shapes (Crypt, Loft) = distorted/asymmetric rings
  - Distinctive shapes (Cathedral = tall/narrow, Tunnel = elongated) have unique character
- Switching shapes triggers a smooth morph transition (~300-500ms)

### Room Size effect
- Both wave reach/extent AND spacing change together
- Small room = tightly spaced waves extending only slightly past the head
- Large room = widely spaced waves filling the entire visualization area
- The visualization breathes with the room — expansive at large sizes, intimate at small

### Proximity effect
- Source point position AND wave intensity balance shift together
- Near = source close to head, strong/opaque direct wavefronts, faint reflections
- Far = source distant, flatter/more parallel arriving waves, stronger reflected/scattered patterns
- Creates a visible shift from intimate to ambient

### Per-stage visual elements
- **Reflections (Stage II):** Concentric wavefronts — direct and reflected wave patterns. The primary visual layer.
- **Diffuse Tail (Stage VI):** Scattered/diffuse wave patterns — the ambient wash beyond the direct reflections.
- **Air (Stage III):** Increasingly dense small particles between the waves — represents air absorption/diffusion. Density scales with Air amount.
- **Excitation (Stage IV):** Small spinning star/spark marks scattered across the wave field — represents room energy/drive. Count/intensity scales with Drive amount.
- **Cabinet Resonance (Stage I):** No visual representation.
- **Room Tone (Stage V):** No visual representation.

### Bypass behavior
- When a stage is bypassed, its visual elements disappear entirely (smooth fade out)
- Not ghost traces — clean removal, only active stages are visible
- When all spatial stages are bypassed, the head profile remains as the constant anchor

### Animation
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

</decisions>

<specifics>
## Specific Ideas

- The psycho-magnetic curves illustration ("Fig. 187. The Psycho-Magnetic Curves") is the strongest single reference — Victorian profile head with flowing wave curves surrounding it
- The ripple tank / Bell Labs wave diffraction photographs inform how wavefronts should look when interacting with boundaries and obstacles
- "The invisible medium through which all sound propagates" (the plugin's tagline) — the visualization should feel like it's making the invisible visible
- The head is the constant — everything else (waves, particles, stars) represents the room environment acting upon the listener

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 07-visualization*
*Context gathered: 2026-02-18*
