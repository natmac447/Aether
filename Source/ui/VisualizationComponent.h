#pragma once
#include <JuceHeader.h>

/**
 * VisualizationComponent - Placeholder for the central display area.
 *
 * Maintains timer and parameter source infrastructure for future
 * custom visualization assets. Currently renders nothing.
 *
 * Parameter state is read via std::atomic<float>* pointers set by the editor.
 */
class VisualizationComponent : public juce::Component,
                                private juce::Timer
{
public:
    VisualizationComponent();
    ~VisualizationComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    /** Start the 30Hz animation timer. Call when editor becomes visible. */
    void startAnimation() { startTimerHz (30); }

    /** Stop the animation timer. Call when editor is closing. */
    void stopAnimation()  { stopTimer(); }

    /**
     * Provide atomic parameter pointers for zero-overhead GUI reads.
     * All pointers may be nullptr (safe -- smoothing holds last value).
     */
    void setParameterSources (std::atomic<float>* reflSize,
                              std::atomic<float>* reflShape,
                              std::atomic<float>* reflProx,
                              std::atomic<float>* airAmount,
                              std::atomic<float>* excitDrive,
                              std::atomic<float>* reflBypass,
                              std::atomic<float>* airBypass,
                              std::atomic<float>* excitBypass,
                              std::atomic<float>* tailBypass,
                              std::atomic<float>* rmsLevel);

private:
    //==========================================================================
    // Timer
    void timerCallback() override;

    //==========================================================================
    // State update
    void updateSmoothedState();

    //==========================================================================
    // Smoothed visual state (interpolated each frame)
    float smoothedSize      = 0.4f;
    float smoothedProx      = 0.3f;
    float smoothedAir       = 0.4f;
    float smoothedDrive     = 0.25f;

    // Bypass fade alphas (1.0 = active, 0.0 = bypassed)
    float smoothedReflAlpha  = 1.0f;
    float smoothedAirAlpha   = 1.0f;
    float smoothedExcitAlpha = 1.0f;
    float smoothedTailAlpha  = 1.0f;

    // Animation phase
    float breathPhase       = 0.0f;
    float currentRmsSmoothed = 0.0f;

    //==========================================================================
    // Atomic parameter pointers (set once, read every frame)
    std::atomic<float>* pRoomSize   = nullptr;
    std::atomic<float>* pShape      = nullptr;
    std::atomic<float>* pProximity  = nullptr;
    std::atomic<float>* pAirAmount  = nullptr;
    std::atomic<float>* pExcitDrive = nullptr;
    std::atomic<float>* pReflBypass = nullptr;
    std::atomic<float>* pAirBypass  = nullptr;
    std::atomic<float>* pExcitBypass = nullptr;
    std::atomic<float>* pTailBypass = nullptr;
    std::atomic<float>* pRmsLevel   = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualizationComponent)
};
