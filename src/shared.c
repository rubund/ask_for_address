#include <stdarg.h>
#include <stdint.h>
#include <nrf.h>
#include <stdio.h>
#include <nrf_gpio.h>


void setup_mode(int mode, int address_size){
    uint8_t ad = address_size - 1;
    NRF_RADIO->MODE = mode << RADIO_MODE_MODE_Pos;
    NRF_RADIO->PCNF0 = 0x00000108;
    NRF_RADIO->PCNF1 = 0x000000FF | (ad << 16);
    NRF_RADIO->CRCPOLY = 0x65B;
    NRF_RADIO->CRCINIT = 0x555555;
    NRF_RADIO->CRCCNF = 0x103;
}

void set_address(uint32_t address, int len){
    NRF_RADIO->BASE0 = address << 8;
    NRF_RADIO->PREFIX0 = (0x00000000 | address >> 24);
    NRF_RADIO->TXADDRESS = 0;
    NRF_RADIO->RXADDRESSES = 1;
}

void set_frequency(int freq){
    NRF_RADIO->FREQUENCY = freq << RADIO_FREQUENCY_FREQUENCY_Pos;
}

void setup_uart(){
#ifdef HAS_UARTE
  #ifndef NRF52840
    //nrf_gpio_cfg_output(6);
    NRF_UARTE0->PSEL.TXD = 6;
  #else
    NRF_P0->OUTSET = 0x0001e000;
    nrf_gpio_cfg_output(14);
    nrf_gpio_cfg_output(15);
    nrf_gpio_cfg_output(16);
    nrf_gpio_cfg_output(13);
    NRF_UARTE0->PSEL.TXD = 13;
  #endif
    NRF_UARTE0->ENABLE = 8;

    //NRF_UARTE0->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud460800 << UARTE_BAUDRATE_BAUDRATE_Pos);
    NRF_UARTE0->BAUDRATE = (UARTE_BAUDRATE_BAUDRATE_Baud38400 << UARTE_BAUDRATE_BAUDRATE_Pos);
#else
    NRF_UART0->ENABLE           = UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos;
  #ifndef NRF52840
    NRF_UART0->PSELTXD          = UART_TX_PIN;
  #else
    NRF_UART0->PSEL.TXD         = 6;
  #endif
    NRF_UART0->BAUDRATE         = UART_BAUDRATE_BAUDRATE_Baud38400 << UART_BAUDRATE_BAUDRATE_Pos;
    NRF_UART0->EVENTS_RXDRDY    = 0;
    NRF_UART0->EVENTS_TXDRDY    = 0;
    NRF_UART0->EVENTS_ERROR     = 0;

    NRF_UART0->TASKS_STARTTX    = 1;
    NRF_UART0->TASKS_STARTRX    = 1;
#endif
}

void send_uart(char *str, int len){
#ifdef HAS_UARTE
    NRF_UARTE0->TXD.PTR = (uint32_t)&str[0];
    NRF_UARTE0->TXD.MAXCNT = len;
    NRF_UARTE0->EVENTS_ENDTX = 0;
    NRF_UARTE0->TASKS_STARTTX = 1;
    while(NRF_UARTE0->EVENTS_ENDTX == 0);
#else
    while (len-- != 0)
    {
        NRF_UART0->EVENTS_TXDRDY = 0;
        NRF_UART0->TXD = *str++;
        while (NRF_UART0->EVENTS_TXDRDY == 0)
        {
            /* Wait until the byte is sent */
        }
        NRF_UART0->EVENTS_TXDRDY = 0;
    }
#endif
}

void printf_uart(char *str, ...)
{
    va_list a_list;
    va_start (a_list, str);
    char tmp[512];
    int len;

    len = vsnprintf(tmp, 510, str, a_list);
    send_uart(tmp, len);
    va_end(a_list);
}

void start_timer(){
    NRF_TIMER0->TASKS_CLEAR = 1;
    NRF_TIMER0->TASKS_START = 1;
}

void capture_timer(int index){
    NRF_TIMER0->TASKS_CAPTURE[index] = 1;
}

uint32_t get_timer_val(int index){
    return NRF_TIMER0->CC[index];
}

void set_led(char en){
    if (en)
      #ifdef NRF51
        nrf_gpio_pin_set(LED0_GPIO);
      #else
        nrf_gpio_pin_clear(LED0_GPIO);
      #endif
    else
      #ifdef NRF51
        nrf_gpio_pin_clear(LED0_GPIO);
      #else
        nrf_gpio_pin_set(LED0_GPIO);
      #endif
    
}
