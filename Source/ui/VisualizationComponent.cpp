#include "VisualizationComponent.h"
#include "AetherColours.h"
#include "ParchmentElements.h"
#include "AetherLookAndFeel.h"

//==============================================================================
// Static shape fingerprint data: 7 room shapes mapped to combo box indices
//==============================================================================
const VisualizationComponent::ShapeFingerprint
    VisualizationComponent::kShapeFingerprints[7] =
{
    // The Parlour:      Small, regular, symmetric
    { 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 5 },
    // The Gallery:      Wide, slightly elongated
    { 1.3f, 0.85f, 0.1f, 0.9f, 0.05f, 6 },
    // The Chamber:      Medium, balanced
    { 1.0f, 1.0f, 0.05f, 1.0f, 0.02f, 6 },
    // The Nave:         Tall, narrow (cathedral-like)
    { 0.7f, 1.4f, 0.15f, 0.85f, 0.1f, 7 },
    // The Alcove:       Compact, slightly irregular
    { 1.1f, 0.9f, 0.2f, 1.1f, 0.15f, 4 },
    // The Crypt:        Irregular, compressed, distorted
    { 1.0f, 0.8f, 0.4f, 1.2f, 0.3f, 5 },
    // The Conservatory: Wide, bright, open
    { 1.2f, 1.1f, 0.1f, 0.8f, 0.05f, 7 },
};

//==============================================================================
// Head profile path: Victorian illustration style, right-facing profile
// Coordinates in normalized 0-100 space
//==============================================================================
juce::Path VisualizationComponent::createHeadProfilePath()
{
    juce::Path head;

    // Crown to forehead
    head.startNewSubPath (48.0f, 8.0f);
    head.cubicTo (55.0f, 7.0f, 63.0f, 8.0f, 69.0f, 14.0f);

    // Forehead curve
    head.cubicTo (74.0f, 19.0f, 77.0f, 26.0f, 78.0f, 33.0f);

    // Brow ridge
    head.cubicTo (78.5f, 36.0f, 78.0f, 39.0f, 76.5f, 42.0f);

    // Eye socket indent
    head.cubicTo (75.5f, 44.0f, 75.0f, 46.0f, 75.5f, 48.0f);

    // Nose bridge
    head.cubicTo (76.0f, 50.0f, 77.0f, 52.0f, 78.0f, 55.0f);

    // Nose tip and nostril
    head.cubicTo (78.5f, 57.0f, 78.0f, 58.5f, 76.0f, 59.0f);
    head.cubicTo (74.5f, 59.5f, 73.0f, 59.0f, 72.5f, 58.0f);

    // Upper lip
    head.cubicTo (72.0f, 59.5f, 72.5f, 61.0f, 73.0f, 62.0f);

    // Lower lip
    head.cubicTo (73.5f, 63.5f, 73.0f, 65.0f, 71.5f, 66.0f);

    // Chin
    head.cubicTo (70.0f, 67.5f, 68.0f, 70.0f, 65.0f, 72.0f);

    // Jaw line
    head.cubicTo (61.0f, 74.0f, 56.0f, 76.0f, 52.0f, 77.0f);

    // Under jaw to throat
    head.cubicTo (48.0f, 78.0f, 45.0f, 79.0f, 43.0f, 82.0f);

    // Neck front
    head.cubicTo (42.0f, 85.0f, 41.5f, 89.0f, 41.0f, 93.0f);
    head.lineTo (41.0f, 98.0f);

    // Neck back
    head.lineTo (36.0f, 98.0f);
    head.lineTo (36.0f, 92.0f);

    // Nape of neck
    head.cubicTo (36.0f, 87.0f, 35.0f, 82.0f, 34.0f, 77.0f);

    // Back of skull
    head.cubicTo (32.0f, 70.0f, 29.0f, 60.0f, 28.0f, 50.0f);

    // Skull curve
    head.cubicTo (27.0f, 40.0f, 28.0f, 30.0f, 32.0f, 22.0f);

    // Back to crown
    head.cubicTo (36.0f, 15.0f, 42.0f, 10.0f, 48.0f, 8.0f);

    head.closeSubPath();

    // Ear detail (small arc on the left side of the head)
    head.startNewSubPath (32.0f, 46.0f);
    head.cubicTo (29.0f, 47.0f, 28.0f, 51.0f, 29.0f, 54.0f);
    head.cubicTo (30.0f, 56.0f, 31.5f, 57.0f, 33.0f, 56.0f);

    return head;
}

