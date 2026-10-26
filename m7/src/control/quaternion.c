#include "control/quaternion.h"

#include <math.h>

#define QUATERNION_EPSILON 1e-6f

quaternion_t Quaternion_Identity(void)
{
    return (quaternion_t){
        .w = 1.0f,
        .x = 0.0f,
        .y = 0.0f,
        .z = 0.0f
    };
}

float Quaternion_Norm(quaternion_t q)
{
    return sqrtf(
        q.w * q.w +
        q.x * q.x +
        q.y * q.y +
        q.z * q.z
    );
}

quaternion_t Quaternion_Normalize(quaternion_t q)
{
    float norm = Quaternion_Norm(q);

    if (norm < QUATERNION_EPSILON)
        return Quaternion_Identity();

    float inv_norm = 1.0f / norm;

    return (quaternion_t){
        .w = q.w * inv_norm,
        .x = q.x * inv_norm,
        .y = q.y * inv_norm,
        .z = q.z * inv_norm
    };
}

quaternion_t Quaternion_Conjugate(quaternion_t q)
{
    return (quaternion_t){
        .w =  q.w,
        .x = -q.x,
        .y = -q.y,
        .z = -q.z
    };
}

quaternion_t Quaternion_Multiply(
    quaternion_t a,
    quaternion_t b
)
{
    return (quaternion_t){
        .w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
        .x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        .y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        .z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    };
}

quaternion_t Quaternion_FromAxisAngle(
    vector3_t axis,
    float angle_rad
)
{
    float magnitude = sqrtf(
        axis.x*axis.x +
        axis.y*axis.y +
        axis.z*axis.z
    );

    if (magnitude < QUATERNION_EPSILON)
        return Quaternion_Identity();

    float half_angle = 0.5f * angle_rad;
    float scale = sinf(half_angle) / magnitude;

    return (quaternion_t){
        .w = cosf(half_angle),
        .x = axis.x * scale,
        .y = axis.y * scale,
        .z = axis.z * scale
    };
}

vector3_t Quaternion_RotateVector(
    quaternion_t q,
    vector3_t v
)
{
    q = Quaternion_Normalize(q);

    quaternion_t vq = {
        .w = 0.0f,
        .x = v.x,
        .y = v.y,
        .z = v.z
    };

    quaternion_t result = Quaternion_Multiply(
        Quaternion_Multiply(q, vq),
        Quaternion_Conjugate(q)
    );

    return (vector3_t){
        .x = result.x,
        .y = result.y,
        .z = result.z
    };
}

vector3_t Quaternion_ToRotationVector(quaternion_t q)
{
    q = Quaternion_Normalize(q);

    // Choose shortest equivalent rotation
    if (q.w < 0.0f) {
        q.w = -q.w;
        q.x = -q.x;
        q.y = -q.y;
        q.z = -q.z;
    }

    float sin_half = sqrtf(
        q.x*q.x +
        q.y*q.y +
        q.z*q.z
    );

    if (sin_half < QUATERNION_EPSILON) {
        return (vector3_t){
            .x = 2.0f * q.x,
            .y = 2.0f * q.y,
            .z = 2.0f * q.z
        };
    }

    float angle = 2.0f * atan2f(sin_half, q.w);
    float scale = angle / sin_half;

    return (vector3_t){
        .x = q.x * scale,
        .y = q.y * scale,
        .z = q.z * scale
    };
}