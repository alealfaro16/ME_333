#include "adc.h"
#include <xc.h>

void init_adc(void){
AD1PCFGbits.PCFG0 = 0;                 // AN0 is an adc pin
AD1CON3bits.ADCS = 2;                   // Tad = 75ns
AD1CON3bits.SAMC = 2;           //        sample for 2 Tad                                    //
AD1CON1bits.ADON = 1;                   // turn on A/D converter
}

int adc_sample_convert(void) {
  unsigned int elapsed = 0, finish_time = 0;
  int readings[5], avg = 0,sum = 0, i;

  for (i = 0; i < 5; i = i + 1){
    AD1CHSbits.CH0SA = 0;  //Choose AN0 to get sampled
    AD1CON1bits.SAMP = 1;  //Start sampling
    elapsed = _CP0_GET_COUNT();
    finish_time = elapsed + SAMPLE_TIME;
    while (_CP0_GET_COUNT() < finish_time) {
      ;                                   // sample for more than 250 ns
    }
    AD1CON1bits.SAMP = 0;                 // stop sampling and start converting
    while (!AD1CON1bits.DONE) {
      ;                                   // wait for the conversion process to finish
    }
    readings[i] = ADC1BUF0;
  }
  for(i=0;i<5;i=i+1){
    sum += readings[i];
  }
  avg = sum/5;
  return avg;
}
