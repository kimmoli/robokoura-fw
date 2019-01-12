#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "chprintf.h"
#include "shellcommands.h"
#include "oled.h"

void cmd_oled(BaseSequentialStream *chp, int argc, char *argv[])
{
    static volatile bool invert = false;

    if (argc >= 1)
    {
        if (strcmp(argv[0], "init") == 0)
        {
            initOled();
        }
        else if (strcmp(argv[0], "clr") == 0)
        {
            clearOled();
        }
        else if (strcmp(argv[0], "inv") == 0)
        {
            invert = !invert;
            invertOled(invert);
        }
        else if ((strncmp(argv[0], "p", 1) == 0) && (argc >=3))
        {
            drawPixel(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 10), 1);
        }
        else if ((strncmp(argv[0], "li", 2) == 0) && (argc >=5))
        {
            drawLine(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 10), strtol(argv[3], NULL, 10), strtol(argv[4], NULL, 10), 1);
        }
        else if ((strncmp(argv[0], "ci", 2) == 0) && (argc >=4))
        {
            drawCircle(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 10), strtol(argv[3], NULL, 10), 1);
        }
        else if ((strcmp(argv[0], "s") == 0) && (argc >=3))
        {
            setBuffer(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 16));
        }
        else if ((strcmp(argv[0], "bounce") == 0) && (argc >= 2))
        {
            if (argc == 2)
                oledBounce(strtol(argv[1], NULL, 10), 0);
            else
                oledBounce(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 10));
        }
        else if ((strncmp(argv[0], "r", 1) == 0) && (argc >= 5))
        {
            fillRect(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 10), strtol(argv[3], NULL, 10), strtol(argv[4], NULL, 10), 1);
        }
        else if ((strncmp(argv[0], "w", 1) == 0) && (argc >= 4))
        {
            int size = 1;

            if (argc == 5)
                size = strtol(argv[4], NULL, 10);

            drawText(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 10), argv[3], size);
        }
        else if (strncmp(argv[0], "aa", 2) == 0)
        {
            antiAlias();
        }
        else
        {
            chprintf(chp, "oled init, clr, invert, pixel x y, line, x0 y0 x1 y1, circle x y r, rect x y w h, write x y text\n\r");
            return;
        }

        updateOled();
    }
}

