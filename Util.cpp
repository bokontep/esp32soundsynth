#include <assert.h>
#include "Util.h"

namespace Fixie {
  namespace Util {
    Num halve(Num n) {
      return Num::createByRaw(n.raw >> 1);
    }

    Num floor(Num n) {
      n.raw &= 0xfffffc00;
      return n;
    }

    Num sqrt(Num n) {
      assert(n >= Num(0));
      Num s = halve(n);
      for(uint8_t i=0; i<6; ++i) {
        s = halve(s + n/s);
      }
      return s;
    }
  }
}
