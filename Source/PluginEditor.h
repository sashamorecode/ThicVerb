/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "TimeKnob.h"

//==============================================================================
/**
*/
class ThicVerbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ThicVerbAudioProcessorEditor(ThicVerbAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~ThicVerbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ThicVerbAudioProcessor& audioProcessor;

    juce::Slider randSeedSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> randSeedAttachment;


    std::unique_ptr<TimeKnob> diffusionLengthKnob;
    std::unique_ptr<TimeKnob> decayLengthKnob;
    std::unique_ptr<TimeKnob> decayComplexityKnob;
    std::unique_ptr<TimeKnob> roomSizeKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThicVerbAudioProcessorEditor)
};
