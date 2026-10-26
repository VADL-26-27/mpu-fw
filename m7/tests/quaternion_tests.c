#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "control/quaternion.h"

#define PI 3.14159265358979323846f
#define TOL 1e-5f

static int NearlyEqual(float a, float b)
{
    return fabsf(a - b) < TOL;
}

static void TestIdentity(void)
{
    quaternion_t q = Quaternion_Identity();

    assert(NearlyEqual(q.w, 1.0f));
    assert(NearlyEqual(q.x, 0.0f));
    assert(NearlyEqual(q.y, 0.0f));
    assert(NearlyEqual(q.z, 0.0f));
}

static void TestNormalize(void)
{
    quaternion_t q = {2.0f, 0.0f, 0.0f, 0.0f};

    q = Quaternion_Normalize(q);

    assert(NearlyEqual(Quaternion_Norm(q), 1.0f));
    assert(NearlyEqual(q.w, 1.0f));
}

static void TestConjugate(void)
{
    quaternion_t q = {1.0f, 2.0f, 3.0f, 4.0f};

    quaternion_t qc = Quaternion_Conjugate(q);

    assert(NearlyEqual(qc.w,  1.0f));
    assert(NearlyEqual(qc.x, -2.0f));
    assert(NearlyEqual(qc.y, -3.0f));
    assert(NearlyEqual(qc.z, -4.0f));
}

static void TestMultiply(void)
{
    quaternion_t q = Quaternion_FromAxisAngle(
        (vector3_t){0.0f, 0.0f, 1.0f},
        PI / 2.0f
    );

    quaternion_t result = Quaternion_Multiply(
        q,
        Quaternion_Conjugate(q)
    );

    assert(NearlyEqual(result.w, 1.0f));
    assert(NearlyEqual(result.x, 0.0f));
    assert(NearlyEqual(result.y, 0.0f));
    assert(NearlyEqual(result.z, 0.0f));
}

static void TestRotateVector(void)
{
    quaternion_t q = Quaternion_FromAxisAngle(
        (vector3_t){0.0f, 0.0f, 1.0f},
        PI / 2.0f
    );

    vector3_t result = Quaternion_RotateVector(
        q,
        (vector3_t){1.0f, 0.0f, 0.0f}
    );

    assert(NearlyEqual(result.x, 0.0f));
    assert(NearlyEqual(result.y, 1.0f));
    assert(NearlyEqual(result.z, 0.0f));
}

static void TestRotationVector(void)
{
    quaternion_t q = Quaternion_FromAxisAngle(
        (vector3_t){0.0f, 0.0f, 1.0f},
        PI / 6.0f
    );

    vector3_t result = Quaternion_ToRotationVector(q);

    assert(NearlyEqual(result.x, 0.0f));
    assert(NearlyEqual(result.y, 0.0f));
    assert(NearlyEqual(result.z, PI / 6.0f));
}

int main(void)
{
    TestIdentity();
    TestNormalize();
    TestConjugate();
    TestMultiply();
    TestRotateVector();
    TestRotationVector();

    printf("All quaternion tests passed.\n");

    return 0;
}