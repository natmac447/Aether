# Feature Research

**Domain:** Room simulation / environment modeling audio plugin for guitar amp sim users
**Researched:** 2026-02-18
**Confidence:** MEDIUM-HIGH (based on competitive analysis of 12+ products, forum research, design handoff validation)

## Feature Landscape

### Table Stakes (Users Expect These)

Features users assume exist. Missing these = product feels incomplete or broken.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Dry/Wet Mix control | Every spatial effect plugin has this. Users must blend processed and unprocessed signal. UAD Ocean Way, Valhalla Room, Altiverb all have it. | LOW | Already in design handoff. 0-100% range is standard. |
| Output gain trim | Users need level matching to prevent "louder = better" bias during A/B testing. Universal across pro plugins. | LOW | Already in design handoff. -24dB to +6dB range is adequate. |
| Per-stage bypass | Users expect to isolate what each processing stage contributes. Comparable to per-band bypass in multiband processors. Two Notes WoS, AmpliTube 5 all offer module-level bypass. | LOW | Already in design handoff. In/Out toggle per stage. |
| Room Size control | Every room simulation plugin from DreamVerb to Valhalla Room has room size as a primary parameter. Users think in terms of "how big is this space." | MEDIUM | Already in design handoff. Drives delay times in Early Reflections and auto-links to Diffuse Tail pre-delay. |
| Early Reflections engine | Defines spatial character. UAD Ocean Way, DreamVerb, Valhalla Room, Altiverb all separate early reflections from late reverb. LiquidSonics Seventh Heaven specifically highlights early/late balance as critical for spatial perception. | HIGH | Already in design handoff. Multi-tap delay network with stereo decorrelation. This is the core differentiator in execution quality. |
| Diffuse reverb tail | Users expect some late reverb even in a "room simulator." Every competitor has this. Valhalla Room, Seventh Heaven, DreamVerb all include controllable decay. | HIGH | Already in design handoff. FDN approach with 50-500ms decay range. The short max (500ms) is a deliberate constraint that differentiates Aether from general-purpose reverbs. |
| Factory presets | Users expect ready-to-use starting points. Waves Abbey Road has preset rooms, UAD Ocean Way has room positions, IK Sunset Sound has 12 spaces. Guitar players especially want presets -- they are not mixing engineers. | LOW | Already in design handoff. 6 presets (Tight Booth through Church Hall). Consider expanding to 10-12 post-v1. |
| DAW parameter automation | All parameters must be automatable from the DAW. This is a VST3/AU baseline requirement. Waves, UAD, and all major plugin vendors support full automation. | LOW | Handled by JUCE APVTS. Ensure all parameters are exposed and smoothed. |
| Parameter smoothing (no zipper noise) | Audible stepping artifacts when turning knobs is unacceptable. DreamVerb specifically advertises "no zipper noise." Users notice this immediately on spatial effects. | MEDIUM | Already in design handoff. 10-50ms ramp times. Critical for the three center knobs (Room Size, Shape, Proximity) which drive many dependent parameters. |
| Latency reporting for DAW PDC | If the plugin introduces latency (oversampling in Excitation stage), the DAW must know for compensation. Standard requirement for any plugin used in mixing. | LOW | Already in design handoff. Use `setLatencySamples()`. Only Excitation oversampling introduces meaningful latency. |
| Stereo output with proper imaging | Room simulation that collapses to mono or has poor stereo imaging defeats the purpose. UAD Ocean Way uses 3 stereo mic pairs. Valhalla Room is true stereo. Users expect spatial width from a room plugin. | MEDIUM | Already in design handoff. L/R decorrelation in Early Reflections, decorrelated noise in Room Tone, stereo FDN in Diffuse Tail. |
| Sample-rate independence | Plugin must work correctly at 44.1, 48, 88.2, 96, 176.4, 192 kHz. All professional plugins handle this. | MEDIUM | Already in design handoff. All delay times and filter frequencies must be sample-rate aware. |
| Preset save/load (user presets) | Users must be able to save their own presets beyond the factory set. Universal expectation in any plugin. | MEDIUM | Not explicitly in design handoff. JUCE supports this through APVTS state save/restore, but a proper preset management UI is needed. |

### Differentiators (Competitive Advantage)