//==============================================================================
// Constructor / Destructor
//==============================================================================
VisualizationComponent::VisualizationComponent()
{
    headPath = createHeadProfilePath();

    // Initialize shape fingerprints to Parlour default
    currentFingerprint = kShapeFingerprints[0];
    targetFingerprint  = kShapeFingerprints[0];
    lastShapeIndex     = 0;
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
// resized: compute head transform from 0-100 space to component bounds
//==============================================================================
void VisualizationComponent::resized()
{
    auto bounds = getLocalBounds().toFloat();

    // Head occupies roughly 50% of component width, centered
    float headScale = bounds.getWidth() * 0.5f / 100.0f;
    float headWidth = 100.0f * headScale;
    float headHeight = 100.0f * headScale;

    // Center horizontally, slightly above center vertically
    float offsetX = bounds.getCentreX() - headWidth * 0.5f;
    float offsetY = bounds.getCentreY() - headHeight * 0.55f;

    headTransform = juce::AffineTransform::scale (headScale)
                        .translated (offsetX, offsetY);
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
    constexpr float kSmooth       = 0.12f;   // ~300ms settle at 30Hz
    constexpr float kBypassSmooth = 0.15f;   // slightly faster for bypass fade
    constexpr float kShapeSmooth  = 0.12f;   // shape fingerprint transition

    // Parameter smoothing
    if (pRoomSize)   smoothedSize  += kSmooth * (pRoomSize->load (std::memory_order_relaxed)  - smoothedSize);
    if (pProximity)  smoothedProx  += kSmooth * (pProximity->load (std::memory_order_relaxed) - smoothedProx);
    if (pAirAmount)  smoothedAir   += kSmooth * (pAirAmount->load (std::memory_order_relaxed) - smoothedAir);
    if (pExcitDrive) smoothedDrive += kSmooth * (pExcitDrive->load (std::memory_order_relaxed) - smoothedDrive);

    // Bypass fade: target 0.0 when bypassed, 1.0 when active
    auto bypassTarget = [] (std::atomic<float>* p) -> float {
        return (p && p->load (std::memory_order_relaxed) >= 0.5f) ? 0.0f : 1.0f;
    };
    smoothedReflAlpha  += kBypassSmooth * (bypassTarget (pReflBypass)  - smoothedReflAlpha);
    smoothedAirAlpha   += kBypassSmooth * (bypassTarget (pAirBypass)   - smoothedAirAlpha);
    smoothedExcitAlpha += kBypassSmooth * (bypassTarget (pExcitBypass) - smoothedExcitAlpha);
    smoothedTailAlpha  += kBypassSmooth * (bypassTarget (pTailBypass)  - smoothedTailAlpha);

    // Shape fingerprint transition
    if (pShape)
    {
        int shapeIndex = static_cast<int> (pShape->load (std::memory_order_relaxed));
        shapeIndex = juce::jlimit (0, 6, shapeIndex);

        if (shapeIndex != lastShapeIndex)
        {
            targetFingerprint = kShapeFingerprints[shapeIndex];
            lastShapeIndex = shapeIndex;
        }
    }

    // Interpolate current fingerprint toward target
    currentFingerprint.xStretch    += kShapeSmooth * (targetFingerprint.xStretch    - currentFingerprint.xStretch);
    currentFingerprint.yStretch    += kShapeSmooth * (targetFingerprint.yStretch    - currentFingerprint.yStretch);
    currentFingerprint.asymmetry   += kShapeSmooth * (targetFingerprint.asymmetry   - currentFingerprint.asymmetry);
    currentFingerprint.densityBias += kShapeSmooth * (targetFingerprint.densityBias - currentFingerprint.densityBias);
    currentFingerprint.angularWarp += kShapeSmooth * (targetFingerprint.angularWarp - currentFingerprint.angularWarp);
    currentFingerprint.ringCount    = targetFingerprint.ringCount;  // discrete, no lerp

    // Breathing phase: 4-second cycle = 2*pi / (4.0 * 30.0) per frame
    constexpr float kBreathIncrement = juce::MathConstants<float>::twoPi / (4.0f * 30.0f);

    // RMS level: read and reset via exchange, then smooth
    float rawRms = pRmsLevel ? pRmsLevel->exchange (0.0f, std::memory_order_relaxed) : 0.0f;

    // Asymmetric smoothing: fast attack (0.3), slow release (0.05)
    if (rawRms > currentRmsSmoothed)
        currentRmsSmoothed += 0.3f * (rawRms - currentRmsSmoothed);
    else
        currentRmsSmoothed += 0.05f * (rawRms - currentRmsSmoothed);

    // Breathing speed: baseline + audio boost
    float speedBoost = 1.0f + currentRmsSmoothed * 0.5f;
    breathPhase += kBreathIncrement * speedBoost;
    if (breathPhase > juce::MathConstants<float>::twoPi)
        breathPhase -= juce::MathConstants<float>::twoPi;

    // Star rotation: continuous spin (~0.6 rad/sec at 30Hz)
    starRotation += 0.02f;
}

//==============================================================================
// paint: layered drawing
//==============================================================================
void VisualizationComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Layer 1: Head profile (always visible -- constant anchor)
    drawHeadProfile (g, bounds);

    // Layer 2: Reflection wavefronts (Stage II)
    if (smoothedReflAlpha > 0.001f)
        drawReflectionWavefronts (g, bounds);

    // Layer 3: Diffuse tail scatter (Stage VI)
    if (smoothedTailAlpha > 0.001f)
        drawDiffuseScatter (g, bounds);

    // Layer 4: Air particles (Stage III)
    if (smoothedAirAlpha > 0.001f && smoothedAir > 0.01f)
        drawAirParticles (g, bounds);

    // Layer 5: Excitation stars (Stage IV)
    if (smoothedExcitAlpha > 0.001f && smoothedDrive > 0.01f)
        drawExcitationStars (g, bounds);

    // Caption (always visible)
    drawCaption (g, bounds);
}

