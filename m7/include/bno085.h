
#ifndef BNO085_H
#define BNO085_H

#include <stdbool.h>
#include <stdint.h>

#include "fsl_common.h"
#include "control/quaternion.h"
#include "control/vector3.h"

typedef struct
{
    quaternion_t orientation;
    vector3_t angular_velocity;

    uint8_t orientation_accuracy;
    uint8_t gyro_accuracy;

    bool orientation_valid;
    bool gyro_valid;

    uint32_t orientation_count;
    uint32_t gyro_count;

} bno085_sample_t;

status_t BNO085_Init(void);
status_t BNO085_Process(void);
status_t BNO085_Read(bno085_sample_t *sample);

bool BNO085_InterruptActive(void);
bool BNO085_ReadNewSample(bno085_sample_t *sample);

status_t BNO085_SetReportIntervals(
    uint32_t rotation_us,
    uint32_t gyro_us
);

#endif
