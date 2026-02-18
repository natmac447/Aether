#pragma once
#include <JuceHeader.h>
#include "Parameters.h"

class AetherProcessor : public juce::AudioProcessor
{
public:
    AetherProcessor();
    ~AetherProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    // Cached atomic parameter pointers (zero-overhead audio-thread access)
    // Stage I: Cabinet
    std::atomic<float>* cabBodyParam   = nullptr;
    std::atomic<float>* cabTypeParam   = nullptr;
    std::atomic<float>* cabBypassParam = nullptr;

    // Stage II: Reflections
    std::atomic<float>* reflSizeParam    = nullptr;
    std::atomic<float>* reflShapeParam   = nullptr;
    std::atomic<float>* reflProxParam    = nullptr;
    std::atomic<float>* reflBypassParam  = nullptr;

    // Stage III: Air
    std::atomic<float>* airAmountParam  = nullptr;
    std::atomic<float>* airCharParam    = nullptr;
    std::atomic<float>* airBypassParam  = nullptr;

    // Stage IV: Excitation
    std::atomic<float>* excitDriveParam  = nullptr;
    std::atomic<float>* excitBypassParam = nullptr;

    // Stage V: Room Tone
    std::atomic<float>* toneAmbParam    = nullptr;
    std::atomic<float>* toneBypassParam = nullptr;

    // Stage VI: Diffuse Tail
    std::atomic<float>* tailDecayParam  = nullptr;
    std::atomic<float>* tailDiffParam   = nullptr;
    std::atomic<float>* tailBypassParam = nullptr;

    // Output
    std::atomic<float>* outMixParam   = nullptr;
    std::atomic<float>* outLevelParam = nullptr;

    // Stored sample rate and block size for DSP stages (Plan 02)
    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherProcessor)
};
