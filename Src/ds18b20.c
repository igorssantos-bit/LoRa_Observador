/**
 ******************************************************************************
 * @file           : ds18b20.c
 * @brief          : temperature sensor operations library.
 ******************************************************************************
 * @attention
 *
 * The DS18B20 sensor uses "one wire" communication and it's advise to pull-up
 * the data line with a 4k7 Ohms resistor.
 *
 * Another thing observed is that one must not change the pin configuration to
 * INPUT, while reading the sensor response.
 *
 ******************************************************************************
 */
#include <stdlib.h>

/* Includes ------------------------------------------------------------------*/
#include "ds18b20.h"
#include <stdio.h>
#include <string.h>
#include "stdarg.h"
#include <stdint.h>
#include <stdlib.h>
#include "debug.h"

/* Function definitions ------------------------------------------------------*/

/**
 * @attention	User must provide a funtion able to create a us delay (minimum of 5 to 10 us).
 * 				This version can be manualy calibrated adding NOPs and changind the offset of index.
 * @param 	us 	Value in microsseconds.
 * @retval		None.
 */
void DelayUs(uint32_t us)
{
	uint32_t i = us;		// The "-5" corrects a constant delay of 5us which indicates it's minimum delay as about 6us.
	while(i)
	{
		__NOP();
		i--;
	}
}



uint8_t _crc_ds18b20_update(uint8_t crc, uint8_t data)
    {
	uint8_t i;

	crc = crc ^ data;
	for (i = 0; i < 8; i++)
	{
	    if (crc & 0x01)
	        crc = (crc >> 1) ^ 0x8C;
	    else
	        crc >>= 1;
	}

	return crc;
    }

/*
 * CalculateCRC8: Verifica se o CRC de um sensor DS18B20 da MAXIM (Dallas)
 *               est· correto em relaÁ„o aos bytes recebidos.
 * Input: ponteiro para o buffer
 *        tamanho do buffer com o CRC (normalmente 9 bytes)
 * Output: 0  -> CRC OK
 *         !0 -> CRC ERRO
 * */

uint8_t CalculateCRC8(uint8_t *data_buffer, uint8_t buffer_size)
{
	uint8_t crc = 0;
	uint8_t inbyte = 0;
	for (uint8_t i = 0; i < (buffer_size - 1); ++i )
	{
		 inbyte = data_buffer[i];
		 crc = _crc_ds18b20_update(crc, inbyte);
	}

	//printf("CRC = %.4x recebido = %.4x\r\n", crc,data_buffer[buffer_size-1]);

	crc = crc ^ data_buffer[buffer_size-1];
    return crc;
}



/**
 * @brief	This function configures the one wire data pin and should be called before start any
 * 			any communications with the sensor.
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @retval 				None.
 */
void OneWireOutputSetup(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{

	GPIOx->BSRR = GPIO_Pin;              // high
	/*Configure GPIO pin : ONE_WIRE_Pin */
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
/**
 * @brief	This function writes one bit symbol in the sensor data wire. 
 * 			It follows the DS18B20 datasheet's timming diagram.
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @param	bit			Zero or One.
 * @retval 				None.
 */

void OneWireBitHigh(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIOx->BRR = GPIO_Pin;
	__NOP();                    //5us -> DelayUs(WRITE_1_LOW_TIME);
	GPIOx->BSRR = GPIO_Pin;
	DelayUs(WRITE_1_HIGH_TIME); //41us
}

void OneWireBitLow(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIOx->BRR = GPIO_Pin;
	DelayUs(WRITE_0_LOW_TIME);  //80us
	GPIOx->BSRR = GPIO_Pin;
	__NOP();                    //5us -> DelayUs(WRITE_0_HIGH_TIME);
}


/**
 * Clear scratch pad
 * */
void clearScratchPad(ds18b20_scratch_pad_t *pbuf){

		pbuf->config_register = 0;
		pbuf->crc = 0;
		pbuf->reserved[0] = 0;
		pbuf->reserved[1] = 0;
		pbuf->reserved[2] = 0;
		pbuf->temperature[0] = 0;
		pbuf->temperature[1] = 0;
		pbuf->th = 0;
		pbuf->tl = 0;

}

/**
 * @brief	This function uses the function "write bit" to write a byte in the sensor data wire. 
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @param	byte		Byte sent in data wire pin.
 * @retval 				None.
 */
void OneWireWriteByte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t byte)
{
	for(uint8_t i = 0; i < 8; i++)
	{
		if(byte & 0x01) OneWireBitHigh(GPIOx, GPIO_Pin);
		else OneWireBitLow(GPIOx, GPIO_Pin);
		byte >>= 1;
	}
}


/**
 * @brief	This function uses the function "read bit" to read a byte in the sensor data wire. 
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @retval 				Byte read.
 */
uint8_t OneWireReadByte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	uint16_t bit = 0;
	uint8_t byte = 0;

	for(uint8_t i = 0; i < 8; ++i)
	{
		GPIOx->BRR = GPIO_Pin;
		__NOP();          //5us -> DelayUs(ONEWIRE_READ_START_TIME);
		GPIOx->BSRR = GPIO_Pin;
		__NOP();        // 5us
		__NOP();        //5us   -> DelayUs(ONEWIRE_READ_DATA_VALID);
		bit = (GPIOx->IDR & GPIO_Pin);
		if(bit != 0) bit = 1;
		else bit = 0;
		byte |= bit << i;
		DelayUs(ONEWIRE_READ_SLOT_DELAY);
	}
	return byte;
}

