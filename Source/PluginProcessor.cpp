#include "PluginProcessor.h"
#include "PluginEditor.h"

AetherProcessor::AetherProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache all parameter pointers (done once, used every processBlock)
    // Stage I: Resonance
    resWeightParam   = apvts.getRawParameterValue (ParamIDs::resWeight);
    resMaterialParam = apvts.getRawParameterValue (ParamIDs::resMaterial);
    resBypassParam   = apvts.getRawParameterValue (ParamIDs::resBypass);

    // Stage II: Reflections
    reflSizeParam   = apvts.getRawParameterValue (ParamIDs::reflSize);
    reflShapeParam  = apvts.getRawParameterValue (ParamIDs::reflShape);
    reflProxParam   = apvts.getRawParameterValue (ParamIDs::reflProx);
    reflWidthParam  = apvts.getRawParameterValue (ParamIDs::reflWidth);
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
double AetherProcessor::getTailLengthSeconds() const { return 2.0; }
int AetherProcessor::getNumPrograms()    { return 1; }
int AetherProcessor::getCurrentProgram() { return 0; }
void AetherProcessor::setCurrentProgram (int index) { juce::ignoreUnused (index); }
const juce::String AetherProcessor::getProgramName (int index) { juce::ignoreUnused (index); return {}; }
void AetherProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused (index, newName); }

void AetherProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;

    // Prepare all DSP sections (ENG-02: sample-rate aware)
    resonanceSection.prepare (sampleRate, samplesPerBlock);
    reflectionsSection.prepare (sampleRate, samplesPerBlock);
    airSection.prepare (sampleRate, samplesPerBlock);
    excitationSection.prepare (sampleRate, samplesPerBlock);
    roomToneSection.prepare (sampleRate, samplesPerBlock);
    diffuseTailSection.prepare (sampleRate, samplesPerBlock);
    mixSection.prepare (sampleRate, samplesPerBlock);
    outputSection.prepare (sampleRate, samplesPerBlock);

    // ENG-03: Report latency (0 in Phase 1, updated when DSP adds latency)
    setLatencySamples (0);
}

void AetherProcessor::releaseResources()
{
    resonanceSection.reset();
    reflectionsSection.reset();
    airSection.reset();
    excitationSection.reset();
    roomToneSection.reset();
    diffuseTailSection.reset();
    mixSection.reset();
    outputSection.reset();
}

void AetherProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;  // ENG-05: prevent denormal CPU spikes

    // Clear unused output channels (ENG-04: works with any channel config)
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Read parameters and push to sections
    updateStageParams();

    // OUT-04: Capture dry signal at input (pre all processing)
    mixSection.pushDrySamples (juce::dsp::AudioBlock<float> (buffer));

    // ENG-06: Fixed serial chain I -> II -> III -> IV -> V -> VI
    resonanceSection.process (buffer);
    reflectionsSection.process (buffer);
    airSection.process (buffer);
    excitationSection.process (buffer);
    roomToneSection.process (buffer);
    diffuseTailSection.process (buffer);

    // OUT-01: Mix dry/wet with equal-power crossfade (sin3dB)
    {
        juce::dsp::AudioBlock<float> wetBlock (buffer);
        mixSection.mixWetSamples (wetBlock);
    }

    // OUT-03: Auto-gain compensation
    const float currentMix = outMixParam->load();
    mixSection.applyAutoGainCompensation (buffer, currentMix);

    // OUT-02: Output level trim
    outputSection.process (buffer, outLevelParam->load());
}

void AetherProcessor::updateStageParams()
{
    // Stage I: Resonance -- forward weight, material, and bypass
    resonanceSection.setWeight (resWeightParam->load());
    resonanceSection.setMaterial (static_cast<int> (resMaterialParam->load()));
    resonanceSection.setBypass (resBypassParam->load() >= 0.5f);
    // Stage II: Reflections
    reflectionsSection.setRoomSize (reflSizeParam->load());
    reflectionsSection.setShape (static_cast<int> (reflShapeParam->load()));
    reflectionsSection.setProximity (reflProxParam->load());
    reflectionsSection.setWidth (reflWidthParam->load());
    reflectionsSection.setBypass (reflBypassParam->load() >= 0.5f);

    airSection.setBypass (airBypassParam->load() >= 0.5f);
    excitationSection.setBypass (excitBypassParam->load() >= 0.5f);
    roomToneSection.setBypass (toneBypassParam->load() >= 0.5f);

    // Stage VI: Diffuse Tail -- forward direct params
    float currentDecay = tailDecayParam->load();
    float currentDiffusion = tailDiffParam->load();
    diffuseTailSection.setDecay (currentDecay);
    diffuseTailSection.setDiffusion (currentDiffusion);
    diffuseTailSection.setBypass (tailBypassParam->load() >= 0.5f);

    // Cross-stage: Room Size -> Tail pre-delay (automatic, always locked)
    float roomSizeNorm = reflSizeParam->load();
    diffuseTailSection.setPreDelay (roomSizeNorm);

    // Cross-stage: Air -> Tail HF damping (automatic, always locked)
    float airAmount = airAmountParam->load();
    diffuseTailSection.setHFDamping (airAmount);

    // Cross-stage: Shape -> Tail character (diffusion density + modal character)
    int shapeIndex = static_cast<int> (reflShapeParam->load());
    diffuseTailSection.setShapeInfluence (shapeIndex);

    // Mix level
    mixSection.setMixLevel (outMixParam->load());
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
