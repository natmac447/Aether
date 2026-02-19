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
    toneGateParam   = apvts.getRawParameterValue (ParamIDs::toneGate);
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

    // Report oversampling latency for DAW PDC (only Excitation adds latency)
    setLatencySamples (excitationSection.getLatencySamples());

    // Inform DryWetMixer of wet path latency for internal compensation
    mixSection.setWetLatency (static_cast<float> (excitationSection.getLatencySamples()));
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

    // Query transport state for Room Tone gating modes (Signal-Gated / Transport-Only)
    bool transportPlaying = true;  // Safe default (standalone, hosts without transport)
    if (auto* playHead = getPlayHead())
    {
        if (auto posInfo = playHead->getPosition())
            transportPlaying = posInfo->getIsPlaying();
    }
    roomToneSection.setTransportPlaying (transportPlaying);

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

    // OUT-03: Auto-gain compensation (factors in mix + energy-adding params)
    const float currentMix   = outMixParam->load();
    const float currentDrive = excitDriveParam->load();                           // 0-1
    const float currentDecayNorm = (tailDecayParam->load() - 50.0f) / 1950.0f;   // 50-2000ms -> 0-1
    const float currentDiff  = tailDiffParam->load();                             // 0-1
    const float currentRoomSize = reflSizeParam->load();                          // 0-1
    const float currentProx  = reflProxParam->load();                             // 0-1
    mixSection.applyAutoGainCompensation (buffer, currentMix, currentDrive, currentDecayNorm, currentDiff,
                                          currentRoomSize, currentProx);

    // OUT-02: Output level trim
    outputSection.process (buffer, outLevelParam->load());

    // RMS level for visualization breathing boost
    {
        float rms = buffer.getRMSLevel (0, 0, buffer.getNumSamples());
        if (getTotalNumInputChannels() > 1)
            rms = std::max (rms, buffer.getRMSLevel (1, 0, buffer.getNumSamples()));

        // Keep peak since last GUI read (compare-and-swap)
        float expected = visualizationRmsLevel.load (std::memory_order_relaxed);
        while (rms > expected &&
               !visualizationRmsLevel.compare_exchange_weak (expected, rms,
                   std::memory_order_relaxed))
        {}
    }
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

    // Stage III: Air -- forward amount, character, and bypass
    float airAmount = airAmountParam->load();
    int airCharIndex = static_cast<int> (airCharParam->load());
    airSection.setAmount (airAmount);
    airSection.setCharacter (airCharIndex);
    airSection.setBypass (airBypassParam->load() >= 0.5f);

    // Stage IV: Excitation -- forward Drive, Material, RoomSize, bypass
    excitationSection.setDrive (excitDriveParam->load());
    excitationSection.setMaterial (static_cast<int> (resMaterialParam->load()));   // Cross-stage: Material from Stage I
    excitationSection.setRoomSize (reflSizeParam->load());                          // Cross-stage: Room Size from Stage II
    excitationSection.setBypass (excitBypassParam->load() >= 0.5f);

    // Stage V: Room Tone -- forward Ambience, Gate mode, bypass, cross-stage coupling
    roomToneSection.setAmbience (toneAmbParam->load());
    roomToneSection.setGateMode (static_cast<int> (toneGateParam->load()));
    roomToneSection.setRoomSize (reflSizeParam->load());                            // Cross-stage: Room Size from Stage II
    roomToneSection.setShape (static_cast<int> (reflShapeParam->load()));           // Cross-stage: Shape from Stage II
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
    diffuseTailSection.setHFDamping (airAmount);

    // Cross-stage: Character -> Tail decay bias
    // Access character preset for coupling factors (kCharacterPresets from AirSection.h)
    float tailDecayBias = 0.0f;
    float reflDarkeningScale = 1.0f;
    if (airCharIndex >= 0 && airCharIndex < kNumCharacters)
    {
        tailDecayBias = kCharacterPresets[airCharIndex].tailDecayBias;
        reflDarkeningScale = kCharacterPresets[airCharIndex].reflDarkeningScale;
    }
    diffuseTailSection.setCharacterDecayBias (tailDecayBias * airAmount);

    // Cross-stage: Air -> Reflections darkening
    // At Air 0%: zero coupling (baseline does NOT propagate to reflections)
    // At Air 100%: full coupling scaled by character
    reflectionsSection.setAirDarkening (airAmount * reflDarkeningScale);

    // Cross-stage: Shape -> Tail character (diffusion density + modal character)
    int shapeIndex = static_cast<int> (reflShapeParam->load());
    diffuseTailSection.setShapeInfluence (shapeIndex);

    // Mix level
    mixSection.setMixLevel (outMixParam->load());
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
