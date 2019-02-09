#include <stdlib.h>
#include "hal.h"
#include "i2c.h"
#include "stepper.h"
#include <math.h>
#include "helpers.h"

static bool i2cBusReset(void);
static int i2cEnabled;
virtual_timer_t i2cVt;
event_source_t i2cPoll;

I2CValues_t *I2CValues;

#define ACC_SCALE (1024.0)

const I2CConfig i2cConfig =
{
    OPMODE_I2C,       // operation mode I2C
    400000,           // speed
//    STD_DUTY_CYCLE   // for 100k
    FAST_DUTY_CYCLE_2 // for 400k
};

static THD_FUNCTION(i2cThread, arg)
{
    (void)arg;
    uint8_t rxBuf[6];
    msg_t ret;
    float p = 0.0;
    int32_t pp = 0;
    int32_t ppp = 0;
    int16_t x,y,z;

    event_listener_t eli2c;

    chEvtRegister(&i2cPoll, &eli2c, 7);

    while (!chThdShouldTerminateX())
    {
        chEvtWaitOne(EVENT_MASK(7));
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

    txBuf[0] = reg;
    txBuf[1] = val;

    i2cAcquireBus(&I2CD1);
    ret = i2cMasterTransmit(&I2CD1, addr, txBuf, 2, NULL, 0);
    i2cReleaseBus(&I2CD1);

    return ret;
}

void initI2c(void)
{
    msg_t ret;

    while (!i2cBusReset()) { }

    i2cStart(&I2CD1, &i2cConfig);

    i2cEnabled = true;

    I2CValues = chHeapAlloc(NULL, sizeof(I2CValues_t));

    I2CValues->sensorPresent[0] = 0;
    I2CValues->x[0] = 0.0;
    I2CValues->y[0] = 0.0;
    I2CValues->z[0] = 0.0;
    I2CValues->Pitch[0] = 0.0;
    I2CValues->Roll[0] = 0.0;

    I2CValues->sensorPresent[1] = 0;
    I2CValues->x[1] = 0.0;
    I2CValues->y[1] = 0.0;
    I2CValues->z[1] = 0.0;
    I2CValues->Pitch[1] = 0.0;
    I2CValues->Roll[1] = 0.0;

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
