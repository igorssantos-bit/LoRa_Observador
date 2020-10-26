/*!
 * \file      app.c
 *
 * \brief     Application functions
 *
 * \copyright 2019 Sigmais.
 *  All rights reserved.
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


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include <accelerometer.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "libs/services/debug.h"
#include "libs/services/delay_loop.h"
#include "app.h"
#include "app_state_machine.h"
#include "system_flags.h"
#include "system_status.h"
#include "system_timer.h"
#include "events.h"
#include "input_analogic.h"
#include "sensors.h"
#include "timestamp.h"
#include "gpio.h"
#include "rtc-board.h"
#include "radio.h"
#include "debug.h"
#include "board.h"
#include "ds18b20.h"
#include "rtc-board.h"



/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/
//#define DEBOUNCE_TIME 1000   // 1000 é um bom valor para retirar repique de botão.
#define DEBOUNCE_TIME 700   // 700 funcionou bem com o rele mecânico na configuração Ton = 300ms e Toff = 700ms.

/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/


// Process events
void fnAPP_Process_Event_RTC ( void );
void fnAPP_Process_Event_System_Tick ( void );
void fnAPP_Init_Standard_Values ( void );
static void restoreData(void);
static void restoreDataFactorytIf(void);

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/
extern Gpio_t Led1;
extern ADC_HandleTypeDef hadc1;
extern uint32_t u32_horimetro_used_counter_s, u32_horimetro_idle_counter_s;

uint32_t previous_seconds, current_seconds, mag_counter = 0;
uint16_t miliseconds = 0;
uint8_t mag_state = 0;

/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnAPP_Init ( void ) {

	fnTIMER_System_Tick_Init();

	fnAPP_Init_Standard_Values();

	restoreDataFactorytIf();
	// restore memory data
	restoreData();

	fnSENSORS_Init();

	un_system_flags.flag.fxos_1_int_threshold = false;

	st_system_status.b_probe1_enabled = false;
	st_system_status.b_probe2_enabled = false;

	fnAPP_STATE_MACHINE_Init( );
	fnAPP_STATE_Machine( EVENT_SYSTEM_INIT );

	return;

}

void fnAPP_Process_Events ( void ) {

	// Processa timers e a maquina de estados a cada segundo
	current_seconds = RtcGetCalendarTime(&miliseconds);
	if (current_seconds != previous_seconds){
		previous_seconds = current_seconds;

		// processa os timers
		fnAPP_Process_Event_RTC();

		// processa a maquina de estados
		fnAPP_STATE_Machine( EVENT_NONE );
	}
	return;

}


void fnAPP_Check_Events ( void ) {

	return;

}

/*************************************************************************************************/
/*    SYSTEM EVENT                                                                               */
/*************************************************************************************************/


void fnAPP_Process_Event_RTC ( void ) {
	//static float timer_ajuste = 0.0;

	fnTIMER_System_Tick();
	fnTIMESTAMP_Run_Time_Counter();

	return;

}
/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


