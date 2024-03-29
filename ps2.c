#include "hal.h"
#include "ps2.h"
#include "i2c.h"
#include "stepper.h"
#include "servo.h"
#include "neopixel.h"
#include <stdlib.h>
#include "helpers.h"
#include "pid_test.h"

PS2Values_t *PS2Values;
event_source_t PS2Poll;
static uint8_t rx[100] = {0};

//uint8_t poll[]={0x01,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t poll[]={0x01,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

uint8_t enter_config[]={0x01,0x43,0x00,0x01,0x00};
uint8_t set_mode[]={0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00};
uint8_t set_bytes_large[]={0x01,0x4F,0x00,0xFF,0xFF,0x03,0x00,0x00,0x00};
uint8_t exit_config[]={0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A};
uint8_t enable_rumble[]={0x01,0x4D,0x00,0x00,0x01};

static void gptps2cb(GPTDriver *gpt_ptr);

const GPTConfig gptps2cfg =
{
    1000,      // timer clock: 1 kHz ~ 1 ms
    gptps2cb,  // Timer callback function
    0,
    0
};

void sendPS2(uint8_t *tx, int count, uint8_t *rx)
{
    int i;

    palClearLine(LINE_SPI_CS_PS2_N);

    for (i=0 ; i < count; i++)
    {
        spiExchange(&SPID1, 1, tx+i, rx+i);
    }

    palSetLine(LINE_SPI_CS_PS2_N);
}

void sendPS2Config(void)
{
    sendPS2(enter_config, sizeof(enter_config), rx);
    chThdSleepMilliseconds(100);
    sendPS2(set_mode, sizeof(set_mode), rx);
    chThdSleepMilliseconds(100);
    sendPS2(set_bytes_large, sizeof(set_bytes_large), rx);
    chThdSleepMilliseconds(100);
    sendPS2(enable_rumble, sizeof(enable_rumble), rx);
    chThdSleepMilliseconds(100);
    sendPS2(exit_config, sizeof(exit_config), rx);
    chThdSleepMilliseconds(10);

    PS2Values->reconfigCount++;
}

