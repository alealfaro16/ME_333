#include "position_control.h"
#include "NU32.h"          // constants, functions for startup and UART
#include <xc.h>

void init_pos(void){

  TRISBbits.TRISB1 = 0; //Initialize pin B1 as an output

  T2CONbits.ON = 1;       // turn Timer4 on, all defaults are fine (1:1 divider, etc.)
  T2CONbits.TCKPS = 6;   // Prescaler 1:64
  PR2 = 6249;  //For desired frequency of 200Hz
  TMR2 = 0;
  IPC2bits.T2IP = 7;                // step 4: interrupt priority
  IPC2bits.T2IS = 0;                // step 4: subp is 0, which is the default
  IFS0bits.T2IF = 0;                // step 5: clear Timer2 interrupt flag
  IEC0bits.T2IE = 1;                // step 6: enable Timer2 interrupt

}

static volatile int Eint = 0, eprev = 0;
static volatile int StoringData = 1;
static volatile int r_array[5000]; //Used to a 1000

void __ISR(_TIMER_2_VECTOR, IPL7SOFT) Position_Control(void) {
  int e = 0,Edot = 0;
  int c = 0;
  volatile static int r;
  static int counter = 0;
  static int y = 0, yt = 0;
  int cnew = 0, excess = 0, counter_max = 0;
  int KpP, KiP,KdP;


  StoringData = 1;

  //LATBINV = 0b10;
  switch(get_mode()){
    case 3://if(get_mode()==3){
    {
      r = (get_theta())*1000; //get desired theta in milidegrees
      encoder_counts();
      y = (int) ((encoder_counts()-32768)*270); //get y value
      //if (y>360000){ //incase of overflow
      //  excess = y-360000;
      //  SPI4BUF = 0;  //reset counter
      //  encoder_counts();
      //  y = (int) ((encoder_counts()-32768)*270) + excess;
      //}
      //else if(y<-360000){
      //  excess = y+360000;
      //  SPI4BUF = 0;  //reset counter
      //  encoder_counts();
      //  y = (int) ((encoder_counts()-32768)*270) + excess;
      //}
      e = r - y;
      Edot = e - eprev;
      Eint = Eint + e;
      KdP = get_position_Kd(); //use
      KpP = get_position_Kp();
      KiP = get_position_Ki();
      c = KpP*e + KiP*Eint +KdP*Edot;
      cnew = c/1000; //convert back
      eprev = e;

      if(cnew>100){ //Avoid saturation
        cnew = 100;
      }
      else if(c<-100){
        cnew= -100;
      }

      send_current(cnew);

      if (counter==1000){ //1000 loops of the ISR before it ends
        StoringData = 0;
        sent_data((r/1000),y/1000,StoringData); //Will send last data and finish HOLD
        //NU32_WriteUART3("DONE\n");
        counter = 0;
        set_mode(0);
      } else {
        sent_data((r/1000),y/1000,StoringData);
        counter++;
      }

      break;
    }
    case 4:
    {
      StoringData = 1; //Used to not be here
      counter_max = get_ref_array(r_array);
      r = r_array[counter]*1000; //get desired theta in milidegrees
      encoder_counts();
      yt = (int) ((encoder_counts()-32768)*270); //get y value

      e = r - yt;
      Edot = e - eprev;
      Eint = Eint + e;
      KdP = get_position_Kd(); //use
      KpP = get_position_Kp();
      KiP = get_position_Ki();
      c = KpP*e + KiP*Eint +KdP*Edot;
      cnew = c/1000; //convert back
      eprev = e;

      if(cnew>100){ //Avoid saturation
        cnew = 100;
      }
      else if(c<-100){
        cnew= -100;
      }

      send_current(cnew);

      if (counter==counter_max){ //1000 loops of the ISR before it ends
        StoringData = 0;
        sent_data((r/1000),yt/1000,StoringData); //Will send last data and finish HOLD
        //NU32_WriteUART3("DONE\n");
        counter = 0;
        set_mode(0);
      } else {
        sent_data((r/1000),yt/1000,StoringData);
        counter++;
      }

      break;
    }
  }
  IFS0bits.T2IF = 0;
}
