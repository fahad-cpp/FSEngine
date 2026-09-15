#ifndef VECTOR_H
#define VECTOR_H
struct Vector2 {
    float x;
    float y;
};
struct Vector3 {
    float x;
    float y;
    float z;
};
struct Vector4 {
    float x;
    float y;
    float z;
    float w;
};

Vector3 cross(Vector3 vec1, Vector3 vec2);
float   dot(Vector3 vec1, Vector3 vec2);
Vector3 normalize(Vector3 vec);
Vector3 dist(Vector3 from, Vector3 to);
#endif