//==============================================================================
// drawHeadProfile: Victorian profile silhouette (ink stroke)
//==============================================================================
void VisualizationComponent::drawHeadProfile (juce::Graphics& g,
                                               juce::Rectangle<float> /*bounds*/)
{
    g.setColour (juce::Colour (AetherColours::ink).withAlpha (0.7f));
    g.strokePath (headPath, juce::PathStrokeType (1.0f), headTransform);
}

//==============================================================================
// drawReflectionWavefronts: concentric rings with shape distortion
//==============================================================================
void VisualizationComponent::drawReflectionWavefronts (juce::Graphics& g,
                                                        juce::Rectangle<float> bounds)
{
    const float layerAlpha = smoothedReflAlpha;
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY() - 10.0f;  // slightly above center

    // Breathing scale: 1.0 to 1.02 baseline + audio boost
    float breathScale = 1.0f + 0.02f * (0.5f + 0.5f * std::sin (breathPhase));
    breathScale += 0.01f * currentRmsSmoothed;

    // Room size controls extent and spacing
    float maxRadius   = 20.0f + smoothedSize * 140.0f;   // 20-160px
    float ringSpacing = 8.0f  + smoothedSize * 12.0f;    // 8-20px
    int ringCount     = currentFingerprint.ringCount;

    // Source position from proximity: closer = source nearer to head
    float sourceDistance = 30.0f + (1.0f - smoothedProx) * 80.0f;
    float sourceX = cx - sourceDistance;

    auto inkLightColour = juce::Colour (AetherColours::inkLight);

    for (int i = 0; i < ringCount; ++i)
    {
        float t = (ringCount > 1)
                    ? static_cast<float> (i) / static_cast<float> (ringCount - 1)
                    : 0.0f;

        // Staggered breathing: inner rings pulse slightly before outer
        float ringBreathScale = 1.0f + 0.02f
            * (0.5f + 0.5f * std::sin (breathPhase + static_cast<float> (i) * 0.3f));
        ringBreathScale += 0.01f * currentRmsSmoothed;

        float radius = ringSpacing * static_cast<float> (i + 1) * ringBreathScale;
        if (radius > maxRadius)
            break;

        // Shape distortion: stretch horizontally and vertically
        float xR = radius * currentFingerprint.xStretch;
        float yR = radius * currentFingerprint.yStretch;

        // Opacity: direct vs reflected balance based on proximity
        float directAlpha    = (1.0f - t * 0.6f) * smoothedProx;
        float reflectedAlpha = t * 0.5f * (1.0f - smoothedProx);
        float ringAlpha      = juce::jlimit (0.0f, 1.0f, directAlpha + reflectedAlpha) * layerAlpha;

        if (ringAlpha < 0.02f)
            continue;

        // Build ellipse ring
        juce::Path ring;
        ring.addEllipse (-xR, -yR, xR * 2.0f, yR * 2.0f);

        // Apply angular warp for irregular shapes
        if (currentFingerprint.angularWarp > 0.001f)
        {
            // Distort by applying a slight rotation proportional to asymmetry
            float warpAngle = currentFingerprint.angularWarp * 0.15f
                              * std::sin (static_cast<float> (i) * 1.5f);
            ring.applyTransform (juce::AffineTransform::rotation (warpAngle));
        }

        auto transform = juce::AffineTransform::translation (sourceX, cy);
        g.setColour (inkLightColour.withAlpha (ringAlpha));
        g.strokePath (ring, juce::PathStrokeType (0.8f), transform);
    }
}

