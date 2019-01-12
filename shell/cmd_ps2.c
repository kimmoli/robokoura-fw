#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"
#include "helpers.h"
#include "ps2.h"

void cmd_ps(BaseSequentialStream *chp, int argc, char *argv[])
{
    if (argc == 1 && strcmp(argv[0], "reconfig") == 0)
        PS2Values->reconfig = 1;

    chprintf(chp, "count %d\n\r", PS2Values->count);
    chprintf(chp, "reconfig count %d\n\r", PS2Values->reconfigCount);
    chprintf(chp, "buttons %04x\n\r", PS2Values->buttons);
    chprintf(chp, "left %d %d\n\r", PS2Values->analog_left_hor, PS2Values->analog_left_ver);
    chprintf(chp, "right %d %d\n\r", PS2Values->analog_right_hor, PS2Values->analog_right_ver);
    chprintf(chp, "up %d down %d\n\r", PS2Values->pressure_up, PS2Values->pressure_down);
}