Features that set Aether apart. Not expected in the category, but valuable for the target audience.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Six-stage physically-motivated processing chain | No competitor breaks room simulation into 6 discrete, independently controllable physical phenomena. UAD Ocean Way offers mic positioning. DreamVerb offers room shape/materials. But nobody offers Cabinet Resonance + Early Reflections + Air Absorption + Excitation + Room Tone + Diffuse Tail as separate stages. This is Aether's core identity. | HIGH | The ordering and independence of stages is what makes this unique. Each stage models a distinct physical phenomenon rather than being "reverb with EQ." |
| Cabinet Resonance stage | Amp sim users deal with IR-based cabinet simulation that lacks the sympathetic vibration and "boxiness" of a real cab in a room. No room reverb plugin addresses cabinet-room interaction. This fills a gap between cab sim and reverb. | MEDIUM | Short feedback delay network (1-5ms). Three cabinet types (Open/Closed/Combo) map to real-world enclosure physics. Unique to Aether. |
| Room Tone (shaped noise floor) | Real rooms have ambient noise floors. Digital recordings are unnervingly silent. No mainstream competitor adds shaped room tone as an independent controllable stage. The closest equivalent is Speakerphone's 5GB ambience library, but that is sample-based, not generated. | LOW | Shaped noise generator. Subtle but psychoacoustically powerful. The "uncanny valley" of digital audio is partly caused by missing noise floor. Default bypassed -- lets users opt in. |
| Excitation (nonlinear room response) | A loud amp in a room excites surfaces nonlinearly. No room reverb plugin models this. It bridges the gap between "clean reverb" and "recorded-in-room" character. Adds density and liveliness that pure reverb cannot. | HIGH | Multiband soft saturation with oversampling. Must be subtle -- "sounds more alive" not "sounds distorted." This is the hardest stage to tune correctly. |
| Air & Distance as independent stage | Most reverbs bake air absorption into the reverb algorithm. Separating it lets users independently control "how far away does this sound" without changing room size or reverb character. DreamVerb has "air density" but it is coupled to the reverb. | MEDIUM | Low-pass filter + allpass diffusion. Warm/Neutral character toggle gives two surface material personalities. Simple DSP, high perceptual impact. |
| Acoustic ray visualization (reactive) | No guitar-focused plugin has a scientific visualization showing how sound propagates in the room. UAD Ocean Way shows a room diagram but it is decorative. Aether's visualization reacts to Room Size, Shape, and Proximity in real time. Educational and builds trust. | MEDIUM | SVG-based, reactive to 3 parameters. Breathing animation. Distinctive Victorian scientific aesthetic. Ships as Phase 4 after DSP is solid. |
| Auto-linked parameters across stages | Room Size automatically adjusts Diffuse Tail pre-delay. Air setting automatically adjusts Diffuse Tail HF damping. Room Size influences Room Tone character. This cross-stage linking means one knob move creates physically coherent changes across the whole plugin. | MEDIUM | No additional controls needed -- the linking is internal. But requires careful DSP architecture to route parameter changes across stage boundaries. |
| Victorian parchment UI aesthetic | Every guitar plugin uses dark themes (Neural DSP, Kemper software, Line 6 HX Edit). A light Victorian scientific illustration aesthetic is visually distinctive and memorable. Communicates "physics-based, not just another reverb." | MEDIUM | Custom knobs, toggles, typography, decorative elements. Significant UI development time but strong brand differentiation. Not technically complex, just labor-intensive. |
| Auto-gain compensation on Mix knob | As wet signal increases, perceived loudness can change, biasing A/B tests. Few plugins compensate for this automatically. Builds trust with users who want honest comparisons. | LOW | Slight output reduction as Mix increases. Simple gain curve, high user trust impact. |
| Targeted at amp sim users specifically | No room reverb plugin is positioned specifically for Kemper/Neural DSP/Helix users. These users are underserved -- they know their tone lacks "room" but generic reverb plugins are not the answer. Aether's marketing, presets, and default settings can speak directly to this audience. | LOW (product positioning, not code) | Preset names, default values, documentation, and marketing all targeted at the amp sim gap. |

### Anti-Features (Commonly Requested, Often Problematic)

