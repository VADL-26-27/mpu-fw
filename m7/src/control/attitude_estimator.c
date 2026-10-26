#include "control/attitude_estimator.h"

#include <math.h>
#include <stddef.h>

void AttitudeEstimator_Init(
    attitude_estimator_t *estimator
)
{
    if (estimator == NULL)
        return;

    estimator->orientation = Quaternion_Identity();

    estimator->gyro_bias = (vector3_t){
        .x = 0.0f,
        .y = 0.0f,
        .z = 0.0f
    };

    estimator->kp = 2.0f;
    estimator->ki = 0.0f;
}

quaternion_t AttitudeEstimator_Update(
    attitude_estimator_t *estimator,
    const attitude_sample_t *sample,
    float dt
)
{
    if (estimator == NULL || sample == NULL ||
        !isfinite(dt) || dt <= 0.0f)
    {
        return estimator != NULL
            ? estimator->orientation
            : Quaternion_Identity();
    }

    vector3_t omega = {
        .x = sample->gyro.x - estimator->gyro_bias.x,
        .y = sample->gyro.y - estimator->gyro_bias.y,
        .z = sample->gyro.z - estimator->gyro_bias.z
    };

    vector3_t error = {0.0f, 0.0f, 0.0f};

    quaternion_t q = estimator->orientation;
    quaternion_t q_inv = Quaternion_Conjugate(q);

    // acceleromter correction -----------------------------------------------

    float accel_mag = sqrtf(
        sample->accel.x * sample->accel.x +
        sample->accel.y * sample->accel.y +
        sample->accel.z * sample->accel.z
    );

    if (isfinite(accel_mag) && accel_mag > 1e-6f)
    {
        vector3_t a = {
            sample->accel.x / accel_mag,
            sample->accel.y / accel_mag,
            sample->accel.z / accel_mag
        };

        vector3_t up_world = {0.0f, 0.0f, 1.0f};

        vector3_t up_predicted = Quaternion_RotateVector(
            q_inv,
            up_world
        );

        error.x += a.y * up_predicted.z
                 - a.z * up_predicted.y;

        error.y += a.z * up_predicted.x
                 - a.x * up_predicted.z;

        error.z += a.x * up_predicted.y
                 - a.y * up_predicted.x;
    }

    // magenetometer correction -------------------------------------------------

    float mag_mag = sqrtf(
        sample->mag.x * sample->mag.x +
        sample->mag.y * sample->mag.y +
        sample->mag.z * sample->mag.z
    );

    if (isfinite(mag_mag) && mag_mag > 1e-6f)
    {
        vector3_t m = {
            sample->mag.x / mag_mag,
            sample->mag.y / mag_mag,
            sample->mag.z / mag_mag
        };

        // Rotate measured magnetic field into world frame
        vector3_t h = Quaternion_RotateVector(q, m);

        // Magnetic reference in world frame
        vector3_t b = {
            .x = sqrtf(h.x * h.x + h.y * h.y),
            .y = 0.0f,
            .z = h.z
        };

        // Predict magnetic field in body frame
        vector3_t mag_predicted = Quaternion_RotateVector(
            q_inv,
            b
        );

        // Measured cross predicted
        error.x += m.y * mag_predicted.z
                 - m.z * mag_predicted.y;

        error.y += m.z * mag_predicted.x
                 - m.x * mag_predicted.z;

        error.z += m.x * mag_predicted.y
                 - m.y * mag_predicted.x;
    }

    // P feedback

    omega.x += estimator->kp * error.x;
    omega.y += estimator->kp * error.y;
    omega.z += estimator->kp * error.z;

    // quaternion integration

    float omega_mag = sqrtf(
        omega.x * omega.x +
        omega.y * omega.y +
        omega.z * omega.z
    );

    if (!isfinite(omega_mag) || omega_mag < 1e-6f)
        return estimator->orientation;

    quaternion_t dq = Quaternion_FromAxisAngle(
        omega,
        omega_mag * dt
    );

    estimator->orientation = Quaternion_Normalize(
        Quaternion_Multiply(q, dq)
    );

    return estimator->orientation;
}