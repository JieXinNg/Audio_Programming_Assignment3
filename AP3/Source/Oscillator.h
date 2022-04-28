/*
  ==============================================================================

    Oscillator.h

    Contains class Oscillator and its subclasses : 
    SineOsc, TriOsc, LinearIncrease, PhaseModulationSineOsc, SquareOsc

    All the classes generates an oscillator with a different type of wave

    Requires <cmath> library for sin() function
    Requires <math.h> library for M_PI (value of pi)

  ==============================================================================
*/


#ifndef Oscillators_h
#define Oscillators_h
#include <cmath>
#define _USE_MATH_DEFINES     // for M_PI
#include <math.h>             // for M_PI

/**
* Oscillator class : generates phasor
* 
* @param _sampleRate (float) sample rate in Hz
* @param _frequency (float) frequency in Hz
* @return process() (float) output of the phase
*/
class Oscillator
{
public:
    
    /**
     * update the phase and output signal 
     *
     * @return phaseOutput(phase) the phase output
     */
    virtual float process() 
    {
        phase += phaseDelta;

        if (phase > 1.0f)
            phase -= 1.0f;

        return phaseOutput(phase);
    }

    /**
    * virtual function to be manipulated in subclasses
    */
    virtual float phaseOutput(float p) 
    {
        return p;    
    }
    
    /**
     * set the sample rate - needs to be called before setting frequency or using process
     * 
     * @param _sampleRate (float) samplerate in Hz
     */
    void setSampleRate(float _sampleRate)
    {
        sampleRate = _sampleRate;
    }
    
    /**
     * set the oscillator frequency - The sample rate has to be set first
     *
     * @param _frequency (float) oscillator frequency in Hz
     */
    void setFrequency(float _frequency)
    {
        frequency = _frequency;
        phaseDelta = frequency / sampleRate;
    }

    /**
    * function to output a sinusoidal wave to be used for subclasses for modulation 
    */
    float sinModulation()
    {
        phase += phaseDelta;

        if (phase > 1.0f)
            phase -= 1.0f;

        return sin(phase * 2 * M_PI);
    }

    /**
    * returns the phase delta (float)
    */
    float getPhaseDelta()
    {
        return phaseDelta;
    }

protected:
    float frequency;
    float sampleRate;
    float phase = 0.0f;
    float phaseDelta;
};

/**
* SineOsc class : generates sine oscillator
* Inherits from Oscillator class
*
* @param _sampleRate (float) sample rate in Hz
* @param _frequency (float) frequency in Hz
* @param _modulationRate (float) the rate of the frequency modulation 
* @param _freqModulationDepth (float) the depth of the frequency modulation
* @param _sinPower (int) the power of the sine wave
* @return process() (float) output of the sine phase
*/
class SineOsc : public Oscillator
{
public:

    /**
    * output the phase as a sinusoidal wave with possible pase modulation, the power of the sine wave is shaped with variable sinPower
    */
    float phaseOutput(float p) override 
    {
        return pow(sin(p * 2 * M_PI), sinPower);
    }


    /**
    * set the depth and frequency of the modulation 
    * 
    * @param _modulationRate frequency of modulation
    * @param _modulationDepth depth of modulation
    */
    void setFreqModulationParams(float _modulationRate, float _freqModulationDepth)
    {
        freqModulationDepth = _freqModulationDepth;
        modulatingOsc.setSampleRate(sampleRate);
        modulatingOsc.setFrequency(_modulationRate);
    }

    /**
    * output the value taken from phaseOutput by inputting the phase, manipulate the wave with modulations
    * 
    * @return phaseOutput(phase) processed phase
    */
    float process() override
    {
        modulation = freqModulationDepth * modulatingOsc.sinModulation();

        phaseDelta = (frequency + modulation) / sampleRate;

        phase += phaseDelta;

        if (phase > 1.0f)
            phase -= 1.0f;

        return phaseOutput(phase);

    }

    /**
    * set the power of the sine wave
    * 
    * @param _sinPower integer value for the power
    */
    void setPower(int _sinPower)
    {
        sinPower = _sinPower;
    }

private:
    // variables for frequency modulation, default values = 0
    float freqModulationDepth = 0;
    float modulation = 0;
    // variable for the power of the sine wave, default value = 1
    int sinPower = 1;
    // oscillator used to modulate frequency
    Oscillator modulatingOsc;
};

