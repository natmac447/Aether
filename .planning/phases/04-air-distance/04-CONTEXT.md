# Phase 4: Air & Distance - Context

**Gathered:** 2026-02-18
**Status:** Ready for planning

<domain>
## Phase Boundary

Frequency-dependent absorption stage (Stage III) — simulating high-frequency absorption and air diffusion that makes the signal sound like it traveled through a real room. Includes HF rolloff, allpass phase smearing for transient softening, character selection, bypass, and cross-stage coupling to Diffuse Tail and Early Reflections.

</domain>

<decisions>
## Implementation Decisions

### Filter Character
- Three character variants instead of two: **Warm, Neutral, Cold**
  - Warm = hot, humid day — thick air, sound hangs longer, soft materials, plush carpet
  - Neutral = moderate room conditions, balanced absorption
  - Cold = cold, dry day — bright, crisp sound with bite, hard reflective surfaces
- Each character should be noticeably different — enough to clearly hear when switching between them, but not a monumental difference
- Physics-driven rolloff curves — model real air absorption coefficients rather than analog-feel musical shaping. Accuracy drives the sound.
- Minimal baseline absorption at 0% Air — even at 0%, a very subtle HF shelf. No room has zero air absorption. Stage is never completely transparent (bypass exists for that).
- At max Air (100%), signal should sound like "next room over" — noticeable HF loss but still intelligible, not dramatically dark
- Must NOT sound like a lowpass filter (tone knob) or a blanket (muffled/smothered). Always spatial, not tonal. Even at max, there should be life in the signal.
- Air knob transitions smoothly ramped (~20-30ms) to avoid clicks or zipper noise during automation

### Claude's Discretion — Filter
- Cold character approach: whether it's a different rolloff flavor (steeper, higher-frequency cut) or includes subtle HF presence boost. Pick what's physically plausible.
- Character selector UI control: 3-position toggle vs continuous blend knob. Pick based on UI consistency and DSP support.
- Whether characters also affect low-frequency behavior (Warm adding low warmth, Cold thinning lows). Pick what sounds most natural for room simulation.

### Transient Softening
- At max Air, signal is allowed to get somewhat "washy" or smeared — that's what real distance does to transients
- Allpass phase smearing should simulate air diffusion, not just be an EQ effect

### Claude's Discretion — Transients
- How noticeable the transient softening should be (clearly audible vs subtle byproduct). Pick the balance that serves room simulation.
- Whether diffusion scales linearly with Air or has a threshold. Pick what produces the most natural result.
- Whether the three characters influence transient diffusion amount (Warm = more diffusion, Cold = less). Pick based on physical plausibility.

### Air-to-Stage Coupling
- Air couples to BOTH Early Reflections AND Diffuse Tail — higher Air darkens individual reflection taps AND increases tail HF damping. More physically accurate: air affects all sound paths.
- Warm character subtly extends perceived decay (humid air holds sound longer). Cold tightens perceived decay. Character influences the coupling, not just the direct signal.

### Claude's Discretion — Coupling
- How strongly Air influences tail/reflections darkness. Pick what sounds cohesive with the direct signal filtering.
- Whether character affects coupling strength or just direct signal. Pick what creates the most cohesive room impression.
- Whether minimal baseline at 0% Air also subtly affects tail/reflections. Pick based on coherence with the baseline decision.

</decisions>

<specifics>
## Specific Ideas

- UAD Ocean Way and Sound City plugins as reference for how distance affects sound source — they nail the spatial distance feel
- "Think of the air as a way to accurately set the vibe in the room" — the character selector is about environmental conditions, not just frequency curves
- Warm = "hot muggy day, sound seems thick and just hangs in the air longer"
- Cold = "cold dry day, sounds are bright and crisp with a little bite just like the cold air"
- Studio live room feel: like the difference between recording in a treated vs untreated room. Subtle but changes the character of everything.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 04-air-distance*
*Context gathered: 2026-02-18*
