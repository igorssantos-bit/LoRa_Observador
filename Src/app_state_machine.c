/*!
 * \file      app_state_machine.c
 *
 * \brief     State Machine Functions
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

/* TODO: ajustar este arquivo */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "libs/services/state_machine.h"
#include "libs/services/debug.h"
#include "libs/services/system_timer.h"
#include "system_flags.h"
#include "board.h"
#include "comm_sigmais.h"

#include "system_status.h"
#include "app.h"
#include "app_state_machine.h"
#include "LmHandler.h"

#include "sensors.h"
#include "timestamp.h"
#include "sx1276.h"
#include "sx1276-board.h"
#include "board-config.h"
#include "main.h"
#include "rtc-board.h"
#include "uart-board.h"



/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/

#define OUT_OF_DAYTIME  0xFF
#define TIME_TO_WAIT_NEW_TRANSMISSION 15
#define MAX_GROUPS 64

/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/
uint8_t fnAPP_STATE_Init ( uint8_t event );
uint8_t fnAPP_STATE_Check_Config ( uint8_t event );
uint8_t fnAPP_STATE_Configuration ( uint8_t event );
uint8_t fnAPP_STATE_Run ( uint8_t event );
uint8_t fnAPP_STATE_BLE_TX ( uint8_t event );
uint8_t fnAPP_STATE_BLE_RX ( uint8_t event );
uint8_t fnAPP_STATE_Send_BLE_Data ( uint8_t event );
uint8_t fnAPP_STATE_Wait_Transmission ( uint8_t event );

void fnAPP_STATE_ENTER_Init ( void );
void fnAPP_STATE_ENTER_Check_Config ( void );
void fnAPP_STATE_ENTER_Configuration ( void );
void fnAPP_STATE_ENTER_Run ( void );
void fnAPP_STATE_ENTER_BLE_TX ( void );
void fnAPP_STATE_ENTER_BLE_RX ( void );
void fnAPP_STATE_ENTER_Send_BLE_Data ( void );
void fnAPP_STATE_ENTER_Wait_Transmission ( void );

void fnAPP_STATE_EXIT_Init ( void );
void fnAPP_STATE_EXIT_Check_Config ( void );
void fnAPP_STATE_EXIT_Configuration ( void );
void fnAPP_STATE_EXIT_Run ( void );
void fnAPP_STATE_EXIT_BLE_TX ( void );
void fnAPP_STATE_EXIT_BLE_RX ( void );
void fnAPP_STATE_EXIT_Send_BLE_Data ( void );
void fnAPP_STATE_EXIT_Wait_Transmission ( void );

void fnAPP_STATE_System_Reconfiguration ( en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type );
// todo: verificar necessidade
bool fnAPP_STATE_Is_At_Working_Hour ( uint8_t u8_actual_hour );
void fnAPP_STATE_New_Day_Event ( void );
en_app_state_t fnAPP_STATE_Check_Day_Event ( en_app_state_t en_app_state );
en_app_state_t fnAPP_STATE_Check_Config_Report_Event ( en_app_state_t en_app_state );

void fnAPP_STATE_State_Timeout ( void * pv_null );
void fnSTATE_MACHINE_Keep_Alive( void * pv_null );
void fnSTATE_MACHINE_Info_Frame_Horimetro( void * pv_null );

uint8_t bufferSize( uint8_t *pbuffer );
void clearBuffer(uint8_t *pbuffer,  uint8_t bufferSize);

void print_device_data(void);
void printDevEUI(void);
void printDevAddr(void);

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/
extern UART_HandleTypeDef huart1;
uint32_t timerTxSeconds = 0, counterState = 0;
uint8_t first_package_of_day = true;

