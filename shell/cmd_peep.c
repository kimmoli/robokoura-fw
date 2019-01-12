#include <stdlib.h>
#include "hal.h"
#include "i2c.h"
#include "chprintf.h"
#include "helpers.h"
#include "shellcommands.h"

void cmd_peep(BaseSequentialStream *chp, int argc, char *argv[])
{
    uint16_t wordaddr;
    uint8_t txBuf[5];
    msg_t ret;

    if (argc < 2 || argc > 3)
    {
        chprintf(chp, "peep wordaddress data0 {data1}\n\r");
        return;
    }

    wordaddr = strtol(argv[0], NULL, 16);
    txBuf[0] = wordaddr & 0xff;

    int i;
    for (i = 1 ; i < argc ; i++)
    {
        txBuf[i] = strtol(argv[i], NULL, 16);
    }

    i2cAcquireBus(&I2CD1);
    ret = i2cMasterTransmit(&I2CD1, EEPROM_ADDR | ((wordaddr & 0x300) >> 8), txBuf, i, NULL, 0);
    i2cReleaseBus(&I2CD1);

    if (ret != MSG_OK)
    {
        chprintf(chp, "[%02x] Error %d\n\r", EEPROM_ADDR, ret);
        return;
    }
}
