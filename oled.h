#ifndef _OLED_H
#define _OLED_H

#define OLED_ADDR 0x3c

#define OLEDWIDTH 128
#define OLEDHEIGHT 32
#define SCREENBUFFERSIZE ((OLEDWIDTH/8)*OLEDHEIGHT)

extern void initOled(void);
extern void updateOled(void);

extern void clearOled(void);
extern void invertOled(bool invert);
extern void drawPixel(int x, int y, int color);
extern void drawLine(int x0, int y0, int x1, int y1, int color);
extern void setBuffer(int addr, uint8_t data);
extern void drawCircle(int x0, int y0, int r, int color);
extern void oledBounce(int delay, int count);
extern void drawRect(int x, int y, int w, int h, int color);
extern void fillRect(int x, int y, int w, int h, int color);
extern void drawChar(int x, int y, unsigned char c, int color, int size);
extern void drawText(int x, int y, char* text, int size);
extern void antiAlias(void);

#endif // _OLED_H

