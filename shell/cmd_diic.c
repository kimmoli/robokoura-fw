#include <stdlib.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"
#include "helpers.h"

void cmd_diic(BaseSequentialStream *chp, int argc, char *argv[])
{
    uint8_t addr;
    uint8_t regaddr;
    int count;
    uint8_t rxBuf[255] = {0};
    msg_t ret = MSG_OK;

    if (argc < 2)
    {
        chprintf(chp, "diic deviceaddress regaddress {count}\n\r");
        return;
    }

    addr = strtol(argv[0], NULL, 16);
    regaddr = strtol(argv[1], NULL, 16);

    if (argc == 3)
        count = strtol(argv[2], NULL, 10);
    else
        count = 1;

    i2cAcquireBus(&I2CD1);
    ret |= i2cMasterTransmit(&I2CD1, addr, (uint8_t[]){regaddr}, 1, rxBuf, count);
    i2cReleaseBus(&I2CD1);

    if (ret != MSG_OK)
    {
        chprintf(chp, "[%02x] Error %d\n\r", addr, ret);
        return;
    }

    dump((char *)rxBuf, count);
}
