/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThicVerbAudioProcessorEditor)
    juce::Slider randSeedSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> randSeedAttachment;

    juce::Slider diffusionLengthSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> diffusionLengthAttachment;
};