//==============================================================================
// drawDiffuseScatter: ambient wash beyond wavefront extent
//==============================================================================
void VisualizationComponent::drawDiffuseScatter (juce::Graphics& g,
                                                   juce::Rectangle<float> bounds)
{
    const float layerAlpha = smoothedTailAlpha * 0.3f;
    if (layerAlpha < 0.02f)
        return;

    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY() - 10.0f;

    // Place scatter segments beyond the wavefront area
    float baseRadius = 20.0f + smoothedSize * 140.0f + 15.0f;
    constexpr int segmentCount = 20;
    constexpr float goldenAngle = 2.39996f;

    auto inkFaintColour = juce::Colour (AetherColours::inkFaint);

    for (int i = 0; i < segmentCount; ++i)
    {
        float angle = static_cast<float> (i) * goldenAngle;
        float dist  = baseRadius + static_cast<float> (i % 5) * 8.0f;

        // Slight drift with breathing
        dist += 2.0f * std::sin (breathPhase + static_cast<float> (i) * 0.5f);

        float sx = cx + dist * std::cos (angle);
        float sy = cy + dist * std::sin (angle);

        // Short line segments radiating outward
        float dx = 6.0f * std::cos (angle);
        float dy = 6.0f * std::sin (angle);

        g.setColour (inkFaintColour.withAlpha (layerAlpha));
        g.drawLine (sx, sy, sx + dx, sy + dy, 0.6f);
    }
}

