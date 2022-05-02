/*
  ==============================================================================

	KeySignatures.h
	Contains class KeySignatures

	Generates a range of notes (vector) based on key
	Generates a music sequencer which plays random notes based on key

	Requires <cmath> library for pow() function
	Requires <vector> to instantiate vectors
	Requires "Oscillator.h" to generate oscillators
	Requires <map> library for mapping the modes to the notes
	Requires "MidiFrequencyConverter.h" to convert midi to frequency

  ==============================================================================
*/

#pragma once
#include <cmath> // library for the function pow()
#include <vector> // library for creating vectors
#include "Oscillator.h" // library for generating oscillators
#include <map> // create map to map the modes to the values of the notes
#include <JuceHeader.h> // library to convert midi values to frequencies


/**
* KeySignatures class :
* generates a set of notes (vector) based on key
* generates music sequencer (oscillator) that plays random notes based on key
* 
* @param _baseNote (int) take in midi value to set as base note
* @paranm _sr (float) set the sample rate
* @param _mode (std::string) set the mode of the key "major" or "minor"
* @param numOctaves (int) set the number of octaves to generate the possible notes
* @param _pulseFreq frequency of pulse
* @param _pulsePower strength of pulse
* @param noteDegree degree of the note in the scale
* 
* @return randomNoteGenerator() (float) outputs the sequencer 
* @return getNotes(int noteDegree) (float) the frequency of the note selected
*/
class KeySignatures {
public:
	/**
	* generate the possible notes based on the key
	* 
	* @param _baseNote (int) take in midi value to set as base note
	* @paranm _sr (float) set the sample rate
	* @param _mode (std::string) set the mode of the key "major" or "minor"
	* @param numOctaves (int) set the number of octaves to generate the possible notes
	*/
	void setKey(int _baseNote, float _sr, std::string _mode, int numOctaves) 
	{
		// set the mode to major if some other string is passed
		if (_mode != "major" && _mode != "minor") 
		{
			_mode = "major";
		}
		
		float baseNote = _baseNote;           // set the base note 
		numNotes = 7 * numOctaves;            // set the number of possible notes in the range

		// set parameters for the oscillators
		sampleRate = _sr;
		sineOsc.setSampleRate(_sr);
		sineOsc.setRampParams(_sr, 0.03333, 240);
		sinePulse.setSampleRate(_sr);
		sinePulse.setFrequency(pulseFreq);
		sinePulse.setPower(pulsePower);
		phasor.setSampleRate(_sr);
		//phasor.setFrequency(0.5);
		//lfo.setSampleRate(_sr);
		//lfo.setFrequency(0.5);

		// created vectors which carries the values of semitones for each note from the base note 
		// further modes can be added here
		std::vector<int> major = { 0, 2, 4, 5, 7, 9, 11 };
		std::vector<int> minor = { 0, 2, 3, 5, 7, 8, 10 };

		if (numOctaves > 1) // add notes if there is more than one octave
		{
			// loop to generate all the possible notes
			for (int i = 1; i < (numOctaves + 1); i++)
			{
				for (int x = 0; x < 7; x++) // loop through each note in one octave
				{
					// append the vectors to cover the number of octaves (numOctaves)
					major.push_back(major[x] + 12 * i);
					minor.push_back(minor[x] + 12 * i);

				}
			}
		}

		// create dictionary to map "major" and "minor" to the correct notes
		// further modes can be mapped here rather than using if statements to check the input
		std::map<std::string, std::vector<int>> keyDictionary; 
		keyDictionary["major"] = major;
		keyDictionary["minor"] = minor;

		// loop to select the correct mode and generate a vector to hold all the notes
		for (int i = 0; i < numNotes; i++)
		{
			std::vector<int> a = keyDictionary.at(_mode);           // select the mode at keyDictionary
			float _note = juce::MidiMessage::getMidiNoteInHertz(baseNote + a[i]); // convert each note from midi to frequency
			notes.push_back(_note);                                 // add the frequency value to notes (vector)
		}

		sineOsc.setFrequency(getNotes(0));                          // set the default frequency
	}

	/**
	* 
	*/
	void setPulseSpeed(float phasorFreq)
	{
		phasor.setFrequency(phasorFreq);
	}

	/**
	* set the frequency and power of sine for sinePulse - has to be called before setKey
	* 
	* @param _pulseFreq frequency of pulse
	* @param _pulsePower strength of pulse
	*/
	void setSinePulseParams(float _pulseFreq, int _pulsePower)
	{
		pulseFreq = _pulseFreq;
		pulsePower = _pulsePower;
	}

	/**
	* random music sequencer - outputs the selected note (pulsed)
	*/
	float randomNoteGenerator()
	{
		float pulseVolume; // = sinePulse.process();
		if (phasor.process() <= 0.5f)
		{
			pulseVolume = sin(2.0 * 3.142 * phasor.process());
		}
		else
		{
			pulseVolume = 0;
		}

		float output = sineOsc.process();
		return output * pulseVolume;
	}

	/**
	* change the frequency of sineOsc every time this function is called
	* this function can be called in the DSP loop to update the frequency of the oscillator
	*/
	void changeFreq()
	{
		//phasor.setFrequency(lfo.process() * 0.5);

		if ((1 - phasor.process()) <= phasor.getPhaseDelta()) // change frequency when the phase is nearing 1 
		{
			int randomInteger = random.nextInt(numNotes - 1); // generate random integer
			int outVal = notes[randomInteger]; // select random note from the scale
			sineOsc.setFrequency(outVal); // set the frequency of sineOsc
		}
	}

	/**
	* outputs the frequency of selected note
	* 
	* @param noteDegree degree of the note in the scale
	* @return notes[noteDegree] the frequency of the note selected
	*/
	float getNotes(int noteDegree)
	{
		return notes[noteDegree];
	}

private:
	// variables to be set in setKey()
	int key;                        // midi value
	float sampleRate;
	std::string mode = "major";     
	int numNotes = 7;               // number of notes according to number of octaves set in setKey

	// variables to be set in setSinePulseParams
	float pulseFreq = 0.1;          // set the default frequency of sinPulse
	int pulsePower = 9;            // set the default power of the sine wave for sinePulse
	
	std::vector<float> notes;       // vector to contain the generated notes for the scale
	
	// oscillators
	PhaseModulationSineOsc sineOsc; // sine oscillator to generate audio
	SineOsc sinePulse;              // sine oscillator to modulate the volume to simulate pulse
	Oscillator phasor;              // phasor to check the time to change frequency
	//SineOsc lfo;					// lfo to modulate the phasor frequency // not working now

	juce::Random random;            // random is called to select the notes to be played

};
