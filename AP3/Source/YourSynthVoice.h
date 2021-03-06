/*
  ==============================================================================

    YourSynthVoice.h
    Created: 7 Mar 2020 4:27:57pm

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Oscillator.h"

/**
* YourSynthVoice class : generates sythesiser
*
* @param 
* @return 
*/
class MySynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote      (int) override      { return true; }
    //--------------------------------------------------------------------------
    bool appliesToChannel   (int) override      { return true; }
};


// =================================
// =================================
// Synthesiser Voice - your synth code goes in here

/*!
 @class MySynthVoice
 @abstract struct defining the DSP associated with a specific voice.
 @discussion multiple YourSynthVoice objects will be created by the Synthesiser so that it can be played polyphicially
 */
class MySynthVoice : public juce::SynthesiserVoice
{
public:
    MySynthVoice() {}

    void init(float sampleRate)
    {
        // set sample rate for oscillators and envelop
        osc.setSampleRate(sampleRate);
        detuneOsc.setSampleRate(sampleRate);
        env.setSampleRate(sampleRate);

        juce::ADSR::Parameters envParams;// create insatnce of ADSR envelop
        envParams.attack = 0.1f; // fade in
        envParams.decay = 0.25f;  // fade down to sustain level
        envParams.sustain = 0.5f; // vol level
        envParams.release = 1.0f; // fade out
        env.setParameters(envParams); // set the envelop parameters
    }

    void setDetunePointer(std::atomic<float>* detuneInput)
    {
        detuneAmount = detuneInput;
    }

    //--------------------------------------------------------------------------
    /**
     What should be done when a note starts

     @param midiNoteNumber
     @param velocity
     @param SynthesiserSound unused variable
     @param / unused variable
     */
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        playing = true;
        ending = false;

        freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

        // set freqeuncies 
        osc.setFrequency(freq);
        detuneOsc.setFrequency(freq - *detuneAmount);

        env.reset(); // can delete this if we dont want it to reset
        env.noteOn();
    }
    //--------------------------------------------------------------------------
    /// Called when a MIDI noteOff message is received
    /**
     What should be done when a note stops

     @param / unused variable
     @param allowTailOff bool to decide if the should be any volume decay
     */
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff) // allow slow release of note
        {
            env.noteOff();
            ending = true;
        }
        else // shut off note
        {
            clearCurrentNote();
            playing = false;
        }
    }
    
    //--------------------------------------------------------------------------
    /**
     The Main DSP Block: Put your DSP code in here
     
     If the sound that the voice is playing finishes during the course of this rendered block, it must call clearCurrentNote(), to tell the synthesiser that it has finished

     @param outputBuffer pointer to output
     @param startSample position of first sample in buffer
     @param numSamples number of smaples in output buffer
     */
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {

        if (playing) // check to see if this voice should be playing
        {
            // if we modulate the detune amount with an lfo, we need to put this inside the dsp loop
            detuneOsc.setFrequency(freq - *detuneAmount); 

            // DSP loop (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample;   sampleIndex < (startSample+numSamples);   sampleIndex++)
            {

                float envVal = env.getNextSample();

                float currentSample = (osc.process() + detuneOsc.process()) * 0.5f * envVal; // apply envelop to oscillator
                
                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan<outputBuffer.getNumChannels(); chan++)
                {
                    // The output sample is scaled by 0.2 so that it is not too loud by default
                    outputBuffer.addSample (chan, sampleIndex, currentSample * 0.5);
                }

                if (ending)
                {
                    if (envVal < 0.0001)
                    {
                        clearCurrentNote();
                        playing = false;
                    }
                }
            }
        }
    }
    //--------------------------------------------------------------------------
    void pitchWheelMoved(int) override {}
    //--------------------------------------------------------------------------
    void controllerMoved(int, int) override {}
    //--------------------------------------------------------------------------
    /**
     Can this voice play a sound. I wouldn't worry about this for the time being

     @param sound a juce::SynthesiserSound* base class pointer
     @return sound cast as a pointer to an instance of YourSynthSound
     */
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<MySynthVoice*> (sound) != nullptr;
    }

    void linkParameters(std::atomic<float>* ptrToParam)
    {
        releaseParam = ptrToParam;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    bool playing = false; // set default value for playing to be false
    bool ending = false; // bool to determine the moment the note is released
    int voiceCount = 8; // set the voice count of our synthesiser
    juce::ADSR env; // envelope for synthesiser

    juce::Random random; //a random object for use in our test noise function
    std::atomic<float>* releaseParam;

    // Oscillators
    SineOsc osc;
    TriOsc detuneOsc;

    std::atomic<float>* detuneAmount;
    float freq;
};
