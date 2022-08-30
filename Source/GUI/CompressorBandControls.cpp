/*
  ==============================================================================

    CompressorBandControls.cpp
    Created: 6 Aug 2022 4:37:07pm
    Author:  kylew

  ==============================================================================
*/

#include "CompressorBandControls.h"
#include "Utilities.h"
#include "../DSP/Params.h"
#include "LookAndFeel.h"

CompressorBandControls::CompressorBandControls(juce::AudioProcessorValueTreeState& apv) : //This will dynamically change, which is why it is initiated differenlty then the globabl controls. I think
    apvts(apv),
    attackSlider(nullptr, "ms", "ATTACK"),
    releaseSlider(nullptr, "ms", "RELEASE"),
    thresholdSlider(nullptr, "dB", "THRESHOLD"),
    ratioSlider(nullptr, "")

{

    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);

    bypassButton.addListener(this);
    soloButton.addListener(this);
    muteButton.addListener(this);


    bypassButton.setName("X");
    bypassButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::yellow);
    bypassButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    soloButton.setName("S");
    soloButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    soloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    muteButton.setName("M");
    muteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
    muteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(bypassButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);

    lowBand.setName("Low");
    lowBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    lowBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    midBand.setName("Mid");
    midBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    midBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    highBand.setName("High");
    highBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    highBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    lowBand.setRadioGroupId(1);
    midBand.setRadioGroupId(1);
    highBand.setRadioGroupId(1);

    auto buttonSwitcher = [safePtr = this->safePtr]()
    {
        if (auto* c = safePtr.getComponent())
        {
            c->updateAttachments();
        }
    };

    lowBand.onClick = buttonSwitcher;
    midBand.onClick = buttonSwitcher;
    highBand.onClick = buttonSwitcher;

    lowBand.setToggleState(true, juce::NotificationType::dontSendNotification);

    updateAttachments();
    updateSliderEnablements();
    updateBandSelectButtonStates();

    addAndMakeVisible(lowBand);
    addAndMakeVisible(midBand);
    addAndMakeVisible(highBand);
}

CompressorBandControls::~CompressorBandControls()
{
    bypassButton.removeListener(this);
    soloButton.removeListener(this);
    muteButton.removeListener(this);
}