Features that seem good but create problems for Aether's focused mission.

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Convolution/IR loading | Users associate "realistic rooms" with impulse responses. Products like Altiverb, Speakerphone, and LiquidSonics Reverberate are IR-based. | IRs are static snapshots -- they cannot react to parameter changes in real time. Loading IRs would undermine Aether's core value of interactive, physically-motivated processing. IRs also add disk size, loading time, and licensing complexity. Algorithmic approach is correct for what Aether does. | Aether's algorithmic approach is the feature. Early Reflections + Diffuse Tail achieve room character through tunable delay networks, not frozen captures. |
| Microphone modeling / selection | UAD Ocean Way has vintage mic selection. Slate VMS is entirely built around mic modeling. Users may ask "which mic am I using?" | Aether simulates the room environment, not the recording chain. Adding mic modeling bloats scope, competes with established products (Slate VMS, Townsend Labs), and muddies the "environment simulator" positioning. Mic character is a recording-chain concern, not a room-physics concern. | Aether's Air & Distance stage handles the "how does the captured sound change over distance" aspect. Users who want mic modeling can use dedicated mic sim plugins in their chain. |
| Surround / Atmos / immersive audio support | Professional studios are moving to immersive formats. LiquidSonics Cinematic Rooms supports up to 9.1.6 Atmos. | Aether's target audience (guitar players with amp sims) works in stereo. Surround support adds massive DSP complexity (multiple FDN instances, channel routing matrices), extends QA scope, and serves users who are not Aether's primary market. | Ship stereo. If demand materializes, surround can be a v2+ feature. The architecture should not preclude it, but should not be designed around it. |
| General-purpose long reverb (cathedral, hall, plate) | Users see "reverb" and want long tails, shimmer effects, and hall sounds. Valhalla Room goes to 100-second decay. | Aether is an environment simulator, not a reverb plugin. Long decay times (>500ms) push into creative reverb territory and away from "mic'd amp in room" realism. The 500ms max on Diffuse Tail is a deliberate design constraint. | Keep the 500ms cap. Position Aether explicitly as "room realism" not "reverb effect." Users who want cathedrals should use Valhalla Room or Seventh Heaven alongside Aether. |
| Real-time spectrum analyzer / metering | Some plugins show frequency content or levels. Users like visual feedback. | Adds CPU overhead, UI complexity, and visual clutter to the Victorian aesthetic. The acoustic ray visualization already provides visual feedback that is more meaningful for spatial processing than a frequency graph. Metering is a mixing tool, not a room simulation tool. | The acoustic ray visualization is Aether's visual feedback. It shows what the plugin is doing spatially, which is more relevant than frequency content. |
| "Room browser" with 50+ named spaces | Altiverb has hundreds of sampled spaces. IK Sunset Sound has 12 rooms. Users expect to browse "Studio A, Studio B, Church, etc." | With 6 presets and continuous parameter control, Aether gives users infinite rooms rather than a fixed library. A large room browser implies convolution/sampling, which contradicts the algorithmic approach. It also sets an expectation of "accuracy to a real place" rather than "tunable physics." | 6 factory presets as starting points, plus user preset save/load. The three center knobs (Room Size, Shape, Proximity) create far more variety than a fixed library. Expand factory presets to 10-12 in post-v1 updates if users want more starting points. |
| MIDI learn / hardware controller mapping | Power users want to map knobs to hardware controllers. | Adds implementation complexity for a feature most guitar players will not use (they set and forget room settings). DAWs already provide host-level MIDI mapping for plugin parameters. | Rely on DAW-level MIDI mapping (which works with APVTS parameters exposed to the host). No custom MIDI learn UI needed. |
| Built-in tuner, noise gate, or other guitar utilities | Guitar players use tuners and noise gates. "Since you are targeting guitar players, add a tuner!" | Feature creep. Aether is a room simulator, not a channel strip. Every guitar player already has a tuner and noise gate in their signal chain. Adding utilities dilutes the focused product identity. | Stay focused. Aether does one thing: room environment simulation. |
| Windows support in v1 | Windows is a huge market. Users will ask. | JUCE supports cross-platform, but testing, packaging, and supporting two platforms doubles QA effort for initial release. Ship macOS first, validate the product, then add Windows. | macOS-first for v1. Windows in v1.1 or v2. The JUCE/CMake architecture already supports this -- it is a build/test/packaging concern, not a code architecture concern. |

## Feature Dependencies

