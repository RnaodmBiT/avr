#pragma once
#include "system.h"

/* @brief initialize the nrf24 radio with default settings.
   @param fast true sets 2Mbps. false sets 1Mbps
*/
void nrf_init(bool fast);

/* @brief Check whether there is an outstanding data IRQ
   @returns true if there is data to be read.
*/
bool nrf_has_data(void);

/* @brief Check whether or not the RX FIFO is empty
   @returns true if the FIFO is empty
*/
bool nrf_rx_fifo_empty(void);

/* @brief read a single packet of data from the RX FIFO
   @param data pointer to the buffer to store the packet in
   @param size of the buffer in bytes.
*/
void nrf_read(void* data, int len);

/* @brief send a single packet over the air to a reciever.
   @param point to the data packet to send.
   @param len size of the data packet in bytes.
   @returns true if the receiver ACKed the packet.
*/
bool nrf_write(const void* data, int len);


/* @brief set the channel to send data on.
   @param 2400 MHz + channel [MHz]. Range of [0, 125] inclusive
*/
void nrf_set_channel(uint8_t channel);


/* @brief set address to transmit and recieve from.
   @param address 5 byte binary address.
*/
void nrf_set_address(uint64_t address);
