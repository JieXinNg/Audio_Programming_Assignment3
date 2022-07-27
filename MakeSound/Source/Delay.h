/*
  ==============================================================================

    Delay.h
    
	Contains class Delay

  ==============================================================================
*/

#pragma once

/**
* outputs the delayed sample ( process() )
* 
* @param inputSample (float) 
* @param sizeInSamples (int) sample rate 
* @param _delayTimeInSamples (int)
* 
* @return output (float) the delayed sample
* @return outVal (float)
*/
class Delay
{
public:

	/**
	* outputs the delayed sample
	* 
	* @param inputSample (float) 
	* @return output (float) the delayed sample
	*/
	float process(float inputSample)
	{
		float output = readVal();
		writeVal(inputSample);

		return output;
	}

	/**
	* called in process()
	* 
	* @return outVal (float)
	*/
	float readVal()
	{
		// get current value at readPos
		float outVal = buffer[readPos];
		readPos++; //increment 

		//increment readPos
		if (readPos >= size)
		{
			readPos = 0;
		}

		return outVal;
	}

	/**
	* called in process()
	* 
	* @param inputSample (float)
	*/
	void writeVal(float inputSample)
	{
		// store current value at writePos
		buffer[writePos] = inputSample;

		//increament writePos
		writePos ++;

		if (writePos >= size)
		{
			writePos = 0;
		}
	}
	
	/**
	* set the size of delay line in samples
	* 
	* @param sizeInSamples (int) sample rate 
	*/
	void setSize(int sizeInSamples)
	{
		size = sizeInSamples;
		buffer = new float[size];

		// initialise all values to zero
		for (int i = 0; i < size; i++)
		{
			buffer[i] = 0.0;
		}
	}


	/**
	* set the delay time 
	* 
	* @param _delayTimeInSamples (int)
	*/
	void setDelayTime(int _delayTimeInSamples) //set the delay time in samples
	{
		delayTimeInSamples = _delayTimeInSamples;

		readPos = writePos - delayTimeInSamples;

		if (readPos < 0)
		{
			readPos += size;
		}
	}

private:
	float* buffer;		// set the buffer size
	int size;
	int readPos = 0;
	int writePos = 0;
	int delayTimeInSamples;

};