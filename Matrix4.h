#ifndef FIXIE_MATRIX4_H
#define FIXIE_MATRIX4_H

#include "Num.h"

namespace Fixie {
  class Matrix4 {
  public:
    Matrix4(
      Num c0,
      Num c1,
      Num c2,
      Num c3,
      Num c4,
      Num c5,
      Num c6,
      Num c7,
      Num c8,
      Num c9,
      Num c10,
      Num c11,
      Num c12,
      Num c13,
      Num c14,
      Num c15
    );
    Matrix4();
    Num& operator[](const int index);
    const Num& operator[](const int index) const;
    Matrix4 operator*(Matrix4 other);
    Matrix4& operator*=(Matrix4 other);
    static Matrix4 identity();
    void reset();
  private:
    Num components[16];
  };
}

#endif