```
[Early Reflections Engine (Stage II)]
    |
    |--drives--> [Diffuse Tail pre-delay] (Room Size auto-links to pre-delay timing)
    |--drives--> [Room Tone character] (Room Size shifts noise floor resonant frequency)
    |--drives--> [Acoustic Ray Visualization] (Room Size, Shape, Proximity control the diagram)

[Air & Distance (Stage III)]
    |
    |--drives--> [Diffuse Tail HF damping] (Air amount auto-links to tail darkness)

[Cabinet Resonance (Stage I)]
    |
    |--feeds-into--> [Early Reflections (Stage II)] (serial signal flow)

[Excitation (Stage IV)]
    |
    |--requires--> [Oversampling infrastructure] (2x-4x oversampling to prevent aliasing)

[Parameter Smoothing]
    |
    |--required-by--> [All knob parameters] (prevents zipper noise)
    |--especially--> [Room Size, Shape, Proximity] (these drive multiple dependent parameters)

[APVTS Parameter Tree]
    |
    |--required-by--> [All DSP stages] (parameter communication)
    |--required-by--> [All UI controls] (knob/toggle state)
    |--required-by--> [Preset system] (state save/restore)
    |--required-by--> [DAW automation] (host parameter exposure)

[Preset System]
    |
    |--requires--> [All 6 DSP stages working] (presets configure all stages)
    |--requires--> [APVTS parameter tree] (state management)

[Acoustic Ray Visualization]
    |
    |--requires--> [Early Reflections parameters] (Room Size, Shape, Proximity)
    |--enhances--> [User understanding of Room Size/Shape/Proximity]
    |--independent-of--> [DSP processing] (purely visual, no audio impact)
```

### Dependency Notes

- **Early Reflections drives multiple downstream systems:** Room Size is the most connected parameter in the plugin. Changes propagate to Diffuse Tail (pre-delay), Room Tone (noise character), and Visualization. This makes Early Reflections the highest-priority DSP stage to implement.
- **Air & Distance influences Diffuse Tail:** The HF damping cross-link means Air & Distance should be implemented before or alongside Diffuse Tail for proper integration testing.
- **Excitation requires oversampling:** This is the only stage that introduces latency. It must be implemented with latency reporting from the start.
- **Visualization is independent of DSP:** It reads parameter values but does not affect audio. Can be developed in parallel with DSP work or deferred entirely without affecting audio functionality.
- **Preset system requires all stages:** Factory presets configure all 6 stages, so presets cannot be finalized until all stages are functional.

## MVP Definition

### Launch With (v1)

Minimum viable product -- what is needed to validate "room environment simulation for amp sim users."

- [x] Six DSP stages with independent bypass -- this is the product identity
- [x] Early Reflections engine with Room Size, Shape, Proximity -- the core spatial engine
- [x] Cabinet Resonance with 3 cabinet types -- unique differentiator for guitar players
- [x] Air & Distance with Warm/Neutral character -- distance perception control
- [x] Excitation with multiband saturation -- "room aliveness" that no competitor offers
- [x] Room Tone with shaped noise generator -- psychoacoustic realism layer
- [x] Diffuse Tail with Decay/Diffusion -- short controlled reverb tail
- [x] Mix and Output controls with auto-gain compensation
- [x] 6 factory presets covering small-to-large room range
- [x] Victorian parchment UI with acoustic ray visualization
- [x] macOS VST3, AU, Standalone
- [x] Full DAW automation support
- [x] Latency reporting for PDC

### Add After Validation (v1.x)

Features to add once core product is working and user feedback is gathered.

- [ ] Expanded factory preset library (10-12 presets) -- add when users report wanting more starting points
- [ ] User preset save/load with file management UI -- add when users report saving/loading friction
- [ ] Windows build -- add when macOS version is stable and validated
- [ ] A/B comparison mode (toggle between two parameter states) -- add when users report difficulty comparing settings
- [ ] Undo/redo for parameter changes -- add based on user workflow feedback
- [ ] Mono-Safe mode (reduces L/R decorrelation) -- add when mono compatibility issues are reported

### Future Consideration (v2+)

Features to defer until product-market fit is established.

- [ ] AAX format for Pro Tools -- defer until Pro Tools users specifically request it
- [ ] Additional cabinet types beyond Open/Closed/Combo -- defer until user feedback on cabinet modeling
- [ ] More room shape controls (height, asymmetry, surface materials) -- defer until users exhaust current Shape knob range
- [ ] Sidechain input for dynamic room behavior -- experimental, defer to v2
- [ ] Surround/Atmos support -- defer unless demand materializes from post-production users
- [ ] iOS/mobile AUv3 version -- defer to v2+

