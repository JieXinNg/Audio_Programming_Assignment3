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
    std::make_unique < juce::AudioParameterFloat >("topVolume", "Top Synth Volume", 0.0f , 1.0f , 0.8f) ,
    std::make_unique < juce::AudioParameterFloat >("middleVolume", "Middle Synth Volume", 0.0f , 1.0f , 0.6f) ,
    std::make_unique < juce::AudioParameterFloat >("bottomVolume", "Bottom Synth Volume", 0.0f , 1.0f , 0.8f) ,
    std::make_unique < juce::AudioParameterFloat >("reverbSize", "Reverb Size", juce::NormalisableRange<float>(0.01, 0.99, 0.05, 1.75) , 0.75f),
    std::make_unique < juce::AudioParameterChoice >("cutOffMode", "Middle Synth Filter Type", juce::StringArray({ "Low-pass", "High-pass", "Band-pass", "None" }), 2),
    std::make_unique < juce::AudioParameterInt >("minCut", "Min cutoff value", 50 , 1000 , 200),
    std::make_unique < juce::AudioParameterInt >("maxCut", "Max cutoff value", 50 , 1000 , 500),
    std::make_unique < juce::AudioParameterBool >("ionian", "Ionian / Major", true),
    std::make_unique < juce::AudioParameterBool >("dorian", "Dorian", true),
    std::make_unique < juce::AudioParameterBool >("phrygian", "Phrygian", true),
    std::make_unique < juce::AudioParameterBool >("lydian", "Lydian", true),
    std::make_unique < juce::AudioParameterBool >("mixolydian", "Mixolydian", true),
    std::make_unique < juce::AudioParameterBool >("aeolian", "Aeolian", true),
    std::make_unique < juce::AudioParameterBool >("locrian", "Locrian", true)
        })
{   
   
    // constructors
    volumeParameterTop = avpts.getRawParameterValue("topVolume");
    volumeParameterMiddle = avpts.getRawParameterValue("middleVolume");
    volumeParameterBottom = avpts.getRawParameterValue("bottomVolume");
    reverbParameter = avpts.getRawParameterValue("reverbSize");
    cuttOffMode = avpts.getRawParameterValue("cutOffMode");
    minVal = avpts.getRawParameterValue("minCut");
    maxVal = avpts.getRawParameterValue("maxCut");
    Ionian = avpts.getRawParameterValue("ionian"); 
    Dorian = avpts.getRawParameterValue("dorian");  
    Phrygian = avpts.getRawParameterValue("phrygian");  
    Lydian = avpts.getRawParameterValue("lydian");
    Mixolydian = avpts.getRawParameterValue("mixolydian");
    Aeolian = avpts.getRawParameterValue("aeolian");
    Locrian = avpts.getRawParameterValue("locrian");

    for (int i = 0; i < voiceCount; i++) // loop to add voice
    {
        synth.addVoice( new MelodyVoice() );
        synthPulse.addVoice(new pulseSynthVoice());
        synth2.addVoice(new FMsynthVoice());
    }

    synth.addSound( new MelodySound() );
    synthPulse.addSound(new pulseSynthSound());
    synth2.addSound(new FMSynthSound());

    // loop that gets updated 
    for (int i = 0; i < voiceCount; i++) // set detune 
    {
        MelodyVoice* v = dynamic_cast<MelodyVoice*>(synth.getVoice(i));
        v->setVolumePointer(volumeParameterTop);

        FMsynthVoice* d = dynamic_cast<FMsynthVoice*>(synth2.getVoice(i));
        d->setVolumePointer(volumeParameterMiddle);
        d->setModFilterParams(cuttOffMode, minVal, maxVal);

        pulseSynthVoice* point = dynamic_cast<pulseSynthVoice*>(synthPulse.getVoice(i));
        point->setVolumePointer(volumeParameterBottom);

    }
}

MakeSoundAudioProcessor::~MakeSoundAudioProcessor()
{
}

void MakeSoundAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // lfo variables for panning
    leftPan.setSampleRate(sampleRate);
    rightPan.setSampleRate(sampleRate);
    leftPan.setFrequency(0.05);
    rightPan.setFrequency(0.1);

    // smooth value setting
    smoothReverb.reset(sampleRate, 1.0f);
    smoothReverb.setCurrentAndTargetValue(0.0);

    // set the sample rate of synths
    synth.setCurrentPlaybackSampleRate(sampleRate); 
    synthPulse.setCurrentPlaybackSampleRate(sampleRate); 
    synth2.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < voiceCount; i++) // set sample rate for each voice
    {
        MelodyVoice* v = dynamic_cast<MelodyVoice*>(synth.getVoice(i));
        v->init(sampleRate);
        pulseSynthVoice* point = dynamic_cast<pulseSynthVoice*>(synthPulse.getVoice(i));
        point->init(sampleRate);
        FMsynthVoice* d = dynamic_cast<FMsynthVoice*>(synth2.getVoice(i));
        d->init(sampleRate);
    }

    // set reverb parameters 
    reverbParams.dryLevel = 0.8f;
    reverbParams.wetLevel = 0.3f;
    reverbParams.roomSize = *reverbParameter;   // this is varied dynamically
    reverb.setParameters(reverbParams);
    reverb.reset();

}

void MakeSoundAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // modeOn has to be set in processBlock to check whether the values have changed
    modeOn = {(int) *Ionian, (int) *Dorian, (int) *Phrygian, (int) *Lydian, (int) *Mixolydian, (int) *Aeolian, (int) *Locrian };
    std::vector<int> selectedModes;     // vector of modes selected

    for (int i = 0; i < modeCount; i++)
    {
        if (modeOn[i] == 1)     // if a mode is selected ( i.e. parameter value == 1 )
        {
            selectedModes.push_back(i);     // append elemenets to selectedModes
        }
    }

    int totalVoiceUsed = -1; // count the order of voices for synth2 ( goes from 0 to 1 to ... to voiceCount and is repeated )

    for (int i = 0; i < voiceCount; i++)
    {
        FMsynthVoice* voicesPointer = dynamic_cast<FMsynthVoice*>(synth2.getVoice(i));
        voicesPointer = dynamic_cast<FMsynthVoice*>(synth2.getVoice(i));
        int voicesUsed = voicesPointer->getVoiceUsed(); 
        totalVoiceUsed += voicesUsed;  // add up all the voices used
    }

    totalVoiceUsed = totalVoiceUsed % voiceCount;   // modulus

    for (int i = 0; i < voiceCount; i++)
    {
        FMsynthVoice* setModePointer = dynamic_cast<FMsynthVoice*>(synth2.getVoice(i)); 

        if (totalVoiceUsed >= 0)
        {
            setModePointer = dynamic_cast<FMsynthVoice*>(synth2.getVoice(totalVoiceUsed));  
        }
            
        setModePointer->setModeLimit(selectedModes);    // set pointer to random mode value ( as long as the voice is used, the pointer will not be null )
        int modeNumber = setModePointer->getMode();     // access mode value
        int baseMidi = setModePointer->getBaseNote();   // access mode value


        if (modeNumber >= 0 && baseMidi >= 0) // if the pointers are not nullptr
        {
            pulseSynthVoice* point = dynamic_cast<pulseSynthVoice*>(synthPulse.getVoice(i));
            point->setMode(modeNumber);         // set random mode for synthPulse based on modes chosen by user

            MelodyVoice* v = dynamic_cast<MelodyVoice*>(synth.getVoice(i));
            v->setMode(baseMidi, modeNumber); // fix the key with baseMidi, set random mode for synthPulse based on modes chosen by user
        }
    }

    juce::ScopedNoDenormals noDenormals;

    // add sample values
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    synthPulse.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    synth2.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    float* left = buffer.getWritePointer(0); // access the left channel
    float* right = buffer.getWritePointer(1); // access the right channel

    for (int sample = 0; sample < buffer.getNumSamples(); sample++) // loop for panning
    {
        left[sample] = left[sample] * leftPan.process();
        right[sample] = right[sample] * rightPan.process();
    }

    // smooth value for reverb change
    smoothReverb.setTargetValue(*reverbParameter);
    float reverbChange = smoothReverb.getNextValue();
    reverbParams.roomSize = reverbChange;
    reverb.setParameters(reverbParams);                         // set reverb parameters
    reverb.processStereo(left, right, buffer.getNumSamples()); // add reverb effect
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

//==============================================================================
bool MakeSoundAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MakeSoundAudioProcessor::createEditor()
{
    return new MakeSoundAudioProcessorEditor (*this);
}

//==============================================================================
void MakeSoundAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = avpts.copyState();
    std::unique_ptr < juce::XmlElement > xml(state.createXml());
    copyXmlToBinary(*xml, destData);

}

void MakeSoundAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr < juce::XmlElement > xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState -> hasTagName(avpts.state.getType()))
            avpts.replaceState(juce::ValueTree::fromXml(*xmlState));

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MakeSoundAudioProcessor();
}
