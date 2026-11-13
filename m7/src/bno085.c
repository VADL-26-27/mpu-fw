
#include "bno085.h"

#include <string.h>

#include "fsl_ecspi.h"
#include "fsl_gpio.h"
#include "fsl_debug_console.h"

// hardware configuration
#define BNO085_SPI       ECSPI2

#define BNO085_CS_GPIO   GPIO5
#define BNO085_CS_PIN    13U

#define BNO085_INT_GPIO  GPIO3
#define BNO085_INT_PIN   6U

#define BNO085_WAKE_GPIO GPIO3
#define BNO085_WAKE_PIN  14U

// define BNO085_WAKE_GPIO and BNO085_WAKE_PIN in board configuration
#if defined(BNO085_WAKE_GPIO) && defined(BNO085_WAKE_PIN)
#define BNO085_HAS_WAKE 1
#else
#define BNO085_HAS_WAKE 0
#endif

// SHTP configuration
#define BNO085_SHTP_HEADER_SIZE    4U
#define BNO085_MAX_PAYLOAD         512U

#define BNO085_CHANNEL_CONTROL     2U
#define BNO085_CHANNEL_REPORTS     3U
#define BNO085_CHANNEL_WAKE        4U

#define BNO085_REPORT_SET_FEATURE  0xFDU
#define BNO085_REPORT_ROTATION     0x05U
#define BNO085_REPORT_GYRO         0x02U
#define BNO085_REPORT_TIMEBASE     0xFBU

#define BNO085_DEFAULT_PERIOD_US   10000U
#define BNO085_READY_SPINS         2000000U

// driver state
static uint8_t s_txSequence[6];
static uint8_t s_payload[BNO085_MAX_PAYLOAD];

static bno085_sample_t s_sample;
static uint32_t s_lastReadCount;
static bool s_initialized;

static uint16_t BNO085_U16(const uint8_t *p)
{
    return (uint16_t)(
        (uint16_t)p[0] |
        ((uint16_t)p[1] << 8)
    );
}

static int16_t BNO085_S16(const uint8_t *p)
{
    return (int16_t)BNO085_U16(p);
}

static void BNO085_PutU32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

bool BNO085_InterruptActive(void)
{
    return GPIO_PinRead(
        BNO085_INT_GPIO,
        BNO085_INT_PIN
    ) == 0U;
}

static void BNO085_ChipSelect(bool active)
{
    GPIO_PinWrite(
        BNO085_CS_GPIO,
        BNO085_CS_PIN,
        active ? 0U : 1U
    );
}

// SPI transfer
static status_t BNO085_TransferByte(
    uint8_t tx,
    uint8_t *rx
)
{
    uint32_t txWord = tx;
    uint32_t rxWord = 0U;

    ecspi_transfer_t transfer = {
        .txData = &txWord,
        .rxData = &rxWord,
        .dataSize = 1U,
        .channel = kECSPI_Channel0
    };

    status_t status = ECSPI_MasterTransferBlocking(
        BNO085_SPI,
        &transfer
    );

    if ((status == kStatus_Success) && (rx != NULL))
    {
        *rx = (uint8_t)rxWord;
    }

    return status;
}

static status_t BNO085_Transfer(
    const uint8_t *tx,
    uint8_t *rx,
    uint16_t count
)
{
    for (uint16_t i = 0U; i < count; ++i)
    {
        uint8_t incoming = 0U;

        status_t status = BNO085_TransferByte(
            tx != NULL ? tx[i] : 0U,
            &incoming
        );

        if (status != kStatus_Success)
        {
            return status;
        }

        if (rx != NULL)
        {
            rx[i] = incoming;
        }
    }

    return kStatus_Success;
}

// wake handshake
static status_t BNO085_WaitReady(void)
{
#if !BNO085_HAS_WAKE

    PRINTF("BNO085 WAKE timeout\r\n");
    return kStatus_Fail;

#else

    GPIO_PinWrite(
        BNO085_WAKE_GPIO,
        BNO085_WAKE_PIN,
        0U
    );

    for (uint32_t i = 0U; i < BNO085_READY_SPINS; ++i)
    {
        if (BNO085_InterruptActive())
        {
            return kStatus_Success;
        }
    }

    GPIO_PinWrite(
        BNO085_WAKE_GPIO,
        BNO085_WAKE_PIN,
        1U
    );

    return kStatus_Timeout;

#endif
}

static void BNO085_EndWake(void)
{
#if BNO085_HAS_WAKE

    GPIO_PinWrite(
        BNO085_WAKE_GPIO,
        BNO085_WAKE_PIN,
        1U
    );

#endif
}


