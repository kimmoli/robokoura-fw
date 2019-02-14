#ifndef _ADC_H
#define _ADC_H

#define EXT_VREF                   ( (float)(1.9) )
#define ADC_MEAS12V_SCALE          ( (float)(EXT_VREF/4096*(5900+825)/825) )

#define ADC_UPDATE_INTERVAL         200

/* Total number of channels to be sampled by a single ADC operation.*/
#define ADC_GRP1_NUM_CHANNELS   3

/* Depth of the conversion buffer, channels are sampled four times each.*/
#define ADC_GRP1_BUF_DEPTH      4

struct MeasurementDriver
{
    uint32_t raw[ADC_GRP1_NUM_CHANNELS];
    float reading[ADC_GRP1_NUM_CHANNELS];
};

typedef struct MeasurementDriver MeasurementDriver;

extern MeasurementDriver MEASD1;

void initMeasurement(void);
void adcStartConv(void);

#endif // _ADC_H
