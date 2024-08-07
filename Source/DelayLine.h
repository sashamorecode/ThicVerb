/*
  ==============================================================================

    DelayLine.h
    Created: 4 Aug 2024 3:21:17pm
    Author:  sasha

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <random>
#define MAX_DELAY_LENGTH_SAMPLES 20000

class hadamardMatrix {
public:
    hadamardMatrix(int numChannels);
    ~hadamardMatrix();
    void processSamples(float* samples);
private:
    int numChannels;
    float* matrix = nullptr;
    float* sampleAccum = nullptr;
    //std::unique_ptr<float*> matrix;
    //std::unique_ptr<float*> sampleAccum;
};

class DelayLine {
public:
    DelayLine(int numSamples, bool polarity);
    void setDelayLength(int numSamples);
    void setSample(float sample);
    float getSample();
    bool polarity;
private:
	int delayLength;
    std::unique_ptr<float[]> delayBuffer;
    int curIndex;
};

class IndependentDelayLine {
public:
    IndependentDelayLine(int numSamples);
    void setDelayLength(int numSamples);
    float getSample();
    void setSample(float sample);
private:
    int delayNumSamples;
    std::unique_ptr<float[]> delayBuffer;
    int curIndex;
};

class MultiChanDelayLine {
public:
    MultiChanDelayLine(double sampleRate, int numChannels, std::atomic<float>* feedbackGain);
    void setDelayLengths(float delayTimeMs, float delayRangeMs);
    void processBlock(juce::AudioBuffer<float>* buffer);
    void getSamples(float* samples);
    void setSamples(float* samples);
private:
	int numChannels;
	int curIndex;
    double sampleRate;
    std::atomic<float>* feedbackGain;
    std::vector<std::unique_ptr<IndependentDelayLine>> delayLines;
    std::unique_ptr<float[]> sampleTemp;
    std::unique_ptr<float[]> sampleDelayedTemp;
    hadamardMatrix mixer;
};
