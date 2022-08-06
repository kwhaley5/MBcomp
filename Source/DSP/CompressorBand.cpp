/*
  ==============================================================================

    CompressorBand.cpp
    Created: 6 Aug 2022 4:45:36pm
    Author:  kylew

  ==============================================================================
*/

#include "CompressorBand.h"

void CompressorBand::prepare(const juce::dsp::ProcessSpec& spec)
{
    compressor.prepare(spec);
}

void CompressorBand::updateCompressorSettings()
{
    compressor.setAttack(Attack->get());
    compressor.setRelease(Release->get());
    compressor.setThreshold(Threshold->get());
    compressor.setRatio(Ratio->getCurrentChoiceName().getFloatValue());
}

void CompressorBand::process(juce::AudioBuffer<float>& buffer)
{
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    context.isBypassed = Bypassed->get();

    compressor.process(context);
}