
#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <sys/types.h>

typedef struct {
    int fd;
} UART;

int uart_init(UART *uart, const char *device, int baudrate);
ssize_t uart_read(UART *uart, void *buffer, size_t length);
ssize_t uart_write(UART *uart, const void *buffer, size_t length);
void uart_close(UART *uart);

#endif
