
#include "uart.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int uart_init(UART *uart, const char *device, int baudrate)
{
    if (!uart || !device)
        return -1;

    uart->fd = -1;

    speed_t speed;

    switch (baudrate) {
        case 9600:   speed = B9600;   break;
        case 115200: speed = B115200; break;
        default: return -1;
    }

    int fd = open(device, O_RDWR | O_NOCTTY);
    if (fd < 0)
        return -1;

    struct termios tty;

    if (tcgetattr(fd, &tty) < 0)
        goto error;

    // Raw mode: no terminal line processing
    cfmakeraw(&tty);

    // 8 data bits, no parity, 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8 | CLOCAL | CREAD;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~PARENB;

    // Blocking read: return after at least one byte
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (cfsetispeed(&tty, speed) < 0 ||
        cfsetospeed(&tty, speed) < 0)
        goto error;

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        goto error;

    uart->fd = fd;
    return 0;

error:
    close(fd);
    return -1;
}

ssize_t uart_read(UART *uart, void *buffer, size_t length)
{
    return read(uart->fd, buffer, length);
}

ssize_t uart_write(UART *uart, const void *buffer, size_t length)
{
    return write(uart->fd, buffer, length);
}

void uart_close(UART *uart)
{
    if (uart && uart->fd >= 0) {
        close(uart->fd);
        uart->fd = -1;
    }
}
