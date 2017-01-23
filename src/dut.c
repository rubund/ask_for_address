#include <nrf.h>
#include <nrf_delay.h>
#include "shared.h"

#define RADIOMODE 0x00
#define AA_SIZE 4
#define RADIOFREQ 85
#define AA_ADDR 0x812f4744

void send_next_address(uint32_t address, int pwr)
{
    uint8_t tx_packet[26];
    tx_packet[0] = 0;
    tx_packet[1] = 13;
    tx_packet[4] = (address >> 0)  & 0xff;
    tx_packet[5] = (address >> 8)  & 0xff;
    tx_packet[6] = (address >> 16) & 0xff;
    tx_packet[7] = (address >> 24) & 0xff;
    tx_packet[8] = (char)pwr;
    NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos;
    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
                        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
    NRF_RADIO->PACKETPTR = (uint32_t)&tx_packet[0];
    NRF_RADIO->MODE = RADIOMODE << RADIO_MODE_MODE_Pos;
    setup_mode(RADIOMODE, AA_SIZE);
    set_frequency(RADIOFREQ);
    set_address(AA_ADDR, 4);
    NRF_RADIO->TXADDRESS = 0 << RADIO_TXADDRESS_TXADDRESS_Pos;

    //printf_uart("Telling TX device to send with address %08x\r\n", address);
    //send_uart(uart_txt, uarttxlen);

    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_TXEN = 1;
    //printf_uart("Starting TX for sending address to tester...\r\n");
    while (NRF_RADIO->EVENTS_DISABLED == 0);
    //printf_uart("Done transmitting\r\n");

}

int wait_for_ack()
{
    uint8_t rx_packet[256];
    NRF_RADIO->PACKETPTR = (uint32_t)&rx_packet[0];
    set_address(AA_ADDR, 4);
    setup_mode(RADIOMODE, AA_SIZE);
    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
                        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
    NRF_RADIO->EVENTS_DISABLED = 0;
    //NRF_RADIO->EVENTS_CRCOK = 0;
    NRF_RADIO->TASKS_RXEN = 1;
    start_timer();
    while (NRF_RADIO->EVENTS_DISABLED == 0){
      capture_timer(0);
      if (get_timer_val(0) > 2000) {
          NRF_RADIO->TASKS_STOP = 1;
          NRF_RADIO->TASKS_DISABLE = 1;
          //uarttxlen = snprintf(uart_txt,255,"-- timed out getting ACK from tester device. requesting address again --\r\n");
          //send_uart(uart_txt, uarttxlen);
          nrf_delay_ms(2);
          //printf_uart("timedout\r\n");
          return 0;
      }
    }
    if(NRF_RADIO->CRCSTATUS){
        //NRF_RADIO->EVENTS_CRCOK = 0;
        //printf_uart("ok\r\n");
        return 1;
    }
    else {
        //NRF_RADIO->EVENTS_CRCOK = 0;
        return 0;
    }
}

int wait_for_test_packet(uint32_t address, char *timed_out, uint8_t *rssi_sample, int *time)
{
    uint8_t rx_packet[256];
    *rssi_sample = 255;
    *timed_out = 0;
    *time = -1;
    NRF_RADIO->PACKETPTR = (uint32_t)&rx_packet[0];
    set_address(address, 4);
    setup_mode(RADIOMODE, AA_SIZE);
    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
                        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->EVENTS_ADDRESS = 0;
    //NRF_RADIO->EVENTS_CRCOK = 0;
    NRF_RADIO->TASKS_RXEN = 1;
    start_timer();
    while (NRF_RADIO->EVENTS_ADDRESS == 0){
      capture_timer(0);
      if (get_timer_val(0) > 2000) {
          NRF_RADIO->TASKS_STOP = 1;
          NRF_RADIO->TASKS_DISABLE = 1;
          nrf_delay_ms(2);
          *timed_out = 1;
          return 0;
      }
    }
    capture_timer(0);
    *time = get_timer_val(0);
    NRF_RADIO->EVENTS_RSSIEND = 0;
    NRF_RADIO->TASKS_RSSISTART = 1;
    while (NRF_RADIO->EVENTS_RSSIEND == 0){
    }
    *rssi_sample = (uint8_t) (NRF_RADIO->RSSISAMPLE & 0xff) ;
    while (NRF_RADIO->EVENTS_DISABLED == 0){
      capture_timer(0);
      if (get_timer_val(0) > 2000) {
          NRF_RADIO->TASKS_STOP = 1;
          NRF_RADIO->TASKS_DISABLE = 1;
          nrf_delay_ms(2);
          *timed_out = 1;
          return 0;
      }
    }
    if(NRF_RADIO->CRCSTATUS){
        //NRF_RADIO->EVENTS_CRCOK = 0;
        //printf_uart("ok\r\n");
        return 1;
    }
    else {
        //NRF_RADIO->EVENTS_CRCOK = 0;
        return 0;
    }

}

int main()
{
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
              (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos) |
              (RADIO_SHORTS_DISABLED_RXEN_Enabled << RADIO_SHORTS_DISABLED_RXEN_Pos)
      ;

    setup_uart();
    printf_uart("Hello from DUT\r\n");
    nrf_delay_ms(1000);
    //for(int i=0;i<100;i++){
    uint32_t address;
    int time;
    char timed_out;
    uint8_t rssi_sample;
    uint8_t max_rssi;
    uint8_t min_rssi;
    int max_time;
    int min_time;
    address = 0x12943377;
    while(1){
        int ret;
        int errors;
        int naer;
        errors = 0;
        naer = 0;
        max_rssi = 0;
        min_rssi = 255;
        max_time = 0;
        min_time = 65536;
        for(int i=0;i<100;i++){
            ret = 0;
            while(ret == 0){
                send_next_address(address, 0);
                ret = wait_for_ack();
            }
            ret = wait_for_test_packet(address, &timed_out, &rssi_sample, &time);
            if (rssi_sample != 255) {
                if (rssi_sample > max_rssi)
                    max_rssi = rssi_sample;
                if (rssi_sample < min_rssi)
                    min_rssi = rssi_sample;
            }
            if ((!ret) && (!timed_out))
                naer++;
            if (ret){
                //printf_uart("One successful: %08x\r\n", address);
            }
            else {
                //printf_uart("One failing: %08x\r\n", address);
                errors++;
            }
            if (time != -1){
                if (time > max_time){
                    max_time = time;
                }
                if (time < min_time)
                    min_time = time;
            }
            nrf_delay_ms(1);
        }
        printf_uart("%08x", address);
        printf_uart(": ");
        printf_uart("%3d/100", errors);
        printf_uart(", ");
        printf_uart("%3d/100", naer);
        printf_uart(", ");
        printf_uart("RSSI: %03d < %03d",-(int)(max_rssi), -(int)(min_rssi));
        printf_uart(", ");
        printf_uart("Time: %03d < %03d",min_time, max_time);
        printf_uart("\r\n");
        address++;
    }
}
