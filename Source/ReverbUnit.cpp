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
    this->delayLengthMs = vts.getRawParameterValue("delayLengthMs");
    lastDelayLengthMs = delayLengthMs->load();
    this->delayRangeMs = vts.getRawParameterValue("delayRangeMs");
    lastDelayRangeMs = delayRangeMs->load();
    this->diffusionTimeMs = vts.getRawParameterValue("diffusionTimeMs");
    lastDiffusionTimeMs = diffusionTimeMs->load();
    this->randomSeed = vts.getRawParameterValue("randomSeed");
    lastRandomSeed = randomSeed->load();
    this->sampleRate = sampleRate;
    for (int i = 0; i < NUM_DIFFUSERS; ++i) {
		this->diffusers[i].reset(new MultiChanDiffuser(sampleRate, numChannels, 100));
	}
    this->feedbackDelay.reset(new MultiChanDelayLine(sampleRate, numChannels, feedbackGain));
    srand(randomSeed->load());
    for (int i = 0; i < NUM_DIFFUSERS; ++i) {
        diffusers[i]->setDiffusionTimeMs(sampleRate, lastDiffusionTimeMs/ NUM_DIFFUSERS);
    }
    srand(randomSeed->load() + 1);
    feedbackDelay->setDelayLengths(lastDelayLengthMs, lastDelayRangeMs);
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
    if (lastDelayLengthMs != delayLengthMs->load() || lastDelayRangeMs != delayRangeMs->load()) {
        lastDelayLengthMs = delayLengthMs->load();
        lastDelayRangeMs = delayRangeMs->load();
        srand(randomSeed->load() + 1);
        feedbackDelay->setDelayLengths(lastDelayLengthMs, lastDelayRangeMs);
    }
    if (lastDiffusionTimeMs != diffusionTimeMs->load()) {
		lastDiffusionTimeMs = diffusionTimeMs->load();
        srand(randomSeed->load());
        for (int i = 0; i < NUM_DIFFUSERS; ++i) {
			diffusers[i]->setDiffusionTimeMs(sampleRate, lastDiffusionTimeMs / NUM_DIFFUSERS);
        }
    }
    if (lastRandomSeed != randomSeed->load()) {
		lastRandomSeed = randomSeed->load();
		srand(lastRandomSeed);
        for (int i = 0; i < NUM_DIFFUSERS; ++i) {
            diffusers[i]->setDiffusionTimeMs(sampleRate, lastDiffusionTimeMs/NUM_DIFFUSERS);
        }
        srand(lastRandomSeed + 1);
        feedbackDelay->setDelayLengths(lastDelayLengthMs, lastDelayRangeMs);
    }
}

