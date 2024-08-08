/*
  ==============================================================================

    ReverbUnit.cpp
    Created: 4 Aug 2024 2:51:28pm
    Author:  sasha

  ==============================================================================
*/

#include "ReverbUnit.h"

ReverbUnit::ReverbUnit(double sampleRate, int numChannels, int samplesPerBlock , juce::AudioProcessorValueTreeState& vts) 
    : tempSplitBuffer(numChannels, samplesPerBlock) {
    this->numChannels = numChannels;
    this->feedbackGain = vts.getRawParameterValue("feedbackGain");
   this->diffusionTimeMs = vts.getRawParameterValue("diffusionTimeMs");
    lastDiffusionTimeMs = diffusionTimeMs->load();
    this->randomSeed = vts.getRawParameterValue("randomSeed");
    this->sampleRate = sampleRate;
    srand(randomSeed->load());
    for (int i = 0; i < NUM_DIFFUSERS; ++i) {
		this->diffusers[i].reset(new MultiChanDiffuser(sampleRate, numChannels, lastDiffusionTimeMs / NUM_DIFFUSERS));
	}
    this->feedbackDelay.reset(new MultiChanDelayLine(sampleRate, numChannels, vts));
    }

void ReverbUnit::processBlock(juce::AudioBuffer<float>& buffer) {
    updateParmas();
    MultiChanDiffuser::splitBuffer(buffer, &tempSplitBuffer, buffer.getNumSamples(), numChannels);
    for (int i = 0; i < NUM_DIFFUSERS; ++i) {
		diffusers[i]->processMultiChannel(&tempSplitBuffer);
	}
    feedbackDelay->processBlock(&tempSplitBuffer);
	MultiChanDiffuser::mergeBuffer(&tempSplitBuffer, buffer);
}

void ReverbUnit::updateParmas(){
    if (lastDiffusionTimeMs != diffusionTimeMs->load()) {
		lastDiffusionTimeMs = diffusionTimeMs->load();
        srand(randomSeed->load());
        for (int i = 0; i < NUM_DIFFUSERS; ++i) {
			diffusers[i]->setDiffusionTimeMs(sampleRate, lastDiffusionTimeMs / NUM_DIFFUSERS);
        }
    }
}

