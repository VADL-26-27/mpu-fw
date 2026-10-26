#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "control/attitude_estimator.h"

#define PI 3.14159265358979323846f
#define TOL 1e-4f

static int NearlyEqual(float a, float b)
{
    return fabsf(a - b) < TOL;
}

static void TestConstantYaw(void)
{
    attitude_estimator_t estimator;
    AttitudeEstimator_Init(&estimator);

    attitude_sample_t sample = {0};

    sample.gyro.z = PI / 2.0f;

    for (int i = 0; i < 100; i++)
    {
        AttitudeEstimator_Update(
            &estimator,
            &sample,
            0.01f
        );
    }

    quaternion_t q = estimator.orientation;

    assert(NearlyEqual(q.w, cosf(PI / 4.0f)));
    assert(NearlyEqual(q.x, 0.0f));
    assert(NearlyEqual(q.y, 0.0f));
    assert(NearlyEqual(q.z, sinf(PI / 4.0f)));
}

static void TestRollConvergence(void)
{
    attitude_estimator_t estimator;
    AttitudeEstimator_Init(&estimator);

    // Begin with a 30-degree roll error
    estimator.orientation = Quaternion_FromAxisAngle(
        (vector3_t){1.0f, 0.0f, 0.0f},
        PI / 6.0f
    );

    attitude_sample_t sample = {0};

    // Stationary, level IMU
    sample.accel.z = 9.80665f;

    // Simulate 5 seconds at 100 Hz
    for (int i = 0; i < 500; i++)
    {
        AttitudeEstimator_Update(
            &estimator,
            &sample,
            0.01f
        );
    }

    quaternion_t q = estimator.orientation;

    // Roll should converge toward zero
    assert(fabsf(q.x) < 1e-3f);
    assert(fabsf(q.y) < 1e-3f);
    assert(fabsf(q.z) < 1e-3f);
    assert(fabsf(q.w - 1.0f) < 1e-3f);
}

static void TestYawConvergence(void)
{
    attitude_estimator_t estimator;
    AttitudeEstimator_Init(&estimator);

    // Initial estimate has a 45-degree yaw error
    estimator.orientation = Quaternion_FromAxisAngle(
        (vector3_t){0.0f, 0.0f, 1.0f},
        PI / 4.0f
    );

    attitude_sample_t sample = {0};

    // Stationary, level IMU
    sample.accel.z = 9.80665f;

    // Magnetic field: northward with downward inclination
    sample.mag.x = 0.5f;
    sample.mag.y = 0.0f;
    sample.mag.z = -0.8660254f;

    // Simulate 30 seconds at 100 Hz
    for (int i = 0; i < 3000; i++)
    {
        AttitudeEstimator_Update(
            &estimator,
            &sample,
            0.01f
        );
    }

    quaternion_t q = estimator.orientation;

    printf(
        "Final quaternion: w=%.6f x=%.6f y=%.6f z=%.6f\n",
        q.w, q.x, q.y, q.z
    );

    assert(fabsf(q.w - 1.0f) < 1e-3f);
    assert(fabsf(q.x) < 1e-3f);
    assert(fabsf(q.y) < 1e-3f);
    assert(fabsf(q.z) < 1e-3f);
}

int main(void)
{
    TestConstantYaw();
    TestRollConvergence();
    TestYawConvergence();

    printf("All attitude estimator tests passed.\n");

    return 0;
}