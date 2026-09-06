#include "Matrix.h"
#include <cmath>
#include <numbers>
#include <iostream>

Matrix4 unitMatrix4(){
    return {{
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,0,1}
    }};
}
Matrix4 multMat4Mat4(Matrix4 mat1, Matrix4 mat2) {
    Matrix4 result = {};
    for (uint32_t i = 0; i < 4; ++i) {
        for (uint32_t j = 0; j < 4; ++j) {
            for (uint32_t k = 0; k < 4; ++k) {
                result.values[i][j] += mat1.values[k][j] * mat2.values[i][k];
            }
        }
    }
    return result;
}
Vector3 multMat4Vec(const Matrix4 mat,const Vector4 vec) {
    float a, b, c, w;

    a = vec.x * mat.values[0][0] + vec.y * mat.values[1][0] + vec.z * mat.values[2][0] + vec.w * mat.values[3][0];
    b = vec.x * mat.values[0][1] + vec.y * mat.values[1][1] + vec.z * mat.values[2][1] + vec.w * mat.values[3][1];
    c = vec.x * mat.values[0][2] + vec.y * mat.values[1][2] + vec.z * mat.values[2][2] + vec.w * mat.values[3][2];
    w = vec.x * mat.values[0][3] + vec.y * mat.values[1][3] + vec.z * mat.values[2][3] + vec.w * mat.values[3][3];

    return { a / w, b / w, c / w };
}
Matrix4 rotate(const Matrix4 matrix, const float angle, const Vector3 axis) {
    float costheta = std::cos(angle);
    float sintheta = std::sin(angle);
    Matrix4 rotationMatrix = {};
    if (axis.x == 0.f && axis.y == 0.f && axis.z >= 1.f) {
        rotationMatrix = {{
            { costheta, sintheta, 0, 0 },
            { -sintheta, costheta , 0, 0 },
            { 0       , 0        , 1, 0 },
            { 0       , 0        , 0, 1 }          
        }};
    }else if(axis.x == 0.f && axis.y >= 1.f && axis.z == 0.f){
        rotationMatrix = {{
            { costheta , 0    , -sintheta, 0 },
            { 0        , 1    , 0       , 0 },
            { sintheta, 0    , costheta, 0 },
            { 0        , 0    , 0       , 1 }
        }}; 
    }else if(axis.x >= 1.f && axis.y == 0.f && axis.z == 0.f){
        rotationMatrix = {{
            { 1, 0       ,  0       , 0 },
            { 0,  costheta, sintheta, 0 },
            { 0, -sintheta, costheta, 0 },
            { 0, 0       ,  0       , 1 }
        }};
    }else{
        std::cerr << "Unhandled axis rotation\n";
    }
    return multMat4Mat4(rotationMatrix,matrix);
}

//Produces a view matrix for camera
Matrix4 lookAt(const Vector3 position, const Vector3 lookPoint, const Vector3 up) {
    Vector3 cForward = normalize(dist(position,lookPoint));
    Vector3 cRight = normalize(cross(cForward, up));
    Vector3 cUp = normalize(cross(cRight,cForward));

    Vector4 rightdir    = { cRight.x  , cRight.y  , cRight.z  , -dot(cRight  ,position) };
    Vector4 updir       = { cUp.x     , cUp.y     , cUp.z     , -dot(cUp     ,position) };
    Vector4 forwarddir  = { cForward.x, cForward.y, cForward.z, -dot(cForward,position) };

    Matrix4 result = {{
        {rightdir.x, updir.x, forwarddir.x, 0},
        {rightdir.y, updir.y, forwarddir.y, 0},
        {rightdir.z, updir.z, forwarddir.z, 0},
        {rightdir.w, updir.w, forwarddir.w, 1}
    }};
    return result;
}
Matrix4 perspective(const float fov, const float aspectRatio, const float nearPlane, const float farPlane) {
    float S = 1 / std::tan(fov/2);
    return {{
        {S / aspectRatio, 0, 0                                            , 0 },
        {0              , S, 0                                            , 0 },
        {0              , 0, farPlane/(farPlane - nearPlane)              , 1 },
        {0              , 0, -(farPlane * nearPlane / (farPlane - nearPlane)), 0 }
    }};
}

float radians(float degree){
    return degree * (static_cast<float>(std::numbers::pi) / 180.f);
}