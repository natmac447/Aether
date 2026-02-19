#pragma once
#include <JuceHeader.h>

/**
 * VisualizationComponent - Victorian perception-inspired acoustic ray diagram.
 *
 * Renders a head profile silhouette with concentric wavefront rings, air particles,
 * excitation stars, and diffuse scatter elements. All rendering uses monochrome ink
 * tones from AetherColours. Timer-driven at 30Hz with breathing animation.
 *
 * Parameter state is read via std::atomic<float>* pointers set by the editor.
 * All visual transitions use exponential smoothing (~300ms settle time).
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
    // Draw methods (each renders one visual layer)
    void drawHeadProfile          (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawReflectionWavefronts (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawDiffuseScatter       (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawAirParticles         (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawExcitationStars      (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCaption              (juce::Graphics& g, juce::Rectangle<float> bounds);

    //==========================================================================
    // Head profile path (built once, transformed per frame)
    static juce::Path createHeadProfilePath();

    //==========================================================================
    // Shape fingerprint: data-driven wave distortion per room shape
    struct ShapeFingerprint
    {
        float xStretch;      // horizontal stretch factor (1.0 = circular)
        float yStretch;      // vertical stretch factor
        float asymmetry;     // 0.0 = symmetric, 1.0 = maximum distortion
        float densityBias;   // ring spacing modifier (1.0 = even)
        float angularWarp;   // angular distortion amount for irregular shapes
        int   ringCount;     // base number of wavefront rings
    };

    static const ShapeFingerprint kShapeFingerprints[7];

    //==========================================================================
    // Smoothed visual state (interpolated each frame)
    float smoothedSize      = 0.4f;   // from reflSize default
    float smoothedProx      = 0.3f;   // from reflProx default
    float smoothedAir       = 0.4f;   // from airAmount default
    float smoothedDrive     = 0.25f;  // from excitDrive default

    // Bypass fade alphas (1.0 = active, 0.0 = bypassed)
    float smoothedReflAlpha  = 1.0f;
    float smoothedAirAlpha   = 1.0f;
    float smoothedExcitAlpha = 1.0f;
    float smoothedTailAlpha  = 1.0f;

    // Animation phase
    float breathPhase       = 0.0f;   // 0..2pi, advances each frame
    float starRotation      = 0.0f;   // continuous spin for excitation stars
    float currentRmsSmoothed = 0.0f;  // smoothed audio RMS level

    // Shape fingerprint interpolation
    ShapeFingerprint currentFingerprint {};
    ShapeFingerprint targetFingerprint {};
    int lastShapeIndex = 0;

    // Cached head profile path and transform
    juce::Path headPath;
    juce::AffineTransform headTransform;

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
