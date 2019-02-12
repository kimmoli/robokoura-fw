#ifndef _SENSOR_H
#define _SENSOR_H

#define IIS328_REG_AUTOINC 0x80
#define IIS328_CTRL1REG 0x20
#define IIS328_CTRL1REG_VALUE 0xC7 /* Low power 10Hz, 37Hz LPF, XYZ enabled */
#define IIS328_CTRL4REG 0x23
#define IIS328_CTRL4REG_VALUE 0xF0 /* No update between reads, MSB, 8g, selftest disabled */
#define IIS328_OUTREG 0x28
#define IIS328_8G_SCALE (0.00391)

struct SensorDriver
{
    uint8_t instance;
    bool active;
    uint8_t i2cAddressIO;
    uint8_t i2cAddressAcc;
    uint8_t leds;
    uint8_t inputs;
    float x;
    float y;
    float z;
    float pitch;
    float roll;
};

typedef struct SensorDriver SensorDriver;

extern SensorDriver SENSORD1;
extern SensorDriver SENSORD2;
extern SensorDriver SENSORD3;
extern SensorDriver SENSORD4;
extern SensorDriver SENSORD5;

extern SensorDriver *activeSensors[6];
extern int maxSensors;

extern bool sensorsEnabled;

extern void initSensor(void);
extern bool detectSensor(SensorDriver *sensor);
extern msg_t updateAcceleration(SensorDriver *sensor);

#endif
