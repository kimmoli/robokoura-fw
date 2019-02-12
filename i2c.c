#include <stdlib.h>
#include "hal.h"
#include "i2c.h"
#include "stepper.h"
#include "sensor.h"
#include <math.h>
#include "helpers.h"

static bool i2cBusReset(void);
static int i2cEnabled;
virtual_timer_t i2cVt;
event_source_t i2cPoll;

uint8_t limits;

#define ACC_SCALE (1024.0)

const I2CConfig i2cConfig =
{
    OPMODE_I2C,       // operation mode I2C
    400000,           // speed
    FAST_DUTY_CYCLE_2 // for 400k
};

static THD_FUNCTION(i2cThread, arg)
{
    (void)arg;

    event_listener_t eli2c;
    int leds = 0x00;

    chEvtRegister(&i2cPoll, &eli2c, 7);

    while (!chThdShouldTerminateX())
    {
        chEvtWaitOne(EVENT_MASK(7));

        if (sensorsEnabled)
        {
            limits = 0;

            for (int i = 0 ; i < maxSensors ; i++)
            {
                if (leds == activeSensors[i]->instance)
                    activeSensors[i]->leds &= 0x01;
                else
                    activeSensors[i]->leds |= 0x02;

                setreg(activeSensors[i]->i2cAddressIO, 0x03, 0xfc | activeSensors[i]->leds);
                activeSensors[i]->inputs = ((getreg(activeSensors[i]->i2cAddressIO, 0x00) & 0x0c) >> 2);

                limits |= activeSensors[i]->inputs << ((activeSensors[i]->instance - 1)*2);

                updateAcceleration(activeSensors[i]);
            }
        }

        leds++;

        if (leds > 5)
            leds = 0x00;
    }

    chThdExit(MSG_OK);
}

void i2cVtCb(void *p)
{
    (void) p;

    chSysLockFromISR();
    chVTResetI(&i2cVt);
    chVTSetI(&i2cVt, MS2ST(I2C_UPDATE_INTERVAL), i2cVtCb, NULL);
    chEvtBroadcastI(&i2cPoll);
    chSysUnlockFromISR();
}

msg_t setreg(uint8_t addr, uint8_t reg, uint8_t val)
{
    uint8_t txBuf[2];
    msg_t ret;

   if (addr == 0)
        return MSG_RESET;

    txBuf[0] = reg;
    txBuf[1] = val;

    i2cAcquireBus(&I2CD1);
    ret = i2cMasterTransmit(&I2CD1, addr, txBuf, 2, NULL, 0);
    i2cReleaseBus(&I2CD1);

    return ret;
}

uint8_t getreg(uint8_t addr, uint8_t reg)
{
    uint8_t txBuf[2];
    uint8_t rxBuf[2];

   if (addr == 0)
        return 0;

    txBuf[0] = reg;

    i2cAcquireBus(&I2CD1);
    i2cMasterTransmit(&I2CD1, addr, txBuf, 1, rxBuf, 1);
    i2cReleaseBus(&I2CD1);

    return rxBuf[0];
}

void initI2c(void)
{
    while (!i2cBusReset()) { }

    i2cStart(&I2CD1, &i2cConfig);

    i2cEnabled = true;

    chEvtObjectInit(&i2cPoll);
    chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(2048), "i2c", NORMALPRIO+1, i2cThread, NULL);

    chVTSet(&i2cVt, MS2ST(I2C_UPDATE_INTERVAL), i2cVtCb, NULL);
}

/*
 * I2C Bus Reset - Analog Devices AN-686 Solution 1.
 */
bool i2cBusReset(void)
{
    int count = 0;
    bool ret = false;

    /* Configure I2C pins to GPIO */
    palSetPadMode(GPIOB, GPIOB_PB6_I2C_SCL, PAL_STM32_MODE_OUTPUT | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(GPIOB, GPIOB_PB7_I2C_SDA, PAL_STM32_MODE_OUTPUT | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST);
    chThdSleepMilliseconds(1);

    /* Try to make signals high */
    palSetPad(GPIOB, GPIOB_PB6_I2C_SCL);
    palSetPad(GPIOB, GPIOB_PB7_I2C_SDA);
    chThdSleepMilliseconds(1);

    /* If SDA is low, clock SCL until SDA is high */
    while (palReadPad(GPIOB, GPIOB_PB7_I2C_SDA) == PAL_LOW && count < 50)
    {
        palClearPad(GPIOB, GPIOB_PB6_I2C_SCL);
        chThdSleepMilliseconds(1);
        palSetPad(GPIOB, GPIOB_PB6_I2C_SCL);
        chThdSleepMilliseconds(1);
        count++;
    }

    /* Generate a stop condition */
    palClearPad(GPIOB, GPIOB_PB6_I2C_SCL);
    chThdSleepMilliseconds(1);
    palClearPad(GPIOB, GPIOB_PB7_I2C_SDA);
    chThdSleepMilliseconds(1);
    palSetPad(GPIOB, GPIOB_PB6_I2C_SCL);
    chThdSleepMilliseconds(1);
    palSetPad(GPIOB, GPIOB_PB7_I2C_SDA);
    chThdSleepMilliseconds(1);

    /* Return true, if SDA is now high */
    ret = (palReadPad(GPIOB, GPIOB_PB7_I2C_SDA) == PAL_HIGH);

    palSetPadMode(GPIOB, GPIOB_PB6_I2C_SCL, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(4));
    palSetPadMode(GPIOB, GPIOB_PB7_I2C_SDA, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(4));

    if (count > 0)
        PRINT(" - I2C was stuck, and took %d cycles to reset.\n\r", count);

    chThdSleepMilliseconds(10);

    return ret;
}
