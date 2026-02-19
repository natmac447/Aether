#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * UserPresetManager - Static utility for saving and loading user presets.
 *
 * Presets are stored as XML files (APVTS state snapshots) in the standard
 * audio presets directory: ~/Library/Audio/Presets/Cairn/Aether/
 *
 * This class is stateless -- all methods are static.
 */
class UserPresetManager
{
public:
    /** Get the user preset directory, creating it if it does not exist.
     *  macOS: ~/Library/Audio/Presets/Cairn/Aether/ */
    static juce::File getPresetDirectory();

    /** Save current APVTS state to a named XML file in the preset directory.
     *  Returns true on success. Filename is sanitised (remove path separators, limit length).
     *  File extension: .xml */
    static bool savePreset (const juce::String& name,
                            juce::AudioProcessorValueTreeState& apvts);

    /** Load a user preset XML file and apply it to the APVTS.
     *  Returns true on success. Validates XML tag matches APVTS state type before applying. */
    static bool loadPreset (const juce::File& presetFile,
                            juce::AudioProcessorValueTreeState& apvts);

    /** Return an array of available user preset files (sorted alphabetically).
     *  Only returns .xml files from the preset directory. */
    static juce::Array<juce::File> getAvailablePresets();

    /** Delete a user preset file. Returns true on success. */
    static bool deletePreset (const juce::File& presetFile);
};
