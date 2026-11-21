
#include "bno085.h"

#include "fsl_ecspi.h"
#include "fsl_gpio.h"
#include "fsl_debug_console.h"

status_t BNO085_ReadPacket(uint8_t *packet, uint16_t *length)
{
    if (packet == NULL || length == NULL)
        return kStatus_InvalidArgument;

    // Check H_INTN (active low)
    if (GPIO_PinRead(GPIO3, 6U) != 0U)
        return kStatus_Fail;

    uint32_t tx[4] = {0};
    uint32_t rx[4] = {0};

    ecspi_transfer_t transfer = {
        .txData = tx,
        .rxData = rx,
        .dataSize = 4,
        .channel = kECSPI_Channel0
    };

    // Assert CS
    GPIO_PinWrite(GPIO5, 13U, 0U);

    // Read SHTP header
    status_t status =
        ECSPI_MasterTransferBlocking(ECSPI2, &transfer);

    if (status != kStatus_Success)
    {
        GPIO_PinWrite(GPIO5, 13U, 1U);
        return status;
    }

    for (uint8_t i = 0; i < 4U; i++)
    {
        packet[i] = (uint8_t)(rx[i] & 0xFFU);
    }

    // Decode packet length
    uint16_t packetLength =
        ((uint16_t)packet[1] << 8) | packet[0];

    packetLength &= 0x7FFFU;

    if (packetLength < 4U ||
        packetLength > BNO085_MAX_PACKET_SIZE)
    {
        GPIO_PinWrite(GPIO5, 13U, 1U);
        return kStatus_Fail;
    }

    uint16_t payloadLength = packetLength - 4U;

    if (payloadLength > 0U)
    {
        uint32_t payloadTx[payloadLength];
        uint32_t payloadRx[payloadLength];

        for (uint16_t i = 0; i < payloadLength; i++)
        {
            payloadTx[i] = 0U;
            payloadRx[i] = 0U;
        }

        ecspi_transfer_t payloadTransfer = {
            .txData = payloadTx,
            .rxData = payloadRx,
            .dataSize = payloadLength,
            .channel = kECSPI_Channel0
        };

        // Read payload
        status = ECSPI_MasterTransferBlocking(ECSPI2, &payloadTransfer);

        if (status != kStatus_Success)
        {
            GPIO_PinWrite(GPIO5, 13U, 1U);
            return status;
        }

        // Copy payload after 4-byte header
        for (uint16_t i = 0; i < payloadLength; i++)
        {
            packet[i + 4U] = (uint8_t)(payloadRx[i] & 0xFFU);
        }
    }

    // Deassert CS
    GPIO_PinWrite(GPIO5, 13U, 1U);

    *length = packetLength;

    return kStatus_Success;
}
