/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : ds18b20.h
 * @brief          : Header for ds18b20.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DS18B20_H
#define __DS18B20_H



#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"


/* Typedef -------------------------------------------------------------------*/

typedef struct DS18B20_ROM
{
	uint8_t family_code;
	uint8_t serial[6];
	uint8_t crc;
} ds18b20_rom_t;

typedef struct DS18B20_SCRATCHPAD
{
	uint8_t temperature[2];
	uint8_t th;
	uint8_t tl;
	uint8_t config_register;
	uint8_t reserved[3];
	uint8_t crc;
} ds18b20_scratch_pad_t;

enum DS18B20_Mode
{
	SINGLE_PROBE,
	MULTIPLE_PROBES
};

/* Defines -------------------------------------------------------------------*/

#define ONE_WIRE_PORT_PIN_1  GPIOB
#define ONE_WIRE_PIN_1       GPIO_PIN_12
#define ONE_WIRE_PORT_PIN_2  GPIOB
#define ONE_WIRE_PIN_2       GPIO_PIN_13

#define SYSCLOCK_1MHZ

/* DS18B20 OneWire commands */
#define CMD_SEARCH_ROM        	0xF0
#define CMD_READ_ROM          	0x33
#define CMD_MATCH_ROM         	0x55
#define CMD_SKIP_ROM          	0xCC

#define CMD_CONVERT_TEMP          0x44
#define CMD_READ_SCRATCHPAD       0xBE
#define CMD_WRITE_SCRATCHPAD      0x4E
#define CMD_COPY_SCRATCHPAD       0x48
#define CMD_REC_EEPROM            0xB8
#define CMD_READ_PWRSUPPLY        0xB4

/* Sensor configurations values */
#define RESOLUTION_9_BITS         0x1F
#define RESOLUTION_10_BITS        0x3F
#define RESOLUTION_11_BITS        0x5F
#define RESOLUTION_12_BITS        0x7F

/* OneWire communication timming variables */
// 16MHz
#ifdef SYSCLOCK_16MHZ
#define WRITE_0_LOW_TIME			180
#define WRITE_0_HIGH_TIME			20
#define WRITE_1_LOW_TIME			10
#define WRITE_1_HIGH_TIME			140

#define ONEWIRE_READ_START_TIME 		10
#define ONEWIRE_READ_DATA_VALID   	40
#define ONEWIRE_READ_SLOT_DELAY 		80

#define ONEWIRE_RESET_TIME_LOW				1400
#define ONEWIRE_PRESENCE_DETECT_DELAY	180
#define ONEWIRE_RESET_TIME_HIGH				1200
#define DELAY_RESOLUTION_12BITS       1600000
#endif

#ifdef SYSCLOCK_4MHZ
#define WRITE_0_LOW_TIME			55   // 75us
#define WRITE_0_HIGH_TIME			3    // 10us
#define WRITE_1_LOW_TIME			1    // 5us
#define WRITE_1_HIGH_TIME			25   // 40us

#define ONEWIRE_READ_START_TIME 	2    //5us
#define ONEWIRE_READ_DATA_VALID   	7    //5 razoavel, 7 exc. 8 bom
#define ONEWIRE_READ_SLOT_DELAY 	13   //11 e 15 ficaram ruins

#define ONEWIRE_RESET_TIME_LOW				250
#define ONEWIRE_PRESENCE_DETECT_DELAY	    30
#define ONEWIRE_RESET_TIME_HIGH				200

#define DELAY_RESOLUTION_12BITS      270000
                                     //290000  regular, apresenta vários erros
                                     //300000  ruim, não funciona
                                     //281000 ruim
                                     //280000  bom, mas apresenta erros depois de um tempo
                                     //279000  bom, apresentou erros, mas poucos
                                     //277000  ruim, apresenta vários erros
                                     //260000  regular, apresenta vários erros
#endif


#ifdef SYSCLOCK_1MHZ
#define WRITE_0_LOW_TIME			4   // 83,32us ->  75us
#define WRITE_0_HIGH_TIME			1   // 20,83us -> 10us
#define WRITE_1_LOW_TIME			1   // 20,83us -> 5us
#define WRITE_1_HIGH_TIME			2   // 41us -> 40us

#define ONEWIRE_READ_START_TIME 	1   // 20,83us -> 5us
#define ONEWIRE_READ_DATA_VALID   	1   //2
#define ONEWIRE_READ_SLOT_DELAY 	2   // 2

#define ONEWIRE_RESET_TIME_LOW				100//35   //  700us
#define ONEWIRE_PRESENCE_DETECT_DELAY	    1    //  20us -> minimo de 75us
#define ONEWIRE_RESET_TIME_HIGH				25   //  5200us

#define DELAY_RESOLUTION_ANY         10
#define DELAY_BREAK                  5000
                                     //290000  regular, apresenta vários erros
                                     //300000  ruim, não funciona
                                     //281000 ruim
                                     //280000  bom, mas apresenta erros depois de um tempo
                                     //279000  bom, apresentou erros, mas poucos
                                     //277000  ruim, apresenta vários erros
                                     //260000  regular, apresenta vários erros
#endif

/* Macros --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
void DelayUs(uint32_t us);

uint8_t CalculateCRC8(uint8_t* addr, uint8_t length);

void OneWireInputSetup(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void OneWireOutputSetup(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

void OneWireWriteBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t bit);
void OneWireWriteByte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t byte);

uint8_t OneWireReadBit	(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t OneWireReadByte	(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t CalculateCRC8	(uint8_t *data_buffer, uint8_t buffer_size);

HAL_StatusTypeDef DS18B20InitSequence(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
HAL_StatusTypeDef DS18B20ReadROM(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, ds18b20_rom_t* ROM);
HAL_StatusTypeDef DS18B20SkipROMReadScratchpad(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, ds18b20_scratch_pad_t* SCRATCHPAD);
HAL_StatusTypeDef DS18B20SkipROMStartConvertion(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
HAL_StatusTypeDef DS18B20ChangeResolution(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t resolution);

float DS18B20GetTemperature(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
float getTemperatureMedian(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* __DS18B20_H */

/***************************** END OF FILE ************************************/