/**
 * @brief	This function creates the sensor initialization condition.
 * 			This sequence must be used before send any command. 
 * 			It's already embedded in ready to use functions. 
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @retval 				HAL_OK: If sensor presence is detected.
 * 						HAL_TIMEOUT: If sensor presence is not detected after 240us.
 */
HAL_StatusTypeDef DS18B20InitSequence(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	uint16_t presence_flag = 0xffff;

	HAL_SuspendTick();
	/* Master TX Reset Pulse*/
	GPIOx->BRR = GPIO_Pin;
	DelayUs(ONEWIRE_RESET_TIME_LOW);        // 700us
	GPIOx->BSRR = GPIO_Pin;                 // libera o pino
	DelayUs(ONEWIRE_PRESENCE_DETECT_DELAY); // 20us

	/* Check presence */
	for (int aux = 0; aux < 100; aux++){
	   presence_flag = (GPIOx->IDR & GPIO_Pin);
	   if (presence_flag == 0) break;
	   __NOP();
	}
	HAL_ResumeTick();

	if(presence_flag != 0) return HAL_ERROR;
	DelayUs(ONEWIRE_RESET_TIME_HIGH);
	return HAL_OK;
}

/**
 * @brief	This function reads sensor ROM data. 
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @param	ROM			Struct like sensor's datasheet definitions.
 * @retval 				HAL_OK: If ROM is read and sensor's family code match with DS18B20 datasheet.
 * 						HAL_TIMEOUT: If sensor presen√ße is not detected.
 * 						HAL_ERROR: If sensor's family code does not match.
 */
HAL_StatusTypeDef DS18B20ReadROM(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, ds18b20_rom_t* ROM)
{

	if(DS18B20InitSequence(GPIOx, GPIO_Pin) == HAL_ERROR){
		return HAL_ERROR;
	}
	HAL_SuspendTick();
	OneWireWriteByte(GPIOx, GPIO_Pin, CMD_READ_ROM);

	ROM->family_code = OneWireReadByte(GPIOx, GPIO_Pin);

	for(uint8_t i = 0; i < 6; i++)
	{
		ROM->serial[i] = OneWireReadByte(GPIOx, GPIO_Pin);
	}

	ROM->crc = OneWireReadByte(GPIOx, GPIO_Pin);
	HAL_ResumeTick();

	if (ROM->family_code != 0x28) return HAL_ERROR;

	uint8_t crc_value = CalculateCRC8((uint8_t*)ROM, 8);
	if(crc_value) return HAL_ERROR;
	return HAL_OK;
}

/**
 * @brief	This function reads sensor scratch pad data. 
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @param	SCRATCHPAD	Struct like sensor's datasheet definitions.
 * @retval 				HAL_OK: If SCRATCHPAD is read.
 * 						HAL_TIMEOUT: If sensor presen√ße is not detected.
 */
