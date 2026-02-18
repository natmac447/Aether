#include "PluginProcessor.h"
#include "PluginEditor.h"

AetherProcessor::AetherProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache all parameter pointers (done once, used every processBlock)
    // Stage I: Cabinet
    cabBodyParam   = apvts.getRawParameterValue (ParamIDs::cabBody);
    cabTypeParam   = apvts.getRawParameterValue (ParamIDs::cabType);
    cabBypassParam = apvts.getRawParameterValue (ParamIDs::cabBypass);

    // Stage II: Reflections
    reflSizeParam   = apvts.getRawParameterValue (ParamIDs::reflSize);
    reflShapeParam  = apvts.getRawParameterValue (ParamIDs::reflShape);
    reflProxParam   = apvts.getRawParameterValue (ParamIDs::reflProx);
    reflBypassParam = apvts.getRawParameterValue (ParamIDs::reflBypass);

    // Stage III: Air
    airAmountParam = apvts.getRawParameterValue (ParamIDs::airAmount);
    airCharParam   = apvts.getRawParameterValue (ParamIDs::airChar);
    airBypassParam = apvts.getRawParameterValue (ParamIDs::airBypass);

    // Stage IV: Excitation
    excitDriveParam  = apvts.getRawParameterValue (ParamIDs::excitDrive);
    excitBypassParam = apvts.getRawParameterValue (ParamIDs::excitBypass);

    // Stage V: Room Tone
    toneAmbParam    = apvts.getRawParameterValue (ParamIDs::toneAmb);
    toneBypassParam = apvts.getRawParameterValue (ParamIDs::toneBypass);

    // Stage VI: Diffuse Tail
    tailDecayParam  = apvts.getRawParameterValue (ParamIDs::tailDecay);
    tailDiffParam   = apvts.getRawParameterValue (ParamIDs::tailDiff);
    tailBypassParam = apvts.getRawParameterValue (ParamIDs::tailBypass);

    // Output
    outMixParam   = apvts.getRawParameterValue (ParamIDs::outMix);
    outLevelParam = apvts.getRawParameterValue (ParamIDs::outLevel);
}

AetherProcessor::~AetherProcessor() {}

const juce::String AetherProcessor::getName() const { return "Aether"; }
bool AetherProcessor::acceptsMidi() const  { return false; }
bool AetherProcessor::producesMidi() const { return false; }
bool AetherProcessor::isMidiEffect() const { return false; }
double AetherProcessor::getTailLengthSeconds() const { return 0.0; }
int AetherProcessor::getNumPrograms()    { return 1; }
int AetherProcessor::getCurrentProgram() { return 0; }
void AetherProcessor::setCurrentProgram (int index) { juce::ignoreUnused (index); }
const juce::String AetherProcessor::getProgramName (int index) { juce::ignoreUnused (index); return {}; }
void AetherProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused (index, newName); }

void AetherProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;

    // DSP stage preparation will be added in Plan 02
}

void AetherProcessor::releaseResources() {}

void AetherProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Clear unused output channels (real-time safe: uses existing buffer)
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Full DSP pipeline (dry capture, 6-stage chain, mix, output) added in Plan 02.
    // Currently passthrough: audio passes through unchanged.
}

bool AetherProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AetherProcessor::createEditor()
{
    // GenericAudioProcessorEditor shows all 20 parameters for testing
    // before the custom Victorian UI is built in Phase 6.
    return new juce::GenericAudioProcessorEditor (*this);
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
