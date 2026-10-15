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

int main(void)
{

    BOARD_InitHardware();

    PRINTF("MCUX SDK version: %s\r\n", MCUXSDK_VERSION_FULL_STR);

    PRINTF("hello world.\r\n");

    PRINTF("Starting FreeRTOS...");

    if (Tasks_Init() != pdPASS)
    {
        PRINTF("Task initialization failed\r\n");

        while (1);
    }

    vTaskStartScheduler();

    while (1);
}
