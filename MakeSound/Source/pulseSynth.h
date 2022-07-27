/*
  ==============================================================================

    pulseSynth.h

    Contains classes pulseSynthSound, pulseSynthVoice

    Inherits from synthesiser class, this is a synthesiser for producing pulse-like sounds

    Requires <JuceHeader.h> 
    Requires "Oscillator.h" to generate oscillators 
    Requires "KeySignatures.h" to set the key of the played notes

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Oscillator.h"
#include "KeySignatures.h"

// ===========================
// ===========================
// SOUND
class pulseSynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int noteIn) override
    {
        if (noteIn > 47) // C3 and above
            return true;
        else
            return false;
    }
    //--------------------------------------------------------------------------
    bool appliesToChannel(int) override { return true; }
};


/**
* a synthesiser voice class which outputs randomNotegenerator() from keySig.h
* inherits from juce::SynthesiserVoice
*
* @param sampleRate (float) sample rate
* @param volumeInput (std::atomic<float>) pointer to volume
* @param instensity (float) set ADSR value
* @param _selectedMode (setModeLimit(std::vector<int>) vector of modes (the number of each mode)
* @output getMode() outputs the mode (int) ( this is set whenever a key is pressed )
*/
class pulseSynthVoice : public juce::SynthesiserVoice
{
public:
    pulseSynthVoice() {}

    /**
    * set sample rate 
    * 
    * @param sampleRate (float) 
    */
    void init(float sampleRate)
    {
        // set sample rate for oscillators and envelop
        env.setSampleRate(sampleRate);
        key.setOscillatorParams(sampleRate);

        // smooth value setting for volume
        smoothVolume.reset(sampleRate, 1.0f);
        smoothVolume.setCurrentAndTargetValue(0.0);
    }

    /**
    * set pointer to volume parameter
    *
    * @param volumeInput pointer to volume input
    */
    void setVolumePointer(std::atomic<float>* volumeInput)
    {
        volume = volumeInput;
    }

    /*
    * set the mode ( the base note is not set, user is free to play in other keys)
    *
    * @param _mode (int) mode number, e.g. 0 = ionian
    */
    void setMode(int _mode)
    {
        mode = _mode;
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
        playing = true;
        ending = false;

        // get local reference of the base note
        baseNote = midiNoteNumber;
        float numOctaves = ceil(velocity * 3) + 1; // set the number of octaves according the velocity
        
        // set freqeuncies 
        key.generateNotesForModes(numOctaves);
        key.changeMode(baseNote, mode, numOctaves);  // the mode is updated whenever a note starts for FMSynth.h
        
        // set lfo frequency accroding to velocity
        float lfoFrequency = velocity / 10; // scaled to between 0 and 0.1
        key.setLfofreq(lfoFrequency);

        setADSRValues(velocity); // set ADSR values
    }

    /**
    * set ADSR values, this is set whenever a note starts
    * 
    * @param instensity (float) 
    */
    void setADSRValues(float instensity)
    {
        
        float envelopeRelease = exp (instensity * 6.0f - 2.0f); // scaled value (range e^-2 to e^3)
        float sustainParameter;

        if (instensity > 0.8f) // sustain value is low if internsity is high
        {
            sustainParameter = juce::jmap(random.nextFloat(), 0.01f, 0.15f);
        }

        if (instensity < 0.3f) // sustain value is high if internsity is low
        {
            sustainParameter = juce::jmap(random.nextFloat(), 0.75f, 1.0f);
        }
        if (instensity >= 0.3f && instensity <= 0.8f) // sustain value is in the mid range if intensity is in the mid range
        {
            sustainParameter = juce::jmap(random.nextFloat(), 0.25f, 0.9f);
        }

        // envelopes
        juce::ADSR::Parameters envParams;       // create insatnce of ADSR envelop
        envParams.attack = 0.1f;                // fade in
        envParams.decay = 0.15f;                // fade down to sustain level
        envParams.sustain = sustainParameter;    // vol level
        envParams.release = envelopeRelease;    // fade out 
        env.setParameters(envParams);           // set the envelop parameters
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
            // smooth value for volume
            smoothVolume.setTargetValue(*volume); 
            float gainVal = smoothVolume.getNextValue();

            // DSP loop (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
            {
                float envVal = env.getNextSample(); // get envelop value
                key.setPulseSpeed(pulseSpeedChange); // change the pulse speed  
                key.changeFreq();                   // change freq every one second

                // output
                float currentSample = key.randomNoteGenerator() * envVal;

                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan < outputBuffer.getNumChannels(); chan++)
                {
                    // The output sample is scaled by 0.2 so that it is not too loud by default
                    outputBuffer.addSample(chan, sampleIndex, currentSample * gainVal);
                }

                if (ending) // if it is entering the ending phase
                { 
                    if (envVal < 0.0001) // turn off the sound envelope < 0.0001
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

    /**
    * change the pulse speed by controlling the mod wheel
    * this is only enabled when playing = true
    * 
    * @param amount1 (int) not used
    * @param amount1 (int) the mod wheel value
    */
    void controllerMoved(int amount1, int amount2) override 
    {
        pulseSpeedChange = amount2 / 127.0 * 2.9;
    }

    //--------------------------------------------------------------------------
    /**
     Can this voice play a sound.

     @param sound a juce::SynthesiserSound* base class pointer
     @return sound cast as a pointer to an instance of pulseSynthSound
     */
    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<pulseSynthSound*> (sound) != nullptr;
    }

    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    bool playing = false;       // set default value for playing to be false
    bool ending = false;        // bool to determine the moment the note is released
    juce::ADSR env;             // envelope for synthesiser

    std::atomic<float>* volume;              // volume parameter
    juce::SmoothedValue<float> smoothVolume; // smooth value

    // used to set the key of sequencer 
    KeySignatures key;
    int baseNote;
    int numOctaves;
    int mode = 0;                   // set default value of the mode ( ionian )

    // pulse speed default value
    float pulseSpeedChange = 0.5;

    juce::Random random;            // random is called to select the notes to be played

};
