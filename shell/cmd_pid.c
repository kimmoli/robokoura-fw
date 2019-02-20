#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "i2c.h"
#include "chprintf.h"
#include "helpers.h"
#include "shellcommands.h"
#include "pid.h"

extern void start_pid(float p, float i, float d);
extern pidc_t pid;
extern bool autoaxis;

void cmd_pid(BaseSequentialStream *chp, int argc, char *argv[])
{
    static volatile bool started = false;

    if (argc == 2 && strcmp(argv[0], "s") == 0)
    {
        *pid.setPoint = (float)strtol(argv[1], NULL, 10);
    }

    if (argc == 2 && strcmp(argv[0], "i") == 0)
    {
        *pid.input = (float)strtol(argv[1], NULL, 10);
    }

    if (!started && argc == 0)
    {
        start_pid(550.0, 45.0, 0.0);
        started = true;
        autoaxis = true;
    }

    if (!started && argc == 3)
    {
        start_pid((float)strtol(argv[0], NULL, 10),
                  (float)strtol(argv[1], NULL, 10),
                  (float)strtol(argv[2], NULL, 10));
        started = true;
        autoaxis = true;
    }
    else if (started && argc == 3)
    {
        pid_setTunings(&pid, (float)strtol(argv[0], NULL, 10),
                             (float)strtol(argv[1], NULL, 10),
                             (float)strtol(argv[2], NULL, 10), PID_ON_M);
    }


    chprintf(chp, "p %.2f i %.2f d %.2f - %.2f %.2f %.2f\n\r", pid.dispKp, pid.dispKi, pid.dispKd, *pid.input, *pid.output, *pid.setPoint);
}
