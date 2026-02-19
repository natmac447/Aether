#include "UserPresetManager.h"

//==============================================================================
juce::File UserPresetManager::getPresetDirectory()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userHomeDirectory)
                   .getChildFile ("Library/Audio/Presets/Cairn/Aether");
    dir.createDirectory();
    return dir;
}

//==============================================================================
bool UserPresetManager::savePreset (const juce::String& name,
                                     juce::AudioProcessorValueTreeState& apvts)
{
    // Sanitise the name
    auto sanitised = name.replaceCharacters ("/\\:", "___")
                         .trim()
                         .substring (0, 64);

    if (sanitised.isEmpty())
        return false;

    auto state = apvts.copyState();
    auto xml = state.createXml();
    if (xml == nullptr)
        return false;

    auto file = getPresetDirectory().getChildFile (sanitised + ".xml");
    return xml->writeTo (file);
}

//==============================================================================
bool UserPresetManager::loadPreset (const juce::File& presetFile,
                                     juce::AudioProcessorValueTreeState& apvts)
{
    auto xml = juce::XmlDocument::parse (presetFile);
    if (xml == nullptr)
        return false;

    if (xml->getTagName() != apvts.state.getType().toString())
        return false;

    apvts.replaceState (juce::ValueTree::fromXml (*xml));
    return true;
}

//==============================================================================
juce::Array<juce::File> UserPresetManager::getAvailablePresets()
{
    juce::Array<juce::File> results;
    auto dir = getPresetDirectory();
    dir.findChildFiles (results, juce::File::findFiles, false, "*.xml");
    results.sort();
    return results;
}

//==============================================================================
bool UserPresetManager::deletePreset (const juce::File& presetFile)
{
    return presetFile.deleteFile();
}
