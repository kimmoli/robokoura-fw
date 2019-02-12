#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"
#include "i2c.h"
#include "sensor.h"

void cmd_accel(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    int i;

    for (i=0; i < maxSensors; i++)
    {
        if (activeSensors[i]->active)
        {
            chprintf(chp, "%d: X %.2f Y %.2f Z %.2f  Pitch %.2f Roll %.2f\n\r",
                activeSensors[i]->instance,
                activeSensors[i]->x,
                activeSensors[i]->y,
                activeSensors[i]->z,
                activeSensors[i]->pitch,
                activeSensors[i]->roll );
        }
    }
}

