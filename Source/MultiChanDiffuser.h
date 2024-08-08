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
#define NUM_DIFFUSERS 2


class MultiChanDiffuser
{
public:
    MultiChanDiffuser(double sampleRate, int numChannels, double delayRangeMs);
    ~MultiChanDiffuser() {
		delete[] samples;
        delayLines.clear();
        for (int i = 0; i < numChannels; ++i) {
			delete delayLengths[i];
			delete delayRanges[i];
		}
	}
    void init(double sampleRate, int numChannels, double delayRangeMs);
    void setDiffusionTimeMs(double sampleRate, double diffusionTimeMs);
    void processSamples(float* samples);
    void processMultiChannel(juce::AudioBuffer<float>* buffer);
    static void splitBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>* newBuffer, int numSamples, int numChannels);
    static void mergeBuffer(juce::AudioBuffer<float>* buffer, juce::AudioBuffer<float>& outBuffer);
    int getNumChannels() { return numChannels; }
private:
    int numChannels;
    std::vector<DelayLine> delayLines;
    hadamardMatrix mixer;
    float* samples;
    double delayRangeMs;
    std::vector<std::atomic<float>*> delayLengths;
    std::vector<std::atomic<float>*> delayRanges;
};

