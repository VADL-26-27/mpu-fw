
#ifndef BNO085_H
#define BNO085_H

#include <stdint.h>
#include "fsl_common.h"

#define BNO085_MAX_PACKET_SIZE 512U

status_t BNO085_ReadPacket(uint8_t *packet, uint16_t *length);

#endif
