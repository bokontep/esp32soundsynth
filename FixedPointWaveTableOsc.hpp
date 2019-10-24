#pragma once
#pragma once
//
//  WaveTableOsc.h
//
//  Created by Nigel Redmon on 2018-10-05
//  EarLevel Engineering: earlevel.com
//  Copyright 2018 Nigel Redmon
//
//  For a complete explanation of the wavetable oscillator and code,
//  read the series of articles by the author, starting here:
//  www.earlevel.com/main/2012/05/03/a-wavetable-oscillator—introduction/
//
//  This version has optimizations described here:
//  www.earlevel.com/main/2019/04/28/wavetableosc-optimized/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code for your own purposes, free or commercial.
//

#ifndef FixedPointWaveTableOsc_h
#define FixedPointWaveTableOsc_h

#include <Arduino.h>
///////////////////////////////////////////////////////////////
// https://embeddedartistry.com/blog/2018/7/9/template-rayb2 //
///////////////////////////////////////////////////////////////
#define FPFB 16
inline double IRAM_ATTR fixed_to_float(int32_t input)
{
	return ((double)input / (double)(1 << FPFB));
}


inline int32_t IRAM_ATTR float_to_fixed(double input)
{
	return (int32_t)(input * (1 << FPFB));

}


IRAM_ATTR class FixedPointWaveTableOsc {
public:
	FixedPointWaveTableOsc(void) {
		for (int idx = 0; idx < numWaveTableSlots; idx++) {
			mWaveTables[idx].topFreq = 0;
			mWaveTables[idx].waveTableLen = 0;
			mWaveTables[idx].waveTable = 0;
		}
	}
	~FixedPointWaveTableOsc(void) {
		for (int idx = 0; idx < numWaveTableSlots; idx++) {
			int8_t *temp = mWaveTables[idx].waveTable;
			if (temp != 0)
				delete[] temp;
		}
	}

	
	void SetFrequency(double freq, double sampleRate)
	{
		double nfreq = freq / sampleRate;
		SetFrequency(nfreq);
		//printf("%f\r\n", freq);
	}
	
	//
	// SetFrequency: Set normalized frequency, typically 0-0.5 (must be positive and less than 1!)
	//
	void SetFrequency(double inc) {
		mPhaseInc = float_to_fixed(inc);

		// update the current wave table selector
		int curWaveTable = 0;
		while ((mPhaseInc >= mWaveTables[curWaveTable].topFreq) && (curWaveTable < (mNumWaveTables - 1))) {
			++curWaveTable;
		}
		mCurWaveTable = curWaveTable;
	}

	//
	// SetPhaseOffset: Phase offset for PWM, 0-1
	//
	void SetPhaseOffset(double offset) {
		mPhaseOfs = offset;
	}

	//
	// UpdatePhase: Call once per sample
	//
	void UpdatePhase(void) {
		mPhasor += mPhaseInc;

		if (mPhasor >= float_to_fixed(1.0))
			mPhasor -= float_to_fixed(1.0);
	}

	//
	// Process: Update phase and get output
	//
	int32_t Process(void) {
		UpdatePhase();
		return GetOutput();
	}

	//
	// GetOutput: Returns the current oscillator output
	//
	int32_t GetOutput(void) {
		waveTable *waveTable = &mWaveTables[mCurWaveTable];

		// linear interpolation
		int32_t temp = mPhasor * waveTable->waveTableLen;
		int32_t intPart = (temp & 0xFFFFFF00);
		int32_t fracPart = temp - intPart;
		int32_t samp0 = waveTable->waveTable[intPart>>FPFB];
		int32_t samp1 = waveTable->waveTable[intPart>>FPFB + 1];
		return samp0 + (samp1 - samp0) * fracPart;
	}

	//
	// getOutputMinusOffset
	//
	// for variable pulse width: initialize to sawtooth,
	// set phaseOfs to duty cycle, use this for osc output
	//
	// returns the current oscillator output
	//
	int32_t GetOutputMinusOffset() {
		waveTable *waveTable = &mWaveTables[mCurWaveTable];
		int len = waveTable->waveTableLen;
		int8_t *wave = waveTable->waveTable;

		// linear
		int32_t temp = mPhasor * len;
		int32_t intPart = temp;
		int32_t fracPart = temp - intPart;
		int32_t samp0 = wave[intPart];
		int32_t samp1 = wave[intPart + 1];
		int32_t samp = samp0 + (samp1 - samp0) * fracPart;

		// and linear again for the offset part
		int32_t offsetPhasor = mPhasor + mPhaseOfs;
		if (offsetPhasor > float_to_fixed(1.0))
			offsetPhasor -= float_to_fixed(1.0);
		temp = offsetPhasor * len;
		intPart = temp;
		fracPart = temp - intPart;
		samp0 = wave[intPart];
		samp1 = wave[intPart + 1];
		return samp - (samp0 + (samp1 - samp0) * fracPart);
	}

	//
	// AddWaveTable
	//
	// add wavetables in order of lowest frequency to highest
	// topFreq is the highest frequency supported by a wavetable
	// wavetables within an oscillator can be different lengths
	//
	// returns 0 upon success, or the number of wavetables if no more room is available
	//
	int AddWaveTable(int len, int8_t *waveTableIn, double topFreq) {
		if (mNumWaveTables < numWaveTableSlots) {
			int8_t *waveTable = mWaveTables[mNumWaveTables].waveTable = new int8_t[len + 1];
			mWaveTables[mNumWaveTables].waveTableLen = len;
			mWaveTables[mNumWaveTables].topFreq = topFreq;
			++mNumWaveTables;

			// fill in wave
			for (long idx = 0; idx < len; idx++)
				waveTable[idx] = waveTableIn[idx];
			waveTable[len] = waveTable[0];  // duplicate for interpolation wraparound

			return 0;
		}
		return mNumWaveTables;
	}

protected:
	int32_t mPhasor = float_to_fixed(0.0);       // phase accumulator
	int32_t mPhaseInc = float_to_fixed(0.0);     // phase increment
	int32_t mPhaseOfs = float_to_fixed(0.5);     // phase offset for PWM

	// array of wavetables
	int mCurWaveTable = 0;      // current table, based on current frequency
	int mNumWaveTables = 0;     // number of wavetable slots in use
	struct waveTable {
		double topFreq;
		int waveTableLen;
		int8_t *waveTable;
	};
	static constexpr int numWaveTableSlots = 40;    // simplify allocation with reasonable maximum
	waveTable mWaveTables[numWaveTableSlots];
};

#endif
