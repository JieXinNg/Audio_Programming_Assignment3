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
#include <cmath>			// library for the function pow()
#include <vector>			// library for creating vectors
#include "Oscillator.h"		// library for generating oscillators
#include <map>				// create map to map the modes to the values of the notes
#include <JuceHeader.h>		// library to convert midi values to frequencies
#include "Delay.h"


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
	void setOscillatorParams(float _sr) 
	{
		// set parameters for the oscillators
		sampleRate = _sr;
		sineOsc.setSampleRate(_sr);
		sineOsc.setRampParams(_sr, 0.03333, 240);
		sqOsc.setSampleRate(_sr);
		triOsc.setSampleRate(_sr);

		sinePulse.setSampleRate(_sr);
		sinePulse.setFrequency(pulseFreq);
		sinePulse.setPower(pulsePower);
		phasor.setSampleRate(_sr);
		phasor.setFrequency(0.5);

		lfo.setSampleRate(_sr);
		lfo.setFrequency(0.01);

		delay.setSize(_sr);
		delay.setDelayTime(0.5 * _sr);
	}

	/**
	* generate possible modes to be selected, called once before changeMode()
	*/
	void generateNotesForModes(int numOctaves)
	{

		numNotes = 7 * numOctaves;            // set the number of possible notes in the range

		if (numOctaves > 1) // add notes if there is more than one octave
		{
			// loop to generate all the possible notes
			for (int i = 1; i < (numOctaves + 1); i++)
			{
				for (int x = 0; x < 7; x++) // loop through each note in one octave
				{
					// append the vectors to cover the number of octaves (numOctaves)
					ionian.push_back(ionian[x] + 12 * i);
					dorian.push_back(dorian[x] + 12 * i);
					phrygian.push_back(phrygian[x] + 12 * i);
					lydian.push_back(lydian[x] + 12 * i);
					mixolydian.push_back(mixolydian[x] + 12 * i);
					aeolian.push_back(aeolian[x] + 12 * i);
					locrian.push_back(locrian[x] + 12 * i);

				}
			}
		}

		// compile all the different modes for for loop
		std::vector<std::vector<int>> listOfModes = { ionian, dorian, phrygian, lydian , mixolydian, aeolian, locrian };

		
		for (int i = 0; i < modeCount; i++)
		{
			keyDictionary[modeList[i]] = listOfModes[i]; // eg: keyDictionary["Major"] = ionian;
		}
	}


	void changeMode(int _baseNote, float _mode, int numOctaves)
	{
		std::vector<float> _notes;       // vector to contain the generated notes for the scale

		mode = modeList[(int)_mode];

		float baseNote = _baseNote;           // set the base note 
		numNotes = 7 * numOctaves;            // set the number of possible notes in the range


		// loop to select the correct mode and generate a vector to hold all the notes
		for (int i = 0; i < numNotes; i++)
		{
			std::vector<int> a = keyDictionary.at(mode);							// select the mode at keyDictionary
			float _note = juce::MidiMessage::getMidiNoteInHertz(baseNote + a[i]);   // convert each note from midi to frequency
			_notes.push_back(_note);												// add the frequency value to notes (vector)
			notes = _notes;															//  set notes to be from the selected mode
		}

		sineOsc.setFrequency(getNotes(0));											// set the default frequency
		sqOsc.setFrequency(getNotes(0));
		triOsc.setFrequency(getNotes(0));

	}

	/**
	* 
	*/
	void setPulseSpeed(float phasorFreq)
	{
		phasor.setFrequency(phasorFreq);
	}

	/**
	*
	*/
	void setLfofreq(float lfoFreq)
	{
		lfo.setFrequency(lfoFreq);
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
		float pulseVolume; 
		float output = 0;
		if (phasor.process() <= 0.5f)
		{
			pulseVolume = sin(2.0 * 3.142 * phasor.process());
		}
		else
		{
			pulseVolume = 0;
		}

		if (randomOsc == 0)
		{
			output = sineOsc.process();
		}

		else if (randomOsc == 1)
		{
			output = sqOsc.process();
		}
		else if (randomOsc == 2)
		{
			output = triOsc.process();
		}

		output = output * lfo.process() * 0.5 * pulseVolume;
		return output + delay.process(output) * 0.5;   
	}

	/**
	* change the frequency of sineOsc every time this function is called
	* this function can be called in the DSP loop to update the frequency of the oscillator
	*/
	void changeFreq()
	{

		if ((1 - phasor.process()) <= phasor.getPhaseDelta()) // change frequency when the phase is nearing 1 
		{
			int randomInteger = random.nextInt(numNotes - 1); // generate random integer
			int outVal = notes[randomInteger];				  // select random note from the scale
			sineOsc.setFrequency(outVal);					  // set the frequency of sineOsc
			sqOsc.setFrequency(outVal);
			triOsc.setFrequency(outVal);
			randomOsc = random.nextInt(3);
			//DBG(numNotes);
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

	/**
	* returns the notes (vector) based on the mode chosen 
	*/
	std::vector<float> getNoteVector()
	{
		return notes;
	}

private:
	// variables to be set in setKey()
	int key;                        // midi value
	float sampleRate;				// sample rate    
	int numNotes = 7;               // number of notes according to number of octaves set in setKey
	std::vector<float> notes;       // vector to contain the generated notes for the scale

	// variables to be set in setSinePulseParams
	float pulseFreq = 0.1;          // set the default frequency of sinPulse
	int pulsePower = 9;            // set the default power of the sine wave for sinePulse
	
	// oscillators
	PhaseModulationSineOsc sineOsc; // sine oscillator to generate audio
	SquareOsc sqOsc;
	TriOsc triOsc;
	SineOsc sinePulse;              // sine oscillator to modulate the volume to simulate pulse
	Oscillator phasor;              // phasor to check the time to change frequency
	SineOsc lfo;					// lfo to modulate the volume

	juce::Random random;            // random is called to select the notes to be played
	int randomOsc;				// choose random oscillator

	// create dictionary to map the selected modes to the correct notes
	// further modes can be mapped here rather than using if statements to check the input
	std::map<std::string, std::vector<int>> keyDictionary;
	std::string mode;
	int modeCount = 7;
	std::string modeList[7] = { "Ionian / Major", "Dorian", "Phrygian", "Lydian", "Mixolydian", "Aeolian / Minor", "Locrian" };
	Delay delay;

	// modes used
	std::vector<int> ionian = { 0, 2, 4, 5, 7, 9, 11 }; // major
	std::vector<int> dorian = { 0, 1, 3, 5, 6, 8, 10 };
	std::vector<int> phrygian = { 0, 1, 3, 5, 7, 8, 10 };
	std::vector<int> lydian = { 0, 2, 4, 6, 7, 9, 11 };
	std::vector<int> mixolydian = { 0, 2, 4, 5, 7, 9, 10 };
	std::vector<int> aeolian = { 0, 2, 3, 5, 7, 8, 10 }; // natural minor
	std::vector<int> locrian = { 0, 1, 3, 5, 6, 8, 10 };
};
