/*
  ==============================================================================

    DelayLine.cpp
    Created: 4 Aug 2024 3:21:17pm
    Author:  sasha

  ==============================================================================
*/

#include "DelayLine.h"

DelayLine::DelayLine(std::atomic<float>* length, std::atomic<float>* range, bool polarity, double sampleRate) {
    for (int i = 0; i < MAX_DELAY_LENGTH_SAMPLES; ++i) {
        delayBuffer[i] = 0;
    }
    curIndex = 0; 
    this->polarity = polarity;
    sampleRateMs = sampleRate * 0.001;
    delayLength = length;
    delayRange = range;
    randVal = static_cast<float>(abs(rand())) / RAND_MAX;
}

void DelayLine::setSample(float sample) {
	delayBuffer[curIndex] = sample;
	curIndex = (curIndex + 1) % MAX_DELAY_LENGTH_SAMPLES;
}

inline int DelayLine::getDelayInSampeles() {
    return static_cast<int>((delayLength->load() + delayRange->load() * randVal) * sampleRateMs) + 1;
}

float DelayLine::getSample() {
    const int delayInSamples = getDelayInSampeles();
    int delayedIndex = curIndex - delayInSamples;
    delayedIndex = delayedIndex < 0 ? MAX_DELAY_LENGTH_SAMPLES + delayedIndex : delayedIndex;
    jassert(delayedIndex >= 0 && delayedIndex < MAX_DELAY_LENGTH_SAMPLES);
    return delayBuffer[delayedIndex] * (this->polarity ? 1 : -1);
}

MultiChanDelayLine::MultiChanDelayLine(double sampleRate, int numChannels, juce::AudioProcessorValueTreeState& vts) 
    : mixer(numChannels) {
    this->numChannels = numChannels;
    this->curIndex = 0;
    this->feedbackGain = vts.getRawParameterValue("feedbackGain");
    this->sampleRate = sampleRate;
    std::atomic<float>* delayLength = vts.getRawParameterValue("delayLengthMs");
    std::atomic<float>* delayRange = vts.getRawParameterValue("delayRangeMs");
    delayLines.clear();
    for (int i = 0; i < numChannels; ++i) {
        delayLines.emplace_back(new DelayLine(delayLength, delayRange, 1, sampleRate));
    }
    sampleDelayedTemp.reset(new float[numChannels]);
    sampleTemp.reset(new float[numChannels]);
}

void MultiChanDelayLine::getSamples(float* samples) {
    for (int i = 0; i < numChannels; ++i) {
        jassert(delayLines[i]);
		samples[i] = delayLines[i]->getSample();
	}
}
void MultiChanDelayLine::setSamples(float* samples) {
    for (int i = 0; i < numChannels; ++i) {
        jassert(samples[i] == 0);
        delayLines[i]->setSample(samples[i]);
	}
}

void MultiChanDelayLine::processBlock(juce::AudioBuffer<float>* buffer) {
    jassert(buffer->getNumChannels() == numChannels);
    const int numSamples = buffer->getNumSamples();
    const float feedback = feedbackGain->load();
    auto* samplesOld = sampleDelayedTemp.get();
    auto* samplesNew = sampleTemp.get();
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
        getSamples(samplesOld);
        for (int chIndex = 0; chIndex < numChannels; ++chIndex) {
            const float sample = buffer->getSample(chIndex, sampleIndex);
            samplesNew[chIndex] = sample + feedback * samplesOld[chIndex];
		    jassert(abs(samplesNew[chIndex]) <= 1);
        }
		for (int chIndex = 0; chIndex < numChannels; ++chIndex) {
            buffer->setSample(chIndex, sampleIndex, samplesOld[chIndex]);
        }
        setSamples(samplesNew);
	}

}

hadamardMatrix::hadamardMatrix(int numChannels) {
    this->numChannels = numChannels;
    this->sampleAccum = new float[numChannels];
    this->matrix = new float[numChannels * numChannels];
    matrix[0] = 1;
    for (int k = 1; k < numChannels; k+=k) {
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < k; ++j) {
                matrix[(i + k) * numChannels + j] = matrix[numChannels*i + j];
                matrix[i * numChannels + j + k] = matrix[i*numChannels + j];
                matrix[(i + k) * numChannels + j + k] = -matrix[i * numChannels + j];
			}
        }
    }
    #if JUCE_DEBUG
    for (int i = 0; i < numChannels; ++i) {
        for (int j = 0; j < numChannels; ++j) {
		    jassert(matrix[i * numChannels + j] == 1 || matrix[i * numChannels + j] == -1);
		}
	}
    #endif
}

hadamardMatrix::~hadamardMatrix() {
	delete[] matrix;
	delete[] sampleAccum;
}
void hadamardMatrix::processSamples(float* sampels) {
    int rowIdx = 0;
    for (int i = 0; i < this->numChannels; ++i) {
		sampleAccum[i] = 0;
        rowIdx = i * numChannels;
		for (int j = 0; j < this->numChannels; ++j) {
            if (matrix[rowIdx + j] == 1) {
				sampleAccum[i] += sampels[j];
            }
            else {
                jassert(matrix[rowIdx + j] == -1);
				sampleAccum[i] -= sampels[j];
			}
		}
	}
	for (int i = 0; i < this->numChannels; ++i) {
		sampels[i] = sampleAccum[i];
	}
}



