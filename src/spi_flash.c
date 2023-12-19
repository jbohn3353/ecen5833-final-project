/***********************************************************************
* @file        spi_flashdriver.c
* @version     0.0.1
* @brief       Function implementation file.
*
* @author      Ayswariya Kannan, ayka9480@Colorado.edu
* @date        Nov 21, 2023
*
* @institution University of Colorado Boulder (UCB)
* @course      ECEN 5833-001: Low Pwr Emb Design Techniques
* @instructor  Chris Choi
*
* @assignment  Final project
*
* @resources   Lecture 6, slide 35
*
* @copyright   All rights reserved. Distribution allowed only for the
*              use of assignment grading. Use of code excerpts allowed at the
*              discretion of author. Contact for permission.
*/

#include "spi_flash.h"

#include "em_device.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "timers.h"

#define US1_PORT       (gpioPortD)
#define US1_TX         (10)
#define US1_RX         (12)
#define US1_SCLK       (11)
#define US1_CS         (13)
#define WEL            (0X06)
#define WB             (0X02)
#define RB             (0X03)
#define RSR            (0X05)

uint8_t RxBuffer;


/***************************************************************************//**
 * @brief
 *  Enable writing of W25X20CL
 *
 * @param[in] none
 *
 * @return [in] none
 ******************************************************************************/
void spi_write_enable(void){

  // Driving CS low
  GPIO_PinOutClear(US1_PORT, US1_CS);

  // Enable write operations by sending write enable command
  RxBuffer = USART_SpiTransfer(USART1, WEL);

  // Driving CS high
  GPIO_PinOutSet(US1_PORT, US1_CS);

  for(int i = 0; i < 100000; i++);
}



/***************************************************************************//**
 * @brief
 *  Write one byte
 *
 * @param[in] address :24 bit address, data: one byte data to be stored
 *
 * @return [in] none
 ******************************************************************************/
void spi_write_byte(uint32_t address, uint8_t data){

  spi_write_enable();

  // Driving CS low
  GPIO_PinOutClear(US1_PORT, US1_CS);

  //Write command for spi transfer
  RxBuffer = USART_SpiTransfer(USART1, 0x02);

  // send the 24-bit address of location
  RxBuffer = USART_SpiTransfer(USART1, (address >> 16) & 0xFF);
  RxBuffer = USART_SpiTransfer(USART1, (address >> 8) & 0xFF);
  RxBuffer = USART_SpiTransfer(USART1, (address) & 0xFF);

  // Send the 1 byte data
  RxBuffer = USART_SpiTransfer(USART1, data);

  // Driving CS high
  GPIO_PinOutSet(US1_PORT, US1_CS);
}



/***************************************************************************//**
 * @brief
 *  Page program :Allow from 1 byte to 256 byte
 *
 * @param[in] none
 *
 * @return [in] none
 ******************************************************************************/
void spi_write_page(uint32_t address, const uint8_t *data, uint32_t size){

  spi_write_enable();

  if(size>256)
    return;
  else if (size == 256){
    if((address % 256) != 0)
      return;
  }

  // Driving CS low
  GPIO_PinOutClear(US1_PORT, US1_CS);

  //Write command for spi transfer
  RxBuffer = USART_SpiTransfer(USART1, WB);

  // send the 24-bit address of location
  RxBuffer = USART_SpiTransfer(USART1, (address >> 16) & 0xFF);
  RxBuffer = USART_SpiTransfer(USART1, (address >> 8) & 0xFF);
  RxBuffer = USART_SpiTransfer(USART1, (address) & 0xFF);

  //send the block of data to be written to memory
  for (uint32_t i = 0; i < size; i++){
      RxBuffer = USART_SpiTransfer(USART1, data[i]);
  }

  // Driving CS high
  for(int i = 0; i <1000000; ++i);

  GPIO_PinOutSet(US1_PORT, US1_CS);
}



/***************************************************************************//**
 * @brief
 *  Page program :Read 1 byte of data
 *
 * @param[in] address :24 bit address
 *
 * @return [in] uint8_t data : data returned
 ******************************************************************************/
