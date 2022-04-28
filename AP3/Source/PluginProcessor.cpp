/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================


AP3AudioProcessor::AP3AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    avpts(*this, nullptr, "ParamTreeIdentifier", {
//std::makeUnique<juce::AudioPrameterFloat>("release", "Release Time", 0.0001, 5.0, 1.0), 
    std::make_unique < juce::AudioParameterFloat >("volume", " Volume ", 0.0f , 1.0f , 0.5f)
, std::make_unique < juce::AudioParameterFloat >("cutoffFreq", "Cutoff Freq", 50.0f , 750.0f , 200.0f)
, std::make_unique < juce::AudioParameterFloat >("delayTime", "Delay Time", 0.01f , 0.99f , 0.25f)
, std::make_unique < juce::AudioParameterChoice >("direction", "Direction", juce::StringArray({"rampUp", "rampDown"}), 0)
    })
{
    volumeParameter = avpts.getRawParameterValue("volume");
    minMaxParameter = avpts.getRawParameterValue("cutoffFreq");
    delayParameter = avpts.getRawParameterValue("delayTime");
    upDownParameter = avpts.getRawParameterValue("direction");
    for (int i = 0; i < voiceCount; i++)
    {
        synth.addVoice( new YourSynthVoice() );
    }
    synth.addSound( new MySynthSound() );
}

AP3AudioProcessor::~AP3AudioProcessor()
{
}

void AP3AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate); // set the sample rate of synth

    sr = sampleRate;
    if (upDownParameter > 0) // if it is not the first choice
    {
        // some code here
    }

    delay.setDelayTime(sr * delayTimeInSeconds);

    // smooth value setting
    smoothVolume.reset(sampleRate, 1.0f);
    smoothVolume.setCurrentAndTargetValue(0.0);
}

void AP3AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    int numSamples = buffer.getNumSamples();

    float* left = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);

    // process entire block of samples for synths
    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);

    delay.setDelayTime(*delayParameter * sr); // delay
    smoothVolume.setTargetValue(*volumeParameter); // smooth value

    for (int i = 0; i < numSamples; i++)
    {
        float gainVal = smoothVolume.getNextValue();

        //left[i] =
        //right[i] =
    }

}
//==============================================================================
const juce::String AP3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AP3AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AP3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AP3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AP3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AP3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AP3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void AP3AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AP3AudioProcessor::getProgramName (int index)
{
    return {};
}

void AP3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================


void AP3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AP3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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



//==============================================================================
bool AP3AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AP3AudioProcessor::createEditor()
{
    return new AP3AudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void AP3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = avpts.copyState();
    std::unique_ptr < juce::XmlElement > xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AP3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
// whose contents will have been created by the getStateInformation() call.
    std::unique_ptr < juce::XmlElement > xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(avpts.state.getType()))
            avpts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AP3AudioProcessor();
}
