#include "Matrix.h"
#include "Logging.h"
#include <cmath>
#include <numbers>
#include <cstdint>
Matrix4 unitMatrix4(float scale){
    return {{
        { scale, 0    , 0    , 0     },
        { 0    , scale, 0    , 0     },
        { 0    , 0    , scale, 0     },
        { 0    , 0    , 0    , 1.f     }
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
    float pi = static_cast<float>(std::numbers::pi);
    float clampedAngle = std::fmod(angle,pi * 2.f);
    if(clampedAngle < 0.f){
        clampedAngle += 2.f * pi;
    }
    float costheta = std::cos(clampedAngle);
    float sintheta = std::sin(clampedAngle);
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
        LOG_ERROR("Unhandled axis rotation");
        return matrix;
    }
    return multMat4Mat4(rotationMatrix,matrix);
}

Vector3 rotate(const Vector3 vec, const float angle, const Vector3 axis) {
    float pi = static_cast<float>(std::numbers::pi);
    float clampedAngle = std::fmod(angle,pi * 2.f);
    if(clampedAngle < 0.f){
        clampedAngle += 2.f * pi;
    }
    float costheta = std::cos(clampedAngle);
    float sintheta = std::sin(clampedAngle);
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
        LOG_ERROR("Unhandled axis rotation");
        return {vec.x,vec.y,vec.z};
    }
    Vector4 tempVec = {vec.x,vec.y,vec.z,1.f};
    return multMat4Vec(rotationMatrix,tempVec);
}
Vector3 rotate(const Vector3 vec, const Vector3 rotation){
    Vector3 res = rotate(vec,rotation.x,Vector3{1.f,0.f,0.f});
    res = rotate(res,rotation.y,Vector3{0.f,1.f,0.f});
    res = rotate(res,rotation.z,Vector3{0.f,0.f,1.f});
    return res;
}
//Produces a view matrix for camera
Matrix4 lookAt(const Vector3 position, const Vector3 lookPoint, const Vector3 up) {
    Vector3 cForward = normalize(dist(position,lookPoint));
    Vector3 cRight = normalize(cross(up, cForward));
    Vector3 cUp = normalize(cross(cForward,cRight));

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

float degree(float radians){
    return radians * (180.f / static_cast<float>(std::numbers::pi));
}