/*
  ==============================================================================

    OscillatorContainerChorus.h
    Contains class OscillatorContainerSine
    Contains class OscillatorContainerPhaseSine

    generates vectors to contain the oscillators

    Requires <vector> library for vectors
    Requires "Oscillator.h" for oscillators

  ==============================================================================
*/

#pragma once
#include <vector>
#include "Oscillator.h"

/**
* generates vectors to contain SineOsc (sine oscillators)
* 
* @param sampleRate (float) sample rate
* @param _frequencies (std::vector<float>) frequencies of oscillators
* @param _frequenciesMods[] (float) frequency for modulation
* @param _modulationDepths[] (float) depth for modulation
* @param _oscCount (int) number of oscillators
* @param _sinePowers (std::vector<float>)
* @param _number (int) the number of the oscillator in the vector (container)
* @output output(int _number) the phase of the oscillator (float)
*/
class OscillatorContainerSine
{
public:

    /**
    * set sampleRate - must be called first
    * 
    * @param sampleRate (float) sample rate
    * @param _oscCount (int) number of oscillators
    */
    void setSampleRate(float sampleRate, int _oscCount)
    {
        for (int i = 0; i < _oscCount; i++)
        {
            SineOsc sineOsc;
            sineOsc.setSampleRate(sampleRate);
            container.push_back(sineOsc);
        }
    } 

    /**
    * set frequencies - called after setSampleRate()
    *
    * @param _frequencies (std::vector<float>) frequencies of oscillators
    * @param _oscCount (int) number of oscillators
    */
    void setFrequencies(std::vector<float> _frequencies, int _oscCount)
    {
        for (int i = 0; i < _oscCount; i++)
        {
            container[i].setFrequency(_frequencies[i]);
        }
    }

    /**
    * set parameters for frequency modulations
    * 
    * @param _frequenciesMods[] (float) frequency for modulation
    * @param _modulationDepths[] (float) depth for modulation
    * @param _oscCount (int) number of oscillators
    */
    void setFrequencyModutions(float _frequenciesMods[], float _modulationDepths[], int _oscCount)
    {
        for (int i = 0; i < _oscCount; i++)
        {
            container[i].setFreqModulationParams(_frequenciesMods[i], _modulationDepths[i]);
        }
    }

    /**
    * sets the powers for sine waves
    * 
    * @param _sinePowers (std::vector<float>)
    */
    void setSinePowers(std::vector<float> _sinePowers)
    {
        for (int i = 0; i < container.size(); i++)
        {
            container[i].setPower(_sinePowers[i]);
        }
    }

    /**
    * outputs the phase of the oscillator chosen
    * 
    * @param _number (int) the number of the oscillator in container
    * @output the phase of the oscillator (float)
    */
    float output(int _number)
    {
        return container[_number].process();
    }

private:
    std::vector<SineOsc> container; // vector to contain the oscillators
};

/**
* generates vectors to contain PhaseModulationSineOsc (sine oscillators with phase modulation)
*
* @param sampleRate (float) sample rate
* @param _frequencies (std::vector<float>) frequencies of oscillators
* @param _frequenciesMods[] (float) frequency for modulation
* @param _modulationDepths[] (float) depth for modulation
* @param _oscCount (int) number of oscillators
* @param _frequencies[] (float) frequency for phase modulation
* @param durationInSeconds (int) duration for one cycle of phase modulation
* @param _number (int) the number of the oscillator in the vector (container)
* @output output(int _number) the phase of the oscillator (float)
*/
class OscillatorContainerPhaseSine
{
public:

    /**
    * set sampleRate - must be called first
    *
    * @param sampleRate (float) sample rate
    * @param _oscCount (int) number of oscillators
    */
    void setSampleRate(float sampleRate, int _oscCount)
    {
        for (int i = 0; i < _oscCount; i++)
        {
            PhaseModulationSineOsc sineOsc;
            sineOsc.setSampleRate(sampleRate);
            container.push_back(sineOsc);
        }
    }

    /**
    * set frequencies - called after setSampleRate()
    *
    * @param _frequencies (std::vector<float>) frequencies of oscillators
    * @param _oscCount (int) number of oscillators
    */
    void setFrequencies(std::vector<float> _frequencies, int _oscCount)
    {
        for (int i = 0; i < _oscCount; i++)
        {
            container[i].setFrequency(_frequencies[i]);
        }
    }

    /**
    * set parameters for frequency modulations
    *
    * @param _frequenciesMods[] (float) frequency for modulation
    * @param _modulationDepths[] (float) depth for modulation
    * @param _oscCount (int) number of oscillators
    */
    void setFrequencyModutions(float _frequenciesMods[], float _modulationDepths[], int _oscCount)
    {
        for (int i = 0; i < _oscCount; i++)
        {
            container[i].setFreqModulationParams(_frequenciesMods[i], _modulationDepths[i]);
        }
    }

    /**
    * set parameters for frequency modulations
    *
    * @param _frequencies[] (float) frequency for phase modulation
    * @param sampleRate (float) sample rate
    * @param durationInSeconds (int) duration for one cycle of phase modulation
    * _oscCount (int) number of oscillators
    */
    void setPhaseModulationParams(float sampleRate, float _frequencies[], int durationInSeconds[], int _oscCount)
    {
        for (int i = 0; i < _oscCount; i++)
        {
            container[i].setRampParams(sampleRate, _frequencies[i], durationInSeconds[i]);
        }
 
    }

    /**
    * outputs the phase of the oscillator chosen
    *
    * @param _number (int) the number of the oscillator in container
    * @output the phase of the oscillator (float)
    */
    float output(int _number)
    {
        return container[_number].process();
    }

private:
    std::vector<PhaseModulationSineOsc> container; // vector to contain the oscillators

};






