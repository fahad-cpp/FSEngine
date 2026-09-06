#ifndef MATRIX_H
#define MATRIX_H
#include "Vector.h"
struct Matrix4 {
    float values[4][4];
};
Vector3 multMat4Vec(const Matrix4 mat,const Vector4 vec);
Matrix4 multMat4Mat4(Matrix4 mat1, Matrix4 mat2);
Matrix4 unitMatrix4();
Matrix4 rotate(const Matrix4 matrix,const float angle,const Vector3 axis);
Matrix4 lookAt(const Vector3 eye,const Vector3 center,const Vector3 up);
Matrix4 perspective(const float fov,const float aspectRatio,const float nearPlane,const float farPlane);
float radians(float degree);
#endif