
#include "uart.h"
#include "rangefinder.h"

#include <stdio.h>

#define UART_DEVICE    "/dev/ttymxc3"
#define UART_BAUD      115200

int main(void)
{
    UART uart;
    float distance;

    // Initialize UART interface
    if (uart_init(&uart, UART_DEVICE, UART_BAUD) < 0)
    {
        perror("UART initialization failed");
        return 1;
    }

    // Initialize rangefinder protocol
    rangefinder_status_t status = Rangefinder_Init(&uart);

    if (status != kRangefinder_Success)
    {
        printf("Rangefinder initialization failed: %d\n", status);
        uart_close(&uart);
        return 1;
    }

    // Perform single measurement
    status = Rangefinder_ReadDistance(&distance);

    if (status == kRangefinder_Success)
    {
        printf("Distance: %.1f m\n", distance);
    }
    else
    {
        printf("Rangefinder measurement failed: %d\n", status);
    }

    uart_close(&uart);

    return (status == kRangefinder_Success) ? 0 : 1;
}
