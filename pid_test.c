#include "ch.h"
#include "hal.h"
#include "pid.h"
#include <stdlib.h>
#include "sensor.h"
#include "stepper.h"

pidc_t pid;
static float input = 0, output = 0, target = 0;
static virtual_timer_t pidVt;
extern bool autoaxis;

static void pidVtCb(void *p)
{
    (void) p;

    chSysLockFromISR();
    chVTResetI(&pidVt);
    chVTSetI(&pidVt, MS2ST(100), pidVtCb, NULL);
    chSysUnlockFromISR();

    input = activeSensors[1]->pitch;

    pid_compute(&pid);

    STEPPERD5.noAccel = autoaxis;

    if (autoaxis)
    {
        if (output >= 0)
            setStepper(&STEPPERD5, output, DIR_CCW);
        else
            setStepper(&STEPPERD5, abs(output), DIR_CW);
    }
}

void start_pid(float p, float i, float d)
{
    pid_create(&pid, &input, &output, &target, p, i, d, PID_ON_M, PID_DIRECT);
    pid_setOutputLimits(&pid, -5000.0, 5000.0);
    pid_setSampleTime(&pid, 100);
    pid_setMode(&pid, PID_AUTOMATIC);

    chVTSet(&pidVt, MS2ST(100), pidVtCb, NULL);
}

