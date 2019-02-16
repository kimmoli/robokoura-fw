#ifndef _NEOPIXEL_H
#define _NEOPIXEL_H

#define COLOR_RED   0x00ff0000
#define COLOR_GREEN 0x0000ff00
#define COLOR_BLUE  0x000000ff
#define COLOR_WHITE 0x00ffffff
#define COLOR_BLACK 0x00000000

#define NUMLEDS 32
#define UPDATE_INTERVAL 1

typedef struct
{
    uint32_t color[2];
    uint32_t dim;
    uint32_t delay;
} ledloopConfig_t;

extern uint32_t neoLedColors[];
extern ledloopConfig_t *ledloopConfig;
extern void initNeopixel(void);
extern uint8_t neoTxBuf[8 * NUMLEDS];

extern uint32_t dim(uint32_t color, uint32_t p);
extern void blink(void);
#endif // _NEOPIXEL_H

