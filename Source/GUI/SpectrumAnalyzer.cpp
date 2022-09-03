/*
  ==============================================================================

    SpectrumAnalyzer.cpp
    Created: 31 Aug 2022 5:30:19pm
    Author:  kylew

  ==============================================================================
*/

#include "SpectrumAnalyzer.h"
#include "Utilities.h"
#include "../DSP/Params.h"
#include "LookAndFeel.h"

SpectrumAnalyzer::SpectrumAnalyzer(SimpleMBCompAudioProcessor& p) :
    audioProcessor(p),
    leftPathProducer(audioProcessor.leftChannelFifo),
    rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    using namespace Params;
    const auto& pramNames = GetParams();
    auto floatHelper = [&apvts = audioProcessor.apvts, &pramNames](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(pramNames.at(paramName)));
        jassert(param != nullptr);
    };

    floatHelper(lowMidXoverParam, names::Low_Mid_Crossover_Freq);
    floatHelper(midHighXoverParam, names::Mid_High_Crossover_Freq);

    floatHelper(lowTheshParam, names::Threshold_Low_Band);
    floatHelper(midThreshParam, names::Threshold_Mid_Band);
    floatHelper(highThreshParam, names::Threshold_High_Band);

    startTimerHz(60);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

void SpectrumAnalyzer::drawFFTAnalysis(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;

    auto responseArea = getAnalysisArea(bounds);

    Graphics::ScopedSaveState sss(g);
    g.reduceClipRegion(responseArea);

    auto leftChannelFFTPath = leftPathProducer.getPath();
    leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), 0));

    g.setColour(Colour(186u, 34u, 34u)); //Colors for left and right of spectrum response
    g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));

    auto rightChannelFFTPath = rightPathProducer.getPath();
    rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), 0));

    g.setColour(Colour(64u, 194u, 230u));
    g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);

    auto bounds = drawModuleBackground(g, getLocalBounds());

    drawBackgroundGrid(g, bounds);

    if (shouldShowFFTAnalysis)
    {
        drawFFTAnalysis(g, bounds);
    }

    g.setColour(Colours::black);

    drawCrossovers(g, bounds);

    drawTextLabels(g, bounds);

}

void SpectrumAnalyzer::drawCrossovers(juce::Graphics& g, juce::Rectangle <int> bounds)
{
    using namespace juce;
    bounds = getAnalysisArea(bounds);

    const auto top = bounds.getY();
    const auto bottom = bounds.getBottom();
    const auto left = bounds.getX();
    const auto right = bounds.getRight();

    auto mapX = [left = bounds.getX(), width = bounds.getWidth()](float frequency)
    {
        auto normX = juce::mapFromLog10(frequency, MIN_FREQUENCY, MAX_FREQUENCY);
        return(left + width * normX);
    };
        
    auto lowMidX = mapX(lowMidXoverParam->get());
    g.setColour(Colour(64u, 194u, 230u));
    g.drawVerticalLine(lowMidX, top, bottom);

    auto midHighX = mapX(midHighXoverParam->get());
    g.setColour(Colour(64u, 194u, 230u));
    g.drawVerticalLine(midHighX, top, bottom);

    auto mapY = [top, bottom](float gDb) 
    {
        return jmap(gDb, NEGATIVE_INFINITY, MAX_DECIBLES, float(bottom), float(top));
    };

    auto zeroDb = mapY(0.f);
    g.setColour(Colour(186u, 34u, 34u).withAlpha(0.5f));

    g.fillRect(Rectangle<float>::leftTopRightBottom(left, zeroDb, lowMidX, mapY(lowBandGR)));
    g.fillRect(Rectangle<float>::leftTopRightBottom(lowMidX, zeroDb, midHighX, mapY(midBandGR)));
    g.fillRect(Rectangle<float>::leftTopRightBottom(midHighX, zeroDb, right, mapY(highBandGR)));

    g.setColour(Colours::white);
    g.drawHorizontalLine(mapY(lowTheshParam->get()), left, lowMidX);
    g.drawHorizontalLine(mapY(midThreshParam->get()), lowMidX, midHighX);
    g.drawHorizontalLine(mapY(highThreshParam->get()), midHighX, right);

}

void SpectrumAnalyzer::update(const std::vector<float>& values)
{
    enum
    {
        LowBandIn, LowBandOut, MidBandIn, MidBandOut, HighBandIn, HighBandOut
    };

    lowBandGR = values[LowBandOut] - values[LowBandIn];
    midBandGR = values[MidBandOut] - values[MidBandIn];
    highBandGR = values[HighBandOut] - values[HighBandIn];

    repaint();
}

