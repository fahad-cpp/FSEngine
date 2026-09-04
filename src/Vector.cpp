#include "Vector.h"
#include <cmath>
Vector3 cross(Vector3 vec1, Vector3 vec2) {
    Vector3 result = {
        (vec1.y * vec2.z) - (vec1.z * vec2.y),
        (vec1.z * vec2.x) - (vec1.x * vec2.z),
        (vec1.x * vec2.y) - (vec1.y * vec2.x)
    };
    return result;
}
float dot(Vector3 vec1,Vector3 vec2){
    return ((vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z));
}
Vector3 normalize(Vector3 vec){
    float length = std::sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
    if(length == 0.f){
        return {0,0,0};
    }
    return {vec.x / length, vec.y / length, vec.z / length};
}
Vector3 dist(Vector3 from,Vector3 to){
    return {to.x - from.x, to.y - from.y, to.z - from.z};
}