# Stack Research

**Domain:** JUCE 8 audio plugin -- room simulation / environment modeling DSP (macOS first)
**Researched:** 2026-02-18
**Confidence:** HIGH (core stack), MEDIUM (external DSP libraries)

## Recommended Stack

### Core Technologies

| Technology | Version | Purpose | Why Recommended |
|------------|---------|---------|-----------------|
| JUCE | 8.0.12 | Audio plugin framework (VST3/AU/Standalone) | Industry standard for C++ audio plugins. Crucible already uses 8.0.4; upgrading to 8.0.12 gets macOS/iOS 26 support, Direct2D fixes, and 6 months of bug fixes. Pin to 8.0.12 via GIT_TAG for reproducibility. |
| C++ | C++20 | Language standard | Matches Crucible. Designated initializers, concepts, `std::span`, `constexpr` improvements all useful for DSP code. C++23 not yet broadly supported by JUCE. |
| CMake | 3.22+ (builds with 4.x) | Build system | Matches Crucible pattern. JUCE's CMake API (`juce_add_plugin`) is the modern standard; Projucer is deprecated for new projects. Minimum 3.22 for FetchContent stability; Crucible's 3.22 floor works fine with CMake 4.2.x installed on the system. |
| Ninja | latest | Build generator | Fast incremental builds on macOS. Crucible uses it. `cmake -G Ninja` is ~3x faster than Make for JUCE projects. |
| Xcode Command Line Tools | 16.x+ | Compiler toolchain (Apple Clang) | Required for macOS builds. Apple Clang in Xcode 16 fully supports C++20. No need for standalone Xcode IDE if using CMake+Ninja. |

### Plugin Formats

| Format | Priority | Notes |
|--------|----------|-------|
| VST3 | Phase 1 | Universal standard. Supported by all major DAWs on macOS. Start here. |
| AU (AudioUnit v2) | Phase 2 | Required for Logic Pro / GarageBand users on macOS. Add via `FORMATS VST3 AU` in CMakeLists.txt. |
| Standalone | Phase 2 | Useful for development/testing without a DAW. Add via `FORMATS VST3 AU Standalone`. |
| CLAP | Defer to JUCE 9 | JUCE 9 will add native CLAP support. The clap-juce-extensions workaround exists but adds complexity for marginal gain right now. Not worth it for launch. |
| AAX | Not recommended | Requires Avid developer agreement and proprietary SDK. Only needed for Pro Tools. Adds significant build complexity. Add only if Pro Tools market demands it post-launch. |

### JUCE Modules (link targets)

| Module | Purpose | Why |
|--------|---------|-----|
| `juce::juce_audio_utils` | Audio utility classes | Standard for plugin projects, includes AudioProcessorValueTreeState |
| `juce::juce_audio_processors` | Plugin hosting/format wrappers | Core plugin infrastructure |
| `juce::juce_dsp` | DSP primitives | IIR/SVF filters, Oversampling, Convolution, DelayLine, SmoothedValue. Critical for Aether -- provides the filter building blocks for air absorption, cabinet resonance filtering, and the oversampling needed in the Excitation stage. |
| `juce::juce_gui_basics` | UI framework | Component system, LookAndFeel, painting |
| `juce::juce_gui_extra` | Extended UI | May need for custom drawing or advanced components |
| `juce::juce_recommended_config_flags` | Build flags | Standard optimization flags |
| `juce::juce_recommended_lto_flags` | Link-time optimization | Important for DSP performance in Release builds |
| `juce::juce_recommended_warning_flags` | Warnings | Catches bugs early |

### Supporting Libraries -- DSP

| Library | Version | Purpose | Why Recommended |
|---------|---------|---------|-----------------|
| Signalsmith DSP | v1.7.1 (GIT_TAG) | Delay lines, interpolators, envelopes, FFT | MIT licensed, header-only C++11 library. Provides high-quality fractional delay lines critical for FDN reverb (multi-tap delay network, diffuse tail). Polyphase and Kaiser-sinc interpolators are superior to naive linear interpolation for delay modulation. The library is actively maintained (Jan 2026 release). Fetch via FetchContent. |
| Signalsmith Reverb Example | main branch | Reference FDN architecture | MIT licensed. Geraint Luff's ADC 2021 "Let's Write A Reverb" implementation. Provides a proven Householder-matrix FDN + Hadamard diffusion architecture. Use as **reference/inspiration** for Aether's Diffuse Tail and Early Reflections stages, not as a drop-in dependency. The code demonstrates proper multi-channel FDN structure. |