std::vector<float> SpectrumAnalyzer::getFrequencies()
{
    return std::vector<float>
    {
        20, /*30, 40,*/ 50, 100,
            200, /*300, 400,*/ 500, 1000,
            2000, /*3000, 4000,*/ 5000, 10000,
            20000
    };
}

std::vector<float> SpectrumAnalyzer::getGains()
{
    std::vector<float> values;

    auto increment = MAX_DECIBLES;
    for (auto db = NEGATIVE_INFINITY; db <= MAX_DECIBLES; db += increment)
    {
        values.push_back(db);
    }
    
    return values;
}

std::vector<float> SpectrumAnalyzer::getXs(const std::vector<float>& freqs, float left, float width)
{
    std::vector<float> xs;
    for (auto f : freqs)
    {
        auto normX = juce::mapFromLog10(f, MIN_FREQUENCY, MAX_FREQUENCY);
        xs.push_back(left + width * normX);
    }

    return xs;
}

void SpectrumAnalyzer::drawBackgroundGrid(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    auto freqs = getFrequencies();

    auto renderArea = getAnalysisArea(bounds);
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    auto xs = getXs(freqs, left, width);

    g.setColour(Colours::dimgrey);
    for (auto x : xs)
    {
        g.drawVerticalLine(x, top, bottom);
    }

    auto gain = getGains();

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, NEGATIVE_INFINITY, MAX_DECIBLES, float(bottom), float(top));

        g.setColour(Colour(64u, 194u, 230u));
        g.setColour(gDb == 0.f ? Colour(186u, 34u, 34u) : Colours::darkgrey);
        g.drawHorizontalLine(y, left, right);
    }
}

void SpectrumAnalyzer::drawTextLabels(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    g.setColour(Colour(64u, 194u, 230u));
    const int fontHeight = 10;
    g.setFont(fontHeight);

    auto renderArea = getAnalysisArea(bounds);
    auto left = renderArea.getX();

    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    auto freqs = getFrequencies();
    auto xs = getXs(freqs, left, width);

    for (int i = 0; i < freqs.size(); ++i)
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;
        if (f > 999.f)
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
            str << "k";
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;

        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(bounds.getY());

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }

    auto gain = getGains();

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, NEGATIVE_INFINITY, MAX_DECIBLES, float(bottom), float(top));

        String str;
        if (gDb > 0)
            str << "+";
        str << gDb;

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(bounds.getRight() - textWidth);
        r.setCentre(r.getCentreX(), y);

        g.setColour(gDb == 0.f ? Colour(186u, 34u, 34u) : Colour(64u, 194u, 230u));

        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);

        //str.clear();
        //str << (gDb - 24.f);

        r.setX(bounds.getX() + 1);
        //textWidth = g.getCurrentFont().getStringWidth(str);
       // r.setSize(textWidth, fontHeight);
        //g.setColour(Colour(64u, 194u, 230u));
        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
    }
}

void SpectrumAnalyzer::resized()
{
    using namespace juce;
    auto bounds = getLocalBounds();
    auto fftBounds = getAnalysisArea(bounds).toFloat();
    auto negInf = jmap(bounds.toFloat().getBottom(), fftBounds.getBottom(), fftBounds.getY(), NEGATIVE_INFINITY, MAX_DECIBLES);
    DBG("Negative infinity: " << negInf);
    leftPathProducer.updateNegInfinity(negInf);
    rightPathProducer.updateNegInfinity(negInf);
}

void SpectrumAnalyzer::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

juce::Rectangle<int> SpectrumAnalyzer::getRenderArea(juce::Rectangle<int> bounds)
{
    //auto bounds = getLocalBounds();

    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);

    return bounds;
}


juce::Rectangle<int> SpectrumAnalyzer::getAnalysisArea(juce::Rectangle<int> bounds)
{
    bounds = getRenderArea(bounds);
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}

void SpectrumAnalyzer::timerCallback()
{
    if (shouldShowFFTAnalysis)
    {
        auto fftBounds = getAnalysisArea(getLocalBounds()).toFloat();
        fftBounds.setBottom(getLocalBounds().getBottom());
        auto sampleRate = audioProcessor.getSampleRate();

        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }

    if (parametersChanged.compareAndSetBool(false, true))
    {

    }

    repaint();
}