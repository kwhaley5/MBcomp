/*
  ==============================================================================

    UtilityComps.cpp
    Created: 6 Aug 2022 4:26:11pm
    Author:  kylew

  ==============================================================================
*/

#include "UtilityComps.h"

Placeholder::Placeholder()
{
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
}

void Placeholder::paint(juce::Graphics& g) 
{
    g.fillAll(customColor);
}

RotarySlider::RotarySlider() :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        juce::Slider::TextEntryBoxPosition::NoTextBox)
{}