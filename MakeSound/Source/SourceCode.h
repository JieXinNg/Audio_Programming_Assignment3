/*
  ==============================================================================

    SourceCode.h
    Created: 13 May 2022 
    Author:  s1859154

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Oscillator.h"
#include <math.h>

// ===========================
// ===========================
// SOUND
class SecondSynth : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int noteIn) override
    {
        if (noteIn <= 45) // change value here
            return true;
        else
            return false;
    }
    //--------------------------------------------------------------------------
    bool appliesToChannel(int) override { return true; }
};

class SecondSynthVoice : public juce::SynthesiserVoice
{
public:
    SecondSynthVoice() {}
    void init(float sampleRate)
    {
        // set sample rate for oscillators and envelop
        osc.setSampleRate(sampleRate);
        detuneOsc.setSampleRate(sampleRate);
        env.setSampleRate(sampleRate);

        // envelopes
        juce::ADSR::Parameters envParams;// create insatnce of ADSR envelop
        envParams.attack = 5.0f; // fade in
        envParams.decay = 3.0f;  // fade down to sustain level
        envParams.sustain = 3.0f; // vol level
        envParams.release = 5.0f; // fade out
        env.setParameters(envParams); // set the envelop parameters

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
        float vel = (float)velocity * 20.0;
        velocityDetune = (float)exp(0.2 * vel) / (float)exp(4.0) * 20.0;
        float envelopeRelease = velocity * 12.0f;

        playing = true;
        ending = false;

        freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber) + 24;
        //osc.setFrequency(freq); // set freqeuncies 
        env.reset(); 
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
            //detuneOsc.setFrequency(freq - velocityDetune); // velocityDetune *detuneAmount

            // DSP loop (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
            {
                float envVal = env.getNextSample();

                osc.setFrequency(envVal * freq);

                float currentSample = (osc.process()) * envVal; // apply envelop to oscillator 

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
        return dynamic_cast<SecondSynth*> (sound) != nullptr;
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
