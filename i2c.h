#ifndef _I2C_H
#define _I2C_H

#define EEPROM_ADDR 0x50
#define ACCEL0_ADDR 0x1c
#define ACCEL1_ADDR 0x1d

#define I2C_UPDATE_INTERVAL 50

typedef struct
{
    int16_t sensorPresent[2];
    float x[2];
    float y[2];
    float z[2];
    float Pitch[2];
    float Roll[2];
} I2CValues_t;

extern I2CValues_t *I2CValues;
extern event_source_t i2cPoll;
extern bool autoaxis;

/*
 * Initialize I2C sybsystem, timer and thread
 *
 * Timer sends event to thread every 100 ms
 * on this event, thread will update temperature reading
 */
extern void initI2c(void);
extern msg_t setreg(uint8_t addr, uint8_t reg, uint8_t val);

#endif // _I2C_H

