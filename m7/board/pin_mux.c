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

    // IMU_CS_AG -> GPIO5_IO13 (J16.4)
    IOMUXC_SetPinMux(IOMUXC_ECSPI2_SS0_GPIO5_IO13, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ECSPI2_SS0_GPIO5_IO13,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U));

    // IMU_CS2_M -> GPIO3_IO06 (J17.3)
    IOMUXC_SetPinMux(IOMUXC_NAND_DATA00_GPIO3_IO06, 0U);
    IOMUXC_SetPinConfig(IOMUXC_NAND_DATA00_GPIO3_IO06,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U));

    // SPI#A_SCK -> ECSPI2_SCLK (J16.2)
    IOMUXC_SetPinMux(IOMUXC_ECSPI2_SCLK_ECSPI2_SCLK, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ECSPI2_SCLK_ECSPI2_SCLK,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U));

    // SPI#A_SDI -> ECSPI2_MISO (J16.6)
    IOMUXC_SetPinMux(IOMUXC_ECSPI2_MISO_ECSPI2_MISO, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ECSPI2_MISO_ECSPI2_MISO,
                        IOMUXC_SW_PAD_CTL_PAD_DSE(3U));

    // SPI#A_SDO -> ECSPI2_MOSI (J16.8)
    IOMUXC_SetPinMux(IOMUXC_ECSPI2_MOSI_ECSPI2_MOSI, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ECSPI2_MOSI_ECSPI2_MOSI,
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