#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"

#include "helpers.h"
#include "shellcommands.h"

#include "i2c.h"
#include "spi.h"
#include "stepper.h"
#include "sensor.h"
#include "ps2.h"
#include "neopixel.h"
#include "servo.h"
#include "oled.h"
#include "adc.h"

int main(void)
{
    halInit();
    chSysInit();

    sdStart(&SD3, NULL);  /* Serial console in USART3, 115200 */

    consoleStream = (BaseSequentialStream *) &SD3;

    PRINT("\n\r");
    PRINT("\n\rROBOKOURA STM32F407ZG BOARD");
    PRINT("\n\r---------------------------");
    PRINT("\n\r\n\r");

    initI2c();
    initSensor();
    initSpi();
    initStepper();
    initPS2();
    initServo();
    initNeopixel();
    initMeasurement();

    shellInit();
    chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell", NORMALPRIO + 1, shellThread, (void *)&shell_cfg_uart);

    palClearLine(LINE_ERR_LED);
    palToggleLine(LINE_DEBUG_LED);

    while (true)
    {
        chThdSleepMilliseconds(100);
        palToggleLine(LINE_RUN_LED);
        palToggleLine(LINE_DEBUG_LED);
    }
}
