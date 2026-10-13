#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_pwm.h"

void BOARD_InitPwm(void)
{
    pwm_config_t config;

    // PWM1 (PWM#A) CONFIG --------------------

    // Configure PWM1 clock root to 24 MHz
    CLOCK_SetRootMux(kCLOCK_RootPwm1, kCLOCK_PwmRootmuxOsc24M);
    CLOCK_SetRootDivider(kCLOCK_RootPwm1, 1U, 1U);
    CLOCK_EnableRoot(kCLOCK_RootPwm1);

    // Configure PWM peripheral
    PWM_GetDefaultConfig(&config);

    config.clockSource = kPWM_PeripheralClock;
    config.prescale = 23U;
    config.outputConfig = kPWM_SetAtRolloverAndClearAtcomparison;
    config.sampleRepeat = kPWM_EachSampleOnce;

    PWM_Init(PWM1, &config);

    // 1 MHz counter, 50 Hz output
    PWM_SetPeriodValue(PWM1, 19998U);

    // Initial 1500 us pulse
    PWM_SetSampleValue(PWM1, 500U);

    PWM_StartTimer(PWM1);

    // PWM2 (PWM#C) CONFIG --------------------

    // Configure PWM2 clock root to 24 MHz
    CLOCK_SetRootMux(kCLOCK_RootPwm2, kCLOCK_PwmRootmuxOsc24M);
    CLOCK_SetRootDivider(kCLOCK_RootPwm2, 1U, 1U);
    CLOCK_EnableRoot(kCLOCK_RootPwm2);

    // Configure PWM peripheral
    PWM_GetDefaultConfig(&config);

    config.clockSource = kPWM_PeripheralClock;
    config.prescale = 23U;
    config.outputConfig = kPWM_SetAtRolloverAndClearAtcomparison;
    config.sampleRepeat = kPWM_EachSampleOnce;

    PWM_Init(PWM2, &config);

    // 1 MHz counter, 50 Hz output
    PWM_SetPeriodValue(PWM2, 19998U);

    // Initial 1500 us pulse
    PWM_SetSampleValue(PWM2, 500U);

    PWM_StartTimer(PWM2);

    // PWM3 (PWM#B) CONFIG --------------------

    // Configure PWM2 clock root to 24 MHz
    CLOCK_SetRootMux(kCLOCK_RootPwm3, kCLOCK_PwmRootmuxOsc24M);
    CLOCK_SetRootDivider(kCLOCK_RootPwm3, 1U, 1U);
    CLOCK_EnableRoot(kCLOCK_RootPwm3);

    // Configure PWM peripheral
    PWM_GetDefaultConfig(&config);

    config.clockSource = kPWM_PeripheralClock;
    config.prescale = 23U;
    config.outputConfig = kPWM_SetAtRolloverAndClearAtcomparison;
    config.sampleRepeat = kPWM_EachSampleOnce;

    PWM_Init(PWM3, &config);

    // 1 MHz counter, 50 Hz output
    PWM_SetPeriodValue(PWM3, 19998U);

    // Initial 1500 us pulse
    PWM_SetSampleValue(PWM3, 500U);

    PWM_StartTimer(PWM3);

}

void BOARD_InitHardware(void)
{
    /* M7 has its local cache and enabled by default,
     * need to set smart subsystems (0x28000000 ~ 0x3FFFFFFF)
     * non-cacheable before accessing this address region */
    BOARD_InitMemory();

    /* Board specific RDC settings */
    BOARD_RdcInit();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    BOARD_InitPwm();
}