uint8_t spi_read_byte(uint32_t address){

  spi_write_enable();

  // Driving CS low
  GPIO_PinOutClear(US1_PORT, US1_CS);

  // Send Read command
  RxBuffer = USART_SpiTransfer(USART1, RB);

  // send the 24-bit address of location
  RxBuffer = USART_SpiTransfer(USART1, (address >> 16) & 0xFF);
  RxBuffer = USART_SpiTransfer(USART1, (address >> 8) & 0xFF);
  RxBuffer = USART_SpiTransfer(USART1, (address) & 0xFF);
  // Read the data byte
  uint8_t data = USART_SpiTransfer(USART1, 0xFF);

  // Driving CS high
  GPIO_PinOutSet(US1_PORT, US1_CS);

  return data;
}



/***************************************************************************//**
 * @brief
 *  Page program :Read the status register
 *
 * @param[in] none
 *
 * @return [in] uint8_t status : status register content
 ******************************************************************************/
uint8_t spi_read_SR(void){

  spi_write_enable();

  // Driving CS low
  GPIO_PinOutClear(US1_PORT, US1_CS);

  // Send Read Status Register command
  RxBuffer = USART_SpiTransfer(USART1,RSR);

  // Read the status register
  uint8_t status = USART_SpiTransfer(USART1, 0xFF);;

  // Driving CS low
  GPIO_PinOutSet(US1_PORT, US1_CS);
  return status;
}



/***************************************************************************//**
 * @brief
 *  Page program :Read Data allows one or more data reads
 *
 * @param[in] address :24 bit address, data : pointer to an array to which data to be stored
 *            size : no of bytes of data to be read
 *
 * @return [in] none
 ******************************************************************************/
void spi_read_block(uint32_t address, uint8_t *data, uint32_t size){

  spi_write_enable();

  // Driving CS low
  GPIO_PinOutClear(US1_PORT, US1_CS);

  // Send Read command
  RxBuffer = USART_SpiTransfer(USART1, RB);

  // send the 24-bit address of location
  RxBuffer = USART_SpiTransfer(USART1, (address >> 16) & 0xFF);
  RxBuffer = USART_SpiTransfer(USART1, (address >> 8) & 0xFF);
  RxBuffer = USART_SpiTransfer(USART1, (address) & 0xFF);

  // Read the data byte
  // Write the data block
  for (uint32_t i = 0; i < size; i++){
      data[i] = USART_SpiTransfer(USART1, 0xFF);
  }

  // Driving CS low
  GPIO_PinOutSet(US1_PORT, US1_CS);
}



/***************************************************************************//**
 * @brief
 *  Erase all memory within specified sector (4K Bytes) to erased state FFh
 *
 * @param[in] address :24 bit address
 *
 * @return [in] none
 ******************************************************************************/
void erase_sector(uint32_t address){

  spi_write_enable();

  if((address % 4096) != 0)
    return;

  // Driving CS low
   GPIO_PinOutClear(US1_PORT, US1_CS);

   // Enable write operations by sending write enable command
   RxBuffer = USART_SpiTransfer(USART1, 0x20);

   // send the 24-bit address of location
   RxBuffer = USART_SpiTransfer(USART1, (address >> 16) & 0xFF);
   RxBuffer = USART_SpiTransfer(USART1, (address >> 8) & 0xFF);
   RxBuffer = USART_SpiTransfer(USART1, (address) & 0xFF);

   // Driving CS high
   GPIO_PinOutSet(US1_PORT, US1_CS);
}



/***************************************************************************//**
 * @brief
 *  Erase all memory within specified block (32K Bytes) to erased state FFh
 *
 * @param[in] address :24 bit address
 *
 * @return [in] none
 ******************************************************************************/
void erase_32Kblock(uint32_t address){

  spi_write_enable();

  if((address % (32 * 1024)) != 0)
    return;
  // Driving CS low
   GPIO_PinOutClear(US1_PORT, US1_CS);

   // Enable write operations by sending write enable command
   RxBuffer = USART_SpiTransfer(USART1, 0x52);

   // send the 24-bit address of location
   RxBuffer = USART_SpiTransfer(USART1, (address >> 16) & 0xFF);
   RxBuffer = USART_SpiTransfer(USART1, (address >> 8) & 0xFF);
   RxBuffer = USART_SpiTransfer(USART1, (address) & 0xFF);

   // Driving CS high
   GPIO_PinOutSet(US1_PORT, US1_CS);

}



