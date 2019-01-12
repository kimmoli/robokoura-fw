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
bool autoaxis = false;

I2CValues_t *I2CValues;

#define ACC_SCALE (1024.0)

const I2CConfig i2cConfig =
{
    OPMODE_I2C,      // operation mode I2C
    400000,          // speek 100kbits
    FAST_DUTY_CYCLE_2//STD_DUTY_CYCLE   // standard duty-cycle
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

        i2cAcquireBus(&I2CD1);
        ret = i2cMasterTransmit(&I2CD1, ACCEL0_ADDR, ((uint8_t[]){ 0x01 }), 1, rxBuf, 6);
        i2cReleaseBus(&I2CD1);

        if (ret == MSG_OK)
        {
            I2CValues->sensorPresent[0] = 1;
            x = (((int16_t)(rxBuf[0] << 8 | rxBuf[1]))/48); // 16
            y = (((int16_t)(rxBuf[2] << 8 | rxBuf[3]))/48);
            z = (((int16_t)(rxBuf[4] << 8 | rxBuf[5]))/48);
            I2CValues->x[0] = (float)(x)/ ACC_SCALE;
            I2CValues->y[0] = (float)(y) / ACC_SCALE;
            I2CValues->z[0] = (float)(z) / ACC_SCALE;
            I2CValues->Roll[0] = (atan2f(-I2CValues->y[0], I2CValues->z[0])*180.0)/M_PI;
            I2CValues->Pitch[0] = (atan2f(I2CValues->x[0], sqrtf(powf(I2CValues->y[0], 2) + powf(I2CValues->z[0], 2)))*180.0)/M_PI;
        }
        else
        {
            I2CValues->sensorPresent[0] = 0;
        }

/*        i2cAcquireBus(&I2CD1);
        ret = i2cMasterTransmit(&I2CD1, ACCEL1_ADDR, ((uint8_t[]){ 0x01 }), 1, rxBuf, 6);
        i2cReleaseBus(&I2CD1);

        if (ret == MSG_OK)
        {
            I2CValues->sensorPresent[1] = 1;
            I2CValues->x[1] = (float)(((int16_t)(rxBuf[0] << 8 | rxBuf[1]))/16) / ACC_SCALE;
            I2CValues->y[1] = (float)(((int16_t)(rxBuf[2] << 8 | rxBuf[3]))/16) / ACC_SCALE;
            I2CValues->z[1] = (float)(((int16_t)(rxBuf[4] << 8 | rxBuf[5]))/16) / ACC_SCALE;
            I2CValues->Roll[1] = (atan2f(-I2CValues->y[1], I2CValues->z[1])*180.0)/M_PI;
            I2CValues->Pitch[1] = (atan2f(I2CValues->x[1], sqrtf(powf(I2CValues->y[1], 2) + powf(I2CValues->z[1], 2)))*180.0)/M_PI;
        }
        else
        {
            I2CValues->sensorPresent[1] = 0;
        }
*/
        if (autoaxis)
        {

            float a = 1.0;
            int32_t max = 10000;

            p = (I2CValues->Roll[0]) * a + (p * (1.0 - a));

//            PRINT("x %04x y %04x z %04x\n\r", x >> 3, y >> 3, z >> 3);
//            PRINT("R %.3f\n\r", p);

            pp = ((int32_t)p) * 1500;
            PRINT("R %d\n\r", pp);

/*            if (pp > ppp + 100)
                pp = ppp + 100;
            else if (pp < ppp -100)
                pp = ppp - 100;
*/
            if (pp > max)
                pp = max;
            else if (pp < -max)
                pp = -max;

//            PRINT("%.2f %d\n\r", p, pp);

            if (pp > 0 && pp != ppp)
                setStepper(&STEPPERD1, (uint32_t)(pp), DIR_CW);
            else if (pp < 0 && pp != ppp)
                setStepper(&STEPPERD1, (uint32_t)(abs(pp)), DIR_CCW);
            else if (pp != ppp)
                setStepper(&STEPPERD1, 0, DIR_RETAIN);

            ppp = pp;
        }
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

static msg_t setreg(uint8_t addr, uint8_t reg, uint8_t val)
{
    uint8_t txBuf[2];
    msg_t ret;

    txBuf[0] = reg;
    txBuf[1] = val; /* DR=50Hz, LNOISE, FREAD=0, ACTIVE */

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

    ret = setreg(ACCEL0_ADDR, 0x2a, 0x00);

    if (ret == MSG_OK)
    {
       I2CValues->sensorPresent[0] = 1;
       setreg(ACCEL0_ADDR, 0x2a, 0x20);
       setreg(ACCEL0_ADDR, 0x2a, 0x21);
    }

    ret = setreg(ACCEL1_ADDR, 0x2a, 0x00);

    if (ret == MSG_OK)
    {
       I2CValues->sensorPresent[1] = 1;
       setreg(ACCEL1_ADDR, 0x2a, 0x28);
       setreg(ACCEL1_ADDR, 0x2a, 0x29);
    }


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
