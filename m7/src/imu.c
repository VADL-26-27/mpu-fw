#include "imu.h"
#include "lsm9ds1.h"

#define IMU_ACCEL_OUT_REG   0x28U
#define IMU_GYRO_OUT_REG    0x18U
#define IMU_MAG_OUT_REG     0x28U

// Sensitivities for current LSM9DS1 configuration
#define IMU_ACCEL_SCALE     (0.000061f * 9.80665f) // m/s^2 per LSB
#define IMU_GYRO_SCALE      (0.00875f * 0.01745329252f) // rad/s per LSB
#define IMU_MAG_SCALE       (0.014f) // uT per LSB

static int16_t IMU_DecodeAxis(const uint8_t *data)
{
    uint16_t raw = (uint16_t)data[0] |
                   ((uint16_t)data[1] << 8);

    return (int16_t)raw;
}

static void IMU_ConvertAxes(
    const uint8_t *raw,
    float scale,
    imu_vector3_t *vector)
{
    vector->x = (float)IMU_DecodeAxis(&raw[0]) * scale;
    vector->y = (float)IMU_DecodeAxis(&raw[2]) * scale;
    vector->z = (float)IMU_DecodeAxis(&raw[4]) * scale;
}

status_t IMU_Init(void)
{
    return LSM9DS1_Init();
}

status_t IMU_ReadAccel(imu_vector3_t *accel)
{
    if (accel == NULL)
        return kStatus_InvalidArgument;

    uint8_t raw[6];

    status_t status = LSM9DS1_ReadRegisters(
        LSM9DS1_AG,
        IMU_ACCEL_OUT_REG,
        raw,
        sizeof(raw)
    );

    if (status != kStatus_Success)
        return status;

    IMU_ConvertAxes(raw, IMU_ACCEL_SCALE, accel);

    return kStatus_Success;
}

status_t IMU_ReadGyro(imu_vector3_t *gyro)
{
    if (gyro == NULL)
        return kStatus_InvalidArgument;

    uint8_t raw[6];

    status_t status = LSM9DS1_ReadRegisters(
        LSM9DS1_AG,
        IMU_GYRO_OUT_REG,
        raw,
        sizeof(raw)
    );

    if (status != kStatus_Success)
        return status;

    IMU_ConvertAxes(raw, IMU_GYRO_SCALE, gyro);

    return kStatus_Success;
}

status_t IMU_ReadMag(imu_vector3_t *mag)
{
    if (mag == NULL)
        return kStatus_InvalidArgument;

    uint8_t raw[6];

    status_t status = LSM9DS1_ReadRegisters(
        LSM9DS1_MAG,
        IMU_MAG_OUT_REG,
        raw,
        sizeof(raw)
    );

    if (status != kStatus_Success)
        return status;

    IMU_ConvertAxes(raw, IMU_MAG_SCALE, mag);

    return kStatus_Success;
}

status_t IMU_ReadSample(imu_sample_t *sample)
{
    if (sample == NULL)
        return kStatus_InvalidArgument;

    status_t status;

    status = IMU_ReadAccel(&sample->accel);
    if (status != kStatus_Success)
        return status;

    status = IMU_ReadGyro(&sample->gyro);
    if (status != kStatus_Success)
        return status;

    status = IMU_ReadMag(&sample->mag);
    if (status != kStatus_Success)
        return status;

    return kStatus_Success;
}