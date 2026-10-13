#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "pin_mux.h"

void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

void BOARD_InitPins(void) {

    // PWM#A -> PWM1_OUT
    IOMUXC_SetPinMux(IOMUXC_SPDIF_EXT_CLK_PWM1_OUT, 0U);
    IOMUXC_SetPinConfig(IOMUXC_SPDIF_EXT_CLK_PWM1_OUT,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U));

    // PWM#B -> PWM3_OUT
    IOMUXC_SetPinMux(IOMUXC_SPDIF_TX_PWM3_OUT, 0U);
    IOMUXC_SetPinConfig(IOMUXC_SPDIF_TX_PWM3_OUT,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U));

    // PWM#C -> PWM2_OUT
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO11_PWM2_OUT, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO11_PWM2_OUT,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U));

    // UART4_RXD UART4
    IOMUXC_SetPinMux(IOMUXC_UART4_RXD_UART4_RX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART4_RXD_UART4_RX, 
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
                        IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    // UART4_TXD UART4
    IOMUXC_SetPinMux(IOMUXC_UART4_TXD_UART4_TX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART4_TXD_UART4_TX, 
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U) |
                        IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_PE_MASK);
}