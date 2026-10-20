#include "lsm9ds1.h"
#include "fsl_ecspi.h"


#include "lsm9ds1.h"
#include "fsl_ecspi.h"
#include "fsl_gpio.h"

#define LSM9DS1_WHO_AM_I       0x0FU

#define LSM9DS1_CTRL_REG1_G    0x10U
#define LSM9DS1_CTRL_REG6_XL   0x20U
#define LSM9DS1_CTRL_REG8      0x22U

#define LSM9DS1_CTRL_REG1_M    0x20U
#define LSM9DS1_CTRL_REG2_M    0x21U
#define LSM9DS1_CTRL_REG3_M    0x22U
#define LSM9DS1_CTRL_REG4_M    0x23U
#define LSM9DS1_CTRL_REG5_M    0x24U

static void LSM9DS1_SetChipSelect(lsm9ds1_device_t device,
                                  bool active)
{
    uint8_t level = active ? 0U : 1U;

    switch (device)
    {
        case LSM9DS1_AG:
            GPIO_PinWrite(GPIO5, 13U, level);
            break;

        case LSM9DS1_MAG:
            GPIO_PinWrite(GPIO3, 6U, level);
            break;

        default:
            break;
    }
}

status_t LSM9DS1_WriteData(lsm9ds1_device_t device,
                           uint8_t reg,
                           uint8_t data)
{
    uint8_t command;

    switch (device)
    {
        case LSM9DS1_AG:
            command = reg & 0x7FU;
            break;

        case LSM9DS1_MAG:
            command = reg & 0x3FU;
            break;

        default:
            return kStatus_InvalidArgument;
    }

    uint32_t txData = ((uint32_t)command << 8) | data;

    ecspi_transfer_t transfer = {
        .txData = &txData,
        .rxData = NULL,
        .dataSize = 1U,
        .channel = kECSPI_Channel0
    };

    LSM9DS1_SetChipSelect(device, true);

    status_t status = ECSPI_MasterTransferBlocking(ECSPI2, &transfer);

    LSM9DS1_SetChipSelect(device, false);

    return status;
}

status_t LSM9DS1_ReadData(lsm9ds1_device_t device,
                          uint8_t reg,
                          uint8_t *data)
{
    if (data == NULL)
    {
        return kStatus_InvalidArgument;
    }

    uint8_t command;

    switch (device)
    {
        case LSM9DS1_AG:
            command = (reg & 0x7FU) | 0x80U;
            break;

        case LSM9DS1_MAG:
            command = (reg & 0x3FU) | 0x80U;
            break;

        default:
            return kStatus_InvalidArgument;
    }

    uint32_t txData = (uint32_t)command << 8;
    uint32_t rxData = 0U;

    ecspi_transfer_t transfer = {
        .txData = &txData,
        .rxData = &rxData,
        .dataSize = 1U,
        .channel = kECSPI_Channel0
    };

    LSM9DS1_SetChipSelect(device, true);

    status_t status = ECSPI_MasterTransferBlocking(ECSPI2, &transfer);

    LSM9DS1_SetChipSelect(device, false);

    if (status != kStatus_Success)
    {
        return status;
    }

    *data = (uint8_t)(rxData & 0xFFU);

    return kStatus_Success;
}

