#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Parameters.h"
#include "ui/AetherColours.h"
#include "ui/ParchmentElements.h"

//==============================================================================
// Layout constants
//==============================================================================
namespace Layout
{
    constexpr int kWidth           = 900;
    constexpr int kHeight          = 530;
    constexpr int kHeaderHeight    = 50;
    constexpr int kFooterY         = 490;
    constexpr int kFooterHeight    = 40;
    constexpr int kContentTop      = 53;  // After double-rule gap
    constexpr int kContentBottom   = 487; // Before footer double-rule
    constexpr int kLeftPanelWidth  = 220;
    constexpr int kRightPanelWidth = 220;
    constexpr int kCenterX         = 220;
    constexpr int kCenterWidth     = 460;
    constexpr int kRightX          = 680;
    constexpr int kSidePadding     = 16;
    constexpr int kCenterPadding   = 20;
}

//==============================================================================
AetherEditor::AetherEditor (AetherProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), presetSelector (processorRef.apvts),
      materialUpArrow (ArrowStepButton::Up, materialCombo),
      materialDownArrow (ArrowStepButton::Down, materialCombo),
      shapeUpArrow (ArrowStepButton::Up, reflShapeCombo),
      shapeDownArrow (ArrowStepButton::Down, reflShapeCombo),
      presetUpArrow (ArrowStepButton::Up, presetSelector.getComboBox()),
      presetDownArrow (ArrowStepButton::Down, presetSelector.getComboBox())
{
    // 1. Set LookAndFeel (NOT setDefaultLookAndFeel -- per Crucible pattern)
    setLookAndFeel (&lookAndFeel);

    // 2. Generate cached background
    backgroundImage = ParchmentElements::generateBackground (Layout::kWidth, Layout::kHeight);

    // 3. Set size
    setSize (Layout::kWidth, Layout::kHeight);

    // =========================================================================
    // 4. Add section components
    // =========================================================================
    addAndMakeVisible (resSection);
    addAndMakeVisible (reflSection);
    addAndMakeVisible (airSection);
    addAndMakeVisible (excitSection);
    addAndMakeVisible (toneSection);
    addAndMakeVisible (tailSection);
    addAndMakeVisible (outputSection);

    // =========================================================================
    // 5. Add knobs
    // =========================================================================
    // Stage I
    addAndMakeVisible (resWeightKnob);

    // Stage II (center -- large 64px knobs)
    addAndMakeVisible (reflSizeKnob);
    addAndMakeVisible (reflProxKnob);
    addAndMakeVisible (reflWidthKnob);

    // Stage III
    addAndMakeVisible (airAmountKnob);

    // Stage IV
    addAndMakeVisible (excitDriveKnob);

    // Stage V
    addAndMakeVisible (toneAmbKnob);

    // Stage VI
    addAndMakeVisible (tailDecayKnob);
    addAndMakeVisible (tailDiffKnob);

    // Output
    addAndMakeVisible (mixKnob);
    addAndMakeVisible (levelKnob);

    // =========================================================================
    // 6. ComboBoxes for Material (10 items) and Shape (30 items)
    // =========================================================================
    materialCombo.addItemList (
        juce::StringArray { "Pine", "Oak", "Walnut", "Mahogany", "Iron",
                            "Steel", "Copper", "Limestone", "Marble", "Granite" }, 1);
    materialCombo.setColour (juce::ComboBox::textColourId, juce::Colour (AetherColours::inkLight));
    materialCombo.setColour (juce::ComboBox::outlineColourId, juce::Colour (AetherColours::inkGhost));
    materialCombo.setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    materialCombo.setColour (juce::ComboBox::arrowColourId, juce::Colour (AetherColours::inkFaint));
    materialCombo.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (materialCombo);

    // Small Rooms
    reflShapeCombo.addSectionHeading ("Small Rooms");
    reflShapeCombo.addItem ("The Parlour", 1);
    reflShapeCombo.addItem ("The Alcove", 2);
    reflShapeCombo.addItem ("The Crypt", 3);
    reflShapeCombo.addItem ("The Vestibule", 4);
    reflShapeCombo.addItem ("The Closet", 5);
    reflShapeCombo.addItem ("The Study", 6);
    reflShapeCombo.addItem ("The Telephone Box", 7);
    reflShapeCombo.addItem ("The Pantry", 8);
    reflShapeCombo.addItem ("The Confessional", 9);
    reflShapeCombo.addItem ("The Powder Room", 10);
    // Medium Rooms
    reflShapeCombo.addSectionHeading ("Medium Rooms");
    reflShapeCombo.addItem ("The Chamber", 11);
    reflShapeCombo.addItem ("The Gallery", 12);
    reflShapeCombo.addItem ("The Conservatory", 13);
    reflShapeCombo.addItem ("The Scriptorium", 14);
    reflShapeCombo.addItem ("The Library", 15);
    reflShapeCombo.addItem ("The Drawing Room", 16);
    reflShapeCombo.addItem ("The Workshop", 17);
    reflShapeCombo.addItem ("The Refectory", 18);
    reflShapeCombo.addItem ("The Solarium", 19);
    reflShapeCombo.addItem ("The Apothecary", 20);
    // Large Rooms
    reflShapeCombo.addSectionHeading ("Large Rooms");
    reflShapeCombo.addItem ("The Nave", 21);
    reflShapeCombo.addItem ("The Ballroom", 22);
    reflShapeCombo.addItem ("The Atrium", 23);
    reflShapeCombo.addItem ("The Chapel", 24);
    reflShapeCombo.addItem ("The Warehouse", 25);
    reflShapeCombo.addItem ("The Cistern", 26);
    reflShapeCombo.addItem ("The Observatory", 27);
    reflShapeCombo.addItem ("The Great Hall", 28);
    reflShapeCombo.addItem ("The Greenhouse", 29);
    reflShapeCombo.addItem ("The Mausoleum", 30);
    reflShapeCombo.setColour (juce::ComboBox::textColourId, juce::Colour (AetherColours::inkLight));
    reflShapeCombo.setColour (juce::ComboBox::outlineColourId, juce::Colour (AetherColours::inkGhost));
    reflShapeCombo.setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    reflShapeCombo.setColour (juce::ComboBox::arrowColourId, juce::Colour (AetherColours::inkFaint));
    reflShapeCombo.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (reflShapeCombo);

    // =========================================================================
    // 7. Toggles
    // =========================================================================
    addAndMakeVisible (airCharToggle);
    addAndMakeVisible (toneGateToggle);

    // =========================================================================
    // 8. Preset selector
    // =========================================================================
    addAndMakeVisible (presetSelector);
    presetSelector.setMixLockButton (&mixLockButton);

    // 8a. Mix lock + arrow step buttons
    addAndMakeVisible (mixLockButton);
    addAndMakeVisible (materialUpArrow);
    addAndMakeVisible (materialDownArrow);
    addAndMakeVisible (shapeUpArrow);
    addAndMakeVisible (shapeDownArrow);
    addAndMakeVisible (presetUpArrow);
    addAndMakeVisible (presetDownArrow);

    // Preset arrows should skip the "Save Preset..." action item
    presetUpArrow.addSkipId (99);    // kSavePresetId
    presetDownArrow.addSkipId (99);

    // 8b. Visualization component
    addAndMakeVisible (vizComponent);

    // =========================================================================
    // 9. Set up ALL parameter attachments AFTER addAndMakeVisible
    //    (per Crucible pattern -- avoids silent initial sync failure)
    // =========================================================================
    auto& apvts = processorRef.apvts;

    // Knob attachments (11 knobs)
    resWeightKnob.attachToParameter (apvts, ParamIDs::resWeight);
    reflSizeKnob.attachToParameter (apvts, ParamIDs::reflSize);
    reflProxKnob.attachToParameter (apvts, ParamIDs::reflProx);
    reflWidthKnob.attachToParameter (apvts, ParamIDs::reflWidth);
    airAmountKnob.attachToParameter (apvts, ParamIDs::airAmount);
    excitDriveKnob.attachToParameter (apvts, ParamIDs::excitDrive);
    toneAmbKnob.attachToParameter (apvts, ParamIDs::toneAmb);
    tailDecayKnob.attachToParameter (apvts, ParamIDs::tailDecay);
    tailDiffKnob.attachToParameter (apvts, ParamIDs::tailDiff);
    mixKnob.attachToParameter (apvts, ParamIDs::outMix);
    levelKnob.attachToParameter (apvts, ParamIDs::outLevel);

    // ComboBox attachments (2 dropdowns)
    materialAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::resMaterial, materialCombo);
    shapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::reflShape, reflShapeCombo);

    // Toggle attachments (2 toggles via hidden ComboBox)
    airCharToggle.attachToParameter (apvts, ParamIDs::airChar);
    toneGateToggle.attachToParameter (apvts, ParamIDs::toneGate);

    // Bypass button attachments (6 bypasses)
    resSection.bypassButton.attachToParameter (apvts, ParamIDs::resBypass);
    reflSection.bypassButton.attachToParameter (apvts, ParamIDs::reflBypass);
    airSection.bypassButton.attachToParameter (apvts, ParamIDs::airBypass);
    excitSection.bypassButton.attachToParameter (apvts, ParamIDs::excitBypass);
    toneSection.bypassButton.attachToParameter (apvts, ParamIDs::toneBypass);
    tailSection.bypassButton.attachToParameter (apvts, ParamIDs::tailBypass);

    // Total: 11 + 2 + 2 + 6 = 21 parameters connected

    // =========================================================================
    // =========================================================================
    // Mix knob accent border
    mixKnob.setAccentBorder (true);

    // 10. Value formatters for special display params
    // =========================================================================
    // Room Size: "62% (Medium)" format
    reflSizeKnob.setValueFormatter ([] (float normalised, const juce::String& paramText) -> juce::String {
        return juce::String (juce::roundToInt (normalised * 100.0f)) + "% (" + paramText + ")";
    });

    // Proximity: "30% (Nr-Mid)" format
    reflProxKnob.setValueFormatter ([] (float normalised, const juce::String& paramText) -> juce::String {
        return juce::String (juce::roundToInt (normalised * 100.0f)) + "% (" + paramText + ")";
    });

    // Decay: "150 ms" or "1.2s" format (switch at 1000ms)
    tailDecayKnob.setValueFormatter ([] (float /*normalised*/, const juce::String& paramText) -> juce::String {
        // paramText from the parameter is already "150 ms" format
        // Parse the ms value to decide on format
        float msValue = paramText.trimCharactersAtEnd (" ms").getFloatValue();
        if (msValue >= 1000.0f)
            return juce::String (msValue / 1000.0f, 1) + "s";
        return paramText;
    });

    // Output Level: "0.0 dB" format with 1 decimal -- uses parameter's own text
    // (already formatted correctly in Parameters.h)

    // Width, Air, Drive, Ambience, Diffusion, Mix, Weight: "62%" whole number percent
    // (already formatted correctly in Parameters.h -- default display works)

    // =========================================================================
    // 11. Bypass state listeners
    // =========================================================================
    apvts.addParameterListener (ParamIDs::resBypass, this);
    apvts.addParameterListener (ParamIDs::reflBypass, this);
    apvts.addParameterListener (ParamIDs::airBypass, this);
    apvts.addParameterListener (ParamIDs::excitBypass, this);
    apvts.addParameterListener (ParamIDs::toneBypass, this);
    apvts.addParameterListener (ParamIDs::tailBypass, this);

    // Sync initial bypass states
    auto syncBypass = [&] (const juce::String& paramID, SectionComponent& section,
                           std::initializer_list<juce::Component*> controls)
    {
        bool isBypassed = apvts.getRawParameterValue (paramID)->load() >= 0.5f;
        section.setBypassed (isBypassed);
        for (auto* ctrl : controls)
        {
            if (auto* knob = dynamic_cast<AetherKnob*> (ctrl))
                knob->setBypassed (isBypassed);
            else if (auto* toggle = dynamic_cast<AetherToggle*> (ctrl))
                toggle->setBypassed (isBypassed);
            else
                ctrl->setAlpha (isBypassed ? 0.35f : 1.0f);
        }
    };

    syncBypass (ParamIDs::resBypass, resSection, { &resWeightKnob, &materialCombo });
    syncBypass (ParamIDs::reflBypass, reflSection, { &reflSizeKnob, &reflProxKnob, &reflWidthKnob });
    syncBypass (ParamIDs::airBypass, airSection, { &airAmountKnob, &airCharToggle });
    syncBypass (ParamIDs::excitBypass, excitSection, { &excitDriveKnob });
    syncBypass (ParamIDs::toneBypass, toneSection, { &toneAmbKnob, &toneGateToggle });
    syncBypass (ParamIDs::tailBypass, tailSection, { &tailDecayKnob, &tailDiffKnob });

    // =========================================================================
    // 12. Wire visualization parameter sources
    // =========================================================================
    vizComponent.setParameterSources (
        apvts.getRawParameterValue (ParamIDs::reflSize),
        apvts.getRawParameterValue (ParamIDs::reflShape),
        apvts.getRawParameterValue (ParamIDs::reflProx),
        apvts.getRawParameterValue (ParamIDs::airAmount),
        apvts.getRawParameterValue (ParamIDs::excitDrive),
        apvts.getRawParameterValue (ParamIDs::reflBypass),
        apvts.getRawParameterValue (ParamIDs::airBypass),
        apvts.getRawParameterValue (ParamIDs::excitBypass),
        apvts.getRawParameterValue (ParamIDs::tailBypass),
        &processorRef.visualizationRmsLevel
    );
    vizComponent.startAnimation();
}

