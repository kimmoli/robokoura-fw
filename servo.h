#ifndef _SERVO_H
#define _SERVO_H

struct ServoDriver
{
    PWMDriver *pwmp;
    uint32_t channel;
};

typedef struct ServoDriver ServoDriver;

extern ServoDriver SERVOD1;
extern ServoDriver SERVOD2;

extern void initServo(void);
extern void setServo(ServoDriver *stepp, uint32_t value);

#endif

