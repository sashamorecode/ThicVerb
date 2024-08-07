/*
  ==============================================================================

    TimeKnob.h
    Created: 5 Aug 2024 1:33:00pm
    Author:  sasha

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class TimeKnob  : public juce::Component
{
public:
    TimeKnob(juce::AudioProcessorValueTreeState& apvts, std::string id, std::string label, std::string suffix);
    ~TimeKnob() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider timeSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeSliderAttachment;
    std::string label;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeKnob)
};
