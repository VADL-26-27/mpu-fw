
#ifndef RANGEFINDER_H
#define RANGEFINDER_H

#include <stdint.h>
#include "uart.h"

typedef enum
{
    kRangefinder_Success = 0,
    kRangefinder_Fail,
    kRangefinder_InvalidArgument,
    kRangefinder_CommunicationError,
    kRangefinder_InvalidResponse,
    kRangefinder_Timeout
} rangefinder_status_t;

typedef enum
{
    kRangefinder_1Hz = 0x89U,
    kRangefinder_2Hz = 0xA9U,
    kRangefinder_5Hz = 0xB9U
} rangefinder_frequency_t;

rangefinder_status_t Rangefinder_Init(UART *uart);

rangefinder_status_t Rangefinder_ReadDistance(float *distance);

rangefinder_status_t Rangefinder_StartContinuous(
    rangefinder_frequency_t frequency);

rangefinder_status_t Rangefinder_ReadMeasurement(float *distance);

rangefinder_status_t Rangefinder_StopContinuous(void);

#endif
