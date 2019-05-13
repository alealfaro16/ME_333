#ifndef UTILITIES__H__
#define UTILITIES__H__

#define PLOTPTS 100

void set_mode(int mode);
int get_mode(void);
int get_duty_cycle(void);
int get_direction(void);

int get_current_Kp(void);
int get_current_Ki(void);
void set_current_gains(void);

void sent_data(int r_data,int adc_data, int Str_data);
int get_Str_data(void);
void get_data(int* r, int* adc);

int get_position_Kp(void);
int get_position_Ki(void);
int get_position_Kd(void);
void set_positions_gains(void);

int get_theta(void);
int get_desired_current(void);
void send_current(int u);

void send_ref_array(int* ref_array,int size_ref);
int get_ref_array(int* array);
void get_data_2(int* r, int* adc);

#endif
