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
    DelayLine(std::atomic<float>* length, std::atomic<float>* range, bool polarity, double sampleRate);
    void setSample(float sample);
    float getSample();
    bool polarity;
private:
    inline int getDelayInSampeles();
    std::atomic<float>* delayLength;
    std::atomic<float>* delayRange;
    float randVal;
    float sampleRateMs;
    float delayBuffer[MAX_DELAY_LENGTH_SAMPLES];
    int curIndex;

};

class MultiChanDelayLine {
public:
    MultiChanDelayLine(double sampleRate, int numChannels, juce::AudioProcessorValueTreeState& vts);
    void processBlock(juce::AudioBuffer<float>* buffer);
    void getSamples(float* samples);
    void setSamples(float* samples);
private:
	int numChannels;
	int curIndex;
    double sampleRate;
    std::atomic<float>* feedbackGain;
    std::vector<std::unique_ptr<DelayLine>> delayLines;
    std::unique_ptr<float[]> sampleTemp;
    std::unique_ptr<float[]> sampleDelayedTemp;
    hadamardMatrix mixer;
};
