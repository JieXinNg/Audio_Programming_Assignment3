/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "YourSynthVoice.h"
#include "pulseSynth.h"
#include "SourceCode.h"

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

    // synthesiser class
    int voiceCount = 8;
    juce::Synthesiser synthPulse;
    juce::Synthesiser synth;
    
    juce::AudioProcessorValueTreeState avpts;
    // parameters 
    std::atomic<float>* volumeParameter;
    std::atomic<float>* detuneParameter;
    std::atomic<float>* modeParameter;
    std::atomic<float>* pulseSpeedParameter;
    std::atomic<float>* reverbParameter;
    std::atomic<float>* sinePulseFreqParameter;
    std::atomic<float>* sinePulsePowerParameter;

    // smooth values
    juce::SmoothedValue<float> smoothVolume;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MakeSoundAudioProcessor)
};
