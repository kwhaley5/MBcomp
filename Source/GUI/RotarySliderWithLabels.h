/*
  ==============================================================================

    RotarySliderWithLabels.h
    Created: 6 Aug 2022 4:19:40pm
    Author:  kylew

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct RotarySliderWithLabels : juce::Slider {
    RotarySliderWithLabels(juce::RangedAudioParameter* rap, const juce::String& unitSuffix, const juce::String title) :
        juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(rap),
        suffix(unitSuffix)
    {
        setName(title);
        //setLookAndFeel(&lnf);
    }

    //  ~RotarySliderWithLabels() {
    //      setLookAndFeel(nullptr);
    //  }

    struct LabelPos {
        float pos;
        juce::String label;

    };

    juce::Array<LabelPos> labels;

    void paint(juce::Graphics& g) override;
    juce::Rectangle<int>  getSliderBounds() const;
    int getTextHeight() const { return 14; };
    virtual juce::String getDisplayString() const;

    void changeParam(juce::RangedAudioParameter* p);

protected:
    juce::RangedAudioParameter* param;
    juce::String suffix;

};

struct RatioSlider : RotarySliderWithLabels
{
    RatioSlider(juce::RangedAudioParameter* rap, const juce::String& unitSuffix) : RotarySliderWithLabels(rap, unitSuffix, "RATIO") {}
    juce::String getDisplayString() const override;
};