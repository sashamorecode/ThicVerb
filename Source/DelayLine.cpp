/*
  ==============================================================================

    DelayLine.cpp
    Created: 4 Aug 2024 3:21:17pm
    Author:  sasha

  ==============================================================================
*/

#include "DelayLine.h"

DelayLine::DelayLine(int length, bool polarity) {
    delayBuffer.reset(new float[MAX_DELAY_LENGTH_SAMPLES]);
    for (int i = 0; i < MAX_DELAY_LENGTH_SAMPLES; ++i) {
        delayBuffer[i] = 0;
    }
    curIndex = 0;
    this->polarity = polarity;
    setDelayLength(length);
}

void DelayLine::setDelayLength(int length) {
    jassert(length < MAX_DELAY_LENGTH_SAMPLES);
    this->delayLength = length;
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
    this->delayBuffer.reset(new float[MAX_DELAY_LENGTH_SAMPLES]);
    for (int i = 0; i < MAX_DELAY_LENGTH_SAMPLES; ++i) {
		delayBuffer[i] = 0;
	}
    this->delayNumSamples = numSamples;
    this->curIndex = 0;
}
void IndependentDelayLine::setDelayLength(int numSamples) {
	this->delayNumSamples = numSamples;
}
float IndependentDelayLine::getSample() {
	const int delayedIndex = this->curIndex - this->delayNumSamples;
    return delayBuffer[delayedIndex < 0 ? MAX_DELAY_LENGTH_SAMPLES + delayedIndex : delayedIndex];
}
void IndependentDelayLine::setSample(float sample) {
	delayBuffer[curIndex] = sample;
	curIndex = (curIndex + 1) % MAX_DELAY_LENGTH_SAMPLES;
}

MultiChanDelayLine::MultiChanDelayLine(double sampleRate, int numChannels, std::atomic<float>* feedbackParam) {
    this->numChannels = numChannels;
    this->curIndex = 0;
    this->feedbackGain = feedbackParam;
    this->sampleRate = sampleRate;
    delayLines.clear();
    for (int i = 0; i < numChannels; ++i) {
        delayLines.emplace_back(new IndependentDelayLine(100));
    }
    setDelayLengths(20, 10);
    mixer.reset(new hadamardMatrix(numChannels));
    sampleDelayedTemp.reset(new float[numChannels]);
    sampleTemp.reset(new float[numChannels]);
}
void MultiChanDelayLine::setDelayLengths(float delayTimeMs, float delayRangeMs) {
    const int delaySamples = static_cast<int>(sampleRate * delayTimeMs * 0.001);
    const int delayRangeSamples = static_cast<int>(sampleRate * delayRangeMs * 0.001);
    for (int i = 0; i < numChannels; ++i) {
        const float rand = abs(static_cast<float>(std::rand()) / RAND_MAX);
        const int delayLength = delaySamples + rand * delayRangeSamples;
        delayLines[i]->setDelayLength(delayLength > 0 ? delayLength : 1);
	}
}

void MultiChanDelayLine::getSamples(float* samples) {
    for (int i = 0; i < numChannels; ++i) {
		samples[i] = delayLines[i]->getSample();
	}
}
void MultiChanDelayLine::setSamples(float* samples) {
    for (int i = 0; i < numChannels; ++i) {
		delayLines[i]->setSample(samples[i]);
	}
}

void MultiChanDelayLine::processBlock(juce::AudioBuffer<float>* buffer) {
    const int numSamples = buffer->getNumSamples();
    auto* writePointers = buffer->getArrayOfWritePointers();
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
        getSamples(sampleDelayedTemp.get());
        for (int chIndex = 0; chIndex < numChannels; ++chIndex) {
			sampleTemp.get()[chIndex] = writePointers[chIndex][sampleIndex] + feedbackGain->load() * sampleDelayedTemp.get()[chIndex];
		}
		for (int chIndex = 0; chIndex < numChannels; ++chIndex) {
            writePointers[chIndex][sampleIndex] = sampleDelayedTemp.get()[chIndex];
        }
		setSamples(sampleTemp.get());
	}

}

hadamardMatrix::hadamardMatrix(int numChannels) {
    this->numChannels = numChannels;
    this->sampleAccum.reset(new float[numChannels]);
    matrix.reset(new float[numChannels * numChannels]);
    matrix[0] = 1;
    for (int k = 1; k < numChannels; k+=k) {
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < k; ++j) {
                jassert(matrix[i * numChannels + j] != 0);
                matrix[(i + k) * numChannels + j] = matrix[numChannels*i + j];
                matrix[i * numChannels + j + k] = matrix[i*numChannels + j];
                matrix[(i + k) * numChannels + j + k] = -matrix[i * numChannels + j];
			}
        }
    }
}
void hadamardMatrix::processSamples(float* sampels) {
    for (int i = 0; i < this->numChannels; ++i) {
		sampleAccum[i] = 0;
		for (int j = 0; j < this->numChannels; ++j) {
			sampleAccum[i] += matrix[i * numChannels + j] * sampels[j];
		}
	}
	for (int i = 0; i < this->numChannels; ++i) {
		sampels[i] = sampleAccum[i];
	}
}