### Supporting Libraries -- Testing & Validation

| Library | Version | Purpose | Why Recommended |
|---------|---------|---------|-----------------|
| Catch2 | v3.7.1+ (pin to 3.x latest) | Unit testing | Matches Crucible pattern. BDD-style test macros (`SECTION`, `REQUIRE`). Fetch via FetchContent. v3.12.0 is latest (Dec 2025) but v3.7.1 proven stable in Crucible -- upgrade to latest 3.x when starting. |
| pluginval | latest binary | Plugin validation | Industry standard for host compatibility testing. Runs automated tests at configurable strictness levels. Crucible already uses it at level 5. Install via `brew install pluginval`. |

### Supporting Libraries -- Development

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| melatonin_inspector | latest | Runtime UI component inspector | Add during UI development phase. Inspect component hierarchy, measure paint times, debug layout. Invaluable for the parchment UI. Fetch via FetchContent as a JUCE module. Strip from Release builds with `#if DEBUG`. |
| melatonin_perfetto | latest | Performance tracing | Add when profiling DSP performance. Google Perfetto integration for JUCE. Helps find bottlenecks in the 6-stage DSP chain. |

### Development Tools

| Tool | Purpose | Notes |
|------|---------|-------|
| Ninja | Fast incremental builds | `brew install ninja`. Use with `cmake -G Ninja`. |
| pluginval | Plugin validation | `brew install pluginval`. Run at strictness 5+ for host compat. |
| REAPER | DAW for testing | Free to evaluate. Lightweight, fast plugin scanning, excellent for development. Test VST3 and AU here. |
| AU Lab / Hosting AU | AU debugging | Apple's AU hosting tool for testing AudioUnit format. |
| Instruments.app | Performance profiling | macOS native. Use Time Profiler for DSP hotspots and Allocations for memory leaks in audio thread. |

## Build Configuration

```cmake
cmake_minimum_required(VERSION 3.22)

project(Aether VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS Universal Binary (arm64 + x86_64)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version")

# Dependencies via FetchContent
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    signalsmith-dsp
    GIT_REPOSITORY https://github.com/Signalsmith-Audio/dsp.git
    GIT_TAG v1.7.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE Catch2 signalsmith-dsp)

# Plugin definition
juce_add_plugin(Aether
    COMPANY_NAME "Cairn"
    PLUGIN_MANUFACTURER_CODE Nmcm
    PLUGIN_CODE Aeth
    FORMATS VST3  # Add AU Standalone in Phase 2
    PRODUCT_NAME "Aether"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_compile_definitions(Aether
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Include signalsmith-dsp headers
target_include_directories(Aether
    PRIVATE
        ${signalsmith-dsp_SOURCE_DIR}
)

target_link_libraries(Aether
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
```

### Build Commands

```bash
# Configure (first time)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

# Build
cmake --build build

# Configure Debug (for development)
cmake -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Run tests
cmake --build build-debug --target AetherTests
cd build-debug && ctest --output-on-failure

# Validate plugin
pluginval --validate-in-process --strictness-level 5 \
    build/Aether_artefacts/Release/VST3/Aether.vst3
```

## Alternatives Considered

