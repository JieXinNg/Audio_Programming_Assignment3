/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MakeSoundAudioProcessor::MakeSoundAudioProcessor()
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
    std::make_unique < juce::AudioParameterFloat >("detune", "Detune (Hz)", 0.0f , 20.0f , 2.0f)
        })
{

    detuneParameter = avpts.getRawParameterValue("detune");

    for (int i = 0; i < voiceCount; i++) // loop to add voice
    {
        synth.addVoice( new MySynthVoice() );
    }
    synth.addSound( new MySynthSound() );

    for (int i = 0; i < voiceCount; i++) // set detune
    {
        MySynthVoice* v = dynamic_cast<MySynthVoice*>(synth.getVoice(i));
        v->setDetunePointer(detuneParameter);
    }
}

MakeSoundAudioProcessor::~MakeSoundAudioProcessor()
{
}

//==============================================================================
const juce::String MakeSoundAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MakeSoundAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MakeSoundAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MakeSoundAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MakeSoundAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MakeSoundAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MakeSoundAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MakeSoundAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MakeSoundAudioProcessor::getProgramName (int index)
{
    return {};
}

void MakeSoundAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MakeSoundAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate); // set the sample rate of synth

    for (int i = 0; i < voiceCount; i++) // set sample rate for each voice
    {
        MySynthVoice* v = dynamic_cast<MySynthVoice*>(synth.getVoice(i));
        v->init(sampleRate);
    }
}

void MakeSoundAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MakeSoundAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MakeSoundAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    // process entire block of samples for synths
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool MakeSoundAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MakeSoundAudioProcessor::createEditor()
{
    return new MakeSoundAudioProcessorEditor (*this);
}

//==============================================================================
void MakeSoundAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MakeSoundAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MakeSoundAudioProcessor();
}
