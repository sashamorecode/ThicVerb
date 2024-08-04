/*
  ==============================================================================

    MultiChanDiffuser.h
    Created: 2 Aug 2024 2:55:53pm
    Author:  sasha

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "DelayLine.h"



class MultiChanDiffuser
{
public:
    MultiChanDiffuser(double sampleRate, int numChannels, double delayRangeMs);
    void init(double sampleRate, int numChannels, double delayRangeMs);
    void setDiffusionTimeMs(double sampleRate, double timeMs);
    void processSamples(float* samples);
    void processMultiChannel(juce::AudioBuffer<float>* buffer);
    static void splitBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>* newBuffer, int numSamples, int numChannels);
    static void mergeBuffer(juce::AudioBuffer<float>* buffer, juce::AudioBuffer<float>& outBuffer);
    int getNumChannels() { return numChannels; }
private:
    int numChannels;
    std::vector<DelayLine> delayLines;
    std::unique_ptr<hadamardMatrix> mixer = nullptr;
    double delayRangeMs;
};

