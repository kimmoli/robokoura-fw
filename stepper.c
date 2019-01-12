#include "hal.h"
#include "stepper.h"
#include "helpers.h"

StepperDriver STEPPERD1;
StepperDriver STEPPERD2;
StepperDriver STEPPERD3;
StepperDriver STEPPERD4;
StepperDriver STEPPERD5;
StepperDriver STEPPERD6;

const PWMConfig stepperConfig =
{
    96000,                                    /* 96 kHz  */
    2,                                        /* 48 khZ ? */
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
    },
    0,
    0
};

void setStepper(StepperDriver *stepp, uint32_t frequency, uint32_t direction)
{
    uint32_t psc;
    uint32_t reload;
    PWMDriver *pwmp = stepp->pwmp;

    if (direction != DIR_RETAIN)
        palWriteLine(stepp->directionLine, direction);

    if (frequency >= 1000)
    {
        psc = 2;
    }
    else if (frequency >= 100)
    {
        psc = 20;
    }
    else if (frequency >= 10)
    {
        psc = 200;
    }
    else if (frequency >= 1)
    {
        psc = 2000;
    }
    else /* 0 turns output off */
    {
        pwmp->tim->CCR[0] = 0;
        return;
    }

    reload = pwmp->clock / (psc * frequency);

    pwmp->tim->PSC = psc -1;
    pwmp->tim->ARR = reload;
    pwmp->tim->CCR[0] = reload/2;
}

static void startStepper(StepperDriver *stepp)
{
    pwmStart(stepp->pwmp, &stepperConfig);
    setStepper(stepp, 0, DIR_CW);
}

void initStepper(void)
{
    STEPPERD1.pwmp = &PWMD4;
    STEPPERD1.directionLine = LINE_GPIOE_DIR1;
    startStepper(&STEPPERD1);

    STEPPERD2.pwmp = &PWMD5;
    STEPPERD2.directionLine = LINE_GPIOE_DIR2;
    startStepper(&STEPPERD2);

    STEPPERD3.pwmp = &PWMD8;
    STEPPERD3.directionLine = LINE_GPIOE_DIR3;
    startStepper(&STEPPERD3);

    STEPPERD4.pwmp = &PWMD9;
    STEPPERD4.directionLine = LINE_GPIOE_DIR4;
    startStepper(&STEPPERD4);

    STEPPERD5.pwmp = &PWMD10;
    STEPPERD5.directionLine = LINE_GPIOE_DIR5;
    startStepper(&STEPPERD5);

    STEPPERD6.pwmp = &PWMD11;
    STEPPERD6.directionLine = LINE_GPIOE_DIR6;
    startStepper(&STEPPERD6);
}


