/*
  ==============================================================================

    ModulatingFilter.h

    Contains class ModulatingFilter

    sets up a cutoff filter which modulates

    Requires "Oscillator.h" to generate lfo

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>     // for using std::string
#include "Oscillator.h"

/**
* sets up a cutoff filter which modulates
*
* @param sampleRate (float) sample of lfo
* @param lfoFreq (float) frequency of lfo
* @param _cutoffMode (std::string) "high" or "low", if some other string is passed through the function, the original audio will be returned
* @param _minVal (float)
* @param _maxVal (float)
* @param sample (float) audio input to be filtered (cut off)
* @return process() (float)
*/
class ModulatingFilter
{
public:

    /**
    * set the frequency, sample rate and the cutoff mode ("high" or "low" pass) of the lfo to be used as the cutoff for the filter
    *
    * @param sampleRate (float) sample of lfo
    * @param lfoFreq (float) frequency of lfo
    */
    void setParams(float _sampleRate, float lfoFreq)
    {
        sampleRate = _sampleRate;
        lfo.setSampleRate(_sampleRate);
        lfo.setFrequency(lfoFreq);
        lfo.setPower(7);
    }

    /**
    * set the filter type, min cutoff, max cutoff
    *
    * @param _cutoffMode (float) 0 - low-pass, 1 - high-pass, 2 - band-pass
    * @param _minVal (float)
    * @param _maxVal (float)
    */
    void setFilter(float _cutoffMode, float _minVal, float _maxVal)
    {
        cutoffMode = filterType[(int) _cutoffMode];
        minVal = _minVal;
        maxVal = _maxVal;
    }

    /**
    * take in audio as input and return the filter audio
    *
    * @param sample (float) audio input to be filtered (cut off)
    */
    float process(float sample)
    {
        // set value1 and value2 to accommodate the minVal and maxVal of the cutoff
        float value1 = (maxVal - minVal) / 2;
        float value2 = (maxVal + minVal) / 2;

        // lfo is used to scale  the cutoff frequency to between 100f and 1100f
        float cutoff = lfo.process() * value1 + value2;

        if (cutoffMode == "Low-pass")
        {
            filter.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, cutoff, resonance));
            return filter.processSingleSampleRaw(sample);
        }

        if (cutoffMode == "High-pass")
        {
            filter.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, cutoff, resonance));
            return filter.processSingleSampleRaw(sample);
        }

        if (cutoffMode == "Band-pass")
        {
            filter.setCoefficients(juce::IIRCoefficients::makeBandPass(sampleRate, cutoff, resonance));
            return filter.processSingleSampleRaw(sample);
        }

        if (cutoffMode == "None") // return original audio
        {
            return sample;
        }
    }

private:
    juce::IIRFilter filter;  // filter used for cut off
    float resonance = 5.0f;  // default value for resonance = 5

    SineOsc lfo;             // generate lfo to modulate cutoff
    float sampleRate;        // local reference to the sample rate

    std::string cutoffMode;  // variable to select cut off mode
    std::string filterType[4] = { "Low-pass", "High-pass", "Band-pass", "None" };

    float minVal;            // min value of cut off
    float maxVal;            // max value of cut off
};