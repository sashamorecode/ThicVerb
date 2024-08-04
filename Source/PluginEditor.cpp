/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ThicVerbAudioProcessorEditor::ThicVerbAudioProcessorEditor (ThicVerbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts) 
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    randSeedSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    randSeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "randomSeed", randSeedSlider);
    addAndMakeVisible(randSeedSlider);

    diffusionLengthSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    diffusionLengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "diffusionTimeMs", diffusionLengthSlider);
    addAndMakeVisible(diffusionLengthSlider);
}

ThicVerbAudioProcessorEditor::~ThicVerbAudioProcessorEditor()
{
}

//==============================================================================
void ThicVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ThicVerbAudioProcessorEditor::resized()
{
    randSeedSlider.setBounds(10, 10, 200, 20);
    diffusionLengthSlider.setBounds(10, randSeedSlider.getBottom() + 10, 200, 20);
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
