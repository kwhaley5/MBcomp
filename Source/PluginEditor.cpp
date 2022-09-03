/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DSP/Params.h"

ControlBar::ControlBar()
{

    analyzerButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(analyzerButton);

    addAndMakeVisible(globalBypassButton);
}

void ControlBar::paint(juce::Graphics& g)
{
    using namespace juce;
    g.setColour(Colours::black); 
    g.fillAll();
}

void ControlBar::resized()
{
    auto bounds = getLocalBounds();
    analyzerButton.setBounds(bounds.removeFromLeft(50).withTrimmedTop(4).withTrimmedBottom(4));

    globalBypassButton.setBounds(bounds.removeFromRight(60).withTrimmedTop(2).withTrimmedBottom(2));
}

SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    controlBar.analyzerButton.onClick = [this]()
    {
        auto shouldBeOn = controlBar.analyzerButton.getToggleState();
        analyzer.toggleAnalysisEnablement(shouldBeOn);
    };

    controlBar.globalBypassButton.onClick = [this]()
    {
        toggleGlobalBypassState();
    };

    setLookAndFeel(&lnf);
    addAndMakeVisible(controlBar);
    addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (600, 500);

    startTimerHz(60);
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

void SimpleMBCompAudioProcessorEditor::timerCallback()
{
    std::vector<float> values
    {
        audioProcessor.lowBandComp.getRMSInputDb(),
        audioProcessor.lowBandComp.getRMSOutputDb(),
        audioProcessor.midBandComp.getRMSInputDb(),
        audioProcessor.midBandComp.getRMSOutputDb(),
        audioProcessor.highBandComp.getRMSInputDb(),
        audioProcessor.highBandComp.getRMSOutputDb(),
    };

    analyzer.update(values);

    updateGlobalBypassButton();
}

void SimpleMBCompAudioProcessorEditor::updateGlobalBypassButton()
{
    auto params = getBypassParams();
    bool allBandsAreBypassed = std::all_of(params.begin(), params.end(), [](const auto& param) {return param->get(); });
    controlBar.globalBypassButton.setToggleState(allBandsAreBypassed, juce::dontSendNotification);
}

std::array<juce::AudioParameterBool*, 3> SimpleMBCompAudioProcessorEditor::getBypassParams()
{
    using namespace Params;
    using namespace juce;
    const auto& params = GetParams();
    auto& apvts = audioProcessor.apvts;

    auto boolHelper = [&apvts, &params](const auto& paramName)
    {
        auto param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);

        return param;
    };

    auto* lowBypassParam = boolHelper(names::Bypassed_Low_Band);
    auto* midBypassParam = boolHelper(names::Bypassed_Mid_Band);
    auto* highBypassParam = boolHelper(names::Bypassed_High_Band);

    return
    {
        lowBypassParam,
        midBypassParam,
        highBypassParam
    };
}

void SimpleMBCompAudioProcessorEditor::toggleGlobalBypassState()
{
    auto shouldEnableEverything = ! controlBar.globalBypassButton.getToggleState();
    auto params = getBypassParams();

    auto bypassParamHelper = [](auto* param, bool shouldbBeBypased)
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost(shouldbBeBypased ? 1.f : 0.f);
        param->endChangeGesture();
    };

    for (auto* param : params)
    {
        bypassParamHelper(param, !shouldEnableEverything);
    }

    bandControls.toggleAllBands(!shouldEnableEverything);
}