#include "hal.h"
#include "neopixel.h"
#include "helpers.h"

static virtual_timer_t vt1;
static virtual_timer_t ledBlinkVt;
static virtual_timer_t ledLoopVt;
static void neoTXEnd(UARTDriver *uartp);
static void neoSend(void *p);
static void encodeBits( uint32_t color , uint8_t *buffer );

uint8_t neoTxBuf[8 * NUMLEDS] = {0};
uint32_t neoLedColors[NUMLEDS] = {0};
ledloopConfig_t *ledloopConfig;

const UARTConfig uartConf =
{
  NULL,   /* UART transmission buffer callback.           */
  neoTXEnd,   /* UART physical end of transmission callback.  */
  NULL,   /* UART Receiver receiver filled callback.      */
  NULL,   /* UART caracter received callback.             */
  NULL,   /* UART received error callback.                */
  1000000,  /* UART baudrate.                               */
  0,
  0,
  0
};


void initNeopixel(void)
{
    ledloopConfig = chHeapAlloc(NULL, sizeof(ledloopConfig_t));

    uartStart(&UARTD2, &uartConf);

    /* Tweak UART to 2.5Mbps, Chibi macros can't do this
        - Requires 8-bit oversampling
        - Divider is 1.5 (60 MHz/8*2.5 MHz)
        - Mantissa = 1
        - Fraction = 8*0.5 = 4

       Also configure UART to half-duplex mode (HDSEL)
       This will automatically release TX pin to a IO pin when
       data is not transmitted.
       IO is configured with pull-down, so all idle times are low
    */

    UARTD2.usart->CR1 &= ~USART_CR1_UE;
    UARTD2.usart->BRR = 0x10;
    UARTD2.usart->CR3 |= USART_CR3_HDSEL;
    UARTD2.usart->CR1 |= (USART_CR1_OVER8 | USART_CR1_UE);

    chVTSet(&vt1, MS2ST(UPDATE_INTERVAL), neoSend, NULL);

    for (int i = 0 ; i < NUMLEDS; i++)
        neoLedColors[i] = 0;
}

void neoSend(void *p)
{
    (void) p;

    int i = 0;

    for (i=0; i < NUMLEDS; i++)
    {
        encodeBits(neoLedColors[i], neoTxBuf+(8 * i));
    }

    chSysLockFromISR();
    uartStartSendI(&UARTD2, 8 * NUMLEDS +1, neoTxBuf);
    chSysUnlockFromISR();
}

void neoTXEnd(UARTDriver *uartp)
{
    (void)uartp;

    chSysLockFromISR();
    chVTResetI(&vt1);
    chVTSetI(&vt1, MS2ST(UPDATE_INTERVAL), neoSend, NULL);
    chSysUnlockFromISR();
}

// Encode a 24 bit value into 8 bytes where each byte has the format...
// 0b?0?10?10
// Where each ? is a bit from the orginal 24 bit value.
// This wonky format is designed to generate correct NeoPixel
// signals when sent out a serial port and surrounded with stop and start bits.

void encodeBits( uint32_t color , uint8_t *b )
{
    int bits=24;

    /* Swap data order, WS2812B data format is GRB */
    uint32_t x = (color & 0xff) | ((color & 0xff00) << 8) | ((color & 0xff0000) >> 8);

    while (bits)
    {

        // Process 3 bits of the input into 1 byte on the output
        //
        // Note that we processes the input by shifting up and checking the high bit (#23)
        // This is becuase NeoPixels actually take thier data in MSB first order
        // while serial ports send in LSB first order

        uint8_t t=0b00010010;     // initialize with all the known 1's

        if (x & (1 << 23))
        {
            t |= 0b00000100;
        }

        x <<= 1;
        bits--;

        if (x & (1 << 23))
        {
            t |= 0b00100000;
        }

        x <<= 1;
        bits--;

        if (x & (1 << 23) )
        {
            t |= 0b10000000;
        }

        x <<= 1;
        bits--;

        *b = t;

        b++;
    }
}

uint32_t dim(uint32_t color, uint32_t p)
{
    if (p >= 100)
        return color;

    return ((color & 0xff) * p / 100) |
           ((((color & 0xff00) >> 8) * p / 100) << 8) |
           ((((color & 0xff0000) >> 16) * p / 100) << 16);
}

void ledBlinkVtCb(void *p)
{
    (void) p;
    static volatile int state = 0;
    static volatile int delay = 300;

    for (int i=0 ; i < NUMLEDS ; i++)
    {
        neoLedColors[i] = dim(((state & 1) == 1) ? 0xffa500 : 0x000000, 2);
    }

    if (state == 6)
    {
        delay = 300;
        state = 0;
    }
    else
    {
        delay = ((state & 1) == 1) ? 50 : 100;
        state++;
    }

    chSysLockFromISR();
    chVTResetI(&ledBlinkVt);
    chVTSetI(&ledBlinkVt, MS2ST(delay), ledBlinkVtCb, NULL);
    chSysUnlockFromISR();
}

void ledLoopVtCb(void *p)
{
    (void) p;
    int i;
    static volatile uint32_t led0 = 0;

    for (i=0; i < NUMLEDS ; i++)
    {
        neoLedColors[i] = dim(0xffffff, 10);
        neoLedColors[led0 % NUMLEDS] = dim(0xffa500, 50);
        neoLedColors[(led0+1) % NUMLEDS] = dim(0xffa500, 50);
        neoLedColors[(led0+2) % NUMLEDS] = dim(0xffa500, 50);
        neoLedColors[(led0+3) % NUMLEDS] = dim(0xffa500, 50);
    }

    led0++;

    chSysLockFromISR();
    chVTResetI(&ledLoopVt);
    chVTSetI(&ledLoopVt, MS2ST(20), ledLoopVtCb, NULL);
    chSysUnlockFromISR();
}


void ledBlink(void)
{
    int i;

    if (chVTIsArmed(&ledBlinkVt))
    {
        chVTReset(&ledBlinkVt);

        for (i=0; i < NUMLEDS ; i++)
        {
            neoLedColors[i] = 0;
        }
    }
    else
    {
        chVTSet(&ledBlinkVt, MS2ST(100), ledBlinkVtCb, NULL);
    }
}

void ledLoop(void)
{
    int i;

    if (chVTIsArmed(&ledLoopVt))
    {
        chVTReset(&ledLoopVt);

        for (i=0; i < NUMLEDS ; i++)
        {
            neoLedColors[i] = 0;
        }
    }
    else
    {
        chVTSet(&ledLoopVt, MS2ST(30), ledLoopVtCb, NULL);
    }
}
