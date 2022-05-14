/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "YourSynthVoice.h"
#include "pulseSynth.h"
#include "FMSynth.h"
#include "Oscillator.h"

//==============================================================================
/**
*/
class MakeSoundAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MakeSoundAudioProcessor();
    ~MakeSoundAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    // audio effects
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    // synthesiser class
    int voiceCount = 8;
    juce::Synthesiser synthPulse;
    juce::Synthesiser synth;
    juce::Synthesiser synth2;
    
    juce::AudioProcessorValueTreeState avpts;
    // parameters 
    std::atomic<float>* volumeParameter;
    std::atomic<float>* detuneParameter;
    std::atomic<float>* modeParameter;          // string to choose mode (scales)
    std::atomic<float>* pulseSpeedParameter;
    std::atomic<float>* reverbParameter;
    std::atomic<float>* cuttOffMode;
    std::atomic<float>* minVal;
    std::atomic<float>* maxVal;

    // smooth values
    juce::SmoothedValue<float> smoothVolume;
    
//    juce::AudioBuffer<float> audioBuffer;
    SineOsc lfo1;
    SineOsc lfo2;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MakeSoundAudioProcessor)
};