status_t LSM9DS1_Init(void)
{
    status_t status;
    uint8_t agId;
    uint8_t magId;

    // Verify device identities
    status = LSM9DS1_ReadData(LSM9DS1_AG, LSM9DS1_WHO_AM_I, &agId);
    if (status != kStatus_Success)
        return status;

    if (agId != 0x68U)
        return kStatus_Fail;

    status = LSM9DS1_ReadData(LSM9DS1_MAG, LSM9DS1_WHO_AM_I, &magId);
    if (status != kStatus_Success)
        return status;

    if (magId != 0x3DU)
        return kStatus_Fail;

    // Accelerometer / gyroscope configuration
    status = LSM9DS1_WriteData(LSM9DS1_AG, LSM9DS1_CTRL_REG1_G, 0x60U);
    if (status != kStatus_Success)
        return status;

    status = LSM9DS1_WriteData(LSM9DS1_AG, LSM9DS1_CTRL_REG6_XL, 0x60U);
    if (status != kStatus_Success)
        return status;

    status = LSM9DS1_WriteData(LSM9DS1_AG, LSM9DS1_CTRL_REG8, 0x44U);
    if (status != kStatus_Success)
        return status;

    // Magnetometer configuration
    status = LSM9DS1_WriteData(LSM9DS1_MAG, LSM9DS1_CTRL_REG1_M, 0x70U);
    if (status != kStatus_Success)
        return status;

    status = LSM9DS1_WriteData(LSM9DS1_MAG, LSM9DS1_CTRL_REG2_M, 0x00U);
    if (status != kStatus_Success)
        return status;

    status = LSM9DS1_WriteData(LSM9DS1_MAG, LSM9DS1_CTRL_REG3_M, 0x00U);
    if (status != kStatus_Success)
        return status;

    status = LSM9DS1_WriteData(LSM9DS1_MAG, LSM9DS1_CTRL_REG4_M, 0x0CU);
    if (status != kStatus_Success)
        return status;

    status = LSM9DS1_WriteData(LSM9DS1_MAG, LSM9DS1_CTRL_REG5_M, 0x40U);
    if (status != kStatus_Success)
        return status;

    return kStatus_Success;
}

status_t LSM9DS1_ReadRegisters(
    lsm9ds1_device_t device,
    uint8_t startReg,
    uint8_t *buffer,
    size_t length)
{
    if (buffer == NULL || length == 0U || length > 6U)
        return kStatus_InvalidArgument;

    uint8_t command;

    switch (device)
    {
        case LSM9DS1_AG:
            command = (startReg & 0x7FU) | 0x80U;
            break;

        case LSM9DS1_MAG:
            command = (startReg & 0x3FU) | 0xC0U;
            break;

        default:
            return kStatus_InvalidArgument;
    }

    // Command + up to six response bytes, packed MSB-first.
    uint32_t txData[2] = {0U, 0U};
    uint32_t rxData[2] = {0U, 0U};

    const size_t totalBytes = length + 1U;
    const size_t wordCount = (totalBytes + 3U) / 4U;

    // ECSPI shifts the most significant bits of the burst first.
    // The first FIFO word contains the earliest transmitted bits.
    size_t firstWordBytes = totalBytes - (wordCount - 1U) * 4U;
    txData[0] = (uint32_t)command << ((firstWordBytes - 1U) * 8U);

    uint32_t originalConreg = ECSPI2->CONREG;

    ECSPI2->CONREG =
        (originalConreg & ~ECSPI_CONREG_BURST_LENGTH_MASK) |
        ECSPI_CONREG_BURST_LENGTH(totalBytes * 8U - 1U);

    ecspi_transfer_t transfer = {
        .txData = txData,
        .rxData = rxData,
        .dataSize = wordCount,
        .channel = kECSPI_Channel0
    };

    LSM9DS1_SetChipSelect(device, true);

    status_t status = ECSPI_MasterTransferBlocking(
        ECSPI2, &transfer);

    LSM9DS1_SetChipSelect(device, false);

    ECSPI2->CONREG = originalConreg;

    if (status != kStatus_Success)
        return status;

    // Extract response bytes, excluding the command phase.
    for (size_t i = 0; i < length; i++)
    {
        size_t byteIndex = i + 1U;
        size_t wordIndex;
        size_t bytePosition;

        if (byteIndex < firstWordBytes)
        {
            wordIndex = 0U;
            bytePosition = firstWordBytes - 1U - byteIndex;
        }
        else
        {
            wordIndex = 1U;
            bytePosition = 3U - (byteIndex - firstWordBytes);
        }

        buffer[i] = (uint8_t)(
            (rxData[wordIndex] >> (bytePosition * 8U)) & 0xFFU);
    }

    return kStatus_Success;
}