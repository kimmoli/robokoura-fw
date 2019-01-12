#include <stdlib.h>
#include "hal.h"
#include "i2c.h"
#include "chprintf.h"
#include "helpers.h"
#include "shellcommands.h"

void cmd_deep(BaseSequentialStream *chp, int argc, char *argv[])
{
    uint16_t wordaddr;
    uint8_t rxBuf[256] = {0};
    uint8_t txBuf[2] = {0};
    msg_t ret = MSG_OK;
    int count;

    if (argc != 2)
    {
        chprintf(chp, "deep wordaddress count\n\r");
        return;
    }

    wordaddr = strtol(argv[0], NULL, 16);
    count = MIN(sizeof(rxBuf), (uint8_t) strtol(argv[1], NULL, 10));

    txBuf[0] = wordaddr & 0xff;

    uint8_t *rxp = rxBuf;

    i2cAcquireBus(&I2CD1);
    i2cMasterTransmit(&I2CD1, EEPROM_ADDR | ((wordaddr & 0x300) >> 8) , txBuf, 1, NULL, 0);
    ret = i2cMasterReceive(&I2CD1, EEPROM_ADDR, rxp, count);
    i2cReleaseBus(&I2CD1);

    if (ret != MSG_OK)
    {
        chprintf(chp, "[%02x] Error %d\n\r", EEPROM_ADDR, ret);
        return;
    }

    dump((char*) rxBuf, count);
}
