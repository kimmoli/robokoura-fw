#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"
#include "servo.h"

void cmd_servo(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) chp;

    if (argc >= 2)
    {
        if (strtol(argv[0], NULL, 10) == 1)
            setServo(&SERVOD1, strtol(argv[1], NULL, 10));
        else if (strtol(argv[0], NULL, 10) == 2)
        {
            if (strcmp(argv[1], "loop") == 0)
            {
                while (1)
                {
                    setServo(&SERVOD2, 1200);
                    chThdSleepMilliseconds(700);
                    setServo(&SERVOD2, 1870);
                    chThdSleepMilliseconds(700);
                }
            }
            else
            {
                setServo(&SERVOD2, strtol(argv[1], NULL, 10));
            }
        }
    }
    else
    {
        chprintf(chp, "servo [channel] [value in us 1000...2000 (1500 = center)]\n\r");
    }
}

