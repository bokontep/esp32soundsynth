#include "Matrix4.h"

namespace Fixie {
  Matrix4::Matrix4(
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
  ) {
    components[0] = c0;
    components[1] = c1;
    components[2] = c2;
    components[3] = c3;
    components[4] = c4;
    components[5] = c5;
    components[6] = c6;
    components[7] = c7;
    components[8] = c8;
    components[9] = c9;
    components[10] = c10;
    components[11] = c11;
    components[12] = c12;
    components[13] = c13;
    components[14] = c14;
    components[15] = c15;
  }

  Matrix4::Matrix4() { }

  Num& Matrix4::operator[](const int index) {
    return components[index];
  }

  const Num& Matrix4::operator[](const int index) const {
    return components[index];
  }

  void Matrix4::reset() {
    for(uint8_t i=0; i<16; ++i) {
      components[i] = 0;
    }
  }

  Matrix4 Matrix4::operator*(Matrix4 vector) {
    Matrix4 result = *this;
    result *= vector;
    return result;
  }

  Matrix4& Matrix4::operator*=(Matrix4 other) {
    Matrix4 original = *this;
    reset();

    int resultIndex;
    for(int row=0; 4>row; row++) {
      for(int column=0; 4>column; column++) {
        resultIndex = column*4+row;
        for(int step=0; 4>step; step++) {
          components[resultIndex] += original[row+step*4] * other[column*4+step];
        }
      }
    }

    return *this;
  }

  Matrix4 Matrix4::identity() {
    Matrix4 matrix;

    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = 1;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = 1;
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;

    return matrix;
  }
}
