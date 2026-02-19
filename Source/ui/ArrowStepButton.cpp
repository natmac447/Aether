#include "ArrowStepButton.h"
#include "AetherColours.h"

//==============================================================================
ArrowStepButton::ArrowStepButton (Direction direction, juce::ComboBox& targetCombo)
    : juce::Button ("ArrowStep"), dir (direction), comboBox (targetCombo)
{
    setSize (14, 12);
}

//==============================================================================
void ArrowStepButton::paintButton (juce::Graphics& g,
                                    bool shouldDrawButtonAsHighlighted,
                                    bool /*shouldDrawButtonAsDown*/)
{
    const auto colour = shouldDrawButtonAsHighlighted
                            ? juce::Colour (AetherColours::accentCopper)
                            : juce::Colour (AetherColours::inkFaint);

    g.setColour (colour);

    const auto bounds = getLocalBounds().toFloat().reduced (2.0f, 2.0f);
    juce::Path triangle;

    if (dir == Up)
    {
        triangle.startNewSubPath (bounds.getCentreX(), bounds.getY());
        triangle.lineTo (bounds.getRight(), bounds.getBottom());
        triangle.lineTo (bounds.getX(), bounds.getBottom());
        triangle.closeSubPath();
    }
    else
    {
        triangle.startNewSubPath (bounds.getX(), bounds.getY());
        triangle.lineTo (bounds.getRight(), bounds.getY());
        triangle.lineTo (bounds.getCentreX(), bounds.getBottom());
        triangle.closeSubPath();
    }

    g.fillPath (triangle);
}

//==============================================================================
void ArrowStepButton::clicked()
{
    const int numItems = comboBox.getNumItems();
    if (numItems == 0) return;

    const int currentId = comboBox.getSelectedId();

    // ComboBox item IDs are 1-based by JUCE convention
    // We need to find the current index in the item list and step from there
    int currentIndex = -1;
    for (int i = 0; i < numItems; ++i)
    {
        if (comboBox.getItemId (i) == currentId)
        {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex < 0)
        currentIndex = 0;

    int newIndex;
    if (dir == Up)
        newIndex = (currentIndex - 1 + numItems) % numItems;
    else
        newIndex = (currentIndex + 1) % numItems;

    comboBox.setSelectedId (comboBox.getItemId (newIndex), juce::sendNotificationAsync);
}