| Recommended | Alternative | When to Use Alternative |
|-------------|-------------|-------------------------|
| JUCE 8.0.12 | JUCE 7.x | Never. JUCE 8 has better CMake support, WebView2 option, and ongoing maintenance. |
| JUCE juce_dsp filters | Custom biquad implementation | Only if juce_dsp filters prove too slow for 6-stage chain (unlikely -- they are well optimized). |
| Signalsmith DSP delay lines | JUCE `juce::dsp::DelayLine` | JUCE DelayLine is adequate for simple cases. Signalsmith provides superior interpolation options (polyphase, Kaiser-sinc) critical for modulated FDN delay lines that need to avoid metallic artifacts. |
| Signalsmith DSP delay lines | Custom circular buffer | Only if you need a non-standard access pattern Signalsmith doesn't support (unlikely). |
| Hand-rolled FDN | Signalsmith Basics reverb (v1.0.1) | The Basics library includes a complete reverb effect but is a black box -- you cannot customize the FDN topology for Aether's specific 6-stage architecture. Use hand-rolled FDN with Signalsmith delay primitives instead. |
| Catch2 v3 | GoogleTest | Catch2 is simpler for audio plugin testing, no separate main() needed with Catch2WithMain. Crucible already uses it. Consistency across Cosmos Series projects. |
| CMake FetchContent | git submodules | FetchContent is cleaner -- no `.gitmodules`, automatic version pinning, better IDE integration. Matches Crucible pattern. |
| CMake FetchContent | Conan/vcpkg | Overkill for 3 dependencies. FetchContent is simpler and self-contained. |
| Ninja | Make (Unix Makefiles) | Make works but is 2-3x slower for incremental builds on JUCE projects. |

## What NOT to Use

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| Projucer | Deprecated workflow. JUCE team recommends CMake for new projects. Can't integrate FetchContent dependencies, no CI/CD support, generates IDE projects that drift from source of truth. | CMake with `juce_add_plugin` |
| juce::dsp::Reverb (Freeverb wrapper) | Extremely basic Schroeder reverb. No early reflections, no frequency-dependent decay, no FDN. Completely unsuitable for room simulation. | Hand-rolled FDN using Signalsmith delay lines + Householder/Hadamard mixing matrices |
| Eigen | Tempting for FDN mixing matrices, but massive dependency for what amounts to 8x8 or 16x16 matrix multiply. Eigen's template metaprogramming inflates compile times. For small fixed-size matrices, hand-coded Householder reflection or Hadamard butterfly is faster and trivial to implement. | Inline Householder reflection: `y[i] = x[i] - (2/N) * sum(x)` -- one line of code |
| KFR DSP library | Heavy dependency with its own SIMD abstraction. Overlaps heavily with juce_dsp which you already have. GPL-licensed (restrictive). | juce_dsp + Signalsmith DSP |
| freeverb3 | GPL licensed -- incompatible with commercial distribution unless you GPL your entire plugin. Also dated API. | Hand-rolled FDN |
| FAUST generated code | Adds a compilation step and toolchain dependency (faust compiler). The generated C++ is hard to debug and customize. For a hand-tuned room sim with 6 specific stages, direct C++ gives full control. | Direct C++ DSP code |
| clap-juce-extensions | CLAP support for JUCE 8 via external library. Adds build complexity, potential ABI issues. JUCE 9 will have native CLAP. | Wait for JUCE 9 if CLAP is needed |
| C++23 | JUCE 8 doesn't fully support C++23 features. Apple Clang's C++23 support is still incomplete. Stick with C++20. | C++20 |

## Stack Patterns by Variant

**If targeting macOS only (current plan):**
- Build Universal Binary (arm64 + x86_64) with `CMAKE_OSX_ARCHITECTURES`
- Set `CMAKE_OSX_DEPLOYMENT_TARGET` to 11.0 (macOS Big Sur) for broad compatibility
- Start with VST3, add AU in Phase 2
- Test in REAPER (VST3) and Logic (AU)

**If adding Windows later:**
- Same CMake setup works. Add MSVC compiler flags.
- Use Pamplejuce GitHub Actions template for CI cross-platform builds
- Test in REAPER (Windows VST3)

**If adding Linux later:**
- JUCE 8 supports Linux. Add LV2 format.
- Need ALSA/JACK headers for Standalone
- Lower priority -- tiny market for guitar amp sim plugins on Linux

## Version Compatibility

| Package | Compatible With | Notes |
|---------|-----------------|-------|
| JUCE 8.0.12 | CMake 3.22+ | JUCE 8 requires CMake 3.22 minimum |
| JUCE 8.0.12 | C++17/20 | JUCE 8 requires C++17 minimum; C++20 works and is recommended |
| JUCE 8.0.12 | Xcode 15/16 | Both work. Xcode 16 recommended for latest Apple Clang. |
| JUCE 8.0.12 | macOS 11.0+ deployment | Works for all Apple Silicon Macs and recent Intel Macs |
| Signalsmith DSP v1.7.1 | C++11+ | Header-only, no build system dependency. Just `#include`. |
| Catch2 v3.7.1+ | C++14+ | Separate compilation (not header-only in v3). Links via CMake. |
| CMake 3.22+ | Ninja 1.10+ | Any recent Ninja works fine |