HAL_StatusTypeDef DS18B20SkipROMReadScratchpad(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, ds18b20_scratch_pad_t* SCRATCHPAD)
{
	clearScratchPad(SCRATCHPAD);
	if(DS18B20InitSequence(GPIOx, GPIO_Pin) == HAL_ERROR){
		//printf("hal error\r\n");
		return HAL_ERROR;
	}
	HAL_SuspendTick();
	OneWireWriteByte(GPIOx, GPIO_Pin, CMD_SKIP_ROM);
	OneWireWriteByte(GPIOx, GPIO_Pin, CMD_READ_SCRATCHPAD);

	SCRATCHPAD->temperature[0] = 	OneWireReadByte(GPIOx, GPIO_Pin);
	SCRATCHPAD->temperature[1] = 	OneWireReadByte(GPIOx, GPIO_Pin);
	SCRATCHPAD->th = 				OneWireReadByte(GPIOx, GPIO_Pin);
	SCRATCHPAD->tl = 				OneWireReadByte(GPIOx, GPIO_Pin);
	SCRATCHPAD->config_register = 	OneWireReadByte(GPIOx, GPIO_Pin);
	SCRATCHPAD->reserved[0] = 		OneWireReadByte(GPIOx, GPIO_Pin);
	SCRATCHPAD->reserved[1] = 		OneWireReadByte(GPIOx, GPIO_Pin);
	SCRATCHPAD->reserved[2] = 		OneWireReadByte(GPIOx, GPIO_Pin);
	SCRATCHPAD->crc = 				OneWireReadByte(GPIOx, GPIO_Pin);

	uint8_t crc_value = CalculateCRC8((uint8_t*)SCRATCHPAD, 9);
	HAL_ResumeTick();
	if(crc_value){
		//fnDEBUG_Const_String("ERRO CRC\r\n");
		return HAL_ERROR;
	}
	//fnDEBUG_Const_String("CRC OK\r\n");
	return HAL_OK;
}

/**
 * @brief	This function starts a temperature conversion of all sensors in same data wire. 
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @retval 				HAL_OK: If command is sent.
 * 						HAL_TIMEOUT: If sensor presen√ße is not detected.
 */
HAL_StatusTypeDef DS18B20SkipROMStartConvertion(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	if(DS18B20InitSequence(GPIOx, GPIO_Pin) == HAL_ERROR) return HAL_ERROR;

	HAL_SuspendTick();
//	printf("start conversion\r\n");
	OneWireWriteByte(GPIOx, GPIO_Pin, CMD_SKIP_ROM);
	OneWireWriteByte(GPIOx, GPIO_Pin, CMD_CONVERT_TEMP);
	HAL_ResumeTick();
	return HAL_OK;
}

/**
 * @brief	This function reads the temperature value of a single sensor in data wire. 
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @retval 				Temperature value in Celsius degrees (float).
 *                      The -56.0 is an error code for this system.
 */
