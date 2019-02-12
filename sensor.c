#include "hal.h"
#include "sensor.h"
#include "i2c.h"
#include "helpers.h"
#include <math.h>

SensorDriver SENSORD1;
SensorDriver SENSORD2;
SensorDriver SENSORD3;
SensorDriver SENSORD4;
SensorDriver SENSORD5;

bool sensorsEnabled = false;

SensorDriver *activeSensors[6] = {0};
int maxSensors = 0;

bool detectSensor(SensorDriver *sensor)
{
    msg_t ret;
    uint8_t rxBuf[2];
    uint8_t addr;

    sensor->active = false;

    if (sensor->i2cAddressIO > 0)
        addr = sensor->i2cAddressIO;
    else
        addr = sensor->i2cAddressAcc;

    i2cAcquireBus(&I2CD1);
    i2cMasterTransmit(&I2CD1, addr, (uint8_t[]){ 0 }, 1, NULL, 0);
    ret = i2cMasterReceive(&I2CD1, addr, rxBuf, 2);
    i2cReleaseBus(&I2CD1);

    if (MSG_OK == ret)
    {
        PRINT("Detected sensor %d at 0x%02x\n\r", sensor->instance, addr);

        if (addr == sensor->i2cAddressIO)
        {
            setreg(addr, 0x01, 0xfc); // Outputs 0, 1 low
        }

        setreg(sensor->i2cAddressAcc, IIS328_CTRL1REG, IIS328_CTRL1REG_VALUE);
        setreg(sensor->i2cAddressAcc, IIS328_CTRL4REG, IIS328_CTRL4REG_VALUE);

        sensor->active = true;
        activeSensors[maxSensors] = sensor;
        maxSensors++;
    }

    return sensor->active;
}

void initSensor(void)
{
    SENSORD1.instance = 1;
    SENSORD1.i2cAddressIO = 0x40;
    SENSORD1.i2cAddressAcc = 0x19;
    SENSORD1.leds = 0x01;
    detectSensor(&SENSORD1);

    SENSORD2.instance = 2;
    SENSORD2.i2cAddressIO = 0x43;
    SENSORD2.i2cAddressAcc = 0x1a;
    SENSORD2.leds = 0x01;
    detectSensor(&SENSORD2);

    SENSORD3.instance = 3;
    SENSORD3.i2cAddressIO = 0x42;
    SENSORD3.i2cAddressAcc = 0x1b;
    SENSORD3.leds = 0x01;
    detectSensor(&SENSORD3);

    SENSORD4.instance = 4;
    SENSORD4.i2cAddressIO = 0x45;
    SENSORD4.i2cAddressAcc = 0x1c;
    SENSORD4.leds = 0x01;
    detectSensor(&SENSORD4);

    SENSORD5.instance = 5;
    SENSORD5.i2cAddressIO = 0x44;
    SENSORD5.i2cAddressAcc = 0x1d;
    SENSORD5.leds = 0x01;
    detectSensor(&SENSORD5);

    sensorsEnabled = true;
}

msg_t updateAcceleration(SensorDriver *sensor)
{
    uint8_t rxBuf[6];
    msg_t ret = MSG_OK;

    i2cMasterTransmit(&I2CD1, sensor->i2cAddressAcc, (uint8_t[]){IIS328_OUTREG | IIS328_REG_AUTOINC}, 1, NULL, 0);
    ret = i2cMasterReceive(&I2CD1, sensor->i2cAddressAcc, rxBuf, 6);

    if (ret == MSG_OK)
    {
        sensor->x = (float)(((int16_t)( (rxBuf[0]<<8) | rxBuf[1]))/16) * IIS328_8G_SCALE;
        sensor->y = (float)(((int16_t)( (rxBuf[2]<<8) | rxBuf[3]))/16) * IIS328_8G_SCALE;
        sensor->z = (float)(((int16_t)( (rxBuf[4]<<8) | rxBuf[5]))/16) * IIS328_8G_SCALE;
        sensor->roll = (atan2f(-sensor->y, sensor->z)*180.0)/M_PI;
        sensor->pitch = (atan2f(sensor->x, sqrtf(powf(sensor->y, 2) + powf(sensor->z, 2)))*180.0)/M_PI;
    }
    else
    {
        sensor->x = (float)0.0;
        sensor->y = (float)0.0;
        sensor->z = (float)0.0;
        sensor->pitch = (float)0.0;
        sensor->roll = (float)0.0;
    }

    return ret;
}

