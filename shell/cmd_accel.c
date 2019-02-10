#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"
#include "i2c.h"
#include "stepper.h"

void cmd_accel(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    int i;

    for (i=0; i < 2; i++)
    {
        if (I2CValues->sensorPresent[i] == 1)
        {
            chprintf(chp, "%d: X %.2f Y %.2f Z %.2f\n\r", i, I2CValues->x[i], I2CValues->y[i], I2CValues->z[i]);
            chprintf(chp, "  Pitch %.2f Roll %.2f\n\r", I2CValues->Pitch[i], I2CValues->Roll[i]);
        }
        else
        {
            chprintf(chp, "%d: Not present\n\r", i);
        }
    }
}

