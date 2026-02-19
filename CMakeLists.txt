cmake_minimum_required(VERSION 3.22)
project(Aether VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version")

include(FetchContent)

# JUCE 8.0.12
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Signalsmith DSP v1.7.1
FetchContent_Declare(
    signalsmith-dsp
    GIT_REPOSITORY https://github.com/Signalsmith-Audio/dsp.git
    GIT_TAG v1.7.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(signalsmith-dsp)

# Create INTERFACE target for Signalsmith DSP (header-only library)
if(NOT TARGET signalsmith-dsp)
    add_library(signalsmith-dsp INTERFACE)
    target_include_directories(signalsmith-dsp INTERFACE ${signalsmith-dsp_SOURCE_DIR})
endif()

# Catch2 v3.8.1 (test framework)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)

# Plugin target
juce_add_plugin(Aether
    COMPANY_NAME "Cairn"
    PLUGIN_MANUFACTURER_CODE Carn
    PLUGIN_CODE Aeth

    FORMATS VST3

    PRODUCT_NAME "Aether"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    VST3_CATEGORIES Reverb
    AU_MAIN_TYPE kAudioUnitType_Effect

    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(Aether)

# Embed fonts as binary data
juce_add_binary_data(AetherBinaryData
    SOURCES
        Resources/fonts/CormorantGaramond-Light.ttf
        Resources/fonts/CormorantGaramond-Regular.ttf
        Resources/fonts/CormorantGaramond-Italic.ttf
        Resources/fonts/EBGaramond-Regular.ttf
        Resources/fonts/EBGaramond-Italic.ttf
        Resources/fonts/Spectral-Regular.ttf
        Resources/fonts/Spectral-Italic.ttf
)

target_sources(Aether
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Parameters.h
        Source/dsp/StageSection.h
        Source/dsp/ResonanceSection.cpp
        Source/dsp/ResonanceSection.h
        Source/dsp/ReflectionsSection.cpp
        Source/dsp/ReflectionsSection.h
        Source/dsp/AirSection.cpp
        Source/dsp/AirSection.h
        Source/dsp/ExcitationSection.cpp
        Source/dsp/ExcitationSection.h
        Source/dsp/RoomToneSection.cpp
        Source/dsp/RoomToneSection.h
        Source/dsp/DiffuseTailSection.cpp
        Source/dsp/DiffuseTailSection.h
        Source/dsp/MixSection.cpp
        Source/dsp/MixSection.h
        Source/dsp/OutputSection.cpp
        Source/dsp/OutputSection.h
        Source/ui/AetherColours.h
        Source/ui/AetherLookAndFeel.h
        Source/ui/AetherLookAndFeel.cpp
        Source/ui/ParchmentElements.h
        Source/ui/ParchmentElements.cpp
        Source/ui/AetherKnob.h
        Source/ui/AetherKnob.cpp
        Source/ui/AetherToggle.h
        Source/ui/AetherToggle.cpp
        Source/ui/AetherBypassButton.h
        Source/ui/AetherBypassButton.cpp
)

target_compile_definitions(Aether
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Aether
    PRIVATE
        AetherBinaryData
        signalsmith-dsp
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