/* Based on en_app_state_t enumeration */ 
const st_state_machine_functions_t st_app_state_machine_functions[] = {

		{  fnAPP_STATE_Init,           		fnAPP_STATE_ENTER_Init,             	fnAPP_STATE_EXIT_Init           		},
		{ 	fnAPP_STATE_Check_Config,			fnAPP_STATE_ENTER_Check_Config, 			fnAPP_STATE_EXIT_Check_Config			},
		{  fnAPP_STATE_Configuration,  		fnAPP_STATE_ENTER_Configuration,   		fnAPP_STATE_EXIT_Configuration		},
		{  fnAPP_STATE_Run,             		fnAPP_STATE_ENTER_Run,              	fnAPP_STATE_EXIT_Run            		},
		{  fnAPP_STATE_BLE_TX,					fnAPP_STATE_ENTER_BLE_TX,    				fnAPP_STATE_EXIT_BLE_TX					},
		{  fnAPP_STATE_BLE_RX,      			fnAPP_STATE_ENTER_BLE_RX,    				fnAPP_STATE_EXIT_BLE_RX					},
		{  fnAPP_STATE_Send_BLE_Data,   		fnAPP_STATE_ENTER_Send_BLE_Data,			fnAPP_STATE_EXIT_Send_BLE_Data		},
		{  fnAPP_STATE_Wait_Transmission,	fnAPP_STATE_ENTER_Wait_Transmission,	fnAPP_STATE_EXIT_Wait_Transmission	},
};

st_state_machine_desc_t st_app_state_machine_desc;
uint16_t u16_app_state_machine_data;
st_timer_index_t st_timer_check_config_timeout;
st_timer_index_t st_timer_confirmation_frame;
st_timer_index_t st_timer_keep_alive;
st_timer_index_t st_timer_confirm_detection;
st_timer_index_t st_timer_periodic_transmission;
uint8_t u8_detection_transmit_counter;

uint8_t f_config;
uint8_t num_Groups;
uint8_t buffer_rx[MAX_GROUPS*8+5];
uint16_t size_buffer;

en_app_state_t en_next_app_state;

bool b_mag_trigger = false;
bool b_next_state = false;

uint8_t u8_config_report_transmition_counter;
uint8_t u8_configuration_transmition_counter;

const uint16_t u16_config_report_timer_table[4] = {  
		00,
		15,
		60,
		360,
};

/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnAPP_STATE_MACHINE_Init ( void ) {

	st_app_state_machine_desc.pst_functions = (st_state_machine_functions_t*)&st_app_state_machine_functions[0];
	fnSTATE_Machine_Init( &st_app_state_machine_desc );
	st_sigfox_events.u16_all_flags = 0;
	return;

}


void fnAPP_STATE_Machine ( en_event_t event ) {

	fnSTATE_Machine( &st_app_state_machine_desc, event );
	return;

}


/*************************************************************************************************/
/*    STATE ENTER FUNCTIONS                                                                      */
/*************************************************************************************************/

void fnAPP_STATE_ENTER_Init ( void ) {
	st_system_status.u8_state_machine_state = APP_STATE_INIT;

	return;
}

void fnAPP_STATE_ENTER_Check_Config ( void ) {
	printf("fnAPP_Check_Config\r\n");
	st_system_status.u8_state_machine_state = APP_STATE_CHECK_CONFIG;

	HAL_RCC_DeInit();
	SystemClockConfig_HighSpeed();
	for (uint16_t atraso= 0; atraso < 15000; atraso++){
		__ASM volatile ("nop");
	}

	BoardDeInitPeriph();
	BoardInitPeriph();

	counterState = 0;
	timerTxSeconds = fnTIMESTAMP_Get_Timestamp_Counter_Seconds();
	fnCOMM_SIGMAIS_Request_Downlink_Frame();

	u8_configuration_transmition_counter = 1;
	f_config = 0;

	return;
}

void fnAPP_STATE_ENTER_Configuration ( void ) {
	st_system_status.u8_state_machine_state = APP_STATE_CONFIGURATION;
	printf("fnAPP_Configuration\r\n");

	return;
}


