#ifndef ADC__H__
#define ADC__H__

#define SAMPLE_TIME 10
#define VOLTS_PER_COUNT (3.3/1024)

void init_adc(void);
int adc_sample_convert(void);

#endif
