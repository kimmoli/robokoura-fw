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
#include "ps2.h"
#include "neopixel.h"
#include "servo.h"
#include "oled.h"

int main(void)
{
    int fill = 0x01;
    limits = 0;

    halInit();
    chSysInit();

    sdStart(&SD3, NULL);  /* Serial console in USART3, 115200 */

    consoleStream = (BaseSequentialStream *) &SD3;

    PRINT("\n\r");
    PRINT("\n\rROBOKOURA STM32F407ZG BOARD");
    PRINT("\n\r---------------------------");
    PRINT("\n\r\n\r");

    initI2c();
    initSpi();
    initStepper();
    initPS2();
    initServo();
    initNeopixel();
#if 0
    initOled();

    clearOled();
    drawText(0, 0, "ROBOKOURA", 2);
#endif
    shellInit();
    chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell", NORMALPRIO + 1, shellThread, (void *)&shell_cfg_uart);

    palClearLine(LINE_ERR_LED);
    palToggleLine(LINE_DEBUG_LED);
    setreg(0x40, 0x01, 0xfc);
    setreg(0x42, 0x01, 0xfc);
    setreg(0x43, 0x01, 0xfc);
    setreg(0x44, 0x01, 0xfc);
    setreg(0x45, 0x01, 0xfc);

    while (true)
    {
        chThdSleepMilliseconds(100);
        palToggleLine(LINE_RUN_LED);
        palToggleLine(LINE_DEBUG_LED);
        setreg(0x40, 0x03, 0xfd | ~((fill & 1) << 1));
        setreg(0x42, 0x03, 0xfd | ~((fill & 2)));
        setreg(0x43, 0x03, 0xfd | ~((fill & 4) >> 1));
        setreg(0x44, 0x03, 0xfd | ~((fill & 8) >> 2));
        setreg(0x45, 0x03, 0xfd | ~((fill & 0x10) >> 3));

        fill = fill << 1;

        if (fill & 0x20)
            fill = 0x01;

        limits = (getreg(0x40, 0x00) & 0x0c) >> 2;
        limits |= (getreg(0x45, 0x00) & 0x0c);

        if (limits != 0x0f)
            PRINT("%01x\n\r", limits);

#if 0

        fillRect(125, 0, 3, 3, fill);
        updateOled();
#endif
    }
}
