/*
  ==============================================================================

    TimeKnob.cpp
    Created: 5 Aug 2024 1:33:00pm
    Author:  sasha

  ==============================================================================
*/

#include "TimeKnob.h"

//==============================================================================
TimeKnob::TimeKnob(juce::AudioProcessorValueTreeState& apvts, std::string id, std::string label, std::string suffix)
	: label(label)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    timeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    timeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    timeSlider.setTextValueSuffix(suffix);
    timeSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, id, timeSlider);
    addAndMakeVisible(timeSlider);
}

TimeKnob::~TimeKnob()
{
    timeSliderAttachment.reset();
}

void TimeKnob::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText (label, getLocalBounds().reduced(5), juce::Justification::centredBottom, true);   // draw some placeholder text
}

void TimeKnob::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    timeSlider.setBounds(0, 0, getWidth(), getHeight() - 20);
    timeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, getWidth(), 20);
}