//==============================================================================
AetherEditor::~AetherEditor()
{
    // Stop visualization timer before any cleanup to prevent dangling pointer access
    vizComponent.stopAnimation();

    // Remove parameter listeners
    auto& apvts = processorRef.apvts;
    apvts.removeParameterListener (ParamIDs::resBypass, this);
    apvts.removeParameterListener (ParamIDs::reflBypass, this);
    apvts.removeParameterListener (ParamIDs::airBypass, this);
    apvts.removeParameterListener (ParamIDs::excitBypass, this);
    apvts.removeParameterListener (ParamIDs::toneBypass, this);
    apvts.removeParameterListener (ParamIDs::tailBypass, this);

    // CRITICAL: clear LookAndFeel to avoid dangling pointer (Crucible pattern)
    setLookAndFeel (nullptr);
}

//==============================================================================
// Bypass listener
//==============================================================================
void AetherEditor::parameterChanged (const juce::String& parameterID, float newValue)
{
    // Called from audio thread -- bounce to message thread
    const bool isBypassed = newValue >= 0.5f;

    juce::MessageManager::callAsync ([this, parameterID, isBypassed]
    {
        if (parameterID == ParamIDs::resBypass)
        {
            resSection.setBypassed (isBypassed);
            resWeightKnob.setBypassed (isBypassed);
            materialCombo.setAlpha (isBypassed ? 0.35f : 1.0f);
        }
        else if (parameterID == ParamIDs::reflBypass)
        {
            reflSection.setBypassed (isBypassed);
            reflSizeKnob.setBypassed (isBypassed);
            reflProxKnob.setBypassed (isBypassed);
            reflWidthKnob.setBypassed (isBypassed);
        }
        else if (parameterID == ParamIDs::airBypass)
        {
            airSection.setBypassed (isBypassed);
            airAmountKnob.setBypassed (isBypassed);
            airCharToggle.setBypassed (isBypassed);
        }
        else if (parameterID == ParamIDs::excitBypass)
        {
            excitSection.setBypassed (isBypassed);
            excitDriveKnob.setBypassed (isBypassed);
        }
        else if (parameterID == ParamIDs::toneBypass)
        {
            toneSection.setBypassed (isBypassed);
            toneAmbKnob.setBypassed (isBypassed);
            toneGateToggle.setBypassed (isBypassed);
        }
        else if (parameterID == ParamIDs::tailBypass)
        {
            tailSection.setBypassed (isBypassed);
            tailDecayKnob.setBypassed (isBypassed);
            tailDiffKnob.setBypassed (isBypassed);
        }
    });
}

