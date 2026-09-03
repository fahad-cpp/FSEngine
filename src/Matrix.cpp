#include "Matrix.h"

Vector3 multVecMat4(const Vector4 vec, const Matrix4 mat) {
    float a, b, c, w;

    a = vec.x * mat.values[0][0] + vec.y * mat.values[1][0] + vec.z * mat.values[2][0] + vec.w * mat.values[3][0];
    b = vec.x * mat.values[0][1] + vec.y * mat.values[1][1] + vec.z * mat.values[2][1] + vec.w * mat.values[3][1];
    c = vec.x * mat.values[0][2] + vec.y * mat.values[1][2] + vec.z * mat.values[2][2] + vec.w * mat.values[3][2];
    w = vec.x * mat.values[0][3] + vec.y * mat.values[1][3] + vec.z * mat.values[2][3] + vec.w * mat.values[3][3];

    return { a / w, b / w, c / w };
}