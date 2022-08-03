/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) {
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    auto enabled = slider.isEnabled();

    g.setColour(enabled ? Colour(97u, 18u, 167u) : Colours::darkgrey);
    g.fillEllipse(bounds);

    g.setColour(enabled ? Colour(255u, 154u, 1u) : Colours::grey);
    g.drawEllipse(bounds, 1.f);

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {
        auto  center = bounds.getCentre();

        Path p;

        Rectangle <float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(center);

        g.setColour(enabled ? Colours::black : Colours::darkgrey);
        g.fillRect(r);

        g.setColour(enabled ? Colours::white : Colours::lightgrey);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    };

}

void LookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& toggleButton, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {

    using namespace juce;

    Path powerButton;

    auto bounds = toggleButton.getLocalBounds();
    auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
    auto r = bounds.withSizeKeepingCentre(size, size).toFloat();

    float ang = 30.f;

    size -= 6;

    powerButton.addCentredArc(r.getCentreX(), r.getCentreY(), size * .5, size * .5, 0.f, degreesToRadians(ang), degreesToRadians(360 - ang), true);

    powerButton.startNewSubPath(r.getCentreX(), r.getY());
    powerButton.lineTo(r.getCentre());

    PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);

    auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);

    g.setColour(color);
    g.strokePath(powerButton, pst);
    g.drawEllipse(r, 2);

}

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

//==============================================================================
Placeholder::Placeholder()
{
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
}

CompressorBandControls::CompressorBandControls(juce::AudioProcessorValueTreeState& apv) : 
    apvts(apv),
    attackSlider(nullptr, "ms", "ATTACK"),
    releaseSlider(nullptr, "ms", "RELEASE"),
    thresholdSlider(nullptr, "dB", "THRESHOLD"),
    ratioSlider(nullptr, "")

{
    using namespace Params;
    const auto& params = GetParams();

    auto getParamHelper = [&params, &apvts = this-> apvts](const auto& name) -> auto&
    {
        return getParam(apvts, params, name);
    };

    attackSlider.changeParam(&getParamHelper(names::Attack_Mid_Band));
    releaseSlider.changeParam(&getParamHelper(names::Release_Mid_Band));
    thresholdSlider.changeParam(&getParamHelper(names::Threshold_Mid_Band));
    ratioSlider.changeParam(&getParamHelper(names::Ratio_Mid_Band));

    addLabelPairs(attackSlider.labels, getParamHelper(names::Attack_Mid_Band), "ms");
    addLabelPairs(releaseSlider.labels, getParamHelper(names::Release_Mid_Band), "ms");
    addLabelPairs(thresholdSlider.labels, getParamHelper(names::Threshold_Mid_Band), "dB");

    ratioSlider.labels.add({ 0.f, "1:1" });
    auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&getParamHelper(names::Ratio_Mid_Band));
    ratioSlider.labels.add({ 1.f, juce::String(ratioParam->choices.getReference(ratioParam->choices.size() - 1).getIntValue()) + ":1"});

    auto makeAttachmentHelper = [&params, &apvts = this->apvts](auto& attachment, const auto& name, auto& slider)
    {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    makeAttachmentHelper(attackSliderAttachment, names::Attack_Mid_Band, attackSlider);
    makeAttachmentHelper(releaseSliderAttachment, names::Release_Mid_Band, releaseSlider);
    makeAttachmentHelper(thresholdSliderAttachment, names::Threshold_Mid_Band, thresholdSlider);
    makeAttachmentHelper(ratioSliderAttachment, names::Ratio_Mid_Band, ratioSlider);

    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
}

void CompressorBandControls::resized()
{
    using namespace juce;

    auto bounds = getLocalBounds().reduced(5);
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);

    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(attackSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(releaseSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(thresholdSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(ratioSlider).withFlex(1.f));
    flexBox.items.add(endCap);

    flexBox.performLayout(bounds);
}

void drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    g.setColour(Colours::blueviolet);
    g.fillAll();

    auto localBounds = bounds;

    bounds.reduce(3, 3); //This creates a boarder
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);

    g.drawRect(localBounds);
}

void CompressorBandControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}


GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace Params;
    const auto& params = GetParams();

    auto getParamHelper = [&params, &apvts](const auto& name) -> auto&
    {
        return getParam(apvts, params, name);
    };

    auto& gainInParam = getParamHelper(names::Gain_in);
    auto& gainLowMidParam = getParamHelper(names::Low_Mid_Crossover_Freq);
    auto& gainMidHighParam = getParamHelper(names::Mid_High_Crossover_Freq);
    auto& gainOutParam = getParamHelper(names::Gain_out);

    inGainSlider = std::make_unique<RSWL>(&gainInParam, "dB", "INPUT TRIM");
    lowMidXoverSlider = std::make_unique<RSWL>(&gainLowMidParam, "Hz", "LOW-MID X-OVER");
    midHighXoverSlider = std::make_unique<RSWL>(&gainMidHighParam, "Hz", "MID-HIGH X-OVER");
    outGainSlider = std::make_unique<RSWL>(&gainOutParam, "dB", "OUTPUT TRIM");

    auto makeAttachmentHelper = [&params, &apvts](auto &attachment, const auto& name, auto& slider)
    {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    makeAttachmentHelper(inGainSliderAttachment, names::Gain_in, *inGainSlider);
    makeAttachmentHelper(lowMidXoverSliderAttachment, names::Low_Mid_Crossover_Freq, *lowMidXoverSlider);
    makeAttachmentHelper(midHighXoverSliderAttachment, names::Mid_High_Crossover_Freq, *midHighXoverSlider);
    makeAttachmentHelper(outGainSliderAttachment, names::Gain_out, *outGainSlider);

    addLabelPairs(inGainSlider->labels, gainInParam, "dB");
    addLabelPairs(lowMidXoverSlider->labels, gainLowMidParam, "Hz");
    addLabelPairs(midHighXoverSlider->labels, gainMidHighParam, "Hz");
    addLabelPairs(outGainSlider->labels, gainOutParam, "dB");

    addAndMakeVisible(*inGainSlider);
    addAndMakeVisible(*lowMidXoverSlider);
    addAndMakeVisible(*midHighXoverSlider);
    addAndMakeVisible(*outGainSlider);
}

void GlobalControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

void GlobalControls::resized()
{
    using namespace juce;

    auto bounds = getLocalBounds().reduced(5);
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);

    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(*inGainSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*lowMidXoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*midHighXoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*outGainSlider).withFlex(1.f));
    flexBox.items.add(endCap);

    flexBox.performLayout(bounds);
}

//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&lnf);
    //addAndMakeVisible(controlBar);
    //addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (600, 500);
}

SimpleMBCompAudioProcessorEditor::~SimpleMBCompAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleMBCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleMBCompAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    controlBar.setBounds(bounds.removeFromTop(32));
    bandControls.setBounds(bounds.removeFromBottom(135));
    analyzer.setBounds(bounds.removeFromTop(225));
    globalControls.setBounds(bounds);


}