
#include "rangefinder.h"

#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>

#define RANGEFINDER_HEADER_1       0x55U
#define RANGEFINDER_HEADER_2       0xAAU

#define RANGEFINDER_SINGLE_CMD     0x88U
#define RANGEFINDER_STOP_CMD       0x8EU

#define RANGEFINDER_PACKET_SIZE    8U
#define RANGEFINDER_TIMEOUT_MS     3000

static UART *s_uart = NULL;
static bool s_continuous = false;
static uint8_t s_frequency = 0U;

static uint8_t Rangefinder_Checksum(
    const uint8_t *data,
    size_t start,
    size_t end)
{
    uint8_t checksum = 0U;

    for (size_t i = start; i < end; i++)
    {
        checksum = (uint8_t)(checksum + data[i]);
    }

    return checksum;
}

static int64_t Rangefinder_GetTimeMs(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        return -1;

    return (int64_t)ts.tv_sec * 1000 +
           ts.tv_nsec / 1000000;
}

static rangefinder_status_t Rangefinder_SendCommand(
    uint8_t command)
{
    if (s_uart == NULL || s_uart->fd < 0)
        return kRangefinder_InvalidArgument;

    uint8_t packet[RANGEFINDER_PACKET_SIZE] = {
        RANGEFINDER_HEADER_1,
        RANGEFINDER_HEADER_2,
        command,
        0xFFU,
        0xFFU,
        0xFFU,
        0xFFU,
        0U
    };

    // TX checksum covers bytes 3 through 7.
    packet[7] = Rangefinder_Checksum(packet, 2U, 7U);

    size_t written = 0U;

    while (written < RANGEFINDER_PACKET_SIZE)
    {
        ssize_t result = uart_write(
            s_uart,
            packet + written,
            RANGEFINDER_PACKET_SIZE - written);

        if (result < 0)
        {
            if (errno == EINTR)
                continue;

            return kRangefinder_CommunicationError;
        }

        if (result == 0)
            return kRangefinder_CommunicationError;

        written += (size_t)result;
    }

    return kRangefinder_Success;
}

static rangefinder_status_t Rangefinder_ReadPacket(
    uint8_t expectedCommand,
    uint8_t *packet)
{
    if (s_uart == NULL || s_uart->fd < 0 || packet == NULL)
        return kRangefinder_InvalidArgument;

    int64_t startTime = Rangefinder_GetTimeMs();

    if (startTime < 0)
        return kRangefinder_CommunicationError;

    int64_t deadline = startTime + RANGEFINDER_TIMEOUT_MS;
    size_t received = 0U;

    while (received < RANGEFINDER_PACKET_SIZE)
    {
        int64_t now = Rangefinder_GetTimeMs();

        if (now < 0)
            return kRangefinder_CommunicationError;

        if (now >= deadline)
            return kRangefinder_Timeout;

        struct pollfd pfd = {
            .fd = s_uart->fd,
            .events = POLLIN
        };

        int result = poll(
            &pfd, 1, (int)(deadline - now));

        if (result == 0)
            return kRangefinder_Timeout;

        if (result < 0)
        {
            if (errno == EINTR)
                continue;

            return kRangefinder_CommunicationError;
        }

        if (pfd.revents & (POLLERR | POLLNVAL | POLLHUP))
            return kRangefinder_CommunicationError;

        uint8_t byte;

        result = (int)uart_read(s_uart, &byte, 1U);

        if (result > 0)
        {
            printf("RX: 0x%02X\n", byte);
        }

        if (result < 0)
        {
            if (errno == EINTR)
                continue;

            return kRangefinder_CommunicationError;
        }

        if (result == 0)
            continue;

        // Synchronize on 0x55 0xAA.
        if (received == 0U && byte != RANGEFINDER_HEADER_1)
            continue;

        if (received == 1U && byte != RANGEFINDER_HEADER_2)
        {
            received = (byte == RANGEFINDER_HEADER_1) ? 1U : 0U;
            continue;
        }

        packet[received++] = byte;
    }

    if (packet[2] != expectedCommand)
        return kRangefinder_InvalidResponse;

    // RX checksum covers bytes 1 through 7.
    if (packet[7] != Rangefinder_Checksum(packet, 0U, 7U))
        return kRangefinder_InvalidResponse;

    return kRangefinder_Success;
}

