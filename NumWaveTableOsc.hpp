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

#ifndef NumWaveTableOsc_h
#define NumWaveTableOsc_h
#include "Num.h"
#include "Util.h"
using namespace Fixie;

class NumWaveTableOsc {
public:
    NumWaveTableOsc(void) {
        for (int idx = 0; idx < numWaveTableSlots; idx++) {
            mWaveTables[idx].topFreq = 0;
            mWaveTables[idx].waveTableLen = 0;
            mWaveTables[idx].waveTable = 0;
        }
    }
    ~NumWaveTableOsc(void) {
        for (int idx = 0; idx < numWaveTableSlots; idx++) {
            int8_t *temp = mWaveTables[idx].waveTable;
            if (temp != 0)
                delete [] temp;
        }
    }



    
    void SetFrequency(double freq, double sampleRate)
    {
      SetFrequency (Num(freq/sampleRate));
    }
    //
    // SetFrequency: Set normalized frequency, typically 0-0.5 (must be positive and less than 1!)
    //
    void SetFrequency(Num inc) {
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

        if (mPhasor >= Num(1.0))
            mPhasor -= Num(1.0);
    }

    //
    // Process: Update phase and get output
    //
    Num Process(void) {
        UpdatePhase();
        return GetOutput();
    }

    //
    // GetOutput: Returns the current oscillator output
    //
    Num GetOutput(void) {
        waveTable *waveTable = &mWaveTables[mCurWaveTable];

        // linear interpolation
        Num temp = mPhasor *  (waveTable->waveTableLen);
        Num intPart = Fixie::Util::floor(temp);
        Num fracPart = temp - intPart;
        Num samp0 = Num(waveTable->waveTable[intPart]);
        Num samp1 = Num(waveTable->waveTable[intPart + Num(1)]);
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
    Fixie::Num GetOutputMinusOffset() {
        waveTable *waveTable = &mWaveTables[mCurWaveTable];
        Num len = waveTable->waveTableLen;
        int8_t *wave = waveTable->waveTable;

        // linear
        Num temp = mPhasor * len;
        Num intPart = Fixie::Util::floor(temp);
        Num fracPart = temp - intPart;
        Num samp0 = Num(wave[intPart]);
        Num samp1 = Num(wave[intPart+Num(1)]);
        Num samp = samp0 + (samp1 - samp0) * fracPart;

        // and linear again for the offset part
        Fixie::Num offsetPhasor = mPhasor + mPhaseOfs;
        if (offsetPhasor > Num(1.0))
            offsetPhasor -= Num(1.0);
        temp = offsetPhasor * Num(len);
        intPart = temp;
        fracPart = temp - intPart;
        samp0 = wave[intPart];
        samp1 = wave[intPart+Num(1)];
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
            mWaveTables[mNumWaveTables].topFreq = Num(topFreq);
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
    Num mPhasor = Num(0.0);       // phase accumulator
    Num mPhaseInc = Num(0.0);     // phase increment
    Num mPhaseOfs = Num(0.5);     // phase offset for PWM

    // array of wavetables
    int mCurWaveTable = 0;      // current table, based on current frequency
    int mNumWaveTables = 0;     // number of wavetable slots in use
    struct waveTable {
        Num topFreq;
        Num waveTableLen;
        int8_t *waveTable;
    };
    static constexpr int numWaveTableSlots = 8;    // simplify allocation with reasonable maximum
    waveTable mWaveTables[numWaveTableSlots];
};

#endif