void CompressorBandControls::resized()
{
    using namespace juce;
    auto bounds = getLocalBounds().reduced(5);

    auto createBandButtonControlBox = [](std::vector<Component*> comps)
    {
        FlexBox flexBox;
        flexBox.flexDirection = FlexBox::Direction::column;
        flexBox.flexWrap = FlexBox::Wrap::noWrap;

        auto spacer = FlexItem().withHeight(2);

        for (auto* comp : comps)
        {
            flexBox.items.add(spacer);
            flexBox.items.add(FlexItem(*comp).withFlex(1.f));
        }

        flexBox.items.add(spacer);

        return flexBox;
    };

    auto bandButtonControBox = createBandButtonControlBox({ &bypassButton, &soloButton, &muteButton });
    auto bandSelectControlBox = createBandButtonControlBox({ &lowBand, &midBand, &highBand });

    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);

    flexBox.items.add(spacer);
    //flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(bandSelectControlBox).withWidth(50));
    flexBox.items.add(FlexItem(attackSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(releaseSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(thresholdSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(ratioSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(bandButtonControBox).withWidth(30));
    //flexBox.items.add(endCap);

    flexBox.performLayout(bounds);
}

void CompressorBandControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

void CompressorBandControls::buttonClicked(juce::Button* button)
{
    updateSliderEnablements();
    updateSoloMuteBypassToggleStates(*button);
    updateActiveBandFillColour(*button);
}

void CompressorBandControls::updateActiveBandFillColour(juce::Button& clickedButton)
{
    if (clickedButton.getToggleState() == false)
    {
        resetActiveBandColours();
    }
    else
    {
        refreshBandButtonColors(*activeBand, clickedButton);
    }
}

void CompressorBandControls::resetActiveBandColours()
{
    activeBand->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    activeBand->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    activeBand->repaint();
}

void CompressorBandControls::refreshBandButtonColors(juce::Button &band, juce::Button &colourSource)
{
    band.setColour(juce::TextButton::ColourIds::buttonOnColourId, colourSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    band.setColour(juce::TextButton::ColourIds::buttonColourId, colourSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    band.repaint();
}

void CompressorBandControls::updateSliderEnablements()
{
    auto disabled = muteButton.getToggleState() || bypassButton.getToggleState();

    attackSlider.setEnabled(!disabled);
    releaseSlider.setEnabled(!disabled);
    ratioSlider.setEnabled(!disabled);
    thresholdSlider.setEnabled(!disabled);
}

void CompressorBandControls::updateBandSelectButtonStates()
{
    using namespace Params;
    std::vector<std::array<names, 3>> paramsToCheck
    {
        {names::Solo_Low_Band, names::Mute_Low_Band, names::Bypassed_Low_Band},
        {names::Solo_Mid_Band, names::Mute_Mid_Band, names::Bypassed_Mid_Band},
        {names::Solo_High_Band, names::Mute_High_Band, names::Bypassed_High_Band}
    };

    const auto& params = GetParams();
    auto paramHelper = [&params, this](const auto& name)
    {
        return dynamic_cast<juce::AudioParameterBool*>(&getParam(apvts, params, name));
    };

    for (size_t i = 0; i < paramsToCheck.size(); i++)
    {
        auto& list = paramsToCheck[i];

        auto* bandButton = (i == 0) ? &lowBand : (i == 1) ? &midBand : &highBand;

        if (auto* solo = paramHelper(list[0]);
            solo->get())
        {
            refreshBandButtonColors(*bandButton, soloButton);
        }
        else if (auto* mute = paramHelper(list[1]);
            mute->get())
        {
            refreshBandButtonColors(*bandButton, muteButton);
        }
        else if (auto* bypass = paramHelper(list[2]);
            bypass->get())
        {
            refreshBandButtonColors(*bandButton, bypassButton);
        }
    }
}

void CompressorBandControls::updateSoloMuteBypassToggleStates(juce::Button& clickedButton)
{
    if (&clickedButton == &soloButton && soloButton.getToggleState())
    {
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if (&clickedButton == &bypassButton && bypassButton.getToggleState())
    {
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if (&clickedButton == &muteButton && muteButton.getToggleState())
    {
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
}

void CompressorBandControls::updateAttachments()
{
    enum BandType
    {
        Low,
        Mid,
        High
    };

    BandType bandType = [this]()
    {
        if (lowBand.getToggleState())
            return BandType::Low;
        if (midBand.getToggleState())
            return BandType::Mid;

        return BandType::High;
    }();

    using namespace Params;
    std::vector<names> Names;

    switch (bandType)
    {
        case Low:
        {
            Names = std::vector<names>
            {
                names::Attack_Low_Band,
                names::Bypassed_Low_Band,
                names::Mute_Low_Band,
                names::Ratio_Low_Band,
                names::Release_Low_Band,
                names::Solo_Low_Band,
                names::Threshold_Low_Band
            };
            activeBand = &lowBand;

            break;
        }

        case Mid:
        {
            Names = std::vector<names>
            {
                names::Attack_Mid_Band,
                names::Bypassed_Mid_Band,
                names::Mute_Mid_Band,
                names::Ratio_Mid_Band,
                names::Release_Mid_Band,
                names::Solo_Mid_Band,
                names::Threshold_Mid_Band
            };
            activeBand = &midBand;

            break;
        }

        case High:
        {
            Names = std::vector<names>
            {
                names::Attack_High_Band,
                names::Bypassed_High_Band,
                names::Mute_High_Band,
                names::Ratio_High_Band,
                names::Release_High_Band,
                names::Solo_High_Band,
                names::Threshold_High_Band
            };
            activeBand = &highBand;

            break;
        }

    }

    enum Pos //I think a good thing to do is figure out the difference between an enum and a template. Seems like enums are good for map type things
    {
        Attack,
        Bypassed,
        Mute,
        Ratio,
        Release,
        Solo,
        Threshold
    };

    const auto& params = GetParams();
    auto getParamHelper = [&params, &apvts = this->apvts, &Names](const auto& pos) -> auto&
    {
        return getParam(apvts, params, Names.at(pos));
    };

    attackSliderAttachment.reset();
    releaseSliderAttachment.reset();
    thresholdSliderAttachment.reset();
    ratioSliderAttachment.reset();
    bypassButtonAttachment.reset();
    soloButtonAttachment.reset();
    muteButtonAttachment.reset();

    auto& attackParam = getParamHelper(Pos::Attack);
    addLabelPairs(attackSlider.labels, attackParam, "ms");
    attackSlider.changeParam(&attackParam);

    auto& releaseParam = getParamHelper(Pos::Release);
    addLabelPairs(releaseSlider.labels, releaseParam, "ms");
    releaseSlider.changeParam(&releaseParam);

    auto& thresholdParam = getParamHelper(Pos::Threshold);
    addLabelPairs(thresholdSlider.labels, thresholdParam, "dB");
    thresholdSlider.changeParam(&thresholdParam);

    auto& ratioParamRap = getParamHelper(Pos::Ratio);
    ratioSlider.labels.clear();
    ratioSlider.labels.add({ 0.f, "1:1" });
    auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&ratioParamRap);
    ratioSlider.labels.add({ 1.f, juce::String(ratioParam->choices.getReference(ratioParam->choices.size() - 1).getIntValue()) + ":1" });
    ratioSlider.changeParam(ratioParam);

    auto makeAttachmentHelper = [&params, &apvts = this->apvts](auto& attachment, const auto& name, auto& slider)
    {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    makeAttachmentHelper(attackSliderAttachment, Names[Pos::Attack], attackSlider);
    makeAttachmentHelper(releaseSliderAttachment, Names[Pos::Release], releaseSlider);
    makeAttachmentHelper(thresholdSliderAttachment, Names[Pos::Threshold], thresholdSlider);
    makeAttachmentHelper(ratioSliderAttachment, Names[Pos::Ratio], ratioSlider);
    makeAttachmentHelper(bypassButtonAttachment, Names[Pos::Bypassed], bypassButton);
    makeAttachmentHelper(soloButtonAttachment, Names[Pos::Solo], soloButton);
    makeAttachmentHelper(muteButtonAttachment, Names[Pos::Mute], muteButton);
}