//==============================================================================
// drawAirParticles: scattered dots scaling with air amount
//==============================================================================
void VisualizationComponent::drawAirParticles (juce::Graphics& g,
                                                juce::Rectangle<float> bounds)
{
    const float layerAlpha = smoothedAirAlpha;

    // Particle count scales linearly: 0 at Air=0%, ~40 at Air=100%
    int particleCount = static_cast<int> (smoothedAir * 40.0f);
    constexpr float particleSize = 1.0f;

    auto inkFaintColour = juce::Colour (AetherColours::inkFaint);
    float alpha = layerAlpha * (0.15f + smoothedAir * 0.35f);

    if (alpha < 0.02f)
        return;

    g.setColour (inkFaintColour.withAlpha (alpha));

    for (int i = 0; i < particleCount; ++i)
    {
        // Deterministic scatter using hash-like distribution (sin-based pseudo-random)
        float hash1 = std::sin (static_cast<float> (i) * 127.1f) * 43758.5453f;
        float hash2 = std::sin (static_cast<float> (i) * 269.5f) * 43758.5453f;
        float px = bounds.getX() + (hash1 - std::floor (hash1)) * bounds.getWidth();
        float py = bounds.getY() + (hash2 - std::floor (hash2)) * bounds.getHeight();

        // Slight drift with breathing phase for organic movement
        px += 1.5f * std::sin (breathPhase + static_cast<float> (i) * 0.3f);
        py += 1.0f * std::cos (breathPhase + static_cast<float> (i) * 0.7f);

        g.fillEllipse (px, py, particleSize, particleSize);
    }
}

//==============================================================================
// drawExcitationStars: spinning 4-pointed stars
//==============================================================================
void VisualizationComponent::drawExcitationStars (juce::Graphics& g,
                                                    juce::Rectangle<float> bounds)
{
    const float layerAlpha = smoothedExcitAlpha;

    int starCount  = static_cast<int> (3.0f + smoothedDrive * 8.0f);   // 3-11 stars
    float starSize = 2.0f + smoothedDrive * 2.0f;                       // 2-4px outer radius
    float innerRadius = starSize * 0.4f;

    constexpr float goldenAngle = 2.39996f;
    constexpr int numPoints = 4;  // 4-pointed star

    auto inkColour = juce::Colour (AetherColours::ink);

    for (int i = 0; i < starCount; ++i)
    {
        float angle = static_cast<float> (i) * goldenAngle;
        float dist  = 30.0f + static_cast<float> (i * 13 % 7) * 15.0f;

        float sx = bounds.getCentreX() + dist * std::cos (angle);
        float sy = bounds.getCentreY() + dist * std::sin (angle);
        float rotation = starRotation + static_cast<float> (i) * 0.5f;
        float starAlpha = layerAlpha * (0.4f + smoothedDrive * 0.6f);

        if (starAlpha < 0.02f)
            continue;

        // Build 4-pointed star path (8 vertices: alternating outer/inner)
        juce::Path star;
        for (int v = 0; v < numPoints * 2; ++v)
        {
            float vAngle = rotation + static_cast<float> (v) * juce::MathConstants<float>::pi
                           / static_cast<float> (numPoints);
            float r = (v % 2 == 0) ? starSize : innerRadius;
            float px = sx + r * std::cos (vAngle);
            float py = sy + r * std::sin (vAngle);

            if (v == 0)
                star.startNewSubPath (px, py);
            else
                star.lineTo (px, py);
        }
        star.closeSubPath();

        g.setColour (inkColour.withAlpha (starAlpha));
        g.strokePath (star, juce::PathStrokeType (0.6f));
    }
}

//==============================================================================
// drawCaption: "Fig. 1 -- The Listener" in EB Garamond italic
//==============================================================================
void VisualizationComponent::drawCaption (juce::Graphics& g,
                                           juce::Rectangle<float> bounds)
{
    // Access EB Garamond italic font via the LookAndFeel
    auto* laf = dynamic_cast<AetherLookAndFeel*> (&getLookAndFeel());
    if (laf == nullptr)
        return;

    auto font = laf->getBodyFontItalic (10.0f);

    // Caption placed at the bottom of the visualization area
    float captionY = bounds.getBottom() - 14.0f;

    ParchmentElements::drawLetterSpacedText (
        g,
        juce::String::fromUTF8 ("Fig. 1 \xe2\x80\x94 The Listener"),
        bounds.getX(), captionY, bounds.getWidth(),
        2.0f,  // tracking
        font,
        juce::Colour (AetherColours::inkFaint),
        juce::Justification::horizontallyCentred);
}
