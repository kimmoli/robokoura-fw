#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"
#include "stepper.h"

void cmd_stepper(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) chp;
    uint32_t dir = DIR_RETAIN;

    if (argc >= 2)
    {
        if (argc >= 3)
        {
            if ((strcmp(argv[2], "r") == 0) || (strcmp(argv[2], "ccw") == 0))
                dir = DIR_CW;
            else if ((strcmp(argv[2], "f") == 0) || (strcmp(argv[2], "cw") == 0))
                dir = DIR_CCW;
        }

        if (strtol(argv[0], NULL, 10) == 1)
            setStepper(&STEPPERD1, strtol(argv[1], NULL, 10), dir);
        else if (strtol(argv[0], NULL, 10) == 2)
            setStepper(&STEPPERD2, strtol(argv[1], NULL, 10), dir);
        else if (strtol(argv[0], NULL, 10) == 3)
            setStepper(&STEPPERD3, strtol(argv[1], NULL, 10), dir);
        else if (strtol(argv[0], NULL, 10) == 4)
            setStepper(&STEPPERD4, strtol(argv[1], NULL, 10), dir);
        else if (strtol(argv[0], NULL, 10) == 5)
            setStepper(&STEPPERD5, strtol(argv[1], NULL, 10), dir);
        else if (strtol(argv[0], NULL, 10) == 6)
            setStepper(&STEPPERD6, strtol(argv[1], NULL, 10), dir);
    }
    else
    {
        chprintf(chp, "step [motor] [frequency] {cw,ccw}\n\r");
    }
}

