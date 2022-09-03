/*
  ==============================================================================

    Utilities.cpp
    Created: 6 Aug 2022 4:31:42pm
    Author:  kylew

  ==============================================================================
*/

#include "Utilities.h"
#include "LookAndFeel.h"

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix)
{
    juce::String str;

    auto val = getLow ? param.getNormalisableRange().start : param.getNormalisableRange().end;

    if (val > 999.f) {
        val /= 1000.f;
        str << val << " k" << suffix;
    }
    else {
        str << val << suffix;
    }

    return str;
}

juce::Rectangle<int> drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    g.setColour(Colours::white); //module boarder
    g.fillAll();

    auto localBounds = bounds;

    bounds.reduce(2, 2); //This creates a boarder
    g.setColour(Colours::black); //this is the background color
    g.fillRoundedRectangle(bounds.toFloat(), 3);

    g.drawRect(localBounds);

    return bounds;
}