## Key Technical Decisions for Aether's DSP

### FDN Mixing Matrix: Householder Reflection
Use Householder reflection (`y[i] = x[i] - (2/N) * sum(x)`) rather than Hadamard for the diffuse tail FDN. Householder works for any channel count N (not just powers of 2), is a single line of arithmetic per delay line, and produces maximal mixing. This is the standard approach in professional FDN reverbs (Geraint Luff's ADC talk confirms this).

### Delay Line Interpolation: Polyphase from Signalsmith
For modulated delay lines in the FDN (time-varying to avoid metallic ringing), use Signalsmith's polyphase interpolators rather than linear interpolation. Linear interpolation acts as a lowpass filter at higher frequencies, dulling the reverb tail. Polyphase interpolation preserves high-frequency content.

### Oversampling: Use juce::dsp::Oversampling for Excitation Stage Only
The Excitation (multiband saturation) stage needs 4x oversampling to prevent aliasing from waveshaping, matching Crucible's approach. The reverb stages (FDN, early reflections) do NOT need oversampling -- they are linear operations. Oversampling only the nonlinear stage saves significant CPU.

### Parameter Smoothing: juce::SmoothedValue
Follow Crucible's pattern: `SmoothedValue<float, ValueSmoothingTypes::Multiplicative>` for gain/frequency parameters, `ValueSmoothingTypes::Linear` for mix/blend parameters. 20ms ramp time for most parameters.

### Thread Safety: Atomic Parameters + APVTS
Follow Crucible's proven pattern: cache `std::atomic<float>*` pointers from APVTS in the processor constructor, read with `load()` in processBlock. Never touch ValueTree from the audio thread.

## Sources

- [JUCE 8.0.12 Release](https://github.com/juce-framework/JUCE/releases/tag/8.0.12) -- verified latest version (Dec 2025), HIGH confidence
- [JUCE GitHub Releases](https://github.com/juce-framework/JUCE/releases) -- version history verified
- [JUCE CMake API docs](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md) -- official CMake integration reference
- [JUCE Roadmap Q3 2025](https://juce.com/blog/juce-roadmap-update-q3-2025/) -- CLAP in JUCE 9 confirmation
- [Signalsmith DSP v1.7.1](https://github.com/Signalsmith-Audio/dsp/tags) -- verified latest tag (Jan 2026), HIGH confidence
- [Signalsmith Reverb Example](https://github.com/Signalsmith-Audio/reverb-example-code) -- FDN architecture reference (ADC 2021)
- [Signalsmith Basics v1.0.1](https://github.com/Signalsmith-Audio/basics/releases) -- complete reverb effect (Aug 2025), MIT license
- [Signalsmith Audio website](https://signalsmith-audio.co.uk/code/dsp/) -- library documentation
- [Catch2 Releases](https://github.com/catchorg/Catch2/releases) -- v3.12.0 latest (Dec 2025), HIGH confidence
- [Pamplejuce template](https://github.com/sudara/pamplejuce) -- JUCE 8 + CMake + Catch2 + GitHub Actions reference
- [melatonin_inspector](https://github.com/sudara/melatonin_inspector) -- JUCE UI debugging tool
- [pluginval](https://github.com/Tracktion/pluginval) -- plugin validation tool
- [How to use CMake with JUCE](https://melatonin.dev/blog/how-to-use-cmake-with-juce/) -- Sudara's comprehensive CMake guide
- [JOS FDN Reverberation](https://www.dsprelated.com/freebooks/pasp/FDN_Reverberation.html) -- Julius O. Smith's FDN reference
- [Jatin Chowdhury FDN research](https://github.com/jatinchowdhury18/Feedback-Delay-Networks) -- nonlinear/time-varying FDN research
- Crucible reference implementation at `~/Projects/Crucible` -- verified JUCE 8.0.4 + CMake + Catch2 + C++20 patterns, HIGH confidence

---
*Stack research for: Aether -- JUCE 8 room simulation audio plugin*
*Researched: 2026-02-18*
