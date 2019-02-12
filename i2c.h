#ifndef _I2C_H
#define _I2C_H

#define EEPROM_ADDR 0x50

#define I2C_UPDATE_INTERVAL 100

extern event_source_t i2cPoll;
extern uint8_t limits;

/*
 * Initialize I2C sybsystem, timer and thread
 *
 * Timer sends event to thread every 100 ms
 * on this event, thread will update temperature reading
 */
extern void initI2c(void);
extern msg_t setreg(uint8_t addr, uint8_t reg, uint8_t val);
extern uint8_t getreg(uint8_t addr, uint8_t reg);

#endif // _I2C_H

