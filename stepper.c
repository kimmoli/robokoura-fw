#include "hal.h"
#include "stepper.h"
#include "helpers.h"

#define STEPPER_UPDATE_INTERVAL 5
#define STEPPER_ACCEL 100
#define STEPPER_DECEL 500

virtual_timer_t stepperVt;
event_source_t stepperPoll;

StepperDriver STEPPERD1;
StepperDriver STEPPERD2;
StepperDriver STEPPERD3;
StepperDriver STEPPERD4;
StepperDriver STEPPERD5;
StepperDriver STEPPERD6;

StepperDriver *steppers[6] = {0};

static void updateStepper(StepperDriver *stepp);

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

static THD_FUNCTION(stepperThread, arg)
{
    (void) arg;

    event_listener_t elStepper;
    chEvtRegister(&stepperPoll, &elStepper, 8);

    while (!chThdShouldTerminateX())
    {
        chEvtWaitOne(EVENT_MASK(8));

        for (int i=0 ; i<6 ; i++)
        {
            // If direction changed, force speed to 0 until direction
            if (steppers[i]->setDirection != DIR_RETAIN &&
                steppers[i]->setDirection != steppers[i]->currentDirection)
            {
                steppers[i]->setFrequency = 0;
            }

            if (steppers[i]->setFrequency > (steppers[i]->currentFrequency + STEPPER_ACCEL))
            {
                steppers[i]->currentFrequency += STEPPER_ACCEL;
            }
            else if (steppers[i]->setFrequency < (steppers[i]->currentFrequency - STEPPER_DECEL))
            {
                steppers[i]->currentFrequency -= STEPPER_DECEL;
            }
            else
            {
                steppers[i]->currentFrequency = steppers[i]->setFrequency;
            }

            if (steppers[i]->currentFrequency == 0)
            {
                steppers[i]->currentDirection = steppers[i]->setDirection;
            }

            updateStepper(steppers[i]);
        }

/*            int i = 2;
            PRINT("%d %d -> %d %d -> %d\n\r", i, steppers[i]->setFrequency,
                                                         steppers[i]->currentFrequency,
                                                         steppers[i]->setDirection,
                                                         steppers[i]->currentDirection);
*/
    }

    chThdExit(MSG_OK);
}

void stepperVtCb(void *p)
{
    (void) p;

    chSysLockFromISR();
    chVTResetI(&stepperVt);
    chVTSetI(&stepperVt, MS2ST(STEPPER_UPDATE_INTERVAL), stepperVtCb, NULL);
    chEvtBroadcastI(&stepperPoll);
    chSysUnlockFromISR();
}

void setStepper(StepperDriver *stepp, int32_t frequency, uint32_t direction)
{
    stepp->setFrequency = frequency;
    stepp->setDirection = direction;
}

static void updateStepper(StepperDriver *stepp)
{
    uint32_t psc;
    uint32_t reload;
    PWMDriver *pwmp = stepp->pwmp;
    int32_t frequency = stepp->currentFrequency;
    uint32_t direction = stepp->currentDirection;

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
    STEPPERD1.directionLine = LINE_DIR_0;
    STEPPERD1.currentFrequency = 0;
    STEPPERD1.currentDirection = DIR_RETAIN;
    STEPPERD1.setFrequency = 0;
    STEPPERD1.setDirection = DIR_RETAIN;
    startStepper(&STEPPERD1);

    STEPPERD2.pwmp = &PWMD5;
    STEPPERD2.directionLine = LINE_DIR_1;
    STEPPERD2.currentFrequency = 0;
    STEPPERD2.currentDirection = DIR_RETAIN;
    STEPPERD2.setFrequency = 0;
    STEPPERD2.setDirection = DIR_RETAIN;
    startStepper(&STEPPERD2);

    STEPPERD3.pwmp = &PWMD8;
    STEPPERD3.directionLine = LINE_DIR_2;
    STEPPERD3.currentFrequency = 0;
    STEPPERD3.currentDirection = DIR_RETAIN;
    STEPPERD3.setFrequency = 0;
    STEPPERD3.setDirection = DIR_RETAIN;
    startStepper(&STEPPERD3);

    STEPPERD4.pwmp = &PWMD9;
    STEPPERD4.directionLine = LINE_DIR_3;
    STEPPERD4.currentFrequency = 0;
    STEPPERD4.currentDirection = DIR_RETAIN;
    STEPPERD4.setFrequency = 0;
    STEPPERD4.setDirection = DIR_RETAIN;
    startStepper(&STEPPERD4);

    STEPPERD5.pwmp = &PWMD10;
    STEPPERD5.directionLine = LINE_DIR_4;
    STEPPERD5.currentFrequency = 0;
    STEPPERD5.currentDirection = DIR_RETAIN;
    STEPPERD5.setFrequency = 0;
    STEPPERD5.setDirection = DIR_RETAIN;
    startStepper(&STEPPERD5);

    STEPPERD6.pwmp = &PWMD11;
    STEPPERD6.directionLine = LINE_DIR_5;
    STEPPERD6.currentFrequency = 0;
    STEPPERD6.currentDirection = DIR_RETAIN;
    STEPPERD6.setFrequency = 0;
    STEPPERD6.setDirection = DIR_RETAIN;
    startStepper(&STEPPERD6);

    steppers[0] = &STEPPERD1;
    steppers[1] = &STEPPERD2;
    steppers[2] = &STEPPERD3;
    steppers[3] = &STEPPERD4;
    steppers[4] = &STEPPERD5;
    steppers[5] = &STEPPERD6;

    chEvtObjectInit(&stepperPoll);
    chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(2048), "stepper", NORMALPRIO+1, stepperThread, NULL);

    chVTSet(&stepperVt, MS2ST(STEPPER_UPDATE_INTERVAL), stepperVtCb, NULL);
}