## Feature Prioritization Matrix

| Feature | User Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| Early Reflections engine | HIGH | HIGH | P1 |
| Cabinet Resonance | HIGH | MEDIUM | P1 |
| Air & Distance | HIGH | LOW | P1 |
| Diffuse Tail (FDN) | HIGH | HIGH | P1 |
| Excitation (saturation) | MEDIUM | HIGH | P1 |
| Room Tone (noise floor) | MEDIUM | LOW | P1 |
| Mix/Output with auto-gain | HIGH | LOW | P1 |
| Per-stage bypass | HIGH | LOW | P1 |
| 6 factory presets | HIGH | LOW | P1 |
| Parameter smoothing | HIGH | MEDIUM | P1 |
| Victorian parchment UI | MEDIUM | HIGH | P1 |
| Acoustic ray visualization | MEDIUM | MEDIUM | P1 |
| Cross-stage parameter linking | MEDIUM | MEDIUM | P1 |
| User preset save/load UI | MEDIUM | MEDIUM | P2 |
| Expanded preset library | LOW | LOW | P2 |
| A/B comparison mode | LOW | MEDIUM | P2 |
| Windows build | MEDIUM | MEDIUM | P2 |
| Mono-Safe mode | LOW | MEDIUM | P2 |
| Undo/redo | LOW | MEDIUM | P3 |
| AAX format | LOW | LOW | P3 |
| Additional cabinet types | LOW | LOW | P3 |

**Priority key:**
- P1: Must have for launch
- P2: Should have, add when possible
- P3: Nice to have, future consideration

## Competitor Feature Analysis

| Feature | UAD Ocean Way Studios | Waves Abbey Road Chambers | DreamVerb | Valhalla Room | IK Sunset Sound | Altiverb/Speakerphone | Aether (Our Approach) |
|---------|----------------------|--------------------------|-----------|---------------|-----------------|----------------------|----------------------|
| Room size control | Fixed rooms (A/B), mic distance | Fixed chambers | 21 shapes, blendable | Size parameter, 12 algorithms | 12 fixed spaces | IR-based (fixed) | Continuous Room Size knob driving multi-tap delays |
| Early/late separation | Implicit via mic positioning (Near/Mid/Far) | Separate chamber + hardware chain | Separate Early/Late intensity and timing | Separate Early/Late sections with crossfeed | No explicit separation | IR captures include both | Fully independent Early Reflections and Diffuse Tail stages |
| Mic simulation | 3 mic pairs (Near/Mid/Far), vintage mic types | Speaker + mic selection per chamber | None | None | MIC selection (L/R/Both) | Speakerphone: 23 mics | None (deliberate -- room physics, not recording chain) |
| Cabinet interaction | None (assumes cab is already recorded) | None | None | None | None | Speakerphone: 400 speaker IRs | Cabinet Resonance stage with Open/Closed/Combo types |
| Air absorption | Implicit in room model | None explicit | Air density control | Damping controls | HPF/LPF | Baked into IRs | Independent Air & Distance stage with Warm/Neutral character |
| Nonlinear room excitation | None | Tape saturation in signal chain | None | None | Console preamp modeling | None | Excitation stage with multiband soft saturation |
| Room tone / noise floor | None | None | None | None | None | Speakerphone: 5GB ambience library (sampled) | Room Tone stage with shaped noise generator linked to Room Size |
| Visualization | Room diagram with mic positions | Hardware rack GUI | Graphic feedback for room shape | Simple UI | Room photo | Room photos | Reactive acoustic ray diagram with breathing animation |
| Preset approach | Fixed studio rooms | Fixed chambers | 100+ presets + room building | Presets per algorithm | 12 spaces | IR library (hundreds) | 6 targeted presets + continuous parameter control |
| Target audience | Mixing engineers | Mixing engineers | Mixing/sound design | General reverb users | Mixing engineers | Post-production, sound design | Guitar amp sim users specifically |
| Price point (approx.) | $149-299 | $149-249 | $199-299 | $50 | $79-149 | $595 (Altiverb), $495 (Speakerphone) | TBD -- position between Valhalla Room and UAD tier |
| CPU model | UAD DSP (now also native) | Native | UAD DSP (now also native) | Native (very efficient) | Native | Native | Native (must be efficient for guitar players tracking) |

