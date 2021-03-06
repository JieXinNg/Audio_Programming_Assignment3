/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MelodySynth.h"    // synthesiser
#include "PulseSynth.h"     // synthesiser
#include "FMSynth.h"        // synthesiser
#include "Oscillator.h"     // generate lfo for panning

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
    juce::Synthesiser synthPulse;
    juce::Synthesiser synth;
    juce::Synthesiser synth2;
    int voiceCount = 8; // voice count for each synthesiser
    
    juce::AudioProcessorValueTreeState avpts;

    // parameters 
    std::atomic<float>* volumeParameterTop;
    std::atomic<float>* volumeParameterMiddle;
    std::atomic<float>* volumeParameterBottom;
    std::atomic<float>* reverbParameter;
    juce::SmoothedValue<float> smoothReverb; // smooth value for reverb
    std::atomic<float>* cuttOffMode;
    std::atomic<float>* minVal;
    std::atomic<float>* maxVal;

    // modes to be selected ( enabled / disabled)
    std::atomic<float>* Ionian;
    std::atomic<float>* Dorian;
    std::atomic<float>* Phrygian;
    std::atomic<float>* Lydian; 
    std::atomic<float>* Mixolydian;
    std::atomic<float>* Aeolian;
    std::atomic<float>* Locrian;
    std::vector<int> modeOn;
    int modeCount = 7;
    
    // lfo to panning channels
    SineOsc leftPan;
    SineOsc rightPan;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MakeSoundAudioProcessor)

};
