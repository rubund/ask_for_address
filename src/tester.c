#include <stdarg.h>
#include <stdint.h>
#include <nrf.h>
#include <nrf_delay.h>
#include <nrf_gpio.h>
#include "shared.h"

#define RADIOMODE 0x00
#define AA_SIZE 4
#define RADIOFREQ 85
#define AA_ADDR 0x812f4744

void send_ack()
{
    uint8_t tx_packet[26];
    tx_packet[0] = 0;
    tx_packet[1] = 5;
    tx_packet[2] = 1;
    tx_packet[3] = 2;
    tx_packet[4] = 3;
    tx_packet[5] = 4;
    tx_packet[6] = 5;  
    setup_mode(RADIOMODE, AA_SIZE);
    set_frequency(RADIOFREQ);
    set_address(AA_ADDR, 4);
    NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos;
    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
                        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
    NRF_RADIO->PACKETPTR = (uint32_t)&tx_packet[0];
    NRF_RADIO->TXADDRESS = 0 << RADIO_TXADDRESS_TXADDRESS_Pos;

    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_TXEN = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0);

}

uint32_t get_next_address(int *pwr)
{ 
    uint32_t nextAddress;
    uint8_t rx_packet[256];
    NRF_RADIO->PACKETPTR = (uint32_t)&rx_packet[0];
    set_address(AA_ADDR, 4);
    setup_mode(RADIOMODE, AA_SIZE);
    set_frequency(RADIOFREQ);
    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
                        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_RXEN = 1;
    //printf_uart("Starting RX to wait for new address...\r\n");
    while (NRF_RADIO->EVENTS_DISABLED == 0);
    nextAddress = 0;
    //if(NRF_RADIO->EVENTS_CRCOK){
    if(NRF_RADIO->CRCSTATUS){
      nextAddress = (rx_packet[4] & 0x000000ff) | ((rx_packet[5] << 8) & 0x0000FF00) |((rx_packet[6] << 16) & 0x00FF0000) | ((rx_packet[7] << 24) & 0xFF000000);
      *pwr = (int)rx_packet[8];
    }
    //printf_uart("Received new address: %08x\r\n", nextAddress);
    //for(int i=0;i<10;i++) {
    //    printf_uart("%02x ", (unsigned char)rx_packet[i]);
    //}
    //printf_uart("\r\n");
    //NRF_RADIO->EVENTS_CRCOK = 0;
    return nextAddress;
}


void send_bad_packet(uint32_t address, int pwr)
{
    uint8_t tx_packet[26];
    tx_packet[0] = 0;
    tx_packet[1] = 5;
    tx_packet[2] = 1;
    tx_packet[3] = 2;
    tx_packet[4] = 3;
    tx_packet[5] = 4;
    tx_packet[6] = 5;
    setup_mode(RADIOMODE, AA_SIZE);
    set_frequency(RADIOFREQ);
    set_address(address, 4);
    switch (pwr){
      case -40:  NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg40dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
      case -30:  NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg30dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
      case -20:  NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg20dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
      case -16:  NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg16dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
      case -12:  NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg12dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
      case -8:   NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg8dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
      case -4:   NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg4dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
      case 0:    NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
      case 4:    NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
      default:  NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg30dBm << RADIO_TXPOWER_TXPOWER_Pos; break;
    }
    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
                        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
    NRF_RADIO->PACKETPTR = (uint32_t)&tx_packet[0];
    NRF_RADIO->TXADDRESS = 0 << RADIO_TXADDRESS_TXADDRESS_Pos;

    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_TXEN = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0);
}

int main()
{
    nrf_gpio_cfg_output(LED0_GPIO);
    //while(1){
    //set_led(1);
    //nrf_delay_ms(200);
    //set_led(0);
    //nrf_delay_ms(200);
    //}
    setup_uart();
    printf_uart("HEISANN\r\n");

    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
              (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos) |
              (RADIO_SHORTS_DISABLED_RXEN_Enabled << RADIO_SHORTS_DISABLED_RXEN_Pos)
      ;


    int pwr;
    uint32_t nextAddress; 
    while(1){ 
        nextAddress = get_next_address(&pwr); 
        if (nextAddress != 0){ 
          nrf_delay_us(200); 
          send_ack(); 
          nrf_delay_us(200); 
          send_bad_packet(nextAddress, pwr); 
          printf_uart("%08x\r\n", nextAddress);
        } 
    } 

}

