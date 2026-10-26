#ifndef QUATERNION_H
#define QUATERNION_H

typedef struct {
    float x;
    float y;
    float z;
} vector3_t;

typedef struct {
    float w;
    float x;
    float y;
    float z;
} quaternion_t;

// Identity quaternion
quaternion_t Quaternion_Identity(void);

// Quaternion magnitude
float Quaternion_Norm(quaternion_t q);

// Normalize to unit quaternion
quaternion_t Quaternion_Normalize(quaternion_t q);

// Quaternion conjugate
quaternion_t Quaternion_Conjugate(quaternion_t q);

// Hamilton quaternion product
quaternion_t Quaternion_Multiply(
    quaternion_t a,
    quaternion_t b
);

// Construct quaternion from axis-angle
quaternion_t Quaternion_FromAxisAngle(
    vector3_t axis,
    float angle_rad
);

// Rotate a vector using a unit quaternion
vector3_t Quaternion_RotateVector(
    quaternion_t q,
    vector3_t v
);

// Convert unit quaternion to shortest rotation vector
vector3_t Quaternion_ToRotationVector(
    quaternion_t q
);

#endif