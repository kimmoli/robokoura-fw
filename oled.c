#include "hal.h"
#include "oled.h"
#include <math.h>
#include <string.h>
#include "helpers.h"

#include "font.c"

static uint8_t screenBuffer[SCREENBUFFERSIZE] = { 0 };

void initOled(void)
{
    uint8_t init_seq[30] = {0xae,         /* display off */
                            0x20,0x01,    /* memory addressing mode = Vertical 01 Horizontal 00 */
                            0xb0,         /* page start address 0 */
                            0xc0,         /* scan direction normal */
                            0x00,         /* lower column start address */
                            0x10,         /* higher column start address */
                            0x40,         /* display start line */
                            0x81,0x8f,    /* contrast */
                            0xa0,         /* segment remap */
                            0xa6,         /* normal display  (a7 = inverse) */
                            0xa8,OLEDHEIGHT-1,    /* mux ratio */
                            0xa4,         /* display follows gdram */
                            0xd3,0x00,    /* display offset = 0*/
                            0xd5,0x80,    /* oscillator */
                            0xd9,0xf1,    /* precharge period */
                            0xda,0x02,    /* configure COM pins */
                            0xdb,0x40,    /* set VCOM level */
                            0x23,0x00,    /* disable blinks and fading */
                            0x8d,0x14,    /* enable charge pump. (0x10 disables) */
                            0xaf};        /* display on*/


    uint8_t pages[6] = {    0x21,         /* Column address */
                            0x00, OLEDWIDTH-1,
                            0x22,         /* page address */
                            0x00, 0x03 };

    uint8_t txBuf[2];
    int i;

    txBuf[0] = 0x80; /* Control reg */

    i2cAcquireBus(&I2CD1);

    for (i=0; i < 30 ; i++)
    {
        txBuf[1] = init_seq[i];
        i2cMasterTransmit(&I2CD1, OLED_ADDR, txBuf, 2, NULL, 0);
    }
    for (i=0; i < 6 ; i++)
    {
        txBuf[1] = pages[i];
        i2cMasterTransmit(&I2CD1, OLED_ADDR, txBuf, 2, NULL, 0);
    }

    i2cReleaseBus(&I2CD1);
}

void setBuffer(int addr, uint8_t data)
{
    screenBuffer[addr] = data;
}

void updateOled(void)
{
    int i;
    uint8_t txBuf[SCREENBUFFERSIZE+1];

    txBuf[0] = 0x40;

    for (i=0 ; i < SCREENBUFFERSIZE ; i++)
    {
        txBuf[i+1] = screenBuffer[i];
    }

    i2cAcquireBus(&I2CD1);
    i2cMasterTransmit(&I2CD1, OLED_ADDR, txBuf, SCREENBUFFERSIZE+1, NULL, 0);
    i2cReleaseBus(&I2CD1);
}

void clearOled(void)
{
    int i;

    for (i=0 ; i<SCREENBUFFERSIZE ; i++)
    {
        screenBuffer[i] = 0;
    }
}

void invertOled(bool invert)
{
    uint8_t txBuf[2] = {0};

    txBuf[0] = 0x80;
    txBuf[1] = (invert ? 0xa7 : 0xa6);

    i2cAcquireBus(&I2CD1);
    i2cMasterTransmit(&I2CD1, OLED_ADDR, txBuf, 2, NULL, 0);
    i2cReleaseBus(&I2CD1);
}

void drawPixel(int x, int y, int color)
{
    uint8_t * sb = screenBuffer;

    if ((x < 0) || (x >= OLEDWIDTH) || (y < 0) || (y >= OLEDHEIGHT))
        return;
    // x is which column
    if (color != 0)
        (*(sb+(x*4)+(y/8))) |= 1 << ( y % 8 );
    else
        (*(sb+(x*4)+(y/8))) &= ~( 1 << ( y % 8 ));
}

void drawLine(int x0, int y0, int x1, int y1, int color)
{
    int dy = y1 - y0;
    int dx = x1 - x0;
    int stepx, stepy;

    if (dy < 0)
    {
        dy = -dy; stepy = -1;
    }
    else
    {
        stepy = 1;
    }
    if (dx < 0)
    {
        dx = -dx; stepx = -1;
    }
    else
    {
        stepx = 1;
    }

    dy <<= 1; // dy is now 2*dy
    dx <<= 1; // dx is now 2*dx

    drawPixel(x0, y0, color);

    if (dx > dy)
    {
        int fraction = dy - (dx >> 1); // same as 2*dy - dx
        while (x0 != x1)
        {
            if (fraction >= 0)
            {
                y0 += stepy;
                fraction -= dx; // same as fraction -= 2*dx
            }
            x0 += stepx;
            fraction += dy; // same as fraction -= 2*dy
            drawPixel(x0, y0, color);
        }
    }
    else
    {
        int fraction = dx - (dy >> 1);
        while (y0 != y1)
        {
            if (fraction >= 0)
            {
                x0 += stepx;
                fraction -= dy;
            }
            y0 += stepy;
            fraction += dx;
            drawPixel(x0, y0, color);
        }
    }
}