/**
* TriOsc class : generates triangle wave oscillator
* Inherits from Oscillator class
*
* @param _sampleRate (float) sample rate in Hz
* @param _frequency (float) frequency in Hz
* @return process() (float) output of the triangle wave phase
*/
class TriOsc : public Oscillator
{
    /**
    * output triangular wave
    */
    float phaseOutput(float p) override
    {
        return fabs(p - 0.5) - 0.25;
    }

};

/**
* LinearIncrease class : generates phasor oscillator 
* resets in a custom set time 
* Inherits from Oscillator class
*
* @param _sampleRate (float) sample rate in Hz
* @param _frequency (float) frequency in Hz
* @param durationInSeconds (int) duration to reset the phase in seconds
* @return process() (float) output of the phasor phase
*/
class LinearIncrease : public Oscillator
{
public:

    /**
    * resets the phase to zero every durationInSeconds
    * 
    * @param durationInSeconds (int) the duration to reset the phase in seconds
    */
    float process(int durationInSeconds)
    {
        phase += 1;
        
        if (phase == (sampleRate * durationInSeconds)) // duration in samples
        {
            phase = 0;
        }

        return phaseOutput(phase);
    }
};


/**
* PhaseModulationSineOsc class : generates phase modulate sine oscillator
* Inherits from Oscillator class
*
* @param _sampleRate (float) sample rate in Hz
* @param _frequency (float) frequency in Hz
* @param _durationInSeconds (int) duration to reset the phase in seconds
* @return process() (float) output of the phase modulated sine phase
*/
class PhaseModulationSineOsc : public Oscillator
{
public:
    float phaseOutput(float p) override
    {
        phaseModulate();
        return (sin(finalModulation + (p * 2 * M_PI)));
    }

    /**
    * sets the parameters for the oscillator used to modulate the phase
    * these parameters must be set in order to use this class
    * 
    * @param _sampleRate (float) sample rate in Hz
    * @param _frequency (float) frequency in Hz
    * @param _durationInSeconds (int) duration to reset the phase in seconds
    */
    void setRampParams(float _sampleRate, float _frequency, int _durationInSeconds)
    {
        linearIncrease.setSampleRate(_sampleRate);
        linearIncrease.setFrequency(_frequency);
        rampMod.setSampleRate(_sampleRate);
        rampMod.setFrequency(_frequency);
        durationInSeconds = _durationInSeconds;
    }

    /**
    * modulates the output, this function is called in phaseOutput() 
    */
    void phaseModulate()
    {
        // linIncrease - variable that increases in value linearly
        float linIncrease = linearIncrease.process(durationInSeconds * sampleRate) / (float) (durationInSeconds * sampleRate); 

        // cycle - scale linIncrease into range -1 to 1
        float cycle = sin(linIncrease * M_PI);

        // modulationIndex - sinusoidal modulation index which increases in amplitude every cycle 
        float modulationIndex = linIncrease * 10 * cycle;

        //output of modulating oscillator
        finalModulation = modulationIndex * sin(rampMod.process() * 2 * M_PI);
    }

      /**
      * set the depth and frequency of the modulation
      *
      * @param _modulationRate frequency of modulation
      * @param _modulationDepth depth of modulation
      */
    void setFreqModulationParams(float _modulationRate, float _freqModulationDepth)
    {
        freqModulationDepth = _freqModulationDepth;
        modulatingOsc.setSampleRate(sampleRate);
        modulatingOsc.setFrequency(_modulationRate);
    }

private:

    // variables for phase modulation
    float finalModulation;
    int durationInSeconds;

    // variables for frequency modulation, default values = 0
    float freqModulationDepth = 0;
    float modulation = 0;

    // oscillators
    Oscillator modulatingOsc;       // oscillator for frequency modulation
    LinearIncrease linearIncrease; // phasor for phase modulation
    Oscillator rampMod;            // phasor for phase modulation
};

/**
* SquareOsc class : generates square wave oscillator
* Inherits from Oscillator class
*
* @param _sampleRate (float) sample rate in Hz
* @param _frequency (float) frequency in Hz
* @param _pulseWidth (float) the pulse width (default value = 0.5)
* @return process() (float) output of the phase modulated sine phase
*/
class SquareOsc : public Oscillator
{
public:

    /**
    * output square wave
    */
    float phaseOutput(float p) override
    {
        float outVal = 0.5;

        if (p > pulseWidth)
            outVal = -0.5;

        return outVal;
    }

    /**
    * set pulse width
    *
    * @param _pulseWidth set the pulse width
    */
    void setPulseWidth(float _pulseWidth)
    {
        pulseWidth = _pulseWidth;
    }

private:
    float pulseWidth = 0.5f; // default value
};

#endif /* Oscillators_h */


