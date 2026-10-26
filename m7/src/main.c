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
#include "control/attitude_estimator.h"

static void PrintSignedScaled(float value)
{
    int32_t scaled = (int32_t)(value * 1000.0f);

    if (scaled < 0)
    {
        PRINTF("-%u", (unsigned int)(-scaled));
    }
    else
    {
        PRINTF("%u", (unsigned int)scaled);
    }
}

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

        attitude_estimator_t estimator;
    AttitudeEstimator_Init(&estimator);

    uint32_t count = 0;

    PRINTF("Starting attitude estimator superloop\r\n");

    while (1)
    {
        imu_sample_t raw;

        status_t readStatus = IMU_ReadSample(&raw);

        if (readStatus == kStatus_Success)
        {
            // Convert hardware sample to estimator input
            attitude_sample_t attitudeSample = {
                .accel = {
                    raw.accel.x,
                    raw.accel.y,
                    raw.accel.z
                },
                .gyro = {
                    raw.gyro.x,
                    raw.gyro.y,
                    raw.gyro.z
                },
                .mag = {
                    raw.mag.x,
                    raw.mag.y,
                    raw.mag.z
                }
            };

            quaternion_t q = AttitudeEstimator_Update(
                &estimator,
                &attitudeSample,
                0.01f
            );

            // Print approximately every 20 updates
            if (++count >= 20)
            {
                count = 0;

                PRINTF("Q: ");

                PrintSignedScaled(q.w);
                PRINTF(" ");

                PrintSignedScaled(q.x);
                PRINTF(" ");

                PrintSignedScaled(q.y);
                PRINTF(" ");

                PrintSignedScaled(q.z);
                PRINTF("\r\n");
            }
        }

        // Approximate 100 Hz sampling
        SDK_DelayAtLeastUs(10000U, SystemCoreClock);
    }
}
