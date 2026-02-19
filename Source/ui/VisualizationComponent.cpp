#include "VisualizationComponent.h"

//==============================================================================
// Constructor / Destructor
//==============================================================================
VisualizationComponent::VisualizationComponent()
{
}

VisualizationComponent::~VisualizationComponent()
{
    stopTimer();
}

//==============================================================================
// setParameterSources
//==============================================================================
void VisualizationComponent::setParameterSources (
    std::atomic<float>* reflSize,
    std::atomic<float>* reflShape,
    std::atomic<float>* reflProx,
    std::atomic<float>* airAmount,
    std::atomic<float>* excitDrive,
    std::atomic<float>* reflBypass,
    std::atomic<float>* airBypass,
    std::atomic<float>* excitBypass,
    std::atomic<float>* tailBypass,
    std::atomic<float>* rmsLevel)
{
    pRoomSize    = reflSize;
    pShape       = reflShape;
    pProximity   = reflProx;
    pAirAmount   = airAmount;
    pExcitDrive  = excitDrive;
    pReflBypass  = reflBypass;
    pAirBypass   = airBypass;
    pExcitBypass = excitBypass;
    pTailBypass  = tailBypass;
    pRmsLevel    = rmsLevel;
}

//==============================================================================
// resized
//==============================================================================
void VisualizationComponent::resized()
{
}

//==============================================================================
// Timer callback
//==============================================================================
void VisualizationComponent::timerCallback()
{
    updateSmoothedState();
    repaint();
}

//==============================================================================
// Smoothed state update (called 30x per second)
//==============================================================================
void VisualizationComponent::updateSmoothedState()
{
    constexpr float kSmooth       = 0.12f;
    constexpr float kBypassSmooth = 0.15f;

    // Parameter smoothing
    if (pRoomSize)   smoothedSize  += kSmooth * (pRoomSize->load (std::memory_order_relaxed)  - smoothedSize);
    if (pProximity)  smoothedProx  += kSmooth * (pProximity->load (std::memory_order_relaxed) - smoothedProx);
    if (pAirAmount)  smoothedAir   += kSmooth * (pAirAmount->load (std::memory_order_relaxed) - smoothedAir);
    if (pExcitDrive) smoothedDrive += kSmooth * (pExcitDrive->load (std::memory_order_relaxed) - smoothedDrive);

    // Bypass fade
    auto bypassTarget = [] (std::atomic<float>* p) -> float {
        return (p && p->load (std::memory_order_relaxed) >= 0.5f) ? 0.0f : 1.0f;
    };
    smoothedReflAlpha  += kBypassSmooth * (bypassTarget (pReflBypass)  - smoothedReflAlpha);
    smoothedAirAlpha   += kBypassSmooth * (bypassTarget (pAirBypass)   - smoothedAirAlpha);
    smoothedExcitAlpha += kBypassSmooth * (bypassTarget (pExcitBypass) - smoothedExcitAlpha);
    smoothedTailAlpha  += kBypassSmooth * (bypassTarget (pTailBypass)  - smoothedTailAlpha);

    // Breathing phase
    constexpr float kBreathIncrement = juce::MathConstants<float>::twoPi / (4.0f * 30.0f);

    float rawRms = pRmsLevel ? pRmsLevel->exchange (0.0f, std::memory_order_relaxed) : 0.0f;
    if (rawRms > currentRmsSmoothed)
        currentRmsSmoothed += 0.3f * (rawRms - currentRmsSmoothed);
    else
        currentRmsSmoothed += 0.05f * (rawRms - currentRmsSmoothed);

    float speedBoost = 1.0f + currentRmsSmoothed * 0.5f;
    breathPhase += kBreathIncrement * speedBoost;
    if (breathPhase > juce::MathConstants<float>::twoPi)
        breathPhase -= juce::MathConstants<float>::twoPi;
}

//==============================================================================
// paint: empty for now -- custom assets will be added later
//==============================================================================
void VisualizationComponent::paint (juce::Graphics& /*g*/)
{
}
