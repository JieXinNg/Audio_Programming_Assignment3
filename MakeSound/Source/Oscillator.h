//  Oscillators.h


#ifndef Oscillators_h
#define Oscillators_h
#include <cmath>

/**
Oscillator class with three wave shapes (processSine, processSquare, processTriangle)
 */
class Oscillator
{
public:

    // Our parent oscillator class does the key things required for most oscillators:
    // -- handles phase
    // -- handles setters and getters for frequency and samplerate

    /**
     update the phase and output as a sinusoidal signal
     */
    float processSine()
    {
        phase += phaseDelta;

        if (phase > 1.0f)
            phase -= 1.0f;

        return sin(phase * 2 * 3.141592653589793);
    }

    /**
     update the phase and output as a square wave signal
     */
    float processSquare()
    {
        phase += phaseDelta;

        if (phase > 1.0f)
            phase -= 1.0f;

        float outVal = 0.5;
        if (phase > 0.5)
            outVal = -0.5;

        return outVal;
    }

    /**
     update the phase and output as a triangular signal
     */
    float processTriangle()
    {
        phase += phaseDelta;

        if (phase > 1.0f)
            phase -= 1.0f;

        return fabs(phase - 0.5) - 0.25;
    }

    /**
     set the sample rate - needs to be called before setting frequency or using process

     @param SR samplerate in Hz
     */
    void setSampleRate(float SR)
    {
        sampleRate = SR;
    }

    /**
     set the oscillator frequency - MAKE SURE YOU HAVE SET THE SAMPLE RATE FIRST

     @param freq oscillator frequency in Hz
     */
    void setFrequency(float freq)
    {
        frequency = freq;
        phaseDelta = frequency / sampleRate;
    }

private:
    float frequency;
    float sampleRate;
    float phase = 0.0f;
    float phaseDelta;
};
//==========================================



#endif /* Oscillators_h */