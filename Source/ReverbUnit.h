/*
  ==============================================================================

    ReverbUnit.h
    Created: 4 Aug 2024 2:51:28pm
    Author:  sasha

  ==============================================================================
*/

#pragma once
#include "MultiChanDiffuser.h"
class ReverbUnit {
public:
    ReverbUnit(double sampleRate, int numChannels,int samplesPerBlock ,juce::AudioProcessorValueTreeState& vts);
    void processBlock(juce::AudioBuffer<float>& buffer);
private:
    void updateParmas();
    int numChannels;
    double sampleRate;
    std::array<std::unique_ptr<MultiChanDiffuser>, NUM_DIFFUSERS> diffusers;
    std::unique_ptr<MultiChanDelayLine> feedbackDelay;
    std::atomic<float>* feedbackGain;
    std::atomic<float>* diffusionTimeMs;
    float lastDiffusionTimeMs;
    std::atomic<float>* randomSeed;
    float lastRandomSeed;
    juce::AudioBuffer<float> tempSplitBuffer;
};