void fnAPP_STATE_ENTER_Run ( void ) {
	printf("fnAPP_Run\r\n");
	st_system_status.u8_state_machine_state = APP_STATE_RUN;

	return;
}

void fnAPP_STATE_ENTER_BLE_TX ( void ) {
	printf("fnAPP_BLE_TX\r\n");
	st_system_status.u8_state_machine_state = APP_STATE_BLE_TX;

	return;
}

void fnAPP_STATE_ENTER_BLE_RX ( void ) {
	printf("fnAPP_BLE_RX\r\n");
	st_system_status.u8_state_machine_state = APP_STATE_BLE_RX;

	// limpa buffer
	uint8_t * prt = &buffer_rx[0];
	clearBuffer( prt, bufferSize( prt ));
	huart1.pRxBuffPtr = &buffer_rx[0];

	return;
}

void fnAPP_STATE_ENTER_Send_BLE_Data ( void ) {
	printf("fnAPP_BLE_Send_Data\r\n");
	st_system_status.u8_state_machine_state = APP_STATE_SEND_BLE_DATA;
	return;
}

void fnAPP_STATE_ENTER_Wait_Transmission ( void ){
	printf("fnAPP_Wait_Transmission\r\n");
	st_system_status.u8_state_machine_state = APP_STATE_WAIT_TRANSMISSION;
	counterState = 0;
}

/*************************************************************************************************/
/*    STATE FUNCTIONS                                                                            */
/*************************************************************************************************/


uint8_t fnAPP_STATE_Init ( uint8_t event ) {

	print_device_data();
	printDevEUI();
	printDevAddr();

	//return APP_STATE_CHECK_CONFIG;
	return APP_STATE_RUN;
}

uint8_t fnAPP_STATE_Check_Config ( uint8_t event ) {

	if( st_sigfox_events.flag.b_downlink_error ) {

		st_sigfox_events.flag.b_downlink_error = false;

		if( u8_configuration_transmition_counter <= 1 ) {
			fnCOMM_SIGMAIS_Request_Downlink_Frame();
			u8_configuration_transmition_counter++;
		}
	}

	counterState++;
	if (counterState > 50){
		return APP_STATE_RUN;
	}

	if( st_sigfox_events.flag.b_downlink_frame_received ) {
		printf("donwlink frame recebido\r\n");
		st_sigfox_events.flag.b_downlink_frame_received = false;

		// calib pend op_code frame_type
		//     0 	0     111        001
		// Config frame = 39 05 3C 00 3C 00
		fnCOMM_SIGMAIS_Decode_Downlink_Frame ( au8_downlink_frame );
	}

	if( st_sigfox_events.flag.b_config_frame_received ) {
		printf("config frame recebido\r\n");
		st_sigfox_events.flag.b_config_frame_received = false;

		return APP_STATE_CONFIGURATION;
	}

	return APP_STATE_CHECK_CONFIG;

}

uint8_t fnAPP_STATE_Configuration ( uint8_t event ) {

	// fazer switch case aqui dps
	if ( st_system_status.u8_op_code == EN_SIGMAIS_OP_CODE_ALL ){
		num_Groups = MAX_GROUPS;
	}

	f_config = 1;


	return APP_STATE_RUN;
}


uint8_t fnAPP_STATE_Run ( uint8_t event ) {
	en_app_state_t  check_day_event_return;

	// Chega se no presente dia o dispositivo ja fez check config
	check_day_event_return = fnAPP_STATE_Check_Day_Event ( APP_STATE_BLE_TX );

	return check_day_event_return;
}

uint8_t fnAPP_STATE_BLE_TX ( uint8_t event ){
	// Acorda o BLE pelo Pino PA8
	HAL_GPIO_WritePin(WKUP_BLE_GPIO_Port, WKUP_BLE, GPIO_PIN_SET);

	if(f_config == 1){
		printf("[H:2,%02X,%02X,%04X]\r\n", st_system_status.u8_op_code,
				st_system_status.u8_janela_BLE, st_system_status.u16_timeOut_BLE);
		f_config = 0;
	}

	return APP_STATE_BLE_RX;
}

