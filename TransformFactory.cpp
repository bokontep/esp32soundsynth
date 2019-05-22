#include "TransformFactory.h"

namespace Fixie {
  namespace TransformFactory {
    Matrix4 translation(Vector3 translation) {
      Matrix4 matrix = Matrix4::identity();
      matrix[12] = translation[0];
      matrix[13] = translation[1];
      matrix[14] = translation[2];
      return matrix;
    }
  }
}
