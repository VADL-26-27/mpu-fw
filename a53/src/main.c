
#include "uart.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define UART_DEVICE "/dev/ttymxc3"
#define UART_BAUD 115200

int main(void)
{
    UART uart;

    if (uart_init(&uart, UART_DEVICE, UART_BAUD) < 0) {
        perror("UART initialization failed");
        return 1;
    }

    // Transmit
    const char *message = "Hello UART!\r\n";

    if (uart_write(&uart, message, strlen(message)) < 0) {
        perror("UART write failed");
    }

    // Receive
    uint8_t buffer[64];

    ssize_t bytes = uart_read(&uart, buffer, sizeof(buffer));

    if (bytes > 0) {
        printf("Received %zd bytes:\n", bytes);

        for (ssize_t i = 0; i < bytes; i++)
            printf("%02X ", buffer[i]);

        printf("\n");
    }

    uart_close(&uart);
    return 0;
}