//==============================================================================
void AetherEditor::paint (juce::Graphics& g)
{
    // 1. Draw cached background
    g.drawImageAt (backgroundImage, 0, 0);

    // 2. Corner brackets
    ParchmentElements::drawCornerBrackets (g, getLocalBounds());

    // 4. Header zone (0-40px)
    {
        // "AETHER" title centred over left column
        ParchmentElements::drawLetterSpacedText (
            g, "AETHER", 0.0f, 26.0f,
            static_cast<float> (Layout::kLeftPanelWidth), 6.0f,
            lookAndFeel.getDisplayFont (30.0f),
            juce::Colour (AetherColours::ink),
            juce::Justification::horizontallyCentred);
    }

    // 5. Header double-rule border
    ParchmentElements::drawDoubleRule (g, 40.0f, 0.0f, 900.0f, true);

    // 6. Footer zone (500-530px)
    {
        // Decorative flourish above footer rule (centered three-dot pattern)
        {
            const float flY = 496.0f;
            g.setColour (juce::Colour (AetherColours::inkGhost).withAlpha (0.5f));
            g.fillEllipse (448.0f, flY, 1.5f, 1.5f);    // center dot
            g.fillEllipse (442.0f, flY, 1.0f, 1.0f);    // left dot
            g.fillEllipse (454.0f, flY, 1.0f, 1.0f);    // right dot
            // Thin flanking lines
            g.drawLine (420.0f, flY + 0.5f, 439.0f, flY + 0.5f, 0.3f);
            g.drawLine (458.0f, flY + 0.5f, 480.0f, flY + 0.5f, 0.3f);
        }

        // Footer double-rule border
        ParchmentElements::drawDoubleRule (g, 500.0f, 0.0f, 900.0f, false);

        // Version at left
        ParchmentElements::drawLetterSpacedText (
            g, "V0.1.0", 30.0f, 518.0f, 200.0f, 2.0f,
            lookAndFeel.getSpectralFont (10.0f),
            juce::Colour (AetherColours::inkFaint),
            juce::Justification::left);

        // Cairn maker's mark centred: triangle icon + "CAIRN" side by side
        {
            const auto markColour = juce::Colour (AetherColours::inkFaint);
            const float baseY = 515.0f;

            // Measure "CAIRN" text width for centering the pair
            const float textWidth = 42.0f;
            const float triH = 9.0f;
            const float triW = triH * 1.15f;
            const float gap = 6.0f;
            const float totalW = triW + gap + textWidth;
            const float startX = (900.0f - totalW) / 2.0f;

            // Triangle icon (vertically centered with text baseline)
            const float triCentreX = startX + triW / 2.0f;
            const float triTop = baseY - triH + 1.0f;

            juce::Path triangle;
            triangle.startNewSubPath (triCentreX, triTop);
            triangle.lineTo (triCentreX + triW / 2.0f, triTop + triH);
            triangle.lineTo (triCentreX - triW / 2.0f, triTop + triH);
            triangle.closeSubPath();

            g.setColour (markColour);
            g.strokePath (triangle, juce::PathStrokeType (0.8f, juce::PathStrokeType::mitered));

            // Bisecting line at ~62% height
            const float lineY = triTop + triH * 0.62f;
            const float extend = triW * 0.15f;
            g.setColour (markColour.withAlpha (0.5f));
            g.drawLine (triCentreX - triW / 2.0f - extend, lineY,
                        triCentreX + triW / 2.0f + extend, lineY, 0.5f);

            // "CAIRN" wordmark to the right of triangle
            const float textX = startX + triW + gap;
            ParchmentElements::drawLetterSpacedText (
                g, "CAIRN", textX, baseY, textWidth + 10.0f, 3.0f,
                lookAndFeel.getDisplayFont (10.0f),
                markColour,
                juce::Justification::left);
        }

    }

    // 7. Column dividers: 1px Ink Ghost vertical lines
    g.setColour (juce::Colour (AetherColours::inkGhost));
    g.drawLine (220.0f, 43.0f, 220.0f, 497.0f, 1.0f);
    g.drawLine (680.0f, 43.0f, 680.0f, 497.0f, 1.0f);

    // 8. Diamond dividers between left panel sections
    {
        float leftX = static_cast<float> (Layout::kSidePadding);
        float rightX = static_cast<float> (Layout::kLeftPanelWidth - Layout::kSidePadding);
        ParchmentElements::drawDiamondDivider (g, 200.0f, leftX, rightX);
        ParchmentElements::drawDiamondDivider (g, 330.0f, leftX, rightX);
    }

    // 9. Diamond dividers between right panel sections
    {
        float leftX = static_cast<float> (Layout::kRightX + Layout::kSidePadding);
        float rightX = static_cast<float> (Layout::kRightX + Layout::kRightPanelWidth - Layout::kSidePadding);
        ParchmentElements::drawDiamondDivider (g, 200.0f, leftX, rightX);
        ParchmentElements::drawDiamondDivider (g, 330.0f, leftX, rightX);
    }

    // 10. Display area frame (recessed groove matching knob indent style)
    {
        auto vizRect = juce::Rectangle<float> (284.0f, 76.0f, 332.0f, 248.0f);
        constexpr float grooveW = 2.5f;
        constexpr float cornerR = 0.0f;

        // Groove channel fill (centered on viz edge, extends outward)
        auto grooveCentre = vizRect.expanded (grooveW / 2.0f);
        g.setColour (juce::Colour (AetherColours::parchmentDark));
        g.drawRoundedRectangle (grooveCentre, cornerR, grooveW);

        // Top-lit shadow (recessed depth)
        g.setColour (juce::Colour (AetherColours::shadow).withAlpha (0.12f));
        g.drawRoundedRectangle (grooveCentre.translated (0.0f, -0.5f), cornerR, grooveW);

        // Outer edge line
        auto outerEdge = vizRect.expanded (grooveW);
        g.setColour (juce::Colour (AetherColours::inkLight).withAlpha (0.3f));
        g.drawRoundedRectangle (outerEdge, cornerR + 0.5f, 0.75f);
    }
}