static status_t BNO085_SendPacket(
    uint8_t channel,
    const uint8_t *payload,
    uint16_t payloadLength
)
{
    if ((payload == NULL) ||
        (channel >= sizeof(s_txSequence)) ||
        (payloadLength > BNO085_MAX_PAYLOAD))
    {
        return kStatus_InvalidArgument;
    }

    PRINTF("BNO085: waiting for WAKE handshake\r\n");

    status_t status = BNO085_WaitReady();

    PRINTF("BNO085: handshake status=%d INT=%d\r\n",
           (int)status,
           (int)BNO085_InterruptActive());

    if (status != kStatus_Success)
    {
        return status;
    }

    uint16_t length =
        payloadLength + BNO085_SHTP_HEADER_SIZE;

    uint8_t header[4] = {
        (uint8_t)length,
        (uint8_t)(length >> 8),
        channel,
        s_txSequence[channel]++
    };

    PRINTF("BNO085: sending SHTP header\r\n");

    BNO085_ChipSelect(true);

    status = BNO085_Transfer(header, NULL, sizeof(header));

    PRINTF("BNO085: header status=%d\r\n", (int)status);

    if (status == kStatus_Success)
    {
        PRINTF("BNO085: sending payload\r\n");

        status = BNO085_Transfer(
            payload,
            NULL,
            payloadLength
        );

        PRINTF("BNO085: payload status=%d\r\n", (int)status);
    }

    BNO085_ChipSelect(false);
    BNO085_EndWake();

    return status;
}


// sensor report parsing
static void BNO085_ParseReports(
    const uint8_t *p,
    uint16_t size
)
{
    uint16_t offset = 0U;

    while (offset < size)
    {
        uint8_t id = p[offset];

        if (id == BNO085_REPORT_TIMEBASE)
        {
            if ((size - offset) < 5U)
            {
                break;
            }

            offset += 5U;
        }
        else if (id == BNO085_REPORT_ROTATION)
        {
            if ((size - offset) < 14U)
            {
                break;
            }

            const uint8_t *r = &p[offset];

            // Q14 quaternion
            s_sample.orientation.x =
                (float)BNO085_S16(&r[4]) / 16384.0f;

            s_sample.orientation.y =
                (float)BNO085_S16(&r[6]) / 16384.0f;

            s_sample.orientation.z =
                (float)BNO085_S16(&r[8]) / 16384.0f;

            s_sample.orientation.w =
                (float)BNO085_S16(&r[10]) / 16384.0f;

            s_sample.orientation_accuracy =
                r[2] & 0x03U;

            s_sample.orientation_valid = true;
            ++s_sample.orientation_count;

            offset += 14U;
        }
        else if (id == BNO085_REPORT_GYRO)
        {
            if ((size - offset) < 10U)
            {
                break;
            }

            const uint8_t *r = &p[offset];

            // Q9 angular velocity in rad/s
            s_sample.angular_velocity.x =
                (float)BNO085_S16(&r[4]) / 512.0f;

            s_sample.angular_velocity.y =
                (float)BNO085_S16(&r[6]) / 512.0f;

            s_sample.angular_velocity.z =
                (float)BNO085_S16(&r[8]) / 512.0f;

            s_sample.gyro_accuracy =
                r[2] & 0x03U;

            s_sample.gyro_valid = true;
            ++s_sample.gyro_count;

            offset += 10U;
        }
        else
        {
            break;
        }
    }
}

// SHTP receive
status_t BNO085_Process(void)
{
    if (!BNO085_InterruptActive())
    {
        return kStatus_Fail;
    }

    uint8_t header[4];

    BNO085_ChipSelect(true);

    status_t status = BNO085_Transfer(
        NULL,
        header,
        sizeof(header)
    );

    if (status != kStatus_Success)
    {
        BNO085_ChipSelect(false);
        return status;
    }

    uint16_t rawLength = BNO085_U16(header);
    bool continuation = (rawLength & 0x8000U) != 0U;
    uint16_t length = rawLength & 0x7FFFU;

    if (length < BNO085_SHTP_HEADER_SIZE)
    {
        BNO085_ChipSelect(false);
        return kStatus_Fail;
    }

    uint16_t payloadLength =
        length - BNO085_SHTP_HEADER_SIZE;

    if (payloadLength > BNO085_MAX_PAYLOAD)
    {
        for (uint16_t i = 0U; i < payloadLength; ++i)
        {
            status = BNO085_TransferByte(0U, NULL);

            if (status != kStatus_Success)
            {
                break;
            }
        }

        BNO085_ChipSelect(false);
        return kStatus_Fail;
    }

    status = BNO085_Transfer(
        NULL,
        s_payload,
        payloadLength
    );

    BNO085_ChipSelect(false);

    if (status != kStatus_Success)
    {
        return status;
    }

    if (continuation)
    {
        return kStatus_Fail;
    }

    if ((header[2] == BNO085_CHANNEL_REPORTS) ||
        (header[2] == BNO085_CHANNEL_WAKE))
    {
        BNO085_ParseReports(
            s_payload,
            payloadLength
        );
    }

    return kStatus_Success;
}

