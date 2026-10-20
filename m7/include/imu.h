#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#include "fsl_common.h"

typedef struct
{
    float x;
    float y;
    float z;
} imu_vector3_t;

typedef struct
{
    imu_vector3_t accel; // m/s^2
    imu_vector3_t gyro; // rad/s
    imu_vector3_t mag; // uT
} imu_sample_t;

status_t IMU_Init(void);

status_t IMU_ReadAccel(imu_vector3_t *accel);
status_t IMU_ReadGyro(imu_vector3_t *gyro);
status_t IMU_ReadMag(imu_vector3_t *mag);

status_t IMU_ReadSample(imu_sample_t *sample);

#endif // IMU_H