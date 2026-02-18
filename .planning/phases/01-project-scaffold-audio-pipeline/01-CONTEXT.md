# Phase 1: Project Scaffold & Audio Pipeline - Context

**Gathered:** 2026-02-18
**Status:** Ready for planning

<domain>
## Phase Boundary

Build system, complete APVTS parameter tree for all 6 stages, processBlock skeleton with mix/output processing, and real-time safety patterns. Everything needed before writing any DSP — the plugin builds, loads in a DAW, passes audio through, and has all parameters exposed and smoothed.

</domain>

<decisions>
## Implementation Decisions

### Crucible Alignment
- Claude reviews Crucible's project structure, CMake patterns, and code organization, then decides what to follow vs adapt for Aether's 6-stage architecture
- Dependency versions: Claude evaluates what's worth upgrading (JUCE 8.0.12 is already decided; Catch2 and other deps are Claude's call)
- Crucible patterns (DSP section interface, APVTS usage, etc.): Claude reviews and adopts what works, adapts what doesn't fit
- **Fully standalone** — no shared code submodules or cross-repo dependencies. Copy patterns, don't link to them.

### DAW Parameter Naming
- Claude decides naming convention for DAW automation list (stage-prefixed, flat, or numbered)
- Claude decides text vs numeric display for text-label parameters (Room Size, Shape, Proximity)
- **Bypass parameters must be automatable** — they appear in the DAW's automation list, not UI-only
- Claude decides internal parameter ID convention (stage-prefixed vs flat)

### Plugin Identity
- **Manufacturer name:** Cairn
- **Manufacturer code (AU):** Carn
- **Plugin code (AU):** Aeth
- **Plugin category:** Reverb (for DAW browser discoverability)

### Signalsmith DSP
- **Include Signalsmith DSP v1.7.1 from Phase 1** — add as CMake FetchContent dependency in the scaffold
- Open to additional DSP libraries in later phases if research recommends them
- Claude decides whether to include test framework (Catch2 + pluginval) from Phase 1 or defer

### Claude's Discretion
- Project directory layout and file organization (informed by Crucible review)
- Dependency version choices beyond JUCE 8.0.12
- Parameter naming convention and display format for DAW
- Internal parameter ID format
- Whether to include Catch2/pluginval in scaffold or defer to Phase 8
- DSP section base class/interface design
- processBlock skeleton structure

</decisions>

<specifics>
## Specific Ideas

- Crucible at ~/Projects/Crucible is the primary reference — follow its proven patterns unless Aether's 6-stage architecture warrants deviation
- Brand guidelines at ~/Projects/brand for any brand-related decisions
- Design handoff at /Users/nathanmcmillan/Downloads/Aether Files/AETHER-DESIGN-HANDOFF.md has all parameter ranges, defaults, and stage specifications

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 01-project-scaffold-audio-pipeline*
*Context gathered: 2026-02-18*
