/*
  ==============================================================================

    UtilityComps.h
    Created: 6 Aug 2022 4:26:11pm
    Author:  kylew

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct Placeholder : juce::Component
{

    Placeholder();

    void paint(juce::Graphics& g) override;

    juce::Colour customColor;
};

struct RotarySlider : juce::Slider
{
    RotarySlider();
};