# Phase 5: Excitation & Room Tone - Context

**Gathered:** 2026-02-18
**Status:** Ready for planning

<domain>
## Phase Boundary

Complete the six-stage DSP chain with Stage IV (Excitation) and Stage V (Room Tone). Excitation adds nonlinear room excitation via Drive knob — modeled as physical acoustic behavior, not gear emulation. Room Tone adds shaped ambient noise floor via Ambience knob with three gating modes. Phase also tunes auto-gain compensation for the full chain. UI, presets, and visualization are separate phases.

</domain>

<decisions>
## Implementation Decisions

### Saturation flavor
- Model physical room excitation, NOT preamp/console/tape character — the nonlinearity should represent how sound excites a physical space at different volumes (air nonlinearity, surface response, modal density)
- Couple saturation curves to Material parameter from Stage I — wood materials excite differently than stone/metal, reinforcing the physical model
- Band balance (low/mid/high curves): Claude's discretion based on physics-accurate room excitation modeling

### Drive intensity range
- At 50%: "alive, not distorted" (per spec)
- At 90-100%: subtle edge/audible nonlinearity — like a room being pushed hard. Noticeable but not harsh
- Input-sensitive: louder input = more excitation at same Drive setting, like a real room responding to dynamics
- At 0% Drive (stage not bypassed): subtle room nonlinearity still present — a real room always has some acoustic effect. Drive scales from "minimal" to "lots." Bypass is the true off switch
- Couple to Room Size: small rooms excite faster/tighter, large rooms excite with more low-end bloom
- Drive response curve: Claude's discretion (sweet spot vs linear)

### Room tone texture
- Three gating modes (user-selectable): Always On, Signal-Gated, Transport-Only (no noise when DAW transport is stopped)
- Spectral character couples to Room Size (per spec: larger = lower resonant character) AND Shape (hallway vs square room have different ambient character)
- Tone LEVEL is purely the Ambience knob — no coupling to Proximity or other parameters. User gets full control over level for mix flexibility (critical for compressed sources like snare drums)
- Static vs modulated quality: Claude's discretion based on what sounds most like a real room

### Excitation-to-room coupling
- Drive -> Diffuse Tail energy: Claude's discretion on whether/how to couple
- Drive -> Early Reflections energy: Claude's discretion on whether/how to couple
- Auto-gain compensation: explicitly tune in Phase 5 now that all 6 stages exist (deferred from Phase 2)
- DAW verification: verify Stages IV + V specifically (not full-chain re-verification of previously validated stages)

### Claude's Discretion
- Multiband saturation curve shapes and band crossover frequencies
- Drive response curve (sweet spot vs linear)
- Room tone modulation (static vs slowly evolving)
- Excitation coupling to downstream stages (Tail, Reflections)
- Oversampling factor within 2x-4x range
- Noise generation method for room tone

</decisions>

<specifics>
## Specific Ideas

- "Since we're trying to recreate the excitation of a room rather than a preamp or console, let's go with whatever gets us the most accurate representation of the way sound excites in a physical space at different volumes"
- Room tone gating modes specifically requested because of practical mixing concerns — compressed sources (snare drums) react very differently to constant room tone
- 0% Drive should still have subtle presence — models "the room is always there" even at minimum settings

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 05-excitation-room-tone*
*Context gathered: 2026-02-18*
