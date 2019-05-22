//
//  ADSR.cpp
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
//  1.01  2016-01-02  njr   added calcCoef to SetTargetRatio functions that were in the ADSR widget but missing in this code
//  1.02  2017-01-04  njr   in calcCoef, checked for rate 0, to support non-IEEE compliant compilers
//


#include "ADSR.h"



ADSR::ADSR(void) {
    Reset();
	SetADSR(1, 1, 1.0, 1);
    
}

ADSR::~ADSR(void) {
}

void ADSR::SetADSR(Num attack, Num decay, Num sustain, Num release)
{
	SetSustain(sustain);
	SetAttack(attack);
	SetDecay(decay);
	SetRelease(release);
}
void ADSR::SetAttack(Num attack) {
    this->attack = attack;
	if (attack < Num(1.0))
	{
		attack = Num(1.0);
	}
    attackCoef = Num(1.0)/attack;
    attackBase = Num(0);
}


void ADSR::SetDecay(Num decay) {
	this->decay = decay;
	if (decay < Num(1))
	{
		decay = Num(1);
	}
	decayCoef = (Num(1.0) - this->sustainLevel) / this->decay;
}

    


void ADSR::SetSustain(Num level) {
    sustainLevel = level;
    decayBase = sustainLevel;
}

void ADSR::SetRelease(Num releaseval)
{
	release = releaseval;
	if (release < Num(1))
	{
		release = Num(1);
	}
	releaseCoef = (this->sustainLevel / this->release);
	releaseBase = this->sustainLevel;
}