/*
  ==============================================================================

    DelayLine.cpp
    Created: 4 Aug 2024 3:21:17pm
    Author:  sasha

  ==============================================================================
*/

#include "DelayLine.h"

DelayLine::DelayLine(int length, bool polarity) {
    jassert(length < MAX_DELAY_LENGTH_SAMPLES);
    for (int i = 0; i < MAX_DELAY_LENGTH_SAMPLES; ++i) {
        delayBuffer[i] = 0;
    }
    curIndex = 0;
    this->polarity = polarity;
    delayLength = length;
    setDelayLength(length);
}

void DelayLine::setDelayLength(int length) {
    jassert(length < MAX_DELAY_LENGTH_SAMPLES);
    if (length < delayLength) {
		curIndex += (length - delayLength);
		curIndex = curIndex < 0 ? MAX_DELAY_LENGTH_SAMPLES + curIndex : curIndex;
    }
    delayLength = length;
}

void DelayLine::setSample(float sample) {
	delayBuffer[curIndex] = sample;
	curIndex = (curIndex + 1) % MAX_DELAY_LENGTH_SAMPLES;
}

float DelayLine::getSample() {
    int delayedIndex = curIndex - delayLength;
    delayedIndex = delayedIndex < 0 ? MAX_DELAY_LENGTH_SAMPLES + delayedIndex : delayedIndex;
    return delayBuffer[delayedIndex] * (this->polarity ? 1 : -1);
}

IndependentDelayLine::IndependentDelayLine(int numSamples) {
    jassert(numSamples < MAX_DELAY_LENGTH_SAMPLES);
    for (int i = 0; i < MAX_DELAY_LENGTH_SAMPLES; ++i) {
		delayBuffer[i] = 0;
	}
    this->delayNumSamples = numSamples;
    this->curIndex = 0;
}

void IndependentDelayLine::setDelayLength(int numSamples) {
    jassert(numSamples < MAX_DELAY_LENGTH_SAMPLES);
    if (numSamples < delayNumSamples) {
        curIndex += (numSamples - delayNumSamples);
        curIndex = curIndex < 0 ? MAX_DELAY_LENGTH_SAMPLES + curIndex : curIndex;
	}
    delayNumSamples = numSamples;
}
float IndependentDelayLine::getSample() {
	const int delayedIndex = this->curIndex - this->delayNumSamples;
    jassert(delayedIndex < MAX_DELAY_LENGTH_SAMPLES);
    return delayBuffer[delayedIndex < 0 ? MAX_DELAY_LENGTH_SAMPLES + delayedIndex : delayedIndex];
}
void IndependentDelayLine::setSample(float sample) {
    jassert(curIndex < MAX_DELAY_LENGTH_SAMPLES);
	delayBuffer[curIndex] = sample;
	curIndex = (curIndex + 1) % MAX_DELAY_LENGTH_SAMPLES;
}

MultiChanDelayLine::MultiChanDelayLine(double sampleRate, int numChannels, std::atomic<float>* feedbackParam)  : mixer(numChannels) {
    this->numChannels = numChannels;
    this->curIndex = 0;
    this->feedbackGain = feedbackParam;
    this->sampleRate = sampleRate;
    delayLines.clear();
    for (int i = 0; i < numChannels; ++i) {
        delayLines.emplace_back(new IndependentDelayLine(100));
    }
    setDelayLengths(20, 10);
    sampleDelayedTemp.reset(new float[numChannels]);
    sampleTemp.reset(new float[numChannels]);
}
void MultiChanDelayLine::setDelayLengths(float delayTimeMs, float delayRangeMs) {
    const int delaySamples = static_cast<int>(sampleRate * delayTimeMs * 0.001);
    const int delayRangeSamples = static_cast<int>(sampleRate * delayRangeMs * 0.001);
    for (int i = 0; i < numChannels; ++i) {
        const float rand = abs(static_cast<float>(std::rand()) / RAND_MAX);
        jassert(rand >= 0 && rand <= 1.0f);
        const int delayLength = delaySamples + rand * delayRangeSamples;
        delayLines[i]->setDelayLength(delayLength);
	}
}

void MultiChanDelayLine::getSamples(float* samples) {
    for (int i = 0; i < numChannels; ++i) {
        jassert(delayLines[i]);
		samples[i] = delayLines[i]->getSample();
	}
}
void MultiChanDelayLine::setSamples(float* samples) {
    for (int i = 0; i < numChannels; ++i) {
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
            samplesNew[chIndex] = buffer->getSample(chIndex, sampleIndex) + feedback * samplesOld[chIndex];
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