uint8_t fnAPP_STATE_BLE_RX ( uint8_t event ){

	// [H:1,1F:0001,20:0001,21:0001]
	// 5B 48 3A 31 2C 31 46 3A 30 30 30 31 2C 32 30 3A 30 30 30 31 2C 32 31 3A 30 30 30 31 5D
	// [  H	 :  1  ,  1  F  :  0  0  0  1  ,  2  0  :  0  0  0  1  ,  2  0  :  0  0  0  1  ]

	uint8_t valid_flag = 0;
	do{
		// habilita a interrupção de leitura
		HAL_UART_Receive(&huart1, buffer_rx, 50, 500);

		// caso não achar o cabeçalho desejado, reinicia
		uint8_t header1[] = "[H:1";
		if ( memcmp(&buffer_rx[0], &header1, 4) != 0 ){
			uint8_t * prt = &buffer_rx[0];
			clearBuffer( prt, bufferSize( prt ));
			huart1.pRxBuffPtr = &buffer_rx[0];
			HAL_Delay(30);
		}else{
			uint8_t * prt = &buffer_rx[0];
			num_Groups = ( bufferSize( prt ) - 5)/8 ;
			st_system_status.data_prt = &buffer_rx[0];
			st_system_status.data_size = num_Groups*6+1; // num de bytes a serem enviados
			valid_flag = 1;
		}

	} while(valid_flag == 0);

	// Dorme o BLE pelo Pino PA8
	HAL_GPIO_WritePin(WKUP_BLE_GPIO_Port, WKUP_BLE, GPIO_PIN_RESET);

	return APP_STATE_SEND_BLE_DATA;
}

uint8_t fnAPP_STATE_Send_BLE_Data ( uint8_t event ){

	fnCOMM_SIGMAIS_Send_Frame_Tabela();

	return APP_STATE_WAIT_TRANSMISSION;
}

uint8_t fnAPP_STATE_Wait_Transmission ( uint8_t event ) {

	if( event == EVENT_WAKEUP ) {
		return APP_STATE_RUN;
	}

	return APP_STATE_WAIT_TRANSMISSION;
}

/*************************************************************************************************/
/*    STATE EXIT FUNCTIONS                                                                       */
/*************************************************************************************************/

void fnAPP_STATE_EXIT_Init ( void ) {
	return;
}

void fnAPP_STATE_EXIT_Check_Config ( void ) {

	if( st_timer_check_config_timeout.b_busy ){
		fnTIMER_Stop( st_timer_check_config_timeout.u8_index );
	}

	if( st_timer_confirmation_frame.b_busy ) {
		fnTIMER_Stop( st_timer_confirmation_frame.u8_index );
	}

	// para enviar informação de bateria
	first_package_of_day = true;
	return;
}

void fnAPP_STATE_EXIT_Configuration ( void ) {
	return;
}

void fnAPP_STATE_EXIT_Run ( void ) {
	return;
}

void fnAPP_STATE_EXIT_BLE_TX ( void ) {
	return;
}

void fnAPP_STATE_EXIT_BLE_RX ( void ) {
	return;
}

void fnAPP_STATE_EXIT_Send_BLE_Data ( void ) {
	return;
}

void fnAPP_STATE_EXIT_Wait_Transmission ( void ){
	return;
}


/*************************************************************************************************/
/*    GENERAL FUNCTIONS                                                                          */
/*************************************************************************************************/


void fnRADIO_CALLBACK_Rx_End_Error ( void ) {

	st_sigfox_events.flag.b_downlink_error = true;

	return;

}

void fnRADIO_CALLBACK_Rx_End_Ok ( void ) {
	/*
   memcpy( au8_downlink_frame, fnRADIO_Get_Sigfox_Downlink_Data(), 8 );
	 */
	st_sigfox_events.flag.b_downlink_frame_received = true;

	return;

}

