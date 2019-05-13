//current control
#include "current_control.h"
#include "utilities.h"
#include "NU32.h"          // constants, functions for startup and UART
#include <xc.h>

//Initialize RB1 (digital output), Timer3 and OC1
void init_pwm(void){
  //Digital Ouput:
  TRISDbits.TRISD1 = 0;   //initialize pin D1 as a digital output

  //PWM:
  T3CONbits.ON = 1;       // turn Timer3 on, all defaults are fine (1:1 divider, etc.)
  T3CONbits.TCKPS = 0;   // Prescaler 1:1
  PR3 = 3999;       // For a frequency of 20KHz
  TMR3 = 0;        // initialize value of Timer3

  OC1CONbits.OCTSEL = 1;  // use Timer3 for OC1
  OC1CONbits.OCM = 0b110; // PWM mode with fault pin disabled
  OC1CONbits.ON = 1;     // Turn OC1 on
  OC1RS = 1000;  //For a duty cycle of 25%
  //Current Control ISR:
  T4CONbits.ON = 1;       // turn Timer4 on, all defaults are fine (1:1 divider, etc.)
  T4CONbits.TCKPS = 1;   // Prescaler 1:2
  PR4 = 7999;  //For desired frequency of 5KHz
  TMR4 = 0;
  IPC4bits.T4IP = 6;                // step 4: interrupt priority
  IPC4bits.T4IS = 0;                // step 4: subp is 0, which is the default
  IFS0bits.T4IF = 0;                // step 5: clear Timer4 interrupt flag
  IEC0bits.T4IE = 1;                // step 6: enable Timer4 interrupt
}


static volatile int Eint = 0;
static volatile int ADCarray[100];
static volatile int REFarray[100];
static volatile int StoringData = 1;

void __ISR(_TIMER_4_VECTOR, IPL6SOFT) Curernt_Control(void) {
  int e = 0;
  int u = 0;
  volatile static int r = 200;
  static int counter = 0;
  static int adcval = 0;
  int unew = 0;
  int Kp, Ki;

  switch(get_mode()){
    case 0: //IDLE
    {
      OC1RS = 0; //Motor is in brake mode
      break;
    }
    case 1: //PWM
    {
      //OC1RS = get_duty_cycle(); //User sets duty cycle
      LATDbits.LATD1 = get_direction(); //0 = forward +ve  1 = reverse -ve
      OC1RS = get_duty_cycle();
      break;
    }
    case 2: //ITEST
    {
      LATDbits.LATD1 = 0;
      if(counter == 25 || counter == 50 || counter == 75){
        r = -1*r;
        //LATDINV = 0b10;
      }
      //adcval = adc_sample_convert();
      adcval = (int) (1.34*adc_sample_convert()-682.16);
      //adcval = (int) (adcval*834-418840)/1000;
      e = r - adcval;
      Eint = Eint + e;
      Kp = get_current_Kp();
      Ki = get_current_Ki();
      u = Kp*e + Ki*Eint;

      if(u < 0){

         LATDSET = 0b10;; // enable direction bit 0 which will be connected to
                        // APHASE in the Hbridge to make it go in reverse
         unew = u*-1;
      }

      else if(u >= 0) {

         //LATDCLR = 0b10; // enable direction bit 0 which will be connected to
                        // APHASE in the Hbridge to make it go forward
         unew = u;
      }

      if (unew > 100) {
        unew = 100;
      }

      OC1RS = (unsigned int) (unew/100)*(PR3+1); // duty cycle = OC1RS/(PR2+1) = 75%

      if (counter==100){
        StoringData = 0;
        sent_data(r,adcval,StoringData);
        //NU32_WriteUART3("DONE\n");
        counter = 0;
        set_mode(0);
      } else {
        sent_data(r,adcval,StoringData);
        counter++;
      }


      break;
    }
    case 3:
    {
      LATDbits.LATD1 = 0;
      r = get_desired_current();
      adcval = (int) (1.34*adc_sample_convert()-682.16);
      e = r - adcval;
      Eint = Eint + e;
      Kp = get_current_Kp();
      Ki = get_current_Ki();
      u = Kp*e + Ki*Eint;

      if(u < 0){

         LATDSET = 0b10;; // enable direction bit 0 which will be connected to
                        // APHASE in the Hbridge to make it go in reverse
         unew = u*-1;
      }

      else if(u >= 0) {

         //LATDCLR = 0b10; // enable direction bit 0 which will be connected to
                        // APHASE in the Hbridge to make it go forward
         unew = u;
      }

      if (unew > 100) {
        unew = 100;
      }

      OC1RS = (unsigned int) (unew/100)*(PR3+1); // duty cycle = OC1RS/(PR2+1) = 75%

      break;
    }

    case 4:
    {
      LATDbits.LATD1 = 0;
      r = get_desired_current();
      adcval = (int) (1.34*adc_sample_convert()-682.16);
      e = r - adcval;
      Eint = Eint + e;
      Kp = get_current_Kp();
      Ki = get_current_Ki();
      u = Kp*e + Ki*Eint;

      if(u < 0){

         LATDSET = 0b10;; // enable direction bit 0 which will be connected to
                        // APHASE in the Hbridge to make it go in reverse
         unew = u*-1;
      }

      else if(u >= 0) {

         //LATDCLR = 0b10; // enable direction bit 0 which will be connected to
                        // APHASE in the Hbridge to make it go forward
         unew = u;
      }

      if (unew > 100) {
        unew = 100;
      }

      OC1RS = (unsigned int) (unew/100)*(PR3+1); // duty cycle = OC1RS/(PR2+1) = 75%

      break;
    }

  }
  IFS0bits.T4IF = 0; //Clear flag
}
