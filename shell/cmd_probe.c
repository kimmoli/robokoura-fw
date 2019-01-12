#include <stdlib.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"
#include "i2c.h"

void cmd_probe(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argv;
    (void) argc;

    msg_t ret;
    uint8_t i;
    uint8_t rxBuf[8];

    chprintf(chp, "Probing i2c devices\n\r");
    for (i=1; i<128; i++)
    {
        i2cAcquireBus(&I2CD1);
        i2cMasterTransmit(&I2CD1, i, (uint8_t[]){ 0 }, 1, NULL, 0);
        ret = i2cMasterReceive(&I2CD1, i, rxBuf, 2);
        i2cReleaseBus(&I2CD1);

        if (MSG_OK == ret)
        {
	        chprintf(chp, "Addr: 0x%02x has device\n\r",i);
        }
    }
}