float DS18B20GetTemperature(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	ds18b20_scratch_pad_t SCRATCHPAD;
	uint8_t resolution;
	uint16_t temp;
	float temperature;
	uint8_t status_conversion = 0;

	if(DS18B20SkipROMReadScratchpad(GPIOx, GPIO_Pin, &SCRATCHPAD) == HAL_OK)
		resolution = SCRATCHPAD.config_register;
	else 	resolution = RESOLUTION_12_BITS;
	/*
	printf("resolution %x\n\r", resolution);
	printf("scratch 0 %x\n\r", SCRATCHPAD.temperature[0]);
	printf("scratch 1 %x\n\r", SCRATCHPAD.temperature[1]);
	printf("scratch 2 %x\n\r", SCRATCHPAD.th);
	printf("scratch 3 %x\n\r", SCRATCHPAD.tl);
	printf("scratch 4 %x\n\r", SCRATCHPAD.config_register);
	printf("scratch 5 %x\n\r", SCRATCHPAD.reserved[0]);
	printf("scratch 6 %x\n\r", SCRATCHPAD.reserved[1]);
	printf("scratch 7 %x\n\r", SCRATCHPAD.reserved[2]);
	printf("scratch 8 %x\n\r", SCRATCHPAD.crc);
	*/

	if(DS18B20SkipROMStartConvertion(GPIOx, GPIO_Pin) == HAL_OK)
	{
		HAL_SuspendTick();
		for (uint16_t aux = 0; aux < 550; aux++){
			DelayUs(DELAY_RESOLUTION_ANY);
			uint8_t byte = OneWireReadByte(GPIOx,GPIO_Pin);
			//printf("zero %d \r\n", byte);
			if(byte != 0)
			{
				OneWireOutputSetup(GPIOx, GPIO_Pin);
				DS18B20InitSequence(GPIOx, GPIO_Pin);
				DelayUs(DELAY_BREAK);
				break;
			}
		}
		HAL_ResumeTick();

		DelayUs(6000);

		if(DS18B20SkipROMReadScratchpad(GPIOx, GPIO_Pin, &SCRATCHPAD) == HAL_OK)
		{
			uint8_t crc_value = CalculateCRC8((uint8_t*)&SCRATCHPAD, 9);
			if(crc_value){
/*
				printf("CRC ERRO 1 \n\r");
				printf("scratch 0 %x\n\r", SCRATCHPAD.temperature[0]);
				printf("scratch 1 %x\n\r", SCRATCHPAD.temperature[1]);
				printf("scratch 2 %x\n\r", SCRATCHPAD.th);
				printf("scratch 3 %x\n\r", SCRATCHPAD.tl);
				printf("scratch 4 %x\n\r", SCRATCHPAD.config_register);
				printf("scratch 5 %x\n\r", SCRATCHPAD.reserved[0]);
				printf("scratch 6 %x\n\r", SCRATCHPAD.reserved[1]);
				printf("scratch 7 %x\n\r", SCRATCHPAD.reserved[2]);
				printf("scratch 8 %x\n\r", SCRATCHPAD.crc);
*/
				return (-56.0); //300.0;
			}

			temp = ((SCRATCHPAD.temperature[1]<<8)|SCRATCHPAD.temperature[0]);

			switch(resolution)
			{
			case RESOLUTION_9_BITS:
				temperature = (temp>>3) * 0.5;
				break;
			case RESOLUTION_10_BITS:
				temperature = (temp>>2) * 0.25;
				break;
			case RESOLUTION_11_BITS:
				temperature = (temp>>1) * 0.125;
				break;
			case RESOLUTION_12_BITS:
				temperature = temp * 0.0625;
				break;
			default:
				temperature = temp * 0.0625;
				break;
			}

/*
			printf("CRC OK\n\r");
			printf("scratch 0 %x\n\r", SCRATCHPAD.temperature[0]);
			printf("scratch 1 %x\n\r", SCRATCHPAD.temperature[1]);
			printf("scratch 2 %x\n\r", SCRATCHPAD.th);
			printf("scratch 3 %x\n\r", SCRATCHPAD.tl);
			printf("scratch 4 %x\n\r", SCRATCHPAD.config_register);
			printf("scratch 5 %x\n\r", SCRATCHPAD.reserved[0]);
			printf("scratch 6 %x\n\r", SCRATCHPAD.reserved[1]);
			printf("scratch 7 %x\n\r", SCRATCHPAD.reserved[2]);
			printf("scratch 8 %x\n\r", SCRATCHPAD.crc);
*/
			return temperature;
		}
		else{
/*
			printf("CRC ERRO 2\r\n");
			printf("scratch 0 %x\n\r", SCRATCHPAD.temperature[0]);
		    printf("scratch 1 %x\n\r", SCRATCHPAD.temperature[1]);
			printf("scratch 2 %x\n\r", SCRATCHPAD.th);
			printf("scratch 3 %x\n\r", SCRATCHPAD.tl);
			printf("scratch 4 %x\n\r", SCRATCHPAD.config_register);
			printf("scratch 5 %x\n\r", SCRATCHPAD.reserved[0]);
			printf("scratch 6 %x\n\r", SCRATCHPAD.reserved[1]);
			printf("scratch 7 %x\n\r", SCRATCHPAD.reserved[2]);
			printf("scratch 8 %x\n\r", SCRATCHPAD.crc);
*/
			return (-56.0); //300.0;
		}
	}
	else{
/*
			printf("CRC ERRO 3\n\r");
			printf("scratch 0 %x\n\r", SCRATCHPAD.temperature[0]);
			printf("scratch 1 %x\n\r", SCRATCHPAD.temperature[1]);
			printf("scratch 2 %x\n\r", SCRATCHPAD.th);
			printf("scratch 3 %x\n\r", SCRATCHPAD.tl);
			printf("scratch 4 %x\n\r", SCRATCHPAD.config_register);
			printf("scratch 5 %x\n\r", SCRATCHPAD.reserved[0]);
			printf("scratch 6 %x\n\r", SCRATCHPAD.reserved[1]);
			printf("scratch 7 %x\n\r", SCRATCHPAD.reserved[2]);
			printf("scratch 8 %x\n\r", SCRATCHPAD.crc);
*/
		return (-56.0); //300.0;
	}
}

