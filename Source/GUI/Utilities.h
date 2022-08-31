/*
  ==============================================================================

    Utilities.h
    Created: 6 Aug 2022 4:31:42pm
    Author:  kylew

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

enum FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

template < //template agrument deduction on cpp reference to understand what the heck this is
    typename Attachment,
    typename APVTS,
    typename Params,
    typename ParamName,
    typename SliderType
>

void makeAttachment(std::unique_ptr<Attachment>& attachment,
    APVTS& apvts,
    const Params& params,
    const ParamName& name,
    SliderType& slider)
{
    attachment = std::make_unique<Attachment>(apvts,
        params.at(name),
        slider);
}

template <
    typename APVTS,
    typename Params,
    typename Name
>

juce::RangedAudioParameter& getParam(APVTS& apvts, const Params& params, const Name& name)
{
    auto param = apvts.getParameter(params.at(name));
    jassert(param != nullptr);

    return *param;
}

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix);

template <
    typename Labels,
    typename ParamType,
    typename SuffixType
>

void addLabelPairs(Labels& labels, const ParamType& param, const SuffixType& suffix)
{
    labels.clear();
    labels.add({ 0.f, getValString(param, true, suffix) });
    labels.add({ 1.f, getValString(param, false, suffix) });
}

void drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds);