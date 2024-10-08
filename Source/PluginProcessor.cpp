/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ThicVerbAudioProcessor::ThicVerbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    parameters(*this, nullptr, juce::Identifier{ "PARAMS" }, {
        std::make_unique<juce::AudioParameterInt>("randomSeed", "Random Seed", 1, 1000, 1),
        std::make_unique<juce::AudioParameterFloat>("diffusionTimeMs", "Diffusion Length", 0.0f, 300.0f, 60.0f),
        std::make_unique<juce::AudioParameterFloat>("feedbackGain", "Feedback Gain", 0.0f, 0.99f, 0.85f),
        std::make_unique<juce::AudioParameterFloat>("delayLengthMs", "Delay Length", 0.0f, 50.0f, 10.0f),
        std::make_unique<juce::AudioParameterFloat>("delayRangeMs", "Delay Range", 0.0f, 200.0f, 15.0f),
        std::make_unique<juce::AudioParameterBool>("highQuality", "High Quality", false),
        })
{
    highQuality = parameters.getRawParameterValue("highQuality");
}

ThicVerbAudioProcessor::~ThicVerbAudioProcessor()
{
	this->reverb.reset(nullptr);
}

//==============================================================================
const juce::String ThicVerbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ThicVerbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ThicVerbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ThicVerbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ThicVerbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ThicVerbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ThicVerbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ThicVerbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ThicVerbAudioProcessor::getProgramName (int index)
{
    return {};
}

void ThicVerbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ThicVerbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    this->reverb.reset(new ReverbUnit(sampleRate, 32, samplesPerBlock, parameters));
}

void ThicVerbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    this->reverb.reset(nullptr);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ThicVerbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ThicVerbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    if (highQuality->load() > 0.5 && !curQuality) {
        curQuality = true;
        reverb.reset(new ReverbUnit(getSampleRate(), 64, buffer.getNumSamples(), parameters));
	}
    else if (highQuality->load() < 0.5 && curQuality) {
		curQuality = false;
		reverb.reset(new ReverbUnit(getSampleRate(), 32, buffer.getNumSamples(), parameters));
    }

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    this->reverb->processBlock(buffer);
}

//==============================================================================
bool ThicVerbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ThicVerbAudioProcessor::createEditor()
{
    return new ThicVerbAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void ThicVerbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::unique_ptr<juce::XmlElement> xmlState(parameters.state.createXml());
    copyXmlToBinary(*xmlState, destData);
    xmlState.reset();
}

void ThicVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
			parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
		}
	}
    xmlState.reset();
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ThicVerbAudioProcessor();
}