//==============================================================================
void AetherEditor::resized()
{
    using namespace Layout;

    // =========================================================================
    // Header
    // =========================================================================
    // Center preset selector in header (220px: ~184 combo + 36 save button)
    presetSelector.setBounds ((kWidth - 220) / 2, 10, 220, 22);
    // Preset arrow buttons (inside combo, offset from right edge)
    presetUpArrow.setBounds (presetSelector.getRight() - 18, presetSelector.getY(), 14, 11);
    presetDownArrow.setBounds (presetSelector.getRight() - 18, presetSelector.getY() + 11, 14, 11);

    // =========================================================================
    // Shared layout metrics
    // =========================================================================
    const int knobH = 89;   // 56px knob + 2+14+1+16 label/value
    const int knobDia = 56;
    const int knobBoundsW = knobDia + 16;  // Extra width for dot markers

    // =========================================================================
    // Left panel: Stages I, IV, V
    // =========================================================================
    {
        const int lx = kSidePadding;
        const int lw = kLeftPanelWidth - 2 * kSidePadding;

        // Stage I: Resonance (y=46, h=148)
        resSection.setBounds (lx, 46, lw, 148);
        auto resCtrl = resSection.getControlArea().translated (lx, 46);
        int knobX = resCtrl.getX() + (resCtrl.getWidth() - knobBoundsW) / 2;
        resWeightKnob.setBounds (knobX, resCtrl.getY(), knobBoundsW, knobH);
        materialCombo.setBounds (resCtrl.getX(), resCtrl.getY() + knobH + 4, resCtrl.getWidth(), 24);
        // Material arrow buttons (inside combo box, offset from right edge)
        materialUpArrow.setBounds (materialCombo.getRight() - 18, materialCombo.getY(), 14, 12);
        materialDownArrow.setBounds (materialCombo.getRight() - 18, materialCombo.getY() + 12, 14, 12);

        // Stage IV: Excitation (y=210, h=115)
        excitSection.setBounds (lx, 210, lw, 115);
        auto excitCtrl = excitSection.getControlArea().translated (lx, 210);
        knobX = excitCtrl.getX() + (excitCtrl.getWidth() - knobBoundsW) / 2;
        excitDriveKnob.setBounds (knobX, excitCtrl.getY(), knobBoundsW, knobH);

        // Stage V: Room Tone (y=340, h=140)
        toneSection.setBounds (lx, 340, lw, 140);
        auto toneCtrl = toneSection.getControlArea().translated (lx, 340);
        knobX = toneCtrl.getX() + (toneCtrl.getWidth() - knobBoundsW) / 2;
        toneAmbKnob.setBounds (knobX, toneCtrl.getY(), knobBoundsW, knobH);
        toneGateToggle.setBounds (toneCtrl.getX(), toneCtrl.getY() + knobH + 4, toneCtrl.getWidth(), 20);
    }

    // =========================================================================
    // Center panel: Stage II (64px knobs for visual hierarchy)
    // =========================================================================
    {
        const int cx = kCenterX + kCenterPadding;
        const int cw = kCenterWidth - 2 * kCenterPadding;
        const int largeKnobDia = 64;
        const int knobHLarge = largeKnobDia + 2 + 14 + 1 + 16;  // 97px

        // Section label at top
        reflSection.setBounds (cx, 46, cw, 24);

        // Visualization component (replaces placeholder rectangle)
        vizComponent.setBounds (284, 76, 332, 248);

        // Shape dropdown above knobs (below visualization + caption)
        const int shapeY = 350;
        reflShapeCombo.setBounds (cx + 60, shapeY, cw - 120, 24);
        // Shape arrow buttons (inside combo box, offset from right edge)
        shapeUpArrow.setBounds (reflShapeCombo.getRight() - 18, reflShapeCombo.getY(), 14, 12);
        shapeDownArrow.setBounds (reflShapeCombo.getRight() - 18, reflShapeCombo.getY() + 12, 14, 12);

        // Room control knobs centred between dropdown bottom and footer rule
        const int dropdownBottom = shapeY + 24;
        const int knobRowY = dropdownBottom + (500 - dropdownBottom - knobHLarge) / 2;
        const int knobW = 80;
        const int knobSpacing = 30;
        const int totalKnobsW = 3 * knobW + 2 * knobSpacing;
        const int knobStartX = cx + (cw - totalKnobsW) / 2;

        reflSizeKnob.setBounds (knobStartX, knobRowY, knobW, knobHLarge);
        reflProxKnob.setBounds (knobStartX + knobW + knobSpacing, knobRowY, knobW, knobHLarge);
        reflWidthKnob.setBounds (knobStartX + 2 * (knobW + knobSpacing), knobRowY, knobW, knobHLarge);
    }

    // =========================================================================
    // Right panel: Stages III, VI, Output
    // =========================================================================
    {
        const int rx = kRightX + kSidePadding;
        const int rw = kRightPanelWidth - 2 * kSidePadding;

        // Stage III: Air (y=46, h=148)
        airSection.setBounds (rx, 46, rw, 148);
        auto airCtrl = airSection.getControlArea().translated (rx, 46);
        int knobX = airCtrl.getX() + (airCtrl.getWidth() - knobBoundsW) / 2;
        airAmountKnob.setBounds (knobX, airCtrl.getY(), knobBoundsW, knobH);
        airCharToggle.setBounds (airCtrl.getX(), airCtrl.getY() + knobH + 4, airCtrl.getWidth(), 20);

        // Stage VI: Diffuse Tail (y=210, h=115)
        tailSection.setBounds (rx, 210, rw, 115);
        auto tailCtrl = tailSection.getControlArea().translated (rx, 210);
        int twoKnobW = rw / 2;
        tailDecayKnob.setBounds (tailCtrl.getX(), tailCtrl.getY(), twoKnobW, knobH);
        tailDiffKnob.setBounds (tailCtrl.getX() + twoKnobW, tailCtrl.getY(), twoKnobW, knobH);

        // Output (y=340, h=140 -- enough room for knobs)
        outputSection.setBounds (rx, 340, rw, 140);
        auto outCtrl = outputSection.getControlArea().translated (rx, 340);
        mixKnob.setBounds (outCtrl.getX(), outCtrl.getY(), twoKnobW, knobH);
        levelKnob.setBounds (outCtrl.getX() + twoKnobW, outCtrl.getY(), twoKnobW, knobH);

        // Mix lock button: upper-left corner of mix knob
        mixLockButton.setBounds (mixKnob.getX() - 4, mixKnob.getY() - 2, 20, 20);
    }
}
