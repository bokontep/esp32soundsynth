#include "AD.h"



AD::AD(void) {
    Reset();
    SetAD(Num(1), Num(1));
    
}

AD::~AD(void) {
}

void AD::SetAD(Num attack, Num decay)
{
  
  SetAttack(attack);
  SetDecay(decay);
  
}
void AD::SetAttack(Num attack) {
    this->attack = attack;
  if (attack < Num(1.0))
  {
    attack = Num(1.0);
  }
    attackCoef = Num(1.0)/attack;
    attackBase = Num(0);
}


void AD::SetDecay(Num decay) {
  this->decay = decay;
  if (decay < Num(1))
  {
    decay = Num(1);
  }
  decayCoef = Num(1)/decay;
  
}

    
