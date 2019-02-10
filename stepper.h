#ifndef _STEPPER_H
#define _STEPPER_H

struct StepperDriver
{
    PWMDriver *pwmp;
    uint32_t directionLine;
};

typedef struct StepperDriver StepperDriver;

extern StepperDriver STEPPERD1;
extern StepperDriver STEPPERD2;
extern StepperDriver STEPPERD3;
extern StepperDriver STEPPERD4;
extern StepperDriver STEPPERD5;
extern StepperDriver STEPPERD6;

#define DIR_CW     PAL_LOW
#define DIR_CCW    PAL_HIGH
#define DIR_RETAIN 9U

#define RATIOD1 50
#define RATIOD2 50
#define RATIOD3 50
#define RATIOD4 50
#define RATIOD5 50

extern void initStepper(void);
extern void setStepper(StepperDriver *stepp, uint32_t frequency, uint32_t direction);

#endif

