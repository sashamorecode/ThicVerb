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
    randSeedSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    randSeedSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    randSeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, "randomSeed", randSeedSlider);
    addAndMakeVisible(randSeedSlider);

    highQualityButton.setButtonText("High Quality");
    highQualityButton.setToggleState(false, juce::NotificationType::sendNotification);
        highQualityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "highQuality", highQualityButton);
    addAndMakeVisible(highQualityButton);

    diffusionLengthKnob.reset(new TimeKnob(vts, "diffusionTimeMs", "Diffusion Length", " ms"));
    addAndMakeVisible(diffusionLengthKnob.get());
    
    decayLengthKnob.reset(new TimeKnob(vts, "feedbackGain", "Decay Length (feedback)", ""));
    addAndMakeVisible(decayLengthKnob.get());
    decayComplexityKnob.reset(new TimeKnob(vts, "delayRangeMs", "Decay Complexity", " ms"));
    addAndMakeVisible(decayComplexityKnob.get());
    roomSizeKnob.reset(new TimeKnob(vts, "delayLengthMs", "Room Size (delay len)", " ms"));
	addAndMakeVisible(roomSizeKnob.get());


    setSize (600, 400);
}

ThicVerbAudioProcessorEditor::~ThicVerbAudioProcessorEditor()
{
    randSeedAttachment.reset();
	diffusionLengthKnob.reset();
	decayComplexityKnob.reset();
}

//==============================================================================
void ThicVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void ThicVerbAudioProcessorEditor::resized()
{
    randSeedSlider.setBounds(10, 10, 200, 20);
    highQualityButton.setBounds(randSeedSlider.getRight() + 10, 10, 100, 20);
    diffusionLengthKnob->setBounds(10, randSeedSlider.getBottom() + 10, 140, 120);
    decayLengthKnob->setBounds(10, diffusionLengthKnob->getBottom() + 10, 140, 120);
    roomSizeKnob->setBounds(decayLengthKnob->getRight() + 10, decayLengthKnob->getY(), 140, 120);
    decayComplexityKnob->setBounds(roomSizeKnob->getRight() + 10, decayLengthKnob->getY(), 140, 120);

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
