/*
  ==============================================================================

    MultiChanDiffuser.cpp
    Created: 2 Aug 2024 2:55:53pm
    Author:  sasha

  ==============================================================================
*/

#include "MultiChanDiffuser.h"


MultiChanDiffuser::MultiChanDiffuser(double sampleRate, int numChannels, double delayRangeMs): mixer(numChannels) {
    this->init(sampleRate, numChannels, delayRangeMs);
}
void MultiChanDiffuser::init(double sampleRate, int numChannels, double delayRangeMs) {
    samples = new float[numChannels];
    this->delayRangeMs = delayRangeMs;
    this->numChannels = numChannels;
    this->delayLines.clear();
    for (int i = 0; i < numChannels; ++i) {
        const double rangeMax = delayRangeMs * (i + 1) / numChannels;
        const double rangeMin = delayRangeMs * i / numChannels;
        const float range = static_cast<float>(rangeMax - rangeMin);
        std::atomic<float>* delayLength = new std::atomic<float>(rangeMin);
        std::atomic<float>* delayRange = new std::atomic<float>(range);
		const bool polarity = rand() % 2 == 0;
		delayRanges.push_back(delayRange);
        delayLengths.push_back(delayLength);
        this->delayLines.emplace_back(delayLength, delayRange, polarity, sampleRate);
    }
}

void MultiChanDiffuser::setDiffusionTimeMs(double sampleRate, double diffusionTimeMs) {
	for (int i = 0; i < numChannels; ++i) {
		const double rangeMax = diffusionTimeMs * (i + 1) / numChannels;
		const double rangeMin = diffusionTimeMs * i / numChannels;
		const float range = static_cast<float>(rangeMax - rangeMin);
		delayRanges[i]->store(range);
		delayLengths[i]->store(rangeMin);
	}
}

void MultiChanDiffuser::processSamples(float* samples) {
    for (int i = 0; i < this->numChannels; ++i) {
        jassert(abs(samples[i]) <= 1);
        this->delayLines[i].setSample(samples[i]);
        samples[i] = this->delayLines[i].getSample();
        jassert(abs(samples[i]) <= 1);
    }
    mixer.processSamples(samples);
}

void MultiChanDiffuser::splitBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>* newBuffer, int numSamples, int numChannels) {
    jassert(newBuffer);
    const float* inputBuffer = buffer.getReadPointer(0);
    newBuffer->setSize(numChannels, numSamples, false, true, true);
    for (int i = 0; i < numChannels; ++i) {
        newBuffer->copyFrom(i, 0, inputBuffer, numSamples);
    }
}

void MultiChanDiffuser::mergeBuffer(juce::AudioBuffer<float>* buffIn, juce::AudioBuffer<float>& buffOut) {
    buffOut.clear();
    const int numSamples = buffIn->getNumSamples();
    const int inChannels = buffIn->getNumChannels();
    const int outChannels = buffOut.getNumChannels();

    for (int i = 0; i < inChannels; ++i) {
        auto* channelData = buffIn->getReadPointer(i);
        for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
            for (int j = 0; j < outChannels; ++j) {
				buffOut.addSample(j, sampleIndex, channelData[sampleIndex] / static_cast<float>(inChannels * outChannels * 2));
			}
		}
    }
}

void MultiChanDiffuser::processMultiChannel(juce::AudioBuffer<float>* buffer) {
    jassert(buffer->getNumChannels() == this->numChannels);
    const int numSamples = buffer->getNumSamples();
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
		for (int i = 0; i < this->numChannels; ++i) {
			samples[i] = buffer->getSample(i, sampleIndex);
            jassert(abs(samples[i]) <= 1);
        }
        this->processSamples(samples);
        for (int i = 0; i < this->numChannels; ++i) {
            jassert(abs(samples[i]) <= 1);
			buffer->setSample(i, sampleIndex, samples[i]);
		}
    }
}



	
