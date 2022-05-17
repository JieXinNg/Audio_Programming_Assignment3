/*
  ==============================================================================

    pulseSynth.h
    Created: 2 May 2022 5:00:37pm
    Author:  s1859154

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Oscillator.h"
#include <math.h>
#include "KeySignatures.h"
#include "FMSynth.h"

// ===========================
// ===========================
// SOUND
class pulseSynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int noteIn) override
    {
        if (noteIn > 47) // change value here
            return true;
        else
            return false;
    }
    //--------------------------------------------------------------------------
    bool appliesToChannel(int) override { return true; }
};

class pulseSynthVoice : public juce::SynthesiserVoice
{
public:
    pulseSynthVoice() {}

    /**
    * set sample rate and envelop
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
    * set mode / key, must be called before init
    */
    void setMode(std::atomic<float>* mode)
    {
        _mode = mode;
    }

    void setMode2(int _mode2)
    {
        mode2 = _mode2;
    }

    /**
    * set volume
    */
    void setVolumePointer(std::atomic<float>* volumeInput)
    {
        volume = volumeInput;
    }

    /**
*
*/
    void setPulseSpeed(std::atomic<float>* _pulseSpeed)
    {
        pulseSpeed = _pulseSpeed;

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
        float numOctaves = ceil(velocity * 3) + 1;
        
        // set freqeuncies 
        
        key.generateNotesForModes(numOctaves);
        key.changeMode(baseNote, mode2, numOctaves);  // baseNote should be which note? //*_mode, mode2 // this can be called in dsp loop if we want it to change instantly
        //DBG(mode2);
        float lfoFrequency = velocity / 10; 
        key.setLfofreq(lfoFrequency);

        setADSRValues(velocity);
    }

    void setADSRValues(float velocity)
    {
        
        float envelopeRelease = exp (velocity * 6.0f - 2.0f); // scaled value (range e^-2 to e^3)
        float sustainParameter;

        if (velocity > 0.8f)
        {
            sustainParameter = juce::jmap(random.nextFloat(), 0.01f, 0.15f);
        }

        if (velocity < 0.3f)
        {
            sustainParameter = juce::jmap(random.nextFloat(), 0.75f, 1.0f);
        }
        if (velocity >= 0.3f && velocity <= 0.8f)
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
            smoothVolume.setTargetValue(*volume); // smooth value
            float gainVal = smoothVolume.getNextValue();

            // DSP loop (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
            {
                float envVal = env.getNextSample(); // get envelop value
                key.setPulseSpeed(pulseSpeedChange); // change the pulse speed  //*pulseSpeed
                key.changeFreq(); // change freq every one second

                float currentSample = key.randomNoteGenerator() * envVal;

                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan < outputBuffer.getNumChannels(); chan++)
                {
                    // The output sample is scaled by 0.2 so that it is not too loud by default
                    outputBuffer.addSample(chan, sampleIndex, currentSample * gainVal);
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
    void pitchWheelMoved(int) override 
    {}
    //--------------------------------------------------------------------------
    void controllerMoved(int amount1, int amount2) override 
    {
        pulseSpeedChange = amount2 / 127.0 * 2.9;
    }

    //--------------------------------------------------------------------------
    /**
     Can this voice play a sound. I wouldn't worry about this for the time being

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

    std::atomic<float>* releaseParam;

    // Oscillators
    SineOsc sineOsc;
    SquareOsc sqOsc;

    std::atomic<float>* volume;              // volume parameter
    juce::SmoothedValue<float> smoothVolume; // smooth value

    std::atomic<float>* _mode; // not needed

    // used to set the key of sequencer 
    KeySignatures key;
    int baseNote;
    int numOctaves;

    // pulse speed, sine pulse freq, sine power
    std::atomic<float>* pulseSpeed;     // not needed
    float pulseSpeedChange = 0.5;

    juce::Random random;            // random is called to select the notes to be played
    int mode2 = 0;  // set default value

};