void fnAPP_STATE_System_Reconfiguration ( en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type ) {

	if( en_sigmais_downlink_frame_type == EN_SIGMAIS_DOWNLINK_DAILY_UPDATE ) {
		st_sigfox_events.flag.b_daily_update_received = true;
	} else if( en_sigmais_downlink_frame_type == EN_SIGMAIS_DOWNLINK_CONFIG_FRAME ) {
		st_sigfox_events.flag.b_config_frame_received = true;
	}

	return;
}

bool fnAPP_STATE_Is_At_Working_Hour ( uint8_t u8_hour_now ) {

	bool b_inverion = ( st_system_status.u8_first_day_start_hour > st_system_status.u8_first_day_end_hour );

	bool b_result =  ( ( u8_hour_now >= st_system_status.u8_first_day_start_hour ) &&
			( u8_hour_now < st_system_status.u8_first_day_end_hour ) );

	return b_inverion != b_result;

	return 0;
}


void fnAPP_STATE_New_Day_Event ( void ) {

	uint8_t u8_cycle_first_day_start = st_system_status.u8_first_day_start_hour;
	uint8_t u8_cycle_first_day_end = st_system_status.u8_first_day_end_hour;

	st_system_status.u8_first_day_start_hour = st_system_status.u8_second_day_start_hour;
	st_system_status.u8_first_day_end_hour = st_system_status.u8_second_day_end_hour;
	st_system_status.u8_second_day_start_hour = st_system_status.u8_third_day_start_hour;
	st_system_status.u8_second_day_end_hour = st_system_status.u8_third_day_end_hour;
	st_system_status.u8_third_day_start_hour = u8_cycle_first_day_start;
	st_system_status.u8_third_day_end_hour = u8_cycle_first_day_end;

	return;
}

en_app_state_t fnAPP_STATE_Check_Day_Event ( en_app_state_t en_app_state ) {
	static uint8_t u8_last_hour = 0;
	static bool b_check_allowed = true;

	uint8_t u8_actual_hour = fnTIMESTAMP_Get_Day_Hour();

	bool b_hour_changed = u8_last_hour != u8_actual_hour;
	bool b_new_day = ( ( u8_last_hour == 23 ) && ( u8_actual_hour == 0 ) );
	u8_last_hour = u8_actual_hour;

	if( b_new_day ) {

		b_check_allowed = true;
		fnAPP_STATE_New_Day_Event();
	}

	if( b_check_allowed &&  b_hour_changed && ( u8_actual_hour == 1 ) ) {

		b_check_allowed = false;

		en_next_app_state = en_app_state;
		return APP_STATE_CHECK_CONFIG;
	}

	return en_app_state;

}   

en_app_state_t fnAPP_STATE_Check_Config_Report_Event ( en_app_state_t en_app_state ) {
	static uint32_t u32_last_hour = 0;
	uint32_t u32_actual_hour  = fnTIMESTAMP_Get_Updated_Timestamp();

	bool b_hour_changed = u32_last_hour != u32_actual_hour;
	u32_last_hour = u32_actual_hour;

	if (u16_config_report_timer_table[st_system_status.u8_config_report_timer_periodico] != 0 ){
		if(b_hour_changed && ( (u32_actual_hour % u16_config_report_timer_table[st_system_status.u8_config_report_timer_periodico]) == 0 ) ) {

			en_next_app_state = en_app_state;
			return APP_STATE_CHECK_CONFIG;
		}
	}

	return en_app_state;

}


void fnAPP_STATE_State_Timeout ( void * pv_null ) {

	//fnCOMM_SIGMAIS_Send_Config_Report_Frame ( );
	//TODO: verificar o acionamento deste flag nesta posicao
	//      esse funcionamento esta diferente do modo original
	st_sigfox_events.flag.b_downlink_error = true;
	fnDEBUG_Const_String("........................... state timeout    ..............\r\n");
	return;
}


