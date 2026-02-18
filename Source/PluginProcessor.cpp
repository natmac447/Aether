#include "PluginProcessor.h"
#include "PluginEditor.h"

AetherProcessor::AetherProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

AetherProcessor::~AetherProcessor() {}

const juce::String AetherProcessor::getName() const { return "Aether"; }
bool AetherProcessor::acceptsMidi() const { return false; }
bool AetherProcessor::producesMidi() const { return false; }
bool AetherProcessor::isMidiEffect() const { return false; }
double AetherProcessor::getTailLengthSeconds() const { return 0.0; }
int AetherProcessor::getNumPrograms() { return 1; }
int AetherProcessor::getCurrentProgram() { return 0; }
void AetherProcessor::setCurrentProgram (int index) { juce::ignoreUnused (index); }
const juce::String AetherProcessor::getProgramName (int index) { juce::ignoreUnused (index); return {}; }
void AetherProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused (index, newName); }

void AetherProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void AetherProcessor::releaseResources() {}

void AetherProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
}

bool AetherProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AetherProcessor::createEditor()
{
    return new AetherEditor (*this);
}

void AetherProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AetherProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AetherProcessor();
}
