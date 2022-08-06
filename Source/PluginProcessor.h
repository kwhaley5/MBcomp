/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/CompressorBand.h"


//namespace Params
//{
//    enum names
//    {
//        //Crossover parameters
//        Low_Mid_Crossover_Freq,
//        Mid_High_Crossover_Freq,
//
//        //Controls for all bands
//        Threshold_Low_Band,
//        Threshold_Mid_Band,
//        Threshold_High_Band,
//
//        Attack_Low_Band,
//        Attack_Mid_Band,
//        Attack_High_Band,
//
//        Release_Low_Band,
//        Release_Mid_Band,
//        Release_High_Band,
//
//        Ratio_Low_Band,
//        Ratio_Mid_Band,
//        Ratio_High_Band,
//
//        Bypassed_Low_Band,
//        Bypassed_Mid_Band,
//        Bypassed_High_Band,
//
//        Mute_Low_Band,
//        Mute_Mid_Band,
//        Mute_High_Band,
//
//        Solo_Low_Band,
//        Solo_Mid_Band,
//        Solo_High_Band,
//
//        Gain_in,
//        Gain_out
//    };
//
//    inline const std::map <names, juce::String>& GetParams()
//    {
//        static std::map<names, juce::String> params = 
//        {
//            {Low_Mid_Crossover_Freq, "Low-Mid Crossover Frequency"},
//            {Mid_High_Crossover_Freq, "Mid-High Crossover Frequency"},
//            {Threshold_Low_Band, "Threshold Low Band"},
//            {Threshold_Mid_Band, "Threshold Mid Band"},
//            {Threshold_High_Band, "Threshold High Band"},
//            {Attack_Low_Band, "Attack Low Band"},
//            {Attack_Mid_Band, "Attack Mid Band"},
//            {Attack_High_Band, "Attack High Band"},
//            {Release_Low_Band, "Release Low Band"},
//            {Release_Mid_Band, "Release Mid Band"},
//            {Release_High_Band, "Release High Band"},
//            {Ratio_Low_Band, "Ratio Low Band"},
//            {Ratio_Mid_Band, "Ratio Mid Band"},
//            {Ratio_High_Band, "Ratio High Band"},
//            {Bypassed_Low_Band, "Bypassed Low Band"},
//            {Bypassed_Mid_Band, "Bypassed Mid Band"},
//            {Bypassed_High_Band, "Bypassed High Band"},
//            {Mute_Low_Band, "Mute Low Band"},
//            {Mute_Mid_Band, "Mute Mid Band"},
//            {Mute_High_Band, "Mute High Band"},
//            {Solo_Low_Band, "Solo Low Band"},
//            {Solo_Mid_Band, "Solo Mid Band"},
//            {Solo_High_Band, "Solo High Band"},
//            {Gain_in, "Gain In"},
//            {Gain_out, "Gain Out"}
//        };
//
//        return params;
//    }
//}

//struct CompressorBand
//{
//    juce::AudioParameterFloat* Attack{ nullptr };
//    juce::AudioParameterFloat* Release{ nullptr };
//    juce::AudioParameterFloat* Threshold{ nullptr };
//    juce::AudioParameterChoice* Ratio{ nullptr };
//    juce::AudioParameterBool* Bypassed{ nullptr };
//    juce::AudioParameterBool* Mute{ nullptr };
//    juce::AudioParameterBool* Solo{ nullptr };
//
//    void prepare(const juce::dsp::ProcessSpec& spec)
//    {
//        compressor.prepare(spec);
//    }
//
//    void updateCompressorSettings()
//    {
//        compressor.setAttack(Attack->get());
//        compressor.setRelease(Release->get());
//        compressor.setThreshold(Threshold->get());
//        compressor.setRatio(Ratio->getCurrentChoiceName().getFloatValue());
//    }
//
//    void process(juce::AudioBuffer<float>& buffer)
//    {
//        auto block = juce::dsp::AudioBlock<float>(buffer);
//        auto context = juce::dsp::ProcessContextReplacing<float>(block);
//
//        context.isBypassed = Bypassed->get();
//
//        compressor.process(context);
//    }
//private:
//    juce::dsp::Compressor<float> compressor;
//};

//==============================================================================
/**
*/
class SimpleMBCompAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SimpleMBCompAudioProcessor();
    ~SimpleMBCompAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    static APVTS::ParameterLayout createParameterLayout();

    APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };
private:
    
    std::array<CompressorBand, 3> compressors;
    CompressorBand& lowBandComp = compressors[0];
    CompressorBand& midBandComp = compressors[1];
    CompressorBand& highBandComp = compressors[2];

    using Filter = juce::dsp::LinkwitzRileyFilter<float>;
    //     fc0  fc1
    Filter LP1, AP2,
           HP1, LP2,
                HP2;

    juce::AudioParameterFloat* lowMidCrossover{ nullptr };
    juce::AudioParameterFloat* midHighCrossover{ nullptr };

    std::array <juce::AudioBuffer<float>, 3> filterBuffers;

    juce::dsp::Gain<float> inputGain, outputGain;

    juce::AudioParameterFloat* inputGainParam{ nullptr };
    juce::AudioParameterFloat* outputGainParam{ nullptr };

    template<typename T, typename U>
    void applyGain(T& buffer, U& gain)
    {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto ctx = juce::dsp::ProcessContextReplacing<float>(block);
        gain.process(ctx);
    }

    void updateState();

    void splitBands(const juce::AudioBuffer<float>& inputBuffer);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessor)
};
