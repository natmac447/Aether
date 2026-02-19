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
    : AudioProcessorEditor (&p), processorRef (p)
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
    // 6. ComboBoxes for Material (10 items) and Shape (7 items)
    // =========================================================================
    materialCombo.addItemList (
        juce::StringArray { "Pine", "Oak", "Walnut", "Mahogany", "Iron",
                            "Steel", "Copper", "Limestone", "Marble", "Granite" }, 1);
    materialCombo.setColour (juce::ComboBox::textColourId, juce::Colour (AetherColours::inkLight));
    materialCombo.setColour (juce::ComboBox::outlineColourId, juce::Colour (AetherColours::inkGhost));
    materialCombo.setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    materialCombo.setColour (juce::ComboBox::arrowColourId, juce::Colour (AetherColours::inkFaint));
    addAndMakeVisible (materialCombo);

    reflShapeCombo.addItemList (
        juce::StringArray { "The Parlour", "The Gallery", "The Chamber", "The Nave",
                            "The Alcove", "The Crypt", "The Conservatory" }, 1);
    reflShapeCombo.setColour (juce::ComboBox::textColourId, juce::Colour (AetherColours::inkLight));
    reflShapeCombo.setColour (juce::ComboBox::outlineColourId, juce::Colour (AetherColours::inkGhost));
    reflShapeCombo.setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    reflShapeCombo.setColour (juce::ComboBox::arrowColourId, juce::Colour (AetherColours::inkFaint));
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
        }
    };

    syncBypass (ParamIDs::resBypass, resSection, { &resWeightKnob });
    syncBypass (ParamIDs::reflBypass, reflSection, { &reflSizeKnob, &reflProxKnob, &reflWidthKnob });
    syncBypass (ParamIDs::airBypass, airSection, { &airAmountKnob, &airCharToggle });
    syncBypass (ParamIDs::excitBypass, excitSection, { &excitDriveKnob });
    syncBypass (ParamIDs::toneBypass, toneSection, { &toneAmbKnob, &toneGateToggle });
    syncBypass (ParamIDs::tailBypass, tailSection, { &tailDecayKnob, &tailDiffKnob });
}

//==============================================================================
AetherEditor::~AetherEditor()
{
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

    // 3. Header zone (0-50px)
    {
        // "AETHER" title
        ParchmentElements::drawLetterSpacedText (
            g, "AETHER", 30.0f, 28.0f, 200.0f, 6.0f,
            lookAndFeel.getDisplayFont (32.0f),
            juce::Colour (AetherColours::ink),
            juce::Justification::left);

        // "Environment Simulator" subtitle
        ParchmentElements::drawLetterSpacedText (
            g, "Environment Simulator", 30.0f, 44.0f, 200.0f, 2.0f,
            lookAndFeel.getSpectralFontItalic (11.0f),
            juce::Colour (AetherColours::inkFaint),
            juce::Justification::left);

        // "COSMOS SERIES" mark at right
        ParchmentElements::drawLetterSpacedText (
            g, "COSMOS SERIES", 0.0f, 34.0f, 880.0f, 2.0f,
            lookAndFeel.getSpectralFont (10.0f),
            juce::Colour (AetherColours::inkFaint),
            juce::Justification::right);
    }

    // 4. Header double-rule border
    ParchmentElements::drawDoubleRule (g, 50.0f, 0.0f, 900.0f, true);

    // 5. Footer zone (490-530px)
    {
        // Footer double-rule border
        ParchmentElements::drawDoubleRule (g, 490.0f, 0.0f, 900.0f, false);

        // "Aether v0.1.0" at left
        ParchmentElements::drawLetterSpacedText (
            g, "AETHER V0.1.0", 30.0f, 510.0f, 200.0f, 2.0f,
            lookAndFeel.getSpectralFont (9.0f),
            juce::Colour (AetherColours::inkFaint),
            juce::Justification::left);

        // Tagline centred
        ParchmentElements::drawLetterSpacedText (
            g, "THE INVISIBLE MEDIUM THROUGH WHICH ALL SOUND PROPAGATES.",
            0.0f, 510.0f, 900.0f, 1.0f,
            lookAndFeel.getSpectralFontItalic (9.0f),
            juce::Colour (AetherColours::inkFaint),
            juce::Justification::horizontallyCentred);

        // "CAIRN" mark at right
        ParchmentElements::drawLetterSpacedText (
            g, "CAIRN", 0.0f, 510.0f, 870.0f, 2.0f,
            lookAndFeel.getSpectralFont (9.0f),
            juce::Colour (AetherColours::inkFaint),
            juce::Justification::right);
    }

    // 6. Column dividers: 1px Ink Ghost vertical lines
    g.setColour (juce::Colour (AetherColours::inkGhost));
    g.drawLine (220.0f, 53.0f, 220.0f, 487.0f, 1.0f);
    g.drawLine (680.0f, 53.0f, 680.0f, 487.0f, 1.0f);

    // 7. Diamond dividers between left panel sections
    {
        float leftX = static_cast<float> (Layout::kSidePadding);
        float rightX = static_cast<float> (Layout::kLeftPanelWidth - Layout::kSidePadding);
        ParchmentElements::drawDiamondDivider (g, 200.0f, leftX, rightX);
        ParchmentElements::drawDiamondDivider (g, 320.0f, leftX, rightX);
    }

    // 8. Diamond dividers between right panel sections
    {
        float leftX = static_cast<float> (Layout::kRightX + Layout::kSidePadding);
        float rightX = static_cast<float> (Layout::kRightX + Layout::kRightPanelWidth - Layout::kSidePadding);
        ParchmentElements::drawDiamondDivider (g, 200.0f, leftX, rightX);
        ParchmentElements::drawDiamondDivider (g, 410.0f, leftX, rightX);
    }

    // 9. Center panel: Visualization placeholder
    {
        auto vizBounds = juce::Rectangle<int> (280, 80, 340, 300);
        g.setColour (juce::Colour (AetherColours::parchmentDark));
        g.drawRect (vizBounds, 1);

        // "Fig. 1" caption below visualization
        ParchmentElements::drawLetterSpacedText (
            g, "Fig. 1", 280.0f, 392.0f, 340.0f, 2.0f,
            lookAndFeel.getSpectralFontItalic (9.0f),
            juce::Colour (AetherColours::inkFaint),
            juce::Justification::horizontallyCentred);
    }
}