static rangefinder_status_t Rangefinder_DecodeDistance(
    const uint8_t *packet,
    float *distance)
{
    if (packet == NULL || distance == NULL)
        return kRangefinder_InvalidArgument;

    if (packet[3] != 0x01U)
        return kRangefinder_Fail;

    uint16_t raw =
        ((uint16_t)packet[5] << 8) |
        (uint16_t)packet[6];

    // Sensor reports distance in units of 0.1 m.
    *distance = (float)raw / 10.0f;

    return kRangefinder_Success;
}

rangefinder_status_t Rangefinder_Init(UART *uart)
{
    if (uart == NULL || uart->fd < 0)
        return kRangefinder_InvalidArgument;

    s_uart = uart;
    s_continuous = false;
    s_frequency = 0U;

    return kRangefinder_Success;
}

rangefinder_status_t Rangefinder_ReadDistance(float *distance)
{
    if (distance == NULL)
        return kRangefinder_InvalidArgument;

    if (s_uart == NULL || s_continuous)
        return kRangefinder_Fail;

    rangefinder_status_t status =
        Rangefinder_SendCommand(RANGEFINDER_SINGLE_CMD);

    if (status != kRangefinder_Success)
        return status;

    uint8_t packet[RANGEFINDER_PACKET_SIZE];

    status = Rangefinder_ReadPacket(
        RANGEFINDER_SINGLE_CMD, packet);

    if (status != kRangefinder_Success)
        return status;

    return Rangefinder_DecodeDistance(packet, distance);
}

rangefinder_status_t Rangefinder_StartContinuous(
    rangefinder_frequency_t frequency)
{
    if (s_uart == NULL || s_continuous)
        return kRangefinder_Fail;

    if (frequency != kRangefinder_1Hz &&
        frequency != kRangefinder_2Hz &&
        frequency != kRangefinder_5Hz)
    {
        return kRangefinder_InvalidArgument;
    }

    rangefinder_status_t status =
        Rangefinder_SendCommand((uint8_t)frequency);

    if (status != kRangefinder_Success)
        return status;

    s_frequency = (uint8_t)frequency;
    s_continuous = true;

    return kRangefinder_Success;
}

rangefinder_status_t Rangefinder_ReadMeasurement(float *distance)
{
    if (distance == NULL)
        return kRangefinder_InvalidArgument;

    if (s_uart == NULL || !s_continuous)
        return kRangefinder_Fail;

    uint8_t packet[RANGEFINDER_PACKET_SIZE];

    rangefinder_status_t status =
        Rangefinder_ReadPacket(s_frequency, packet);

    if (status != kRangefinder_Success)
        return status;

    return Rangefinder_DecodeDistance(packet, distance);
}

rangefinder_status_t Rangefinder_StopContinuous(void)
{
    if (s_uart == NULL || !s_continuous)
        return kRangefinder_Fail;

    rangefinder_status_t status =
        Rangefinder_SendCommand(RANGEFINDER_STOP_CMD);

    if (status != kRangefinder_Success)
        return status;

    uint8_t packet[RANGEFINDER_PACKET_SIZE];

    status = Rangefinder_ReadPacket(
        RANGEFINDER_STOP_CMD, packet);

    if (status != kRangefinder_Success)
        return status;

    if (packet[3] != 0x01U)
        return kRangefinder_Fail;

    s_continuous = false;
    s_frequency = 0U;

    return kRangefinder_Success;
}