void fnSTATE_MACHINE_On_State_Changed( st_state_machine_desc_t * pst_desc ) {
	/*
   if(!( ( ( pst_desc->current_state == APP_STATE_RUN ) && ( pst_desc->previous_state == APP_STATE_CONFIRMING_DETECTION ) ) ||
        ( ( pst_desc->current_state == APP_STATE_CONFIRMING_DETECTION ) && ( pst_desc->previous_state == APP_STATE_RUN ) ) ) ) {

      if( st_timer_keep_alive.b_busy ) {
         fnTIMER_Stop( st_timer_keep_alive.u8_index );
      }

      fnTIMER_Start( &st_timer_keep_alive,
      MS_TO_TICKS( fnTIMESTAMP_Covert_Time_Bitfield_Into_Secs(  (st_sigmais_time_byte_bitfield_t *) &st_system_status.st_sigmais_keep_alive_time ) * 1000 ),
      TIMER_TYPE_CONTINUOUS,
      fnSTATE_MACHINE_Keep_Alive,
      NULL );

   }
	 */
	return;
}

uint8_t bufferSize( uint8_t *pbuffer ){

	uint8_t posFimString = 0;
	for (uint8_t ii = 0; ii < MAX_GROUPS*8+5 ; ii++ ){
		if (pbuffer[ii] == '\0')
			break;
		posFimString++;
	}

	return posFimString;
}

void clearBuffer(uint8_t *pbuffer,  uint8_t bufferSize){

	for (uint8_t ii = 0; ii < bufferSize; ii++ ){
		pbuffer[ii] = 0;
	}
}


void print_device_data(void){

	printf("\r\n");
	fnDEBUG_8bit_Value("Versao: ", st_system_status.fwVersion.fwa,".");
	fnDEBUG_8bit_Value("", st_system_status.fwVersion.fwb,".");
	fnDEBUG_8bit_Value("", st_system_status.fwVersion.fwc,".");
	fnDEBUG_16bit_Value("", st_system_status.fwVersion.fwd,"\r\n");

	fnDEBUG_Const_String("Dispositivo: Horimetro\r\n");
	if ( st_system_status.b_data_processed) {fnDEBUG_Const_String("Transmissao: Eventos\r\n");}
	else {
		fnDEBUG_Const_String("Transmissao: Periodica\r\n");
		fnDEBUG_8bit_Value("Periodicidade: ", st_system_status.st_sigmais_transmission_timer.time_value, " ");
		switch (st_system_status.st_sigmais_transmission_timer.time_unity){
		case EN_SIGMAIS_TIME_SECONDS: fnDEBUG_Const_String("segundos\r\n");
		break;
		case EN_SIGMAIS_TIME_MINUTES: fnDEBUG_Const_String("minutos\r\n");
		break;
		case EN_SIGMAIS_TIME_HOURS: fnDEBUG_Const_String("horas\r\n");
		break;
		case EN_SIGMAIS_TIME_DAYS: fnDEBUG_Const_String("dias\r\n");
		break;
		default:
			break;
		}
	}
	fnDEBUG_8bit_Value("Config Report Timer: ", st_system_status.u8_config_report_timer_periodico,"\r\n");

	fnDEBUG_Const_String( "\r\n");

}


/*
 * printDevEUI: Prints DevEUI e DevAddr over serial port
 * */
void printDevEUI(void){

	fnDEBUG_Const_String("DevEUI = 0x");
	for (int i=0; i< sizeof(CommissioningParams.DevEui); i++){
		fnDEBUG_8bit_Hex ( "", CommissioningParams.DevEui[i], "");
	}
	fnDEBUG_Const_String("\r\n");
}

/*
 * printDevEUI: Prints DevEUI e DevAddr over serial port
 * */
void printDevAddr(void){

	fnDEBUG_Const_String("DevAddr = 0x");
	fnDEBUG_32bit_Hex ( "", CommissioningParams.DevAddr, "");
	fnDEBUG_Const_String("\r\n");
}

/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