void fnAPP_Init_Standard_Values ( void ) {
	const char *pdate = __DATE__;
	const char *ptime = __TIME__;
	const char *ptimestamp = __TIMESTAMP__;
	uint16_t fwVersao = 0;

	st_system_status.u8_strong_mag_sensivity = 4;
	st_system_status.u8_n_retransmitions = 1;

	st_system_status.u8_temperature_internal = 25;
	st_system_status.u8_battery = 33;

	st_system_status.b_battery_status = true;
	st_system_status.b_detection_status = false;

	st_system_status.st_sigmais_confirmation_time.time_unity = EN_SIGMAIS_TIME_SECONDS;
	st_system_status.st_sigmais_confirmation_time.time_value = 60;

	st_system_status.st_sigmais_keep_alive_time.time_unity = EN_SIGMAIS_TIME_MINUTES;
	st_system_status.st_sigmais_keep_alive_time.time_value = 60;

	//st_system_status.st_sigmais_keep_alive_time.time_unity = EN_SIGMAIS_TIME_HOURS;
	//st_system_status.st_sigmais_keep_alive_time.time_value = 2;

	st_system_status.st_sigmais_detection_debounce.time_unity = EN_SIGMAIS_TIME_SECONDS;
	//st_system_status.st_sigmais_detection_debounce.time_value = 20;
	st_system_status.st_sigmais_detection_debounce.time_value = 5;

	st_system_status.u8_first_day_start_hour = 0;
	st_system_status.u8_first_day_end_hour = 24;

	st_system_status.u8_second_day_start_hour = 0;
	st_system_status.u8_second_day_end_hour = 24;

	st_system_status.u8_third_day_start_hour = 0;
	st_system_status.u8_third_day_end_hour = 24;

	st_system_status.u32_timestamp = 540; // 9AM 01/01/2017

	st_system_status.u8_sensor_sensivity = 2;
	st_system_status.u32_timer_on = 0;
	st_system_status.u32_timer_off = 0;
	st_system_status.st_sigmais_transmission_timer.time_unity = EN_SIGMAIS_TIME_MINUTES;
	st_system_status.st_sigmais_transmission_timer.time_value = 5;

	//st_system_status.b_data_processed = false;    // park = raw data e sense = transmissao periodica
	st_system_status.b_data_processed = true;       // sense = transmissao por eventos
	st_system_status.b_configuration_pending = false;
	st_system_status.u8_state_machine_state = 0;       //state_init

	// versionamento
	// Arquitetura: 02 -> STM + LSM
	// Funcionalidades: 01 -> firmware original Sigfox + Lora
	//                  04 -> sense de baixo consumo
	//                  05 -> sense  com timer off
	//                  06 -> sense com contadores
	//
	// bugs:
	//                  01.00 -> nenhum bug detectado
	//                  06.01 -> a implementação com ADR_ON não garantiu melhor desempenho em regioes afastadas das ERBs

	st_system_status.fwVersion.fwa = 2; // arquitetura
	st_system_status.fwVersion.fwb = 6; // funcionalidades
	st_system_status.fwVersion.fwc = 1; // uso do adr_off e dr2 para maior potência



	for (int cont = 0; cont< (strlen(pdate) - 1); cont++){
		fwVersao ^= pdate[cont] | (pdate[cont+1] << 8);
	}

	for (int cont = 0; cont<(strlen(ptime) - 1); cont++){
		fwVersao ^= ptime[cont] | (ptime[cont+1] << 8);
	}

	for (int cont = 0; cont<(strlen(ptimestamp) - 1); cont++){
		fwVersao ^= ptimestamp[cont] | (ptimestamp[cont+1] << 8);
	}

	st_system_status.fwVersion.fwd = fwVersao;

	st_system_status.u8_config_report_timer_periodico = 0;    // 0, 1, 2, 3

	st_system_status.b_calibration_authorized = true;
	st_system_status.b_downlink_config_frame_received = false;

	st_system_status.u32_machine_on = 0;

	st_system_status.u8_op_code = 7;
	st_system_status.u8_janela_BLE = 5;
	st_system_status.u16_timeOut_BLE = 60;
	st_system_status.u16_timer_Uplink = 60;

	return;

}

/*
 * @brief Function used to restore sctrutured data on internal EEPROM
 * @param none
 * @retval
 *
 * */
static void restoreData(void){
	uint8_t flags = readFromEEPROM(ADD_FLAGS);

	// verify if a value was written on downlink area,
	// if true restore else use default
	if ( (flags & 0x01) == 0){
		//downlink data
		uint32_t aux32 = readFromEEPROM(ADD_DATA_TYPE);
		uint8_t aux8;
		st_system_status.b_data_processed = (uint8_t)aux32 & 0xff;

		aux32 >>= 8;
		st_system_status.u8_sensor_sensivity = (uint8_t)aux32 & 0xff;
		st_system_status.u8_sensor_sensivity &= 0x07; // 3 bits apenas

		aux32 >>= 8;
		aux8 = (uint8_t)aux32 & 0xff;
		st_system_status.st_sigmais_detection_debounce.time_value = (aux8 & (~0x03)) >> 2;
		st_system_status.st_sigmais_detection_debounce.time_unity = aux8 & 0x03;

		aux32 >>= 8;
		aux8 = (uint8_t)aux32 & 0xff;
		st_system_status.st_sigmais_transmission_timer.time_value = (aux8 & (~0x03)) >> 2;
		st_system_status.st_sigmais_transmission_timer.time_unity = aux8 & 0x03;

		validateTransmissionTimer();

		printf("\r\n*************************************************************\r\n");
		printf(" timer value %x, time unit %x  \r\n", st_system_status.st_sigmais_transmission_timer.time_value, st_system_status.st_sigmais_transmission_timer.time_unity );
		printf("*************************************************************\r\n");

	}

	// verify if a value was written on uplink area,
	// if true restore else use default
	if ((flags & 0x02) == 0){
		//uplink data
		st_system_status.u32_machine_on = readFromEEPROM(ADD_HORIMETRO_COUNTER);
		st_system_status.u32_timer_on = readFromEEPROM(ADD_HORIMETRO_TIME_ON);
		st_system_status.u32_timer_off = readFromEEPROM(ADD_HORIMETRO_TIME_OFF);
		st_system_status.u32_number_of_pulsos_1 = readFromEEPROM(ADD_COUNTER_1);
		st_system_status.u32_number_of_pulsos_2 = readFromEEPROM(ADD_COUNTER_2);
		u32_horimetro_used_counter_s = st_system_status.u32_timer_on * 60;
		u32_horimetro_idle_counter_s = st_system_status.u32_timer_off * 60;


		printf("\r\n*************************************************************\r\n");
		printf("counter %lu, time on %lu  time off %lu\r\n", st_system_status.u32_machine_on, st_system_status.u32_timer_on, st_system_status.u32_timer_off );
		printf("*************************************************************\r\n");
	}


}

