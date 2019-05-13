#include "utilities.h"
#include <xc.h>
#include <stdio.h>

#define BUF_SIZE 200

enum Modes {
          IDLE = 0,
          PWM = 1,
          ITEST = 2,
          HOLD = 3,
          TRACK = 4
                };

static volatile int modep = 0; duty_cycle = 0,direction = 0; KpC = 0; KiC = 0;
static volatile int  KpP = 0; KiP = 0, KdP = 0; desired_theta = -1000; desired_current = 0;
static volatile int ADCarrayU[5000]; //Used to be 1000 size array
static volatile int REFarrayU[1000];
static volatile int StoringData = 1;
static volatile int s_ref = 0;
static volatile int ref_position[5000];

void set_mode(int mode){
  char buffer[BUF_SIZE];

  switch(mode){
    case IDLE:
    {
      modep = 0;
      duty_cycle = 0;
      break;
    }
    case PWM:
    {
      modep = 1;
      NU32_ReadUART3(buffer,BUF_SIZE);
      sscanf(buffer, "%d" , &duty_cycle);
      if(duty_cycle < 0){
        duty_cycle = -1*duty_cycle;
        direction = 1; //Go Revere (CW) direction
      }
      else{
        direction = 0; //Go Foward (CCW) direction
      }
      break;
    }
    case ITEST:
    {
      modep = 2;
      StoringData = 1;
      break;
    }
    case HOLD:
    {
      StoringData = 1;
      NU32_ReadUART3(buffer,BUF_SIZE);
      sscanf(buffer, "%d" , &desired_theta);
      while(desired_theta==-1000){
        //wait for user input
      }
      modep = 3; //Trigger ISRs
      break;
    }
    case TRACK:
    {
      StoringData = 1;
      modep = 4; //Trigger ISRs
      break;
    }
    default: {break;}
  }
}

void sent_data(int r_data, int adc_data, int Str_data){
  static int i = 0; //used to be static
  ADCarrayU[i] = adc_data;
  REFarrayU[i] = r_data;
  StoringData =Str_data;
  i++;
  if(Str_data==0){
    i = 0;
    desired_theta = -1000;
    Str_data = 1;
  }
}

void send_current(int u){
  desired_current = u;
}

void send_ref_array(int* ref_array,int size_ref){
  s_ref = size_ref;
  int j; //used to not be static
  for(j=0; j<size_ref;j++){
    *(ref_position + j) = ref_array[j];
  }
}

int get_desired_current(void){
  return desired_current;
}

int get_Str_data(void){
  return StoringData;
}
void get_data(int* r, int* adc){
  int i;
  for(i=0; i<1000; i++){
    *(adc + i) = ADCarrayU[i];
    *(r + i) = REFarrayU[i];
  }
}

void get_data_2(int* r, int* adc){
  int i;
  for(i=0; i<s_ref; i++){
    *(adc + i) = ADCarrayU[i];
    *(r + i) = ref_position[i];
  }
}

int get_ref_array(int* array){
  int i;
  for(i=0; i<s_ref; i++){
    *(array + i) = ref_position[i];
  }
  return s_ref;
}

void set_current_gains(void){
  char buffer[BUF_SIZE];
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &KpC);
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &KiC);
}

void set_positions_gains(void){
  char buffer[BUF_SIZE];
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &KpP);
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &KiP);
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &KdP);
}

int get_mode(void){
  return modep;
}

int get_duty_cycle(void){
  return (duty_cycle*40);
}

int get_direction(void){
  return direction;
}

int get_current_Kp(void){
  return KpC;
}

int get_current_Ki(void){
  return KiC;
}

int get_position_Kp(void){
  return KpP;
}

int get_position_Ki(void){
  return KiP;
}

int get_position_Kd(void){
  return KdP;
}

int get_theta(void){
  return desired_theta;
}
