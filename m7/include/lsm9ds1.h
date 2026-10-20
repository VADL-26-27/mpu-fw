#ifndef LSM9DS1_H
#define LSM9DS1_H



#include <stdint.h>
#include "fsl_common.h"

typedef enum {
    LSM9DS1_AG,
    LSM9DS1_MAG
} lsm9ds1_device_t;

status_t LSM9DS1_WriteData(lsm9ds1_device_t device, uint8_t reg, uint8_t data);
status_t LSM9DS1_ReadData(lsm9ds1_device_t device, uint8_t reg, uint8_t *data);
status_t LSM9DS1_Init(void);

#endif // LSM9DS1_H