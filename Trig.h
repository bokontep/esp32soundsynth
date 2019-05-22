#ifndef FIXIE_TRIG_H
#define FIXIE_TRIG_H

namespace Fixie {
  namespace Trig {
    extern const Num pi;
    extern const Num twoPi;
    extern const Num halfPi;
    extern const Num inverseTwoPi;

    Num sin(Num n);
    Num cos(Num n);
    Num acos(Num n);
  }
}

#endif
