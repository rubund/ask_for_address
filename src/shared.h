#ifndef INC_SHARED_H
#define INC_SHARED_H

#include <stdint.h>

void setup_mode(int mode, int address_size);
void set_address(uint32_t address, int len);
void set_frequency(int freq);
void setup_uart();
void send_uart(char *str, int len);
void printf_uart(char *str, ...);
char get_last_demod_active();
void make_std_mode_shutup();
void start_timer();
void capture_timer(int index);
uint32_t get_timer_val(int index);
void set_led(char en);

#endif

