# Phase 2: Cabinet Resonance - Context

**Gathered:** 2026-02-18
**Status:** Ready for planning

<domain>
## Phase Boundary

First DSP stage — adds physical low-mid presence and resonant material character to any audio signal. This is NOT about simulating a mic'd cabinet. It fills in the missing physical body that digital/clean signals lack, making them sound real and present. Uses short FDN with material-based tonal profiles.

</domain>

<decisions>
## Implementation Decisions

### Section Rename
- Section renamed from "Cabinet" to **"Resonance"** — labeled "I. Resonance" with Roman numeral
- Body knob renamed to **"Weight"** — controls intensity of resonance effect
- Cabinet Type selector renamed to **"Material"** — selects resonant character profile
- Internal parameter IDs must be renamed: `cab_body` → `res_weight`, `cab_type` → `res_material`, `cab_bypass` → `res_bypass`
- Parameter display names updated accordingly (e.g., "I Resonance - Weight", "I Resonance - Material")

### Material Types (formerly Cabinet Types)
- Expand from 3 types (Open/Closed/Combo) to **8-10+ material types** from day one
- Material families: **woods** (e.g., Pine, Oak, Walnut, Mahogany), **metals** (e.g., Iron, Steel), **stones** (e.g., Limestone, Granite) — Claude determines final set based on what sounds distinct and musical
- Each material has inherent resonance characteristics — lighter materials (Pine) bloom more, heavier materials (Granite) are dense and damped
- Material names are evocative and period-appropriate — no real-world cabinet type references (no "Open-back", "Closed-back", "Combo")
- UI shows **name only** in selector with **hover tooltip** describing the tonal character
- Selector is a **dropdown menu with up/down arrows** for cycling without reopening the dropdown

### Tonal Character
- Frequency range spread and resonance decay per material: **Claude's discretion** — determine what sounds most natural and creates meaningful differentiation
- Full-spectrum materials (not guitar-frequency-locked) — should work on any source

### Weight Knob (formerly Body)
- At 100%: **obviously colored** — thick, resonant, adds real heft/weight to anemic tones
- Better to have range and dial back than wish for more
- Smooth sweep 0-100%, no center detent or visual markers
- Response curve at low values: Claude's discretion
- Interaction with material type: Claude's discretion

### Bypass Behavior
- Crossfade vs instant toggle: Claude's discretion — prioritize clean A/B comparison

### Stereo Behavior
- Whether Resonance stage affects stereo width: Claude's discretion — if subtle L/R decorrelation helps realism without clashing with downstream spatial stages (Reflections, Tail), include it; otherwise keep mono

### Claude's Discretion
- Response curve of Weight knob (how it scales from 0-100%)
- Body knob / material interaction model (independent vs interactive)
- Bypass transition behavior (instant vs crossfade)
- Stereo width decision (mono vs subtle decorrelation)
- Dynamic input response (level-reactive vs consistent)
- Exact material list and their specific FDN parameters
- Frequency range per material type
- Resonance decay characteristics per material

</decisions>

<specifics>
## Specific Ideas

- "Cabinet Resonance and Body should be about giving the guitar tone more of a physical presence in the lows and low mids rather than trying to sound like a specific microphone"
- "The amp tone is already mimicking a mic'd amp sound by using an IR in most cases so we don't want to make it sound like a microphone capturing a microphone capturing a performance"
- "This is about filling in the missing pieces that are necessary to make a digitally created signal sound real"
- Pine might have more bloom than Granite — material choice should feel physically different, not just tonally different
- Resonance bloom as an inherent characteristic tied to material weight/density
- Brand voice for descriptions: Victorian pseudo-scientific language — "Aether adds realism to any sound through psychoacoustic manipulation and physics-based frequency resonance"
- Material names should feel period-appropriate (Victorian era)

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 02-cabinet-resonance*
*Context gathered: 2026-02-18*
