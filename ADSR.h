//
//  ADSR.h
//
//  Created by Nigel Redmon on 12/18/12.
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//
//  For a complete explanation of the ADSR envelope generator and code,
//  read the series of articles by the author, starting here:
//  http://www.earlevel.com/main/2013/06/01/envelope-generators/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code for your own purposes, free or commercial.
//

#ifndef ADSR_h
#define ADSR_h
#include "Num.h"
using namespace Fixie;
class ADSR {
public:
	ADSR(void);
	~ADSR(void);

	Num Process(void);
    Num GetOutput(void);
    int GetState(void);
	void SetADSR(Num attack, Num decay, Num sustain, Num release);
	void Gate(int on);
    void SetAttack(Num attack);
    void SetDecay(Num decay);
    void SetRelease(Num release);
	void SetSustain(Num level);

    void Reset(void);

    enum envState {
        env_idle = 0,
        env_attack,
        env_decay,
        env_sustain,
        env_release
    };

protected:

    int state;
    Num output;
	Num attack;
	Num decay;
	Num release;
	int32_t counter;
	Num attackCoef;
	Num decayCoef;
	Num releaseCoef;
	Num sustainLevel;
    
    Num attackBase;
    Num decayBase;
    Num releaseBase;
 
    
};

inline Num ADSR::Process() {
	switch (state) {
        case env_idle:
			output = Num(0);
            break;
        case env_attack:
            output = output+attackBase +  attackCoef;
			
            if (output >= Num(1.0)) {
                output = Num(1.0); //http://www.earlevel.com/main/2013/06/01/envelope-generators/
                state = env_decay;
				
            }
            break;
        case env_decay:
            output = output - decayCoef;
			
            if (output <= sustainLevel) {
                output = sustainLevel;
                state = env_sustain;
            }
            break;
        case env_sustain:
			//output = sustainLevel;
            break;
        case env_release:
            output = output - releaseCoef;
			
            if (output <= Num(0.0)) {
                output = Num(0);
                state = env_idle;
            }
			break;
	}
	return output;
}

inline void ADSR::Gate(int gate) {
	if (gate)
	{

		output = Num(0);
		state = env_attack;
	}
	else if (state != env_idle)
	{
		state = env_release;
		
	}
}

inline int ADSR::GetState() {
    return state;
}

inline void ADSR::Reset() {
    state = env_idle;
    output = Num(0);
}

inline Num ADSR::GetOutput() {
	return output;
}

#endif