/**
 * @brief	This function changes the temperature value of a single sensor. 
 * 
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @retval 				HAL_OK: If resolutions is changed in sensor scratch pad.
 * 						HAL_ERROR: If resolution does not change or if scratch pad reading fails.
 */
HAL_StatusTypeDef DS18B20ChangeResolution(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t resolution)
{
	ds18b20_scratch_pad_t SCRATCHPAD;
	uint8_t temp_resolution;

	if(DS18B20SkipROMReadScratchpad(GPIOx, GPIO_Pin, &SCRATCHPAD) == HAL_OK)
		temp_resolution = SCRATCHPAD.config_register;
	else return HAL_ERROR;

	if(temp_resolution == resolution) return HAL_OK;
	else
	{
		if(DS18B20InitSequence(GPIOx, GPIO_Pin) == HAL_ERROR) return HAL_ERROR;
		HAL_SuspendTick();
		OneWireWriteByte(GPIOx, GPIO_Pin, CMD_SKIP_ROM);
		OneWireWriteByte(GPIOx, GPIO_Pin, CMD_WRITE_SCRATCHPAD);
		OneWireWriteByte(GPIOx, GPIO_Pin, SCRATCHPAD.th);
		OneWireWriteByte(GPIOx, GPIO_Pin, SCRATCHPAD.tl);
		OneWireWriteByte(GPIOx, GPIO_Pin, resolution);
		HAL_ResumeTick();
	}
	if(DS18B20SkipROMReadScratchpad(GPIOx, GPIO_Pin, &SCRATCHPAD) == HAL_OK)
		temp_resolution = SCRATCHPAD.config_register;
	else return HAL_ERROR;

	if(temp_resolution == resolution) return HAL_OK;
	else return HAL_ERROR;
}


/**
 * brief: returns the median of 3 measurements
 *
 * */

float getTemperatureMedian(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	const uint8_t BUF_SIZE = 3;
	float temperature[BUF_SIZE];

	   temperature[0] = -56.0;
	   temperature[1] = -56.0;
	   temperature[2] = -56.0;
	   if (DS18B20InitSequence(GPIOx, GPIO_Pin)  == HAL_OK){
	      for (uint8_t i = 0; i < BUF_SIZE; i++){
	    	  temperature[i] = DS18B20GetTemperature(GPIOx, GPIO_Pin);
		  }

	      //find the median
	      for (uint8_t i = 0; i < (BUF_SIZE - 1); i++){
	         if (temperature[i] > temperature[i+1]){
	        	 //temperaturaMediana = temperatura[i+1];
	        	 temperature[i+1] = temperature[i];
	        	 //printf("tmp = %.4f ", temperature[i]);
	         }
	      }
	      if (temperature[1] < temperature[0]){
	    	  temperature[1] = temperature[0];
	      }
	      // temperature[1] contains the median
	   }

	   return (temperature[1]);
}
/******************************************************************************/
/*                     NOT USED                                               */
/******************************************************************************/
/**
 * @brief	This function should only be used if user wants to change the one wire data pin function.
 * 			To read the sensor feedback, the data pin should be set ans OUTPUT.
 *
 * @param	GPIOx		GPIO port of sensor's one wire data pin.
 * @param	GPIO_Pin	GPIO pin of sensor's one wire data pin.
 * @retval 				None.
 */
/*
void OneWireInputSetup(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	//Configure GPIO pin : ONE_WIRE_Pin
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
*/

/**
 * TODO: This function is not working.
 * The CRC-8 function must have a equivalent polinomial function of:
 * CRC = X8 + X5 + X4 + 1
 */
/*
uint8_t CalculateCRC8(uint8_t *data_buffer, uint8_t buffer_size)
{
	uint8_t crc = 0;
	for (uint8_t i = 0; i < buffer_size; ++i )
	{
		uint8_t inbyte = data_buffer[i];
		for ( uint8_t j = 0; j < 8; ++j )
		{
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if ( mix ) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}
*/

/***************************** END OF FILE ************************************/