static THD_FUNCTION(PS2Thread, arg)
{
    (void)arg;
    event_listener_t elPS2;
    bool sel = false;
    bool start = false;

    chEvtRegister(&PS2Poll, &elPS2, 9);

    while (!chThdShouldTerminateX())
    {
        chEvtWaitOne(EVENT_MASK(9));

        if (PS2Values->reconfig)
        {
            sendPS2Config();
            PS2Values->reconfig = 0;
            continue;
        }

        poll[3] = PS2Values->motor1;
        poll[4] = PS2Values->motor2;

        sendPS2(poll, sizeof(poll), rx);

        if (rx[1] == 0x79)
        {
            PS2Values->buttons = ~((rx[3] << 8) | rx[4]);
            PS2Values->analog_right_hor = (int)rx[5] - 128;
            PS2Values->analog_right_ver = (int)rx[6] - 128;
            PS2Values->analog_left_hor = (int)rx[7] - 128;
            PS2Values->analog_left_ver = (int)rx[8] - 128;
            PS2Values->pressure_right = (int)rx[9];
            PS2Values->pressure_left = (int)rx[10];
            PS2Values->pressure_up = (int)rx[11];
            PS2Values->pressure_down = (int)rx[12];
            PS2Values->pressure_triangle = (int)rx[13];
            PS2Values->pressure_circle = (int)rx[14];
            PS2Values->pressure_x = (int)rx[15];
            PS2Values->pressure_square = (int)rx[16];
        }
        else
        {
            PS2Values->reconfig = 1;
        }

        PS2Values->count++;

        if (PS2Values->buttons & BUTTON_START)
        {
            PS2Values->motor2 = 255;

            if (!start)
            {
                autoaxis = !autoaxis;
                if (autoaxis)
                {
                    start_pid(550.0, 45.0, 3.0);
                }
                start = true;
            }
        }
        else
        {
            PS2Values->motor2 = 0;
            start = false;
        }

        if (PS2Values->buttons & BUTTON_SELECT)
        {
            if (!sel)
            {
                palToggleLine(LINE_ENABLE_N);
                ledLoop();
                sel = true;
            }
        }
        else
        {
            sel = false;
        }

        /* Stepper 1, axis 1, main rotate, left and right buttons */
        if (PS2Values->buttons & BUTTON_LEFT)
        {
            setStepper(&STEPPERD1, RATIOD1 * PS2Values->pressure_left, DIR_CCW);
        }
        else if (PS2Values->buttons & BUTTON_RIGHT)
        {
            setStepper(&STEPPERD1, RATIOD1 * PS2Values->pressure_right, DIR_CW);
        }
        else
        {
            setStepper(&STEPPERD1, 0, DIR_RETAIN);
        }

        /* stepper 2 not used */

        /* stepper 3, axis 2, main up down, up and down buttons */
        if (PS2Values->buttons & BUTTON_UP)
        {
            setStepper(&STEPPERD3, RATIOD3 * PS2Values->pressure_up, DIR_CCW);
        }
        else if (PS2Values->buttons & BUTTON_DOWN)
        {
            setStepper(&STEPPERD3, RATIOD3 * PS2Values->pressure_down, DIR_CW);
        }
        else
        {
            setStepper(&STEPPERD3, 0, DIR_RETAIN);
        }

        /* stepper 4, axis 3, second up down, triangle and X */
        if (PS2Values->buttons & BUTTON_TRIANGLE && (limits & 0x02) == 0x02)
        {
            setStepper(&STEPPERD4, RATIOD4 * PS2Values->pressure_triangle, DIR_CCW);
        }
        else if (PS2Values->buttons & BUTTON_X && (limits & 0x01) == 0x01)
        {
            setStepper(&STEPPERD4, RATIOD4 * PS2Values->pressure_x, DIR_CW);
        }
        else
        {
            setStepper(&STEPPERD4, 0, DIR_RETAIN);
        }

        /* stepper 5, arm up down, buttons square and circle */
        if (PS2Values->buttons & BUTTON_SQUARE)
        {
            autoaxis = false;
            setStepper(&STEPPERD5, RATIOD5 * PS2Values->pressure_square, DIR_CCW);
        }
        else if (PS2Values->buttons & BUTTON_CIRCLE)
        {
            autoaxis = false;
            setStepper(&STEPPERD5, RATIOD5 * PS2Values->pressure_circle, DIR_CW);
        }
        else if (!autoaxis)
        {
            setStepper(&STEPPERD5, 0, DIR_RETAIN);
        }

        /* stepper 6, arm rotate, L1 and L2 buttons */
        if (PS2Values->buttons & BUTTON_L1)
        {
            setStepper(&STEPPERD6, RATIOD6 * 10, DIR_CCW);
        }
        else if (PS2Values->buttons & BUTTON_L2)
        {
            setStepper(&STEPPERD6, RATIOD6 * 10, DIR_CW);
        }
        else
        {
            setStepper(&STEPPERD6, 0, DIR_RETAIN);
        }

        /* servo 1, arm clamp, R1 and R2 buttons */
        if (PS2Values->buttons & BUTTON_R1)
        {
            SERVOD1.value += 5;
        }
        else if (PS2Values->buttons & BUTTON_R2)
        {
            SERVOD1.value -= 5;
        }

        updateServo(&SERVOD1);
    }

    chThdExit(MSG_OK);
}


void gptps2cb(GPTDriver *gpt_ptr)
{
    (void) gpt_ptr;

    osalSysLockFromISR();
    chEvtBroadcastI(&PS2Poll);
    osalSysUnlockFromISR();
}

void initPS2(void)
{
    chEvtObjectInit(&PS2Poll);

    PS2Values = chHeapAlloc(NULL, sizeof(PS2Values_t));

    PS2Values->buttons = 0;
    PS2Values->reconfig = 0;
    PS2Values->analog_left_hor = 0;
    PS2Values->analog_left_ver = 0;
    PS2Values->analog_right_hor = 0;
    PS2Values->analog_right_ver = 0;
    PS2Values->pressure_right = 0;
    PS2Values->pressure_left = 0;
    PS2Values->pressure_up = 0;
    PS2Values->pressure_down = 0;
    PS2Values->pressure_triangle = 0;
    PS2Values->pressure_circle = 0;
    PS2Values->pressure_x = 0;
    PS2Values->pressure_square = 0;
    PS2Values->motor1 = 0;
    PS2Values->motor2 = 0;
    PS2Values->count = 0;
    PS2Values->reconfigCount = 0;

    sendPS2Config();

    chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(512), "ps2", NORMALPRIO+1, PS2Thread, NULL);

    gptStart(&GPTD12, &gptps2cfg);
    gptStartContinuous(&GPTD12, PS2_UPDATE_INTERVAL);
}

