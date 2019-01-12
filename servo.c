#include "hal.h"
#include "servo.h"
#include "helpers.h"

ServoDriver SERVOD1;
ServoDriver SERVOD2;

const PWMConfig servoConfig =
{
    1000000,
    20000,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
    },
    0,
    0
};

/* Set servo output in us */
void setServo(ServoDriver *servop, uint32_t value)
{
    pwmEnableChannelI(servop->pwmp, servop->channel, value);
}

void initServo(void)
{
    pwmStart(&PWMD3, &servoConfig);

    SERVOD1.pwmp = &PWMD3;
    SERVOD1.channel = 0;
    setServo(&SERVOD1, 1500);

    SERVOD2.pwmp = &PWMD3;
    SERVOD2.channel = 1;
    setServo(&SERVOD2, 1500);
}


