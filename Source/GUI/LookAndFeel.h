/*
  ==============================================================================

    LookAndFeel.h
    Created: 6 Aug 2022 4:14:24pm
    Author:  kylew

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define USE_LIVE_CONSTANT true

#if USE_LIVE_CONSTANT
#define colorHelper(c) JUCE_LIVE_CONSTANT(c);
#else
#define colorHelper(c) c;
#endif

namespace ColorScheme
{
    inline juce::Colour getSliderBorderColor()
    {
        return colorHelper(juce::Colours::blue);
    }
}

struct LookAndFeel : juce::LookAndFeel_V4 {
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;

    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& toggleButton,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonasDown) override;

};