void validateTransmissionTimer(void){
	if (st_system_status.st_sigmais_transmission_timer.time_value == 0){
		st_system_status.st_sigmais_transmission_timer.time_value = 2;
		if ( (st_system_status.st_sigmais_transmission_timer.time_unity == EN_SIGMAIS_TIME_SECONDS) && (st_system_status.st_sigmais_transmission_timer.time_value < 60) ){
			st_system_status.st_sigmais_transmission_timer.time_unity = EN_SIGMAIS_TIME_MINUTES;
			st_system_status.st_sigmais_transmission_timer.time_value = 2;
		}
	}
}


/*
 * @brief Function used to restore factory data on internal EEPROM if
 *        factory code in EEPROM doesn't correspond to original one
 * @param none
 * @retval
 *
 * */
#define FACTORY_CODE 0xa8
#define FACTORY_MASK 0xf8

static void restoreDataFactorytIf(void){
	uint8_t flags = readFromEEPROM(ADD_FLAGS);
	// verify if is the first time the code was initialized
	// if true salve the default values on EEPROM
	if ( (flags & FACTORY_MASK) != FACTORY_CODE){
		st_system_status.u32_number_of_pulsos_1 = 0;
		st_system_status.u32_last_timer_pulsos_1 = 0;
		st_system_status.u32_number_of_pulsos_2 = 0;
		st_system_status.u32_last_timer_pulsos_2 = 0;
		st_system_status.b_probe1_enabled = false;
		st_system_status.b_probe2_enabled = false;
		st_system_status.u32_machine_on = 0;
		st_system_status.u32_timer_on = 0;
		st_system_status.u32_timer_off = 0;
		st_system_status.u8_sensor_sensivity = 2;
		st_system_status.st_sigmais_transmission_timer.time_unity = EN_SIGMAIS_TIME_MINUTES;
		st_system_status.st_sigmais_transmission_timer.time_value = 15;
		st_system_status.b_data_processed = false; // transmissao periodica
		//st_system_status.b_data_processed = true;// transmissao por eventos
		st_system_status.st_sigmais_detection_debounce.time_unity = EN_SIGMAIS_TIME_SECONDS;
		st_system_status.st_sigmais_detection_debounce.time_value = 10;

		flags = FACTORY_CODE | 0x07;
		writeByteToEEPROM(ADD_FLAGS,flags);
		writeWordToEEPROM(ADD_HORIMETRO_COUNTER,st_system_status.u32_machine_on);
		writeWordToEEPROM(ADD_HORIMETRO_TIME_ON, st_system_status.u32_timer_on);
		writeWordToEEPROM(ADD_HORIMETRO_TIME_OFF, st_system_status.u32_timer_off);
		writeWordToEEPROM(ADD_COUNTER_1, st_system_status.u32_number_of_pulsos_1);
		writeWordToEEPROM(ADD_COUNTER_2, st_system_status.u32_number_of_pulsos_2);

		uint32_t valor;
		valor = (st_system_status.st_sigmais_transmission_timer.time_value << 2) | (st_system_status.st_sigmais_transmission_timer.time_unity);
		valor <<= 8;
		valor |= (st_system_status.st_sigmais_detection_debounce.time_value << 2) | (st_system_status.st_sigmais_detection_debounce.time_unity);
		valor <<= 8;
		valor |=  st_system_status.u8_sensor_sensivity;
		valor <<= 8;
		valor |= st_system_status.b_data_processed;
		writeWordToEEPROM(ADD_DATA_TYPE,valor);

	}
}
/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
