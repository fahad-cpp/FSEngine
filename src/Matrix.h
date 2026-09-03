#include "Vector.h"
struct Matrix4 {
    float values[4][4];
};
Vector3 multVecMat4(const Vector4 vec, const Matrix4 mat);