/* Draws circle with radius r to x0,y0 color 1=white, 0=black */
void drawCircle(int x0, int y0, int r, int color)
{
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    drawPixel(x0 , y0+r, color);
    drawPixel(x0 , y0-r, color);
    drawPixel(x0+r, y0 , color);
    drawPixel(x0-r, y0 , color);

    while (x<y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void oledBounce(int delay, int count)
{
    int i, x, y, maxx, maxy, r;
    double dx, dy, mx, my;
    int flagy = 0;
    int flagx = 0;

    /* mid positions in x and y-axis */
    dx = 64.0;
    dy = 16.0;
    maxx = 127;
    maxy = 31;
    r = 5;
    i = count;

    mx = (double) 2.7;
    my = (double) 1.2;

    clearOled();

    while (i > 0 || count == 0)
    {
        if (count > 0)
            i--;

        x = (int)dx;
        y = (int)dy;

        if (y >= maxy-r || y <= r)
        {
            flagy = !flagy;
        }

        if (x >= maxx-r || x <= r)
        {
            flagx = !flagx;
        }

        drawCircle(x, y, r, 1);
        updateOled();
        drawCircle(x, y, r, 0);

        if (delay > 0)
            chThdSleepMilliseconds(delay);

        if (flagy)
        {
            dy = dy + my;
        }
        else
        {
            dy = dy - my;
        }

        if (flagx)
        {
            dx = dx + mx;
        }
        else
        {
            dx = dx - mx;
        }
    }
}

void drawRect(int x, int y, int w, int h, int color)
{
    drawLine(x,     y,     x,     y+h-1, color);
    drawLine(x,     y+h-1, x+w-1, y+h-1, color);
    drawLine(x+w-1, y+h-1, x+w-1, y,     color);
    drawLine(x+w-1, y,     x,     y,     color);
}

void fillRect(int x, int y, int w, int h, int color)
{
    for (int16_t i=x; i < x+w; i++)
    {
        drawLine(i, y, i, y+h-1, color);
    }
}

void drawChar(int x, int y, unsigned char c, int color, int size)
{
    if ((x >= OLEDWIDTH)            || // Clip right
        (y >= OLEDHEIGHT)           || // Clip bottom
        ((x + 6 * size - 1) < 0) || // Clip left
        ((y + 8 * size - 1) < 0))   // Clip top
    {
        return;
    }

    for (int8_t i=0; i<5; i++ ) // Char bitmap = 5 columns
    {
        uint8_t line = font[c * 5 + i];

        for (int8_t j=0; j<8; j++, line >>= 1)
        {
            if (line & 1)
            {
                if (size == 1)
                    drawPixel(x+i, y+j, color);
                else
                    fillRect(x+i*size, y+j*size, size, size, color);
            }
            else
            {
                if (size == 1)
                    drawPixel(x+i, y+j, !color);
                else
                    fillRect(x+i*size, y+j*size, size, size, !color);
            }
        }
    }
}

void drawText(int x, int y, char* text, int size)
{
    int len = strlen(text);

    for (int i = 0 ; i< len; i++)
    {
        drawChar(x + i*size*6, y, text[i], 1, size);
    }

    if (size > 1)
        antiAlias();
}

void antiAlias(void)
{
    uint8_t * sb = screenBuffer;

    int x, y, xx, yy;
    int pp = 0;
    uint16_t p = 0;

    for (x=0; x < OLEDWIDTH-4; x++)
    {
        for (y=0 ; y< OLEDHEIGHT-4; y++)
        {
            pp = 0;
            p = 0;

            for (xx=x ; xx < x+4 ; xx++)
            {
                for (yy=y ; yy < y+4 ; yy++)
                {
                    if ( (*(sb+(xx*4)+(yy/8))) & (1 << ( yy % 8 )) )
                        p |= 1 << pp;
                    pp++;
                }
            }

            if (p == 0xcc33)
            {
                (*(sb+((x+1)*4)+((y+2)/8))) |= 1 << ( (y+2) % 8 );
                (*(sb+((x+2)*4)+((y+1)/8))) |= 1 << ( (y+1) % 8 );
            }
            else if (p == 0x33cc)
            {
                (*(sb+((x+1)*4)+((y+1)/8))) |= 1 << ( (y+1) % 8 );
                (*(sb+((x+2)*4)+((y+2)/8))) |= 1 << ( (y+2) % 8 );
            }
        }
    }
}

