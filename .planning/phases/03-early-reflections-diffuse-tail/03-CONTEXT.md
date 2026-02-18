# Phase 3: Early Reflections & Diffuse Tail - Context

**Gathered:** 2026-02-18
**Status:** Ready for planning

<domain>
## Phase Boundary

Core spatial engine delivering two DSP stages: Early Reflections (Stage II) with multi-tap delays defining room shape/size/proximity, and Diffuse Tail (Stage VI) with FDN reverb providing room decay. Cross-stage parameter linking ties Room Size to tail pre-delay and Air to tail HF damping. Shape selection also influences tail character.

New parameter: `refl_width` for stereo width control. Shape parameter changes from continuous float to discrete room geometry presets (6-8 options with Victorian naming).

</domain>

<decisions>
## Implementation Decisions

### Room character & size range
- Room Size range pushes into hall/cathedral territory at max (full 30ms reflection range)
- Nearly dry at minimum Room Size -- barely perceptible ambience, not fully anechoic
- Room Size knob curve weighted toward smaller rooms -- first 50% covers booth-to-studio, last 50% covers studio-to-hall. More resolution in the practical guitar zone
- Smooth morphing required when sweeping Room Size in real-time (delay time interpolation for DAW automation)
- Shape parameter changes from continuous float to discrete room geometry presets (6-8 shapes)
- Claude curates the shape set based on acoustic distinctiveness -- should produce maximally different reflection patterns
- Shape names use Victorian/evocative naming (e.g., "The Parlour", "The Nave", "The Gallery") -- not generic labels
- Parameter version bump required for Shape (float -> choice)

### Reflection spatial feel
- Default settings should be subtle until pushed -- adds depth without screaming "reverb"
- Proximity Near extreme: very tight close-mic'd feel with just a breath of air (not fully anechoic). Important for non-mic'd sources like synths
- New `refl_width` parameter added for dedicated stereo width control (0-100%, natural to dramatic spread)
- Width defaults to natural stereo field; pushing it creates dramatic enveloping spread for solo instruments

### Tail texture & decay
- Characterful and textured reverb tail -- organic, you can hear the room's personality. Not perfectly smooth studio reverb
- Decay range extended to 50ms-2s (up from 50-500ms in original spec). Allows dramatic long tails
- Decay knob curve weighted toward shorter tails -- first 60-70% covers 50-500ms (practical zone), last 30% opens to 2s

### Cross-stage linking
- Room Size -> Tail pre-delay: purely automatic, always locked, not overridable
- Air -> Tail HF damping: purely automatic, always locked, not overridable
- Shape -> Tail character: room shape influences diffusion density and modal character in the tail (cathedral tail sounds different from studio tail)
- No visual feedback for links (UI stays clean) -- coupling happens behind the scenes

### Claude's Discretion
- Specific real-space mapping for room size extremes (abstract vs specific spaces)
- Whether Room Size affects reflection tonality (darker in larger rooms) vs keeping tone-neutral for Air stage
- Whether Stage I material influences reflection character (material -> reflection timbre coupling)
- Size-Shape interaction model (independent vs shape constraining size range)
- Far extreme Proximity behavior (how much direct signal floor to maintain)
- ER-to-Tail transition (seamless blend vs perceptible pre-delay gap)
- Room Size -> Tail pre-delay curve (proportional vs physically modeled)
- Whether Room Size also drives Tail FDN delay lengths (full room scaling vs pre-delay only)
- Minimum room size character (closet slapback vs subliminal ambience)
- Shape preset count and specific shapes (6-8, curated for acoustic distinctiveness)

</decisions>

<specifics>
## Specific Ideas

- User noted this may be used on non-mic'd sources (synths, DI signals) -- Proximity Near should still work well for those, not just guitar amp sim
- User wants room shapes to feel like real spaces -- "playing in a long rectangular room would sound different than a non-symmetrical room with odd angles and alcoves"
- User emphasized wall materials matter: "playing in a stone room would sound significantly different than a room with wooden walls and thick carpets" -- this informs the deferred room surface parameter
- Victorian naming for shapes fits the Aether brand aesthetic

</specifics>

<deferred>
## Deferred Ideas

- **Dedicated room surface material/absorption parameters** -- User wants wall material (stone vs wood vs carpet) to affect reflections independently from Stage I material. Warrants its own parameter set and possibly a future phase insertion. Would complement Shape (geometry) + Surface (material) for full room definition.
- **Dedicated stereo width as a separate stage or global control** -- Currently adding `refl_width` to reflections. Could expand to a global stereo field control affecting all stages.

</deferred>

---

*Phase: 03-early-reflections-diffuse-tail*
*Context gathered: 2026-02-18*
