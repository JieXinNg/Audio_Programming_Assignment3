/*
  ==============================================================================

    FMSynth.h

    Contains classes FMSynthSound, FMsynthVoice

    Inherits from synthesiser class, this is a synthesiser for producing sine oscillators with frequency modulations

    Requires <JuceHeader.h>
    Requires "OscillatorContainer.h" to generate oscillators (vectors of oscillators)
    Requires "ModulatingFilter.h" to filter the output of oscillators
    Requires "KeySignatures.h" to set the key of the chords
    Requires "Delay.h" for delays

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "OscillatorContainer.h"
#include "ModulatingFilter.h"
#include "KeySignatures.h"
#include "Delay.h"

// ===========================
// ===========================
// SOUND
class FMSynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int noteIn) override
    {
        if (noteIn > 35 && noteIn <= 47) // change value here noteIn > 35 && noteIn <= 47
            return true;
        else
            return false;
    }
    //--------------------------------------------------------------------------
    bool appliesToChannel(int) override { return true; }
};

class FMsynthVoice : public juce::SynthesiserVoice
{
public:
    FMsynthVoice() {}
    void init(float sampleRate)
    {
        sr = sampleRate; // local reference of the sample rate to be used in setModulationParameters()

        // set sample rate
        sineOscs.setSampleRate(sampleRate, 4); // the oscillator count can be changed here
        env.setSampleRate(sampleRate);
        modFilter.setParams(sampleRate, 0.05f);
        key.setOscillatorParams(sampleRate);
        key.generateNotesForModes(3); 
        delay.setSize(sampleRate);
        delay.setDelayTime(0.5 * sampleRate);
        setModulationParameters(sampleRate); 

        // smooth value setting for volume
        smoothVolume.reset(sampleRate, 1.0f);
        smoothVolume.setCurrentAndTargetValue(0.0);

        // ADSR envelope
        juce::ADSR::Parameters envParams;// create instance of ADSR envelop
        envParams.attack = 2.0f;         // fade in
        envParams.decay = 0.5f;         // fade down to sustain level
        envParams.sustain = 0.5f;       // vol level
        envParams.release = 4.0f;       // fade out
        env.setParameters(envParams);   // set the envelop parameters

    }

    /**
    * set volume, the input is from the interface (slider)
    * 
    * @param volumeInput (std::atomic<float>) pointer to volume
    */
    void setVolumePointer(std::atomic<float>* volumeInput)
    {
        volume = volumeInput;
    }


    /**
    * set filter parameters
    * 
    * @param _cutoffMode (0 - low-pass, 1 - high-pass, 2 - band-pass)
    * @param _minVal
    * @param _maxVal
    */
    void setModFilterParams(std::atomic<float>* _cutoffMode, std::atomic<float>* _minVal, std::atomic<float>* _maxVal)
    {
        cutoffMode = _cutoffMode;
        minVal = _minVal;
        maxVal = _maxVal;
    }

    /**
    * set oscillator modulation parameters - randomly selected from predefined values
    * 
    * @param _sampleRate
    */
    void setModulationParameters(float _sampleRate)
    {
        float sineOscsModFreq[4] = { 0.25, 0.5, 0.75, 1.0 };
        int sineOscsModDurations[4] = { 180, 240, 300, 360 };
        float freqs[4] = { 0.00166667f, 0.003333333f, 0.005f, 0.006666667f }; // 0.00166667 = 1 / 4 cycle / 2.5mins (1 / (10 * 60s) cycle/s)
        float depths[4] = { 20, 30, 50, 70 };

        float phaseModFreq = sineOscsModFreq[random.nextInt(4)];
        int phaseModDuration = sineOscsModDurations[random.nextInt(4)];     
        float modFreq[4] = { phaseModFreq, phaseModFreq, phaseModFreq,phaseModFreq };
        int modDurations[4] = { phaseModDuration, phaseModDuration, phaseModDuration, phaseModDuration };
        sineOscs.setPhaseModulationParams(sr, modFreq, modDurations, 4); // change the num of oscillators here

        float fmRate = freqs[random.nextInt(4)];
        float fmDepth = depths[random.nextInt(4)];
        float fmFreq[4] = { fmRate, fmRate, fmRate,fmRate };
        float modDepth[4] = { fmDepth, fmDepth, fmDepth, fmDepth };
        sineOscs.setFrequencyModutions(fmFreq, modDepth, 4); // change the num of oscillators here

    }

    /**
    * set frequencies of the oscillators - chosen notes from predefined chords
    * this is called whenever a key is pressed, in startNote()
    */
    void setFrequencies()
    {
        std::vector<float> variation1 = { key.getNotes(0), key.getNotes(6), key.getNotes(11), key.getNotes(16) }; // 1, 7, 5, 3
        std::vector<float> variation2 = { key.getNotes(14), key.getNotes(16), key.getNotes(18), key.getNotes(20) }; // 1, 3, 5, 7  
        std::vector<float> variation3 = { key.getNotes(7), key.getNotes(12), key.getNotes(16), key.getNotes(20) }; // 1, 5, 3, 7
        std::vector<float> variation4 = { key.getNotes(0), key.getNotes(4), key.getNotes(7), key.getNotes(9) }; // 1, 5, 1, 3
        std::vector<float> variation5 = { key.getNotes(7), key.getNotes(9), key.getNotes(11), key.getNotes(13) }; // 1, 3, 5, 7
        std::vector<float> variation6 = { key.getNotes(0), key.getNotes(4), key.getNotes(9), key.getNotes(13) }; // 1, 5, 3, 7
        std::vector<std::vector<float>> notes = { variation1, variation2, variation3, variation4, variation5, variation6 };
        int pickChord = random.nextInt(6);
        sineOscs.setFrequencies(notes[pickChord], 4);
    }


    /**
    * outputs the mode (int) ( this is set whenever a key is pressed ) 
    */
    int getMode()
    {
        return mode;
    }

    /**
    * outputs the baseNote ( this is set whenever a key is pressed ) 
    */
    int getBaseNote()
    {
        return baseNote;
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
        env.reset();
        env.noteOn();
        playing = true;
        ending = false;

        baseNote = midiNoteNumber - 12;
        mode = random.nextInt(7);
        key.changeMode(midiNoteNumber, mode, 3); 
        setFrequencies();               // set freqeuncies 
         
    }

    //--------------------------------------------------------------------------
    /**
     What should be done when a note stops ( Called when a MIDI noteOff message is received )

     @param / unused variable
     @param allowTailOff bool to decie if the should be any volume decay
     */
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff) // allow slow release of note
        {
            DBG("allow tail off");
            env.noteOff();
            ending = true;
        }
        else // shut off note
        {
            DBG("stop note else statement");
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
        float* left = outputBuffer.getWritePointer(0); // access the left channel
        float* right = outputBuffer.getWritePointer(1); // access the right channel

        if (playing) // check to see if this voice should be playing
        {

            // DSP loop (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
            {
                
                smoothVolume.setTargetValue(*volume); // smooth value
                float gainVal = smoothVolume.getNextValue();

                modFilter.setFilter(*cutoffMode, *minVal, *maxVal); // set filter values
                float envVal = env.getNextSample();
                float delayEnv = delay.process(envVal);

                // outputs of oscillators
                float totalOscs = (sineOscs.output(0) + sineOscs.output(1) + sineOscs.output(2) + sineOscs.output(3)) / 4;
                float delayOutput = delay.process(totalOscs) * 0.5;
                // applt filter to output
                float currentSample = modFilter.process(totalOscs * envVal + delayOutput * delayEnv) / 2;

                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan < outputBuffer.getNumChannels(); chan++)
                {
                    outputBuffer.addSample(chan, sampleIndex, gainVal * currentSample);
                }

                // if it is entering the ending phase
                if (ending)
                {
                    if (delayEnv < 0.0001 && envVal < 0.0001) // turn off sound when both envelopes are < 0.0001
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
     Can this voice play a sound.

     @param sound a juce::SynthesiserSound* base class pointer
     @return sound cast as a pointer to an instance of YourSynthSound
     */
    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<FMSynthSound*> (sound) != nullptr;
    }
    //--------------------------------------------------------------------------

private:
    //--------------------------------------------------------------------------
    bool playing = false;       // set default value for playing to be false
    bool ending = false;        // bool to determine the moment the note is released
    float sr;                   // sample rate

    // Oscillators
    OscillatorContainerPhaseSine sineOscs;
    juce::ADSR env;             // envelope for synthesiser

    // effects
    ModulatingFilter modFilter;
    // filter parameters
    std::atomic<float>* cutoffMode;
    std::atomic<float>* minVal;
    std::atomic<float>* maxVal;
    
    // volume parameter
    std::atomic<float>* volume;
    juce::SmoothedValue<float> smoothVolume; // smooth value

    // variables for setting chords
    KeySignatures key;
    float baseNote;
    float mode;

    Delay delay;            
    juce::Random random;    // to generate random values

};
