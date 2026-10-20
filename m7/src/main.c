/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "hardware_init.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tasks.h"
#include "fsl_ecspi.h"
#include <stdint.h>
#include "lsm9ds1.h"
#include "fsl_gpio.h"
#include "imu.h"

// static void SPI_LoopbackTest(void)
// {
//     uint32_t txData = 0xA55AU;
//     uint32_t rxData = 0U;

//     ecspi_transfer_t transfer = {
//         .txData = &txData,
//         .rxData = &rxData,
//         .dataSize = 1U,
//         .channel = kECSPI_Channel0
//     };

//     status_t status = ECSPI_MasterTransferBlocking(ECSPI2, &transfer);

//     PRINTF("SPI TX: 0x%04X\r\n", (unsigned int)txData);
//     PRINTF("SPI RX: 0x%04X\r\n", (unsigned int)rxData);
//     PRINTF("SPI status: %d\r\n", (int)status);

//     if ((status == kStatus_Success) && ((rxData & 0xFFFFU) == txData))
//     {
//         PRINTF("SPI LOOPBACK PASS\r\n");
//     }
//     else
//     {
//         PRINTF("SPI LOOPBACK FAIL\r\n");
//     }
// }

int main(void)
{

    BOARD_InitHardware();

    PRINTF("MCUX SDK version: %s\r\n", MCUXSDK_VERSION_FULL_STR);

    PRINTF("hello world.\r\n");

    uint8_t agId = 0;
    uint8_t magId = 0;

    status_t agStatus = LSM9DS1_ReadData(LSM9DS1_AG, 0x0FU, &agId);
    status_t magStatus = LSM9DS1_ReadData(LSM9DS1_MAG, 0x0FU, &magId);

    PRINTF("AG WHO_AM_I: 0x%02X (status %d)\r\n", agId, agStatus);
    PRINTF("MAG WHO_AM_I: 0x%02X (status %d)\r\n", magId, magStatus);

    status_t status = IMU_Init();

    if (status != kStatus_Success)
    {
        PRINTF("IMU initialization failed: %u\r\n",
            (unsigned int)status);

        while (1);
    }

    PRINTF("IMU initialized successfully\r\n");

    imu_sample_t sample;

    status = IMU_ReadSample(&sample);

    if (status == kStatus_Success)
    {
        // Print scaled integer values to avoid SDK float
        // and signed-formatting limitations.

        PRINTF("Accel X magnitude (mm/s^2): %u\r\n",
            (unsigned int)(
                sample.accel.x < 0.0f
                ? -sample.accel.x * 1000.0f
                : sample.accel.x * 1000.0f));

        PRINTF("Gyro X magnitude (mrad/s): %u\r\n",
            (unsigned int)(
                sample.gyro.x < 0.0f
                ? -sample.gyro.x * 1000.0f
                : sample.gyro.x * 1000.0f));

        PRINTF("Mag X magnitude (uT): %u\r\n",
            (unsigned int)(
                sample.mag.x < 0.0f
                ? -sample.mag.x
                : sample.mag.x));
    }
    else
    {
        PRINTF("IMU read failed: %u\r\n",
            (unsigned int)status);
    }

    // PRINTF("Starting FreeRTOS...");

    // if (Tasks_Init() != pdPASS)
    // {
    //     PRINTF("Task initialization failed\r\n");

    //     while (1);
    // }

    // vTaskStartScheduler();

    while (1);
}
