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

#ifndef WaveTableOsc_h
#define WaveTableOsc_h

class IRAM_ATTR WaveTableOsc {
public:
    WaveTableOsc(void) {
        for (int idx = 0; idx < numWaveTableSlots; idx++) {
            mWaveTables[idx].topFreq = 0;
            mWaveTables[idx].waveTableLen = 0;
            mWaveTables[idx].waveTable = 0;
        }
    }
    ~WaveTableOsc(void) {
        for (int idx = 0; idx < numWaveTableSlots; idx++) {
            double *temp = mWaveTables[idx].waveTable;
            if (temp != 0)
                delete [] temp;
        }
    }


    void SetFrequency(double frequency, double sample_rate)
    {
      SetFrequency(2.0*frequency/sample_rate);
    }
    //
    // SetFrequency: Set normalized frequency, typically 0-0.5 (must be positive and less than 1!)
    //
    void SetFrequency(double inc) {
        mPhaseInc = inc;

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

        if (mPhasor >= 1.0)
            mPhasor -= 1.0;
    }

    //
    // Process: Update phase and get output
    //
    float Process(void) {
        UpdatePhase();
        return GetOutput();
    }

    //
    // GetOutput: Returns the current oscillator output
    //
    float GetOutput(void) {
        waveTable *waveTable = &mWaveTables[mCurWaveTable];

        // linear interpolation
        double temp = mPhasor * waveTable->waveTableLen;
        int intPart = temp;
        double fracPart = temp - intPart;
        double samp0 = waveTable->waveTable[intPart];
        double samp1 = waveTable->waveTable[intPart + 1];
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
    float GetOutputMinusOffset() {
        waveTable *waveTable = &mWaveTables[mCurWaveTable];
        int len = waveTable->waveTableLen;
        double *wave = waveTable->waveTable;

        // linear
        double temp = mPhasor * len;
        int intPart = temp;
        double fracPart = temp - intPart;
        double samp0 = wave[intPart];
        double samp1 = wave[intPart+1];
        double samp = samp0 + (samp1 - samp0) * fracPart;

        // and linear again for the offset part
        double offsetPhasor = mPhasor + mPhaseOfs;
        if (offsetPhasor > 1.0)
            offsetPhasor -= 1.0;
        temp = offsetPhasor * len;
        intPart = temp;
        fracPart = temp - intPart;
        samp0 = wave[intPart];
        samp1 = wave[intPart+1];
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
    int AddWaveTable(int len, double *waveTableIn, double topFreq) {
        if (mNumWaveTables < numWaveTableSlots) {
            double *waveTable = mWaveTables[mNumWaveTables].waveTable = new double[len + 1];
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
    double mPhasor = 0.0;       // phase accumulator
    double mPhaseInc = 0.0;     // phase increment
    double mPhaseOfs = 0.5;     // phase offset for PWM

    // array of wavetables
    int mCurWaveTable = 0;      // current table, based on current frequency
    int mNumWaveTables = 0;     // number of wavetable slots in use
    struct waveTable {
        double topFreq;
        int waveTableLen;
        double *waveTable;
    };
    static constexpr int numWaveTableSlots = 1;    // simplify allocation with reasonable maximum
    waveTable mWaveTables[numWaveTableSlots];
};

#endif
