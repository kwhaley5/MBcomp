/*
  ==============================================================================

    CompressorBand.h
    Created: 6 Aug 2022 4:45:36pm
    Author:  kylew

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../GUI/Utilities.h"

struct CompressorBand
{
    juce::AudioParameterFloat* Attack{ nullptr };
    juce::AudioParameterFloat* Release{ nullptr };
    juce::AudioParameterFloat* Threshold{ nullptr };
    juce::AudioParameterChoice* Ratio{ nullptr };
    juce::AudioParameterBool* Bypassed{ nullptr };
    juce::AudioParameterBool* Mute{ nullptr };
    juce::AudioParameterBool* Solo{ nullptr };

    void prepare(const juce::dsp::ProcessSpec& spec);

    void updateCompressorSettings();

    void process(juce::AudioBuffer<float>& buffer);
private:
    juce::dsp::Compressor<float> compressor;
};