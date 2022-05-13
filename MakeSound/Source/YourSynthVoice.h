/*
  ==============================================================================

    YourSynthVoice.h
    Created: 7 Mar 2020 4:27:57pm

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Oscillator.h"
#include <math.h>

// ===========================
// ===========================
// SOUND
class MySynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int noteIn) override 
    { 
        if (noteIn <= 60) // change value here
            return true;
        else
            return false;
    }
    //--------------------------------------------------------------------------
    bool appliesToChannel(int) override { return true; }
};

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
        envParams.release = 0.2f; // fade out
        env.setParameters(envParams); // set the envelop parameters
        
        setReverbParams();
    }

    /**
    * set detune amount
    */
    void setDetunePointer(std::atomic<float>* detuneInput)
    {
        detuneAmount = detuneInput;
    }

    /**
    * set volume
    */
    void setVolumePointer(std::atomic<float>* volumeInput)
    {
        volume = volumeInput;
    }

    void setReverbParams()
    {
        reverbParams.dryLevel = 0.8f;
        reverbParams.wetLevel = 0.3f;
        reverbParams.roomSize = 0.99f;
        reverb.setParameters(reverbParams);
        //reverb.reset();
    }

    //--------------------------------------------------------------------------
    /**
     What should be done when a note starts

     @param midiNoteNumber
     @param velocity
     @param SynthesiserSound unused variable
     @param / unused variable
     */
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        float vel = (float) velocity * 20.0;
        velocityDetune = (float) exp(0.2 * vel) / (float) exp(4.0) * 20.0;
        DBG(velocityDetune);
        playing = true;
        freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        // set freqeuncies 
        osc.setFrequency(freq);

        DBG(1 / (float) pow((vel + 1), 1));

        // create different envelops based on the velocity of the note
        juce::ADSR::Parameters envParams;// create insatnce of ADSR envelop
        envParams.attack = 0.2f; // fade in
        envParams.decay = 0.25f;  // fade down to sustain level
        envParams.sustain = 0.5f; // vol level
        envParams.release = 1 / (float) pow((vel + 1), 1); // fade out
        env.setParameters(envParams); // set the envelop parameters

        env.reset(); // can delete this if we dont want it to reset
        env.noteOn();
    }
    //--------------------------------------------------------------------------
    /// Called when a MIDI noteOff message is received
    /**
     What should be done when a note stops

     @param / unused variable
     @param allowTailOff bool to decie if the should be any volume decay
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
            //// if we modulate the detune amount with an lfo, we need to put this inside the dsp loop
            detuneOsc.setFrequency(freq - velocityDetune); // velocityDetune *detuneAmount

            // DSP loop (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
            {
                float envVal = env.getNextSample();

                float currentSample = (osc.process() + detuneOsc.process()) * envVal; // apply envelop to oscillator 

                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan < outputBuffer.getNumChannels(); chan++)
                {                    
                    outputBuffer.addSample(chan, sampleIndex, *volume * currentSample);
                    //reverb.processMono(outputBuffer.getWritePointer(chan), currentSample); // dont hear a difference
                }

                // add reverb, does not work
                 reverb.processStereo(outputBuffer.getWritePointer(0), outputBuffer.getWritePointer(1), currentSample);

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
    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<MySynthSound*> (sound) != nullptr;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    bool playing = false; // set default value for playing to be false
    bool ending = false; // bool to determine the moment the note is released
    juce::ADSR env; // envelope for synthesiser

    std::atomic<float>* releaseParam;

    // Oscillators
    TriOsc osc;
    TriOsc detuneOsc;

    std::atomic<float>* detuneAmount;
    std::atomic<float>* volume;
    float freq;
    float velocityDetune;

    // effects
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

};
