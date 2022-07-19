/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleMBCompAudioProcessor::SimpleMBCompAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    using namespace Params;
    const auto& params = GetParams();

    auto floatHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    auto choiceHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    auto boolHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    //Low end compressor
    floatHelper(lowBandComp.Attack, names::Attack_Low_Band);
    floatHelper(lowBandComp.Release, names::Release_Low_Band);
    floatHelper(lowBandComp.Threshold, names::Threshold_Low_Band);
    choiceHelper(lowBandComp.Ratio, names::Ratio_Low_Band);
    boolHelper(lowBandComp.Bypassed, names::Bypassed_Low_Band);

    //mid compressor
    floatHelper(midBandComp.Attack, names::Attack_Mid_Band);
    floatHelper(midBandComp.Release, names::Release_Mid_Band);
    floatHelper(midBandComp.Threshold, names::Threshold_Mid_Band);
    choiceHelper(midBandComp.Ratio, names::Ratio_Mid_Band);
    boolHelper(midBandComp.Bypassed, names::Bypassed_Mid_Band);

    //high end compressor
    floatHelper(highBandComp.Attack, names::Attack_High_Band);
    floatHelper(highBandComp.Release, names::Release_High_Band);
    floatHelper(highBandComp.Threshold, names::Threshold_High_Band);
    choiceHelper(highBandComp.Ratio, names::Ratio_High_Band);
    boolHelper(highBandComp.Bypassed, names::Bypassed_High_Band);

    //crossovers
    floatHelper(lowMidCrossover, names::Low_Mid_Crossover_Freq);
    floatHelper(midHighCrossover, names::Mid_High_Crossover_Freq);

    LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);

    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
}

SimpleMBCompAudioProcessor::~SimpleMBCompAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleMBCompAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleMBCompAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMBCompAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMBCompAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleMBCompAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleMBCompAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleMBCompAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleMBCompAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleMBCompAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleMBCompAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleMBCompAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock; //maximum number of samples
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    for (auto comp : compressors)
        comp.prepare(spec);

    LP1.prepare(spec);
    HP1.prepare(spec);

    AP2.prepare(spec);

    LP2.prepare(spec);
    HP2.prepare(spec);

    for (auto& buffer : filterBuffers) //Not really sure what this is, come back to it
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    }
}

void SimpleMBCompAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleMBCompAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleMBCompAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    for (auto& compressor : compressors)
        compressor.updateCompressorSettings();

    // compressors.process(buffer);

    for (auto& fb : filterBuffers)
    {
        fb = buffer;
    }

    auto lowMidCutoff = lowMidCrossover->get();
    LP1.setCutoffFrequency(lowMidCutoff);
    HP1.setCutoffFrequency(lowMidCutoff);

    auto midHighCutoff = midHighCrossover->get();
    AP2.setCutoffFrequency(midHighCutoff);
    LP2.setCutoffFrequency(midHighCutoff);
    HP2.setCutoffFrequency(midHighCutoff);

    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);

    auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    auto fb2Ctx = juce::dsp::ProcessContextReplacing<float>(fb2Block);

    LP1.process(fb0Ctx);
    AP2.process(fb0Ctx);

    HP1.process(fb1Ctx);
    filterBuffers[2] = filterBuffers[1];
    LP2.process(fb1Ctx);

    HP2.process(fb2Ctx);
    
    for (size_t i = 0; i < filterBuffers.size(); ++i)
    {
        compressors[i].process(filterBuffers[i]);
    }
    
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    buffer.clear();

    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source)
    {
        for (auto i = 0; i < nc; i++)
        {
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        };
    };

    addFilterBand(buffer, filterBuffers[0]);
    addFilterBand(buffer, filterBuffers[1]);
    addFilterBand(buffer, filterBuffers[2]);
}



//==============================================================================
bool SimpleMBCompAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleMBCompAudioProcessor::createEditor()
{
    //return new SimpleMBCompAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleMBCompAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream mos(destData, true); 
    apvts.state.writeToStream(mos);
}

void SimpleMBCompAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleMBCompAudioProcessor::createParameterLayout() 
{
    APVTS::ParameterLayout layout;

    using namespace juce;
    using namespace Params;
    const auto& params = GetParams();

    //low end compressor parameters
    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Threshold_Low_Band), params.at(names::Threshold_Low_Band), NormalisableRange<float>(-60, 12, 1, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Attack_Low_Band), params.at(names::Attack_Low_Band), NormalisableRange<float>(5, 500, 1, 1), 50));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Release_Low_Band), params.at(names::Release_Low_Band), NormalisableRange<float>(5, 500, 1, 1), 250));

    //Mid compressor paramaters
    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Threshold_Mid_Band), params.at(names::Threshold_Mid_Band), NormalisableRange<float>(-60, 12, 1, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Attack_Mid_Band), params.at(names::Attack_Mid_Band), NormalisableRange<float>(5, 500, 1, 1), 50));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Release_Mid_Band), params.at(names::Release_Mid_Band), NormalisableRange<float>(5, 500, 1, 1), 250));

    //HIgh end compressor prameters
    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Threshold_High_Band), params.at(names::Threshold_High_Band), NormalisableRange<float>(-60, 12, 1, 1), 0));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Attack_High_Band), params.at(names::Attack_High_Band), NormalisableRange<float>(5, 500, 1, 1), 50));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Release_High_Band), params.at(names::Release_High_Band), NormalisableRange<float>(5, 500, 1, 1), 250));

    auto ratioChoices = std::vector<double>{ 1, 1.5, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 50, 100 };
    juce::StringArray sa;
    for (auto choice : ratioChoices)
    {
        sa.add(juce::String(choice, 1));
    }

    //Ratio Parameters
    layout.add(std::make_unique<AudioParameterChoice>(params.at(names::Ratio_Low_Band), params.at(names::Ratio_Low_Band), sa, 3));
    layout.add(std::make_unique<AudioParameterChoice>(params.at(names::Ratio_Mid_Band), params.at(names::Ratio_Mid_Band), sa, 3));
    layout.add(std::make_unique<AudioParameterChoice>(params.at(names::Ratio_High_Band), params.at(names::Ratio_High_Band), sa, 3));

    //Bypass Parameters
    layout.add(std::make_unique<AudioParameterBool>(params.at(names::Bypassed_Low_Band), params.at(names::Bypassed_Low_Band), false));
    layout.add(std::make_unique<AudioParameterBool>(params.at(names::Bypassed_Mid_Band), params.at(names::Bypassed_Mid_Band), false));
    layout.add(std::make_unique<AudioParameterBool>(params.at(names::Bypassed_High_Band), params.at(names::Bypassed_High_Band), false));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Low_Mid_Crossover_Freq), params.at(names::Low_Mid_Crossover_Freq), NormalisableRange<float>(20, 999, 1, 1), 400));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(names::Mid_High_Crossover_Freq), params.at(names::Mid_High_Crossover_Freq), NormalisableRange<float>(1000, 20000, 1, 1), 2000));
        

    return layout;
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleMBCompAudioProcessor();
}