// sensor configuration
static status_t BNO085_SetFeature(
    uint8_t reportId,
    uint32_t intervalUs
)
{
    uint8_t payload[17] = {0};

    payload[0] = BNO085_REPORT_SET_FEATURE;
    payload[1] = reportId;

    BNO085_PutU32(
        &payload[5],
        intervalUs
    );

    return BNO085_SendPacket(
        BNO085_CHANNEL_CONTROL,
        payload,
        sizeof(payload)
    );
}

status_t BNO085_SetReportIntervals(
    uint32_t rotation_us,
    uint32_t gyro_us
)
{
    status_t status = BNO085_SetFeature(
        BNO085_REPORT_ROTATION,
        rotation_us
    );

    if (status != kStatus_Success)
    {
        return status;
    }

    for (uint32_t i = 0U;
         i < 16U && BNO085_InterruptActive();
         ++i)
    {
        status = BNO085_Process();

        if (status != kStatus_Success)
        {
            return status;
        }
    }

    return BNO085_SetFeature(
        BNO085_REPORT_GYRO,
        gyro_us
    );
}

// initialization

status_t BNO085_Init(void)
{
    memset(s_txSequence, 0, sizeof(s_txSequence));
    memset(&s_sample, 0, sizeof(s_sample));

    s_lastReadCount = 0U;
    s_initialized = false;

    BNO085_ChipSelect(false);

#if !BNO085_HAS_WAKE

    PRINTF("BNO085: WAKE GPIO undefined\r\n");
    return kStatus_Fail;

#else

    GPIO_PinWrite(
        BNO085_WAKE_GPIO,
        BNO085_WAKE_PIN,
        1U
    );

    PRINTF("BNO085: starting initialization\r\n");

    PRINTF("BNO085: WAKE=%d INT=%d\r\n",
        (int)GPIO_PinRead(BNO085_WAKE_GPIO, BNO085_WAKE_PIN),
        (int)GPIO_PinRead(BNO085_INT_GPIO, BNO085_INT_PIN));

    // process startup packets
    for (uint32_t i = 0U;
        i < 32U && BNO085_InterruptActive();
        ++i)
    {
        PRINTF("BNO085: reading startup packet %d\r\n", (int)i);

        status_t status = BNO085_Process();

        PRINTF("BNO085: startup packet status=%d\r\n", (int)status);

        if (status != kStatus_Success)
        {
            return status;
        }
    }

    PRINTF("BNO085: configuring rotation vector\r\n");

    status_t status = BNO085_SetFeature(
        BNO085_REPORT_ROTATION,
        BNO085_DEFAULT_PERIOD_US
    );

    PRINTF("BNO085: rotation config status=%d\r\n", (int)status);

    if (status != kStatus_Success)
    {
        return status;
    }

    // process pending packets
    for (uint32_t i = 0U;
        i < 16U && BNO085_InterruptActive();
        ++i)
    {
        PRINTF("BNO085: processing packet %d\r\n", (int)i);

        status = BNO085_Process();

        PRINTF("BNO085: packet status=%d\r\n", (int)status);

        if (status != kStatus_Success)
        {
            return status;
        }
    }

    PRINTF("BNO085: configuring gyroscope\r\n");

    status = BNO085_SetFeature(
        BNO085_REPORT_GYRO,
        BNO085_DEFAULT_PERIOD_US
    );

    PRINTF("BNO085: gyro config status=%d\r\n", (int)status);

    if (status != kStatus_Success)
    {
        return status;
    }

    s_initialized = true;

    PRINTF("BNO085: initialization complete\r\n");

    return kStatus_Success;

#endif
}


// sensor data access
status_t BNO085_Read(bno085_sample_t *sample)
{
    if ((sample == NULL) || !s_initialized)
    {
        return kStatus_Fail;
    }

    *sample = s_sample;

    return kStatus_Success;
}

bool BNO085_ReadNewSample(bno085_sample_t *sample)
{
    if ((sample == NULL) ||
        !s_initialized ||
        !s_sample.orientation_valid ||
        (s_sample.orientation_count == s_lastReadCount))
    {
        return false;
    }

    *sample = s_sample;
    s_lastReadCount = s_sample.orientation_count;

    return true;
}