//==============================================================================
void AetherEditor::resized()
{
    using namespace Layout;

    // =========================================================================
    // Header
    // =========================================================================
    presetSelector.setBounds (500, 16, 160, 24);

    // =========================================================================
    // Left panel: Stages I, IV, V
    // =========================================================================
    {
        const int lx = kSidePadding;
        const int lw = kLeftPanelWidth - 2 * kSidePadding;

        // Stage I: Resonance (y=55, ~134px)
        resSection.setBounds (lx, 55, lw, 140);
        auto resCtrl = resSection.getControlArea().translated (lx, 55);
        int knobW = 90;  // 56px knob + 34px label/value
        int knobX = resCtrl.getX() + (resCtrl.getWidth() - 56) / 2;
        resWeightKnob.setBounds (knobX, resCtrl.getY(), 56, knobW);
        materialCombo.setBounds (resCtrl.getX(), resCtrl.getY() + 95, resCtrl.getWidth(), 24);

        // Stage IV: Excitation (y=210, ~110px)
        excitSection.setBounds (lx, 210, lw, 100);
        auto excitCtrl = excitSection.getControlArea().translated (lx, 210);
        knobX = excitCtrl.getX() + (excitCtrl.getWidth() - 56) / 2;
        excitDriveKnob.setBounds (knobX, excitCtrl.getY(), 56, knobW);

        // Stage V: Room Tone (y=330, ~134px)
        toneSection.setBounds (lx, 330, lw, 150);
        auto toneCtrl = toneSection.getControlArea().translated (lx, 330);
        knobX = toneCtrl.getX() + (toneCtrl.getWidth() - 56) / 2;
        toneAmbKnob.setBounds (knobX, toneCtrl.getY(), 56, knobW);
        toneGateToggle.setBounds (toneCtrl.getX(), toneCtrl.getY() + 95, toneCtrl.getWidth(), 24);
    }

    // =========================================================================
    // Center panel: Stage II
    // =========================================================================
    {
        const int cx = kCenterX + kCenterPadding;
        const int cw = kCenterWidth - 2 * kCenterPadding;

        // Section label at top
        reflSection.setBounds (cx, 55, cw, 24);

        // Room control knobs below visualization (y=410)
        const int knobRowY = 410;
        const int largeKnobH = 98;  // 64px knob + 34px label/value
        const int knobSpacing = 10;
        const int totalKnobsW = 3 * 64 + 2 * knobSpacing;
        const int knobStartX = cx + (cw - totalKnobsW) / 2;

        reflSizeKnob.setBounds (knobStartX, knobRowY, 64, largeKnobH);
        reflProxKnob.setBounds (knobStartX + 64 + knobSpacing, knobRowY, 64, largeKnobH);
        reflWidthKnob.setBounds (knobStartX + 2 * (64 + knobSpacing), knobRowY, 64, largeKnobH);

        // Shape dropdown below knobs (full width)
        reflShapeCombo.setBounds (cx + 40, knobRowY + largeKnobH + 2, cw - 80, 24);
    }

    // =========================================================================
    // Right panel: Stages III, VI, Output
    // =========================================================================
    {
        const int rx = kRightX + kSidePadding;
        const int rw = kRightPanelWidth - 2 * kSidePadding;

        // Stage III: Air (y=55, ~134px)
        airSection.setBounds (rx, 55, rw, 140);
        auto airCtrl = airSection.getControlArea().translated (rx, 55);
        int knobW = 90;
        int knobX = airCtrl.getX() + (airCtrl.getWidth() - 56) / 2;
        airAmountKnob.setBounds (knobX, airCtrl.getY(), 56, knobW);
        airCharToggle.setBounds (airCtrl.getX(), airCtrl.getY() + 95, airCtrl.getWidth(), 24);

        // Stage VI: Diffuse Tail (y=218, ~180px)
        tailSection.setBounds (rx, 218, rw, 180);
        auto tailCtrl = tailSection.getControlArea().translated (rx, 218);
        // Two knobs side by side
        int twoKnobW = 80;  // Slightly compressed to fit 188px panel
        int twoKnobGap = rw - 2 * twoKnobW;
        int tailKnob1X = tailCtrl.getX();
        int tailKnob2X = tailCtrl.getX() + twoKnobW + twoKnobGap;
        tailDecayKnob.setBounds (tailKnob1X, tailCtrl.getY(), twoKnobW, knobW);
        tailDiffKnob.setBounds (tailKnob2X, tailCtrl.getY(), twoKnobW, knobW);

        // Output (y=420, ~68px -- compact, no bypass)
        outputSection.setBounds (rx, 420, rw, 68);
        auto outCtrl = outputSection.getControlArea().translated (rx, 420);
        int outKnob1X = outCtrl.getX();
        int outKnob2X = outCtrl.getX() + twoKnobW + twoKnobGap;
        mixKnob.setBounds (outKnob1X, outCtrl.getY(), twoKnobW, 56);
        levelKnob.setBounds (outKnob2X, outCtrl.getY(), twoKnobW, 56);
    }
}
