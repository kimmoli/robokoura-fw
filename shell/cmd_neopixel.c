#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"
#include "neopixel.h"
#include "helpers.h"

static virtual_timer_t ledloopVt;

const uint8_t HSVpower[121] =
{0, 2, 4, 6, 8, 11, 13, 15, 17, 19, 21, 23, 25, 28, 30, 32, 34, 36, 38, 40,
42, 45, 47, 49, 51, 53, 55, 57, 59, 62, 64, 66, 68, 70, 72, 74, 76, 79, 81,
83, 85, 87, 89, 91, 93, 96, 98, 100, 102, 104, 106, 108, 110, 113, 115, 117,
119, 121, 123, 125, 127, 130, 132, 134, 136, 138, 140, 142, 144, 147, 149,
151, 153, 155, 157, 159, 161, 164, 166, 168, 170, 172, 174, 176, 178, 181,
183, 185, 187, 189, 191, 193, 195, 198, 200, 202, 204, 206, 208, 210, 212,
215, 217, 219, 221, 223, 225, 227, 229, 232, 234, 236, 238, 240, 242, 244,
246, 249, 251, 253, 255};

uint32_t parseColor(char *arg)
{
    uint32_t color = 0;

    if (strcmp(arg, "r") == 0)
        color = COLOR_RED;
    else if (strcmp(arg, "g") == 0)
        color = COLOR_GREEN;
    else if (strcmp(arg, "b") == 0)
        color = COLOR_BLUE;
    else if (strcmp(arg, "w") == 0)
        color = COLOR_WHITE;
    else
        color = strtol(arg, NULL, 16);

    return color;
}

void ledrainbow(void *p)
{
    (void) p;
    int i;
    static volatile uint32_t led = 0;
    static volatile uint32_t angle0 = 0;

    angle0++;

    for (i=0; i < NUMLEDS ; i++)
    {
        uint8_t red, green, blue;
        uint32_t angle = (angle0 + (i * (360/NUMLEDS))) % 360;

        if (angle<120)
        {
            red = HSVpower[120-angle];
            green = HSVpower[angle];
            blue = 0;
        }
        else if (angle<240)
        {
            red = 0;
            green = HSVpower[240-angle];
            blue = HSVpower[angle-120];
        }
        else
        {
            red = HSVpower[angle-240];
            green = 0;
            blue = HSVpower[360-angle];
        }

        neoLedColors[i] = dim((red << 16) | (green << 8) | blue, ledloopConfig->dim);
    }

    if (led++ >= NUMLEDS)
        led = 0;

    chSysLockFromISR();
    chVTResetI(&ledloopVt);
    chVTSetI(&ledloopVt, MS2ST(ledloopConfig->delay), ledrainbow, NULL);
    chSysUnlockFromISR();
}

void cmd_neopixel(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) chp;

    int i;
    uint32_t color;

    color = 0;

    if (argc >= 1)
    {
        if (strcmp(argv[0], "dump") == 0)
        {
            dump((char *)neoTxBuf, 8*NUMLEDS);
        }
        else if (strcmp(argv[0], "set") == 0)
        {
            if (argc >= 3)
            {
                color = parseColor(argv[2]);
            }

            if (argc >= 4 )
            {
                color = dim(color, strtol(argv[3], NULL, 10));
            }

            if (argc >= 2)
            {
                int led = strtol(argv[1], NULL, 10);

                if (led >= 0 && led < NUMLEDS)
                    neoLedColors[led] = color;
            }
        }
        else if (strcmp(argv[0], "fill") == 0)
        {
            if (argc >= 2)
            {
                color = parseColor(argv[1]);
            }

            if (argc >= 3 )
            {
                color = dim(color, strtol(argv[2], NULL, 10));
            }

            for (i=0; i < NUMLEDS ; i++)
                neoLedColors[i] = color;
        }
        else if (strcmp(argv[0], "loop") == 0)
        {
            ledLoop();
        }
        else if (strcmp(argv[0], "rainbow") == 0)
        {
            if (argc >= 2)
            {
                if (strcmp(argv[1], "stop") == 0)
                {
                    if (chVTIsArmed(&ledloopVt))
                        chVTReset(&ledloopVt);
                    return;
                }

                ledloopConfig->delay = strtol(argv[1], NULL, 10);
            }
            else
            {
                ledloopConfig->delay = 100;
            }

            if (argc >= 3)
            {
                ledloopConfig->dim = strtol(argv[2], NULL, 10);
            }
            else
            {
                ledloopConfig->dim = 100;
            }

            if (chVTIsArmed(&ledloopVt))
                chVTReset(&ledloopVt);
            chVTSet(&ledloopVt, MS2ST(ledloopConfig->delay), ledrainbow, NULL);
        }
        else if (strcmp(argv[0], "blink") == 0)
        {
            ledBlink();
        }
        else if (strcmp(argv[0], "stop") == 0)
        {
            if (chVTIsArmed(&ledloopVt))
            {
                chVTReset(&ledloopVt);
            }

            for (i=0; i < NUMLEDS ; i++)
            {
                neoLedColors[i] = 0;
            }
        }

    }
    else
    {
        chprintf(chp, "neo {command} {args}\n\r");
        chprintf(chp, " - fill {color} {brightness}\n\r");
        chprintf(chp, " - set {led#} {color} {brightness}\n\r");
        chprintf(chp, " - loop {color | stop} {brightness} {delay ms}\n\r");
        chprintf(chp, " - rainbow {delay ms | stop } {brightness}\n\r");
        chprintf(chp, "\n\r");
        chprintf(chp, "color = hex value or r,g,b,w\n\r");
        chprintf(chp, "brightness = 0...100\n\r");
    }
}

