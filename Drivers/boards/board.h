/*!
 * \file      board.h
 *
 * \brief     Target board general functions implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \code
 *				 _____ _                       _
 *				/  ___(_)                     (_)
 *				\ `--. _  __ _ _ __ ___   __ _ _ ___
 *				 `--. \ |/ _` | '_ ` _ \ / _` | / __|
 *				/\__/ / | (_| | | | | | | (_| | \__ \
 *				\____/|_|\__, |_| |_| |_|\__,_|_|___/
 *						  __/ | (C)2018-2020 Sigmais
 *						 |___/
 *
 * \endcode
 *
 * \author    Marcelo Souza Fassarella ( EBM )
 *
 * This software component is a Sigmais property.
 * You may not use this file except previously authorized by Sigmais.
 *
 */

#ifndef __BOARD_H__
#define __BOARD_H__


#include <stdint.h>
#include "utilities.h"
#include "stm32l0xx_hal.h"
#include <stdbool.h>



/*!
 * Possible power sources
 */
enum BoardPowerSources
{
    USB_POWER = 0,
    BATTERY_POWER,
};


extern I2C_HandleTypeDef hi2c1;

/*!
 * \brief Initializes the mcu.
 */
void BoardInitMcu( void );

/*!
 * \brief Resets the mcu.
 */
void BoardResetMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De Initializes the boards peripherals.
 */
void BoardDeInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Gets the current potentiometer level value
 *
 * \retval value  Potentiometer level ( value in percent )
 */
uint8_t BoardGetPotiLevel( void );

/*!
 * \brief Measure the Battery voltage
 *
 * \retval value  battery voltage in volts
 */
uint32_t BoardGetBatteryVoltage( void );

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level [  0: USB,
 *                                 1: Min level,
 *                                 x: level
 *                               254: fully charged,
 *                               255: Error]
 */
uint8_t BoardGetBatteryLevel( void );


/*!
 * Returns the actual state of pulse1 input
 *
 * \retval
 */
uint8_t BoardGetPulse1Estado (void);


/*!
 * Returns the actual state of pulse2 input
 *
 * \retval
 */
uint8_t BoardGetPulse2Estado (void);

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Manages the entry into ARM cortex deep-sleep mode
 */
void BoardLowPowerHandler( void );

/*!
 * \brief Get the board power source
 *
 * \retval value  power source [0: USB_POWER, 1: BATTERY_POWER]
 */
//uint8_t GetBoardPowerSource( void );

/*!
 * \brief Get the board version
 *
 * \retval value  Version
 */
Version_t BoardGetVersion( void );


//void SystemClockConfig( void );
void SystemClockConfig_HighSpeed( void );
void SystemClockConfig_MSI( void );

//void Debug_Init( void );
void Debug_DeInit( void );

void MX_GPIO_Init(void);
void MX_GPIO_DeInit(void);

void MX_I2C1_Init(void);
void MX_I2C1_DeInit(void);
void MX_USART1_UART_Init(void);
void MX_USART1_UART_DeInit(void);

void MX_ADC_Init(void);

bool writeByteToEEPROM (uint32_t address, uint8_t value);
bool writeWordToEEPROM (uint32_t address, uint32_t value);
uint32_t readFromEEPROM(uint32_t address);

void LpmEnterStopMode( void);
void LpmExitStopMode( void );


/* Private defines -----------------------------------------------------------*/
#define LED                                         PB_5
#define I2C_SCL                                     PB_8
#define I2C_SDA                                     PB_9

#define EXTI2_MAGNET_Pin 			GPIO_PIN_2
#define EXTI2_MAGNET_GPIO_Port 		GPIOB

// novo
// ST_LINK
#define PA13_SWDIO_Pin 				GPIO_PIN_13
#define PA13_SWDIO_Port 			GPIOA
#define PA14_SWCLK_Pin 				GPIO_PIN_14
#define PA14_SWCLK_Port 			GPIOA

// LED
#define PB5_LED1_Pin 				GPIO_PIN_5
#define PB5_LED1_Port 				GPIOB

// BLE
#define WKUP_BLE 						GPIO_PIN_8
#define WKUP_BLE_GPIO_Port 		GPIOA

// SX1276
#define PA1_ANT_SW_RX_Pin			GPIO_PIN_1
#define PA1_ANT_SW_RX_Port 			GPIOA
#define PC0_RADIO_RESET_Pin			GPIO_PIN_0
#define PC0_RADIO_RESET_Port 		GPIOC
#define PC1_ANT_SW_TX_BOOST_Pin		GPIO_PIN_1
#define PC1_ANT_SW_TX_BOOST_Port	GPIOC
#define PC2_ANT_SW_TX_RFO_Pin		GPIO_PIN_2
#define PC2_ANT_SW_TX_RFO_Port		GPIOC

// SPI
#define RADIO_SCLK_Pin 				GPIO_PIN_3
#define RADIO_SCLK_GPIO_Port 		GPIOB
#define RADIO_MISO_Pin 				GPIO_PIN_6
#define RADIO_MISO_GPIO_Port 		GPIOA
#define RADIO_MOSI_Pin 				GPIO_PIN_7
#define RADIO_MOSI_GPIO_Port 		GPIOA

// LSM303AGR
#define EXTI0_XL_Pin 				GPIO_PIN_0
#define EXTI0_XL_GPIO_Port 			GPIOA
#define EXTI0_XL_EXTI_IRQn 			EXTI0_1_IRQn


// EEPROM STRUCTURE
// ADDRESSES ARE DEFINED FROM EEPROM DATA BASE
// ADD_FLAGS
//          bit0 = 0 -> flag downlink area was written
//          bit1 = 0 -> flag uplink area was written
//          bit2 = RES
//          bit [7 .. 3] -> flag used when eeprom was initialized by the program. Code used = b'10101

#define ADD_FLAGS                  0 // bitfield:
#define ADD_1                      1 // RES
#define ADD_2                      2 // RES
#define ADD_3                      3 // RES

                                     // downlink
#define ADD_DATA_TYPE              4 // NUMERIC VALUE
#define ADD_SENSIVITY              5 // NUMERIC VALUE
#define ADD_DEBOUNCE               6 // TIMEBYTE
#define ADD_TRANSMISSION_TYME      7 // TIMEBYTE

                                     // uplink
#define ADD_DETECTION_STATUS       8 // BYTE
#define ADD_9                      1 // RES
#define ADD_10                     1 // RES
#define ADD_11                     1 // RES
#define ADD_HORIMETRO_COUNTER      12 // WORD (12 - 15)
#define ADD_HORIMETRO_TIME_ON      16 // WORD (16 - 19) obrigatoriamente em multiplos de 4
#define ADD_HORIMETRO_TIME_OFF     20 // WORD (20 - 23)

#define ADD_COUNTER_1              24 // WORD (24 - 27)
#define ADD_COUNTER_2              28 // WORD (24 - 27)


#endif // __BOARD_H__
