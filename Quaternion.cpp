#include "Quaternion.h"

namespace Fixie {
  Quaternion::Quaternion() { }

  Quaternion::Quaternion(Num real, Vector3 imaginaries) {
    this->real = real;
    this->imaginaries = imaginaries;
  }

  Quaternion Quaternion::identity() {
    return Quaternion(1, Vector3(0, 0, 0));
  }
}
