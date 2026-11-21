
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_ecspi.h"
#include "fsl_gpio.h"
#include "board.h"
#include "hardware_init.h"
#include <stdint.h>
#include "bno085.h"


int main(void)
{
    BOARD_InitHardware();

    PRINTF("MCUX SDK version: %s\r\n", MCUXSDK_VERSION_FULL_STR);

    GPIO_PinWrite(GPIO5, 13U, 1U);

    while (1)
    {
        uint8_t packet[BNO085_MAX_PACKET_SIZE];
        uint16_t length = 0;

        status_t status = BNO085_ReadPacket(packet, &length);

        if (status == kStatus_Success) {
            PRINTF("Length: %u\r\n", (unsigned)length);
            PRINTF("Packet: ");
            for (uint16_t i = 0; i < length; i++)
            {
                PRINTF("%02X ", (unsigned)packet[i]);
            }
            PRINTF("\r\n");
        }
        

        SDK_DelayAtLeastUs(100000U, SystemCoreClock);

    }
}