/***************************************************************************//**
 * @brief
 *  Erase all memory within specified block(64K Bytes) to erased state FFh
 *
 * @param[in] address :24 bit address
 *
 * @return [in] none
 ******************************************************************************/
void erase_64kblock(uint32_t address){

  spi_write_enable();

  if((address % (64 * 1024)) != 0)
    return;

  // Driving CS low
   GPIO_PinOutClear(US1_PORT, US1_CS);

   // Enable write operations by sending write enable command
   RxBuffer = USART_SpiTransfer(USART1, 0xD8);

   // send the 24-bit address of location
   RxBuffer = USART_SpiTransfer(USART1, (address >> 16) & 0xFF);
   RxBuffer = USART_SpiTransfer(USART1, (address >> 8) & 0xFF);
   RxBuffer = USART_SpiTransfer(USART1, (address) & 0xFF);

   // Driving CS high
   GPIO_PinOutSet(US1_PORT, US1_CS);

}



/***************************************************************************//**
 * @brief
 *  Erase all memory within the device to erased state FFh
 *
 * @param[in] none
 *
 * @return [in] none
 ******************************************************************************/
void chip_erase(void){

  spi_write_enable();

  // Driving CS low
   GPIO_PinOutClear(US1_PORT, US1_CS);

   // Enable write operations by sending write enable command
   RxBuffer = USART_SpiTransfer(USART1, 0x60);

   // Driving CS high
   GPIO_PinOutSet(US1_PORT, US1_CS);
}



/**************************************************************************//**
 * @brief Initialize USART1
 *****************************************************************************/
void spi_init(void){

  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_USART1, true);

  // Configure GPIO mode
  GPIO_PinModeSet(gpioPortD, 11, gpioModePushPull, 0); // US1_CLK is push pull
  GPIO_PinModeSet(gpioPortD, 13, gpioModePushPull, 1); // US1_CS is push pull
  GPIO_PinModeSet(gpioPortD, 10, gpioModePushPull, 1); // US1_TX (MOSI) is push pull
  GPIO_PinModeSet(gpioPortD, 12, gpioModeInput, 1);    // US1_RX (MISO) is input

  // Start with default config, then modify as necessary
  USART_InitSync_TypeDef config = USART_INITSYNC_DEFAULT;
  config.master       = true;            // master mode
  config.baudrate     = 1000000;         // CLK freq is 1 MHz
  config.autoCsEnable = false;            // CS pin controlled by hardware, not firmware
  config.clockMode    = usartClockMode0; // clock idle low, sample on rising/first edge
  config.msbf         = true;            // send MSB first
  config.enable       = usartDisable;    // Make sure to keep USART disabled until it's all set up
  USART_InitSync(USART1, &config);

  // Set USART pin locations
  USART1->ROUTELOC0 = (USART_ROUTELOC0_CLKLOC_LOC17) | // US1_CLK       on location 11 = PC8 per datasheet section 6.4 = EXP Header pin 8
//                      (USART_ROUTELOC0_CSLOC_LOC11)  | // US1_CS        on location 11 = PC9 per datasheet section 6.4 = EXP Header pin 10
                      (USART_ROUTELOC0_TXLOC_LOC18)  | // US1_TX (MOSI) on location 11 = PC6 per datasheet section 6.4 = EXP Header pin 4
                      (USART_ROUTELOC0_RXLOC_LOC19);   // US1_RX (MISO) on location 11 = PC7 per datasheet section 6.4 = EXP Header pin 6

  // Enable USART pins
  USART1->ROUTEPEN = USART_ROUTEPEN_CLKPEN | USART_ROUTEPEN_CSPEN | USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN;

  // Enable USART1
  USART_Enable(USART1, usartEnable);

  chip_erase();
}
