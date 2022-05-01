/*
  ==============================================================================

    TMSampler.h
    Created: 1 May 2022 12:59:00am
    Author:  s1859154

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class TMSampler : public juce::Synthesiser
{
public:
    void init()
    {
        // allows us to use WAv and AIFF files
        formatManager.registerBasicFormats();

        // load audio file
        juce::File* file = new juce::File("C:/Users/s1859154/Documents/GitHub/composition.shakeTiming.wav");
        std::unique_ptr<juce::AudioFormatReader> reader;
        reader.reset( formatManager.createReaderFor(*file) );

        juce::BigInteger allNotes;
        allNotes.setRange(0, 120, true);
        addSound(new juce::SamplerSound("default", *reader, allNotes, 60, 0, 0.1, 2.0));
;    }

private:
    juce::AudioFormatManager formatManager;

};