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

    float getRMSInputDb() const { return rmsInputDb; }
    float getRMSOutputDb() const { return rmsOutputDb; }
private:
    juce::dsp::Compressor<float> compressor;

    std::atomic<float> rmsInputDb{ NEGATIVE_INFINITY };
    std::atomic<float> rmsOutputDb{ NEGATIVE_INFINITY };

    template<typename T>
    float computeRMSLevel(const T& buffer)
    {
        int numChannels = static_cast<int>(buffer.getNumChannels());
        int numSamples = static_cast<int>(buffer.getNumSamples());
        auto rms = 0.f;
        for (int chan = 0; chan < numChannels; chan++)
        {
            rms += buffer.getRMSLevel(chan, 0, numSamples);
        }
        rms /= static_cast<float>(numChannels);
        return rms;
    }
};