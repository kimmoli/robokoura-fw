#include <stdlib.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"

void cmd_piic(BaseSequentialStream *chp, int argc, char *argv[])
{
    uint8_t addr;
    uint8_t txBuf[2];
    msg_t ret;

    if (argc != 3)
    {
        chprintf(chp, "piic deviceaddress regaddress data\n\r");
        return;
    }

    addr = strtol(argv[0], NULL, 16);
    txBuf[0] = strtol(argv[1], NULL, 16);
    txBuf[1] = strtol(argv[2], NULL, 16);

    i2cAcquireBus(&I2CD1);
    ret = i2cMasterTransmit(&I2CD1, addr, txBuf, 2, NULL, 0);
    i2cReleaseBus(&I2CD1);

    if (ret != MSG_OK)
    {
        chprintf(chp, "[%02x] Error %d\n\r", addr, ret);
        return;
    }

    chprintf(chp, "[%02x] %02x: %02x\n\r", addr, txBuf[0], txBuf[1]);
}
