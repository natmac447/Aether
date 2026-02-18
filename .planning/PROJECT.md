# Aether

## What This Is

Aether is a JUCE audio plugin (VST3/AU/Standalone) that reconstructs the physical realism of a real room for digitally-captured audio sources. It simulates cabinet resonance, early reflections, air absorption, room excitation, ambient noise floor, and diffuse reverb tail — the layers that real microphones capture in real spaces but that amp sims and DI recordings lack. Part of the Cairn Cosmos Series, with a Victorian scientific illustration parchment aesthetic.

## Core Value

Make digital amp sims and DI recordings sound like they were recorded in a real room with a real microphone — not through reverb effects, but through physically-modeled environmental layers.

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] Six-stage DSP processing chain: Cabinet Resonance, Early Reflections, Air & Distance, Excitation, Room Tone, Diffuse Tail
- [ ] Per-stage independent bypass
- [ ] Early Reflections engine with multi-tap delay network, stereo decorrelation, Room Size/Shape/Proximity controls
- [ ] Cabinet Resonance with short feedback delay network, 3 cabinet types (Open/Closed/Combo)
- [ ] Air & Distance with frequency-dependent absorption, Warm/Neutral character toggle
- [ ] Excitation with frequency-dependent multiband soft saturation and oversampling
- [ ] Room Tone with shaped noise generator, decorrelated stereo, linked to Room Size
- [ ] Diffuse Tail with FDN reverb, Decay/Diffusion controls, auto pre-delay linked to Room Size
- [ ] Mix (dry/wet) and Output (gain trim) with auto-gain compensation
- [ ] Three-column Victorian parchment UI (900x530px) with acoustic ray visualization
- [ ] Center SVG visualization reactive to Room Size, Shape, Proximity with breathing animation
- [ ] Custom knob, toggle, and bypass components matching parchment/ink aesthetic
- [ ] 6 factory presets (Tight Booth, Live Room, Garage, Warehouse, Bedroom, Church Hall)
- [ ] Preset selector with Roman numeral naming
- [ ] All parameters smoothed (10-50ms ramp) to prevent zipper noise
- [ ] Sample-rate-aware delay times and filter frequencies
- [ ] Latency reporting for DAW PDC
- [ ] macOS build (VST3, AU, Standalone)
- [ ] Custom typography: Cormorant Garamond, EB Garamond, Spectral (embedded)

### Out of Scope

- Windows build — deferred to post-v1
- Mono-Safe mode — acknowledged in spec as future feature
- Real-time chat/messaging features — N/A
- Mobile/iOS version — web/desktop plugin only
- AAX format — not in v1

## Context

- **Existing ecosystem:** Cairn has multiple plugins; Crucible (at `~/Projects/Crucible`) is the most complete reference implementation following current brand guidelines. Brand guidelines live at `~/Projects/brand`.
- **Design handoff:** Complete spec at `/Users/nathanmcmillan/Downloads/Aether Files/AETHER-DESIGN-HANDOFF.md` with full DSP specs, UI specs, color system, typography, layout, and implementation phases. HTML mockup at `/Users/nathanmcmillan/Downloads/Aether Files/aether-concept.html`.
- **Primary users:** Guitar players using digital amp sims (Kemper, Neural DSP, Helix, etc.) who want recordings that sound like a mic'd amp in a real room rather than a sterile DI.
- **Secondary users:** Bass players, synth users, digital piano players, vocal producers dealing with overly-clean sources.
- **Unique approach:** Not a reverb plugin — it's an environment simulator. Six physically-motivated stages that model distinct phenomena (cabinet vibration, early reflections, air absorption, nonlinear room excitation, ambient noise, diffuse tail) rather than a single reverb algorithm.
- **Aesthetic:** Victorian acoustic science — parchment background, ink linework, anatomical/perceptual illustrations, hand-drawn scientific diagrams. First "light theme" Cairn plugin.

## Constraints

- **Framework:** JUCE 8, C++, CMake build system (not Projucer)
- **Platform:** macOS for v1
- **Formats:** VST3, AU, Standalone
- **Performance:** Real-time audio processing — all DSP must be deterministic and audio-thread safe. No allocations on audio thread.
- **Thread safety:** DSP on audio thread, UI on message thread. Lock-free parameter communication via APVTS.
- **Brand consistency:** Must follow Cairn design system adapted for parchment theme. Reference Crucible for patterns.
- **Plugin size:** 900 x 530px fixed window
- **DSP libraries:** Open to external libraries (FFTW, etc.) if they improve quality, but JUCE dsp module is the baseline
- **Test audio:** User has DI guitar and amp sim recordings available for A/B testing

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| JUCE 8 | Latest version, best features and performance | — Pending |
| macOS-first | Ship faster, add Windows post-v1 | — Pending |
| Design handoff as definitive spec | Comprehensive enough to build from directly | — Pending |
| Parchment light theme | Victorian scientific illustration aesthetic, unique in Cairn lineup | — Pending |
| CMake over Projucer | Modern build system, better CI/CD, matches existing Cairn plugins | — Pending |
| Open to external DSP libs | Best quality matters more than minimal dependencies | — Pending |

---
*Last updated: 2026-02-18 after initialization*
