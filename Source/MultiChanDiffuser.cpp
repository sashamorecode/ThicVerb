/*
  ==============================================================================

    MultiChanDiffuser.cpp
    Created: 2 Aug 2024 2:55:53pm
    Author:  sasha

  ==============================================================================
*/

#include "MultiChanDiffuser.h"


MultiChanDiffuser::MultiChanDiffuser(double sampleRate, int numChannels, double delayRangeMs) {
    this->init(sampleRate, numChannels, delayRangeMs);
}
void MultiChanDiffuser::init(double sampleRate, int numChannels, double delayRangeMs) {
    this->delayRangeMs = delayRangeMs;
    this->numChannels = numChannels;
    const double delaySampleRange =   sampleRate * delayRangeMs * 0.001;
    this->delayLines.clear();
    for (int i = 0; i < numChannels; ++i) {
        const double rangeMax = delaySampleRange * (i + 1) / numChannels;
        const double rangeMin = delaySampleRange * i / numChannels;
        const double randDouble = static_cast<double>(abs(rand()))/RAND_MAX;
        const int delayLength = static_cast<int>(rangeMin + randDouble * (rangeMax - rangeMin) + 1);
        const bool polarity = rand() % 2 == 0;
        this->delayLines.emplace_back(delayLength, polarity);
    }
    mixer.reset(new hadamardMatrix(numChannels));
}

void MultiChanDiffuser::setDiffusionTimeMs(double sampleRate, double timeMs) {
    this->delayRangeMs = timeMs;
    const double delaySampleRange =  sampleRate * timeMs * 0.001;
    for (int i = 0; i < numChannels; ++i) {
        const double rangeMax = delaySampleRange * (i + 1) / numChannels;
        const double rangeMin = delaySampleRange * i / numChannels;
        const double randDouble = static_cast<double>(abs(rand()))/RAND_MAX;
        const int delayLength = static_cast<int>(rangeMin + randDouble * (rangeMax - rangeMin) + 1);
        const bool polarity = rand() % 2 == 0;
		this->delayLines[i].setDelayLength(delayLength);
        this->delayLines[i].polarity = polarity;
    }
}


void MultiChanDiffuser::processSamples(float* samples) {
    for (int i = 0; i < this->numChannels; ++i) {
        this->delayLines[i].setSample(samples[i]);
        samples[i] = this->delayLines[i].getSample();
    }
    mixer->processSamples(samples);
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
    float* samples = new float[this->numChannels];
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
		for (int i = 0; i < this->numChannels; ++i) {
			samples[i] = buffer->getSample(i, sampleIndex);
        }
        this->processSamples(samples);
        for (int i = 0; i < this->numChannels; ++i) {
			buffer->setSample(i, sampleIndex, samples[i]);
		}
    }
    delete[] samples;
}



	
