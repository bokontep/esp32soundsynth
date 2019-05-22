#ifndef FIXIE_QUATERNION_H
#define FIXIE_QUATERNION_H

#include "Num.h"
#include "Vector3.h"

namespace Fixie {
  class Quaternion {
  public:
    Quaternion();
    Quaternion(Num real, Vector3 imaginaries);
    Num real;
    Vector3 imaginaries;
    static Quaternion identity();
  };
}

#endif
