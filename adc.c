#include "hal.h"
#include "adc.h"
#include "helpers.h"

event_source_t adcConvStart;

MeasurementDriver MEASD1;

static void adccb1(ADCDriver *adcp, adcsample_t *buffer, size_t n);
static void gptadccb(GPTDriver *gpt_ptr);

const ADCConversionGroup adcgrpcfg1 =
{
    /* Circular */ FALSE,
    /* numbr of channeld */ ADC_GRP1_NUM_CHANNELS,
    /* conversion end callback*/ adccb1,
    /* conversion error callback */ NULL,
    /* ADC CR1 */ 0,
    /* ADC CR2 */ ADC_CR2_SWSTART,
    /* ADC SMPR1 */ ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_144) |  ADC_SMPR1_SMP_VREF(ADC_SAMPLE_144),
    /* ADC SMPR2 */ ADC_SMPR2_SMP_AN0(ADC_SAMPLE_144),
    /* ADC SQR1 */ ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
    /* ADC SQR2 */ 0,
    /* ADC SQR3 */ ADC_SQR3_SQ3_N(ADC_CHANNEL_VREFINT) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN0) | ADC_SQR3_SQ1_N(ADC_CHANNEL_SENSOR)
};

const GPTConfig gptadccfg =
{
    1000,      // timer clock: 1 kHz ~ 1 ms
    gptadccb,  // Timer callback function
    0,
    0
};

static adcsample_t adcSamples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
static adcsample_t adcAvgReading[ADC_GRP1_NUM_CHANNELS];

/*
 * ADC end conversion callback, ADC1
 */
void adccb1(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
    (void) buffer;
    (void) n;
    int i, j;

    /* Note, only in the ADC_COMPLETE state because the ADC driver fires an intermediate callback when the buffer is half full.*/
    if (adcp->state == ADC_COMPLETE)
    {
        /* Calculates the average values from the ADC samples.*/
        for (j = 0 ; j < ADC_GRP1_NUM_CHANNELS ; j++)
        {
            for (i = 0; i < ADC_GRP1_BUF_DEPTH; i++)
            {
                adcAvgReading[j] += adcSamples1[ i * ADC_GRP1_NUM_CHANNELS + j];
            }

            adcAvgReading[j] /= ADC_GRP1_BUF_DEPTH;
            MEASD1.raw[j] = adcAvgReading[j];
        }

        // Temp at 25C 0.76V, Average slope 2.5mV/C
        MEASD1.reading[0] = ((( EXT_VREF / 4096 * (float)adcAvgReading[0]) - 0.76  ) / 0.0025 ) + 30.0;
        MEASD1.reading[1] = ADC_MEAS12V_SCALE * (float)adcAvgReading[1];
    }
}

void gptadccb(GPTDriver *gpt_ptr)
{
    (void) gpt_ptr;

    osalSysLockFromISR();
    chEvtBroadcastI(&adcConvStart);
    adcStartConversionI(&ADCD1, &adcgrpcfg1, adcSamples1, ADC_GRP1_BUF_DEPTH);
    osalSysUnlockFromISR();
}

void initMeasurement(void)
{
    MEASD1.reading[0] = 0.0;
    MEASD1.reading[1] = 0.0;

    chEvtObjectInit(&adcConvStart);

    adcStart(&ADCD1, NULL);

    adcSTM32EnableTSVREFE();
    adcStartConv();
}

void adcStartConv(void)
{
    gptStart(&GPTD14, &gptadccfg);
    gptStartContinuous(&GPTD14, ADC_UPDATE_INTERVAL);
}