### Key Competitive Insights

1. **No competitor combines all 6 physical phenomena in one plugin.** UAD Ocean Way is closest to "room realism" but focuses on mic positioning, not physical modeling. DreamVerb offers room customization but lacks cabinet interaction, excitation, and room tone.

2. **Nobody targets amp sim users specifically.** Every competitor is positioned for mixing engineers or post-production. Aether's positioning as "the missing room for your amp sim" is an uncontested niche.

3. **Convolution vs. algorithmic is a false choice for Aether's use case.** Convolution gives "real room accuracy" but no interactivity. Algorithmic gives interactivity but often sounds "synthetic." Aether's multi-stage algorithmic approach achieves interactivity while the physical motivation of each stage grounds it in realism.

4. **Room Tone is a genuinely novel feature.** Speakerphone has sampled ambiences, but no plugin generates room-appropriate shaped noise as an independent, tunable stage. This is psychoacoustically significant and trivially cheap to implement.

5. **Excitation (nonlinear room response) is completely unaddressed.** No competitor models how loud sources excite room surfaces nonlinearly. This is the most technically challenging differentiator but also the hardest for competitors to copy quickly.

6. **Price positioning matters.** Valhalla Room at $50 is the floor for "good room reverb." UAD Ocean Way at $149-299 is the ceiling for "studio room modeling." Aether should position in the $79-149 range, justified by the unique six-stage approach and amp-sim-specific targeting.

## Sources

- [Waves Abbey Road Chambers](https://www.waves.com/plugins/abbey-road-chambers) -- Chamber reverb with speaker/mic selection and STEED signal chain
- [Waves Abbey Road Studio 3](https://www.waves.com/plugins/abbey-road-studio-3) -- Control room monitoring simulation (different category)
- [UAD Ocean Way Studios](https://www.uaudio.com/uad-plugins/reverbs/ocean-way-studios.html) -- Room + mic modeling with 3 mic pairs, 8 source types
- [UAD DreamVerb Room Modeler](https://www.uaudio.com/products/dreamverb) -- 21 room shapes, material blending, air density, 5-band EQ
- [Valhalla Room](https://valhalladsp.com/shop/reverb/valhalla-room/) -- 12 algorithms, separate Early/Late sections, true stereo
- [LiquidSonics Seventh Heaven](https://www.liquidsonics.com/software/seventh-heaven/) -- Fusion-IR (modulated convolution), Bricasti M7 modeling
- [IK Multimedia Sunset Sound Studio Reverb](https://www.ikmultimedia.com/products/trsunsetsound/) -- 12 spaces with VRM technology, console modeling
- [Audio Ease Speakerphone](https://www.audioease.com/speakerphone/) -- 400 speaker IRs, 23 mics, 106 covers, Altiverb-powered rooms
- [Audio Ease Altiverb](https://www.audioease.com/altiverb/) -- Original convolution reverb, extensive IR library
- [Slate Digital VMS](https://slatedigital.com/virtual-microphone-system/) -- Mic modeling with blending, proximity, and stereo widening
- [Two Notes Torpedo Wall of Sound](https://www.two-notes.com/en/torpedo-series/torpedo-wall-of-sound/) -- DynIR cab simulation with mic positioning and room ambience
- [Softube Amp Room](https://www.softube.com/us/plug-ins/amp-room-vintage-suite) -- Modular amp sim with moveable mics and phase alignment
- [IK Multimedia AmpliTube 5](https://www.ikmultimedia.com/products/amplitube5/) -- VIR technology with 600 IRs per speaker, room selection
- [Kemper Forum: Best reverb to simulate room mics](https://forum.kemper-amps.com/forum/thread/27074-best-reverb-plugin-to-simulate-room-mics/) -- User discussion on room simulation needs for profiler users
- [The Gear Forum: Favourite plugins for amp sims](https://thegearforum.com/threads/favourite-plugins-to-use-with-your-amp-sims.6065/) -- User discussion on what plugins complement amp simulators
- [LiquidSonics: Algorithmic vs Convolution Reverb](https://www.liquidsonics.com/2019/06/26/what-is-the-difference-between-algorithmic-and-convolution-reverb/) -- Technical comparison of reverb approaches

---
*Feature research for: Aether -- Room simulation / environment modeling audio plugin*
*Researched: 2026-02-18*
