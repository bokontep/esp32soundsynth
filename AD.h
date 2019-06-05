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

#ifndef AD_h
#define AD_h
#include "Num.h"
using namespace Fixie;
class AD {
public:
  AD(void);
  ~AD(void);

  Num Process(void);
    Num GetOutput(void);
    int GetState(void);
  void SetAD(Num attack, Num decay);
  void Gate(int on);
    void SetAttack(Num attack);
    void SetDecay(Num decay);


    void Reset(void);

    enum envState {
        env_idle = 0,
        env_attack,
        env_decay
    };

protected:

    int state;
    Num output;
  Num attack;
  Num decay;
  int32_t counter;
  Num attackCoef;
  Num decayCoef;
  
    
    Num attackBase;
    Num decayBase;
 
    
};

inline Num AD::Process() {
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
      
            if (output <= Num(0)) {
                output = Num(0);
                state = env_idle;
            }
            break;
        
  }
  return output;
}

inline void AD::Gate(int gate) {
  if (gate)
  {

    output = Num(0);
    state = env_attack;
  }
  
}

inline int AD::GetState() {
    return state;
}

inline void AD::Reset() {
    state = env_idle;
    output = Num(0);
}

inline Num AD::GetOutput() {
  return output;
}

#endif
