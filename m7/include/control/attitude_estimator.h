#ifndef ATTITUDE_ESTIMATOR_H
#define ATTITUDE_ESTIMATOR_H

#include "control/quaternion.h"
#include "control/vector3.h"

typedef struct {
    vector3_t accel;  // m/s^2
    vector3_t gyro;   // rad/s
    vector3_t mag;    // uT
} attitude_sample_t;

typedef struct {
    quaternion_t orientation;
    vector3_t gyro_bias;
    float kp;
    float ki;
} attitude_estimator_t;

void AttitudeEstimator_Init(
    attitude_estimator_t *estimator
);

quaternion_t AttitudeEstimator_Update(
    attitude_estimator_t *estimator,
    const attitude_sample_t *sample,
    float dt
);

#endif