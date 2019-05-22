#include "Util.h"
#include "Vector3.h"

namespace Fixie {
  Vector3::Vector3() { }

  Vector3::Vector3(Num x, Num y, Num z) {
    components[0] = x;
    components[1] = y;
    components[2] = z;
  }

  Num& Vector3::operator[](const int index) {
    return components[index];
  }

  const Num& Vector3::operator[](const int index) const {
    return components[index];
  }

  Vector3 Vector3::operator+(Vector3 vector) {
    Vector3 result = *this;
    result += vector;
    return result;
  }

  Vector3& Vector3::operator+=(Vector3 other) {
    components[0] += other[0];
    components[1] += other[1];
    components[2] += other[2];
    return *this;
  }

  Vector3 Vector3::operator-(Vector3 vector) {
    Vector3 result = *this;
    result -= vector;
    return result;
  }

  Vector3& Vector3::operator-=(Vector3 other) {
    components[0] -= other[0];
    components[1] -= other[1];
    components[2] -= other[2];
    return *this;
  }

  Vector3 Vector3::operator*(Num divisor) {
    Vector3 result = *this;
    result *= divisor;
    return result;
  }

  Vector3& Vector3::operator*=(Num divisor) {
    components[0] *= divisor;
    components[1] *= divisor;
    components[2] *= divisor;
    return *this;
  }

  Vector3 Vector3::operator/(Num divisor) {
    Vector3 result = *this;
    result /= divisor;
    return result;
  }

  Vector3& Vector3::operator/=(Num divisor) {
    components[0] /= divisor;
    components[1] /= divisor;
    components[2] /= divisor;
    return *this;
  }

  Num Vector3::dot(Vector3 a, Vector3 b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
  }

  Vector3 Vector3::cross(Vector3 a, Vector3 b) {
    return Vector3(
      a[1]*b[2] - a[2]*b[1],
      a[2]*b[0] - a[0]*b[2],
      a[0]*b[1] - a[1]*b[0]
    );
  }

  void Vector3::reset() {
    components[0] = 0;
    components[1] = 0;
    components[2] = 0;
  }

  Vector3 Vector3::normalize(Vector3 v) {
    return v/v.calcLength();
  }

  Num Vector3::calcLength() const {
    return Util::sqrt(calcSquaredLength());
  }

  Num Vector3::calcSquaredLength() const {
    return (
      components[0] * components[0] +
      components[1] * components[1] +
      components[2] * components[2]
    );
  }
}
