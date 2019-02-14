#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "adc.h"
#include "chprintf.h"
#include "shellcommands.h"

void cmd_volts(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    float extvref = 1.21 * 4096.0 / (float)MEASD1.raw[2];
    float supply = (float)( extvref / 4096 * (5900+825)/825) * (float)MEASD1.raw[1];
    float temp = (float)((( extvref / 4096 * (float)MEASD1.raw[0]) - 0.76  ) / 0.0025 ) + 30.0;

    chprintf(chp, "CPU temp       %.2f C\n\r", temp);
    chprintf(chp, "Supply voltage %.2f V\n\r", supply);
    chprintf(chp, "Ext vref       %.2f V\n\r", extvref);

    for (int i=0 ; i<ADC_GRP1_NUM_CHANNELS; i++)
    {
        chprintf(chp, "%d %04x %d\n\r", i, MEASD1.raw[i], MEASD1.raw[i]);
    }
}

