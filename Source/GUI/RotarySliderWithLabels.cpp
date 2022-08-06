/*
  ==============================================================================

    RotarySliderWithLabels.cpp
    Created: 6 Aug 2022 4:19:40pm
    Author:  kylew

  ==============================================================================
*/

#include "RotarySliderWithLabels.h"
#include "Utilities.h"
#include "LookAndFeel.h"

void RotarySliderWithLabels::paint(juce::Graphics& g) {
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    auto bounds = getLocalBounds();
    g.setColour(Colours::blueviolet);
    g.drawFittedText(getName(), bounds.removeFromTop(getTextBoxHeight() - 3), Justification::centredBottom, 1);

    //g.setColour(Colours::red);
    //g.drawRect(getLocalBounds());
    //g.setColour(Colours::yellow);
    //g.drawRect(sliderBounds);

    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        startAng,
        endAng,
        *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * .5f;

    g.setColour(Colour(0u, 172u, 1u));
    g.setFont(15);

    auto numChoices = labels.size();

    //labels for rotarty
    for (int i = 0; i < numChoices; i++) {

        auto pos = labels[i].pos;
        //jassert(0.f <= pos);
        //jassert(pos = 1.f);

        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * .5f + 1, ang);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {

    auto bounds = getLocalBounds();

    bounds.removeFromTop(getTextHeight() * 1.5);

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextBoxHeight() * 1;

    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    //r.setY(2);
    r.setY(bounds.getY());

    return r;
}


juce::String RotarySliderWithLabels::getDisplayString() const {

    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();

    juce::String str;
    bool addk = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {

        float val = getValue();

        if (val > 999) {
            val /= 1000.f;
            addk = true;
        }

        str = juce::String(val, (addk ? 2 : 0));

    }
    else {

        jassertfalse; //this shouldn't happen!
    }

    if (suffix.isNotEmpty()) {

        str << " ";
        if (addk)
            str << "k";

        str << suffix;
    }

    return str;
}

void RotarySliderWithLabels::changeParam(juce::RangedAudioParameter* p)
{
    param = p;
    repaint();
}

juce::String RatioSlider::getDisplayString() const
{
    auto choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param);
    jassert(choiceParam != nullptr);

    auto currentChoice = choiceParam->getCurrentChoiceName();

    if (currentChoice.contains(".0"))
        currentChoice = currentChoice.substring(0, currentChoice.indexOf("."));

    currentChoice << ":1";

    return currentChoice;
}
