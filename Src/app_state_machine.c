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



/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/

#define OUT_OF_DAYTIME  0xFF
#define TIME_TO_WAIT_NEW_TRANSMISSION 15
/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/

uint8_t fnAPP_STATE_Init ( uint8_t event );
uint8_t fnAPP_STATE_Check_Config ( uint8_t event );
uint8_t fnAPP_STATE_Run ( uint8_t event );
uint8_t fnAPP_STATE_Detection ( uint8_t event );
uint8_t fnAPP_STATE_Configuration ( uint8_t event );
uint8_t fnAPP_STATE_Confirming_Detection ( uint8_t event );
uint8_t fnAPP_STATE_Wait_Transmission ( uint8_t event );

void fnAPP_STATE_ENTER_Init ( void );
void fnAPP_STATE_ENTER_Check_Config ( void );
void fnAPP_STATE_ENTER_Run ( void );
void fnAPP_STATE_ENTER_Detection ( void );
void fnAPP_STATE_ENTER_Configuration ( void );
void fnAPP_STATE_ENTER_Confirming_Detection ( void );
void fnAPP_STATE_ENTER_Wait_Transmission ( void );

void fnAPP_STATE_EXIT_Init ( void );
void fnAPP_STATE_EXIT_Check_Config ( void );
void fnAPP_STATE_EXIT_Run ( void );
void fnAPP_STATE_EXIT_Detection ( void );
void fnAPP_STATE_EXIT_Configuration ( void );
void fnAPP_STATE_EXIT_Confirming_Detection ( void );
void fnAPP_STATE_EXIT_Wait_Transmission ( void );

void fnLED_Change_Effect ( void );

void fnAPP_STATE_Axsigfox_Downlink ( uint8_t * pu8_message_received, uint8_t u8_size );
void fnAPP_STATE_System_Reconfiguration ( en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type );
void fnAPP_STATE_Detection_Confirmed ( void * pv_null );
bool fnAPP_STATE_Is_At_Working_Hour ( uint8_t u8_actual_hour );
void fnAPP_STATE_New_Day_Event ( void );
en_app_state_t fnAPP_STATE_Check_Day_Event ( en_app_state_t en_app_state );
en_app_state_t fnAPP_STATE_Check_Config_Report_Event ( en_app_state_t en_app_state );

void fnAPP_STATE_State_Timeout ( void * pv_null );
void fnSTATE_MACHINE_Keep_Alive( void * pv_null );
//void fnSTATE_MACHINE_Confirmation( void * pv_null );
void fnSTATE_MACHINE_Info_Frame_Horimetro( void * pv_null );
void print_device_data(void);
void printDevEUI(void);
void printDevAddr(void);

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/
uint32_t timerTxSeconds = 0, counterState = 0, timeMagOn = 0;
uint8_t MagInitialized = 0;
uint8_t first_package_of_day = true;

//extern float fltTime;

/* Based on en_app_state_t enumeration */ 
const st_state_machine_functions_t st_app_state_machine_functions[] = {

		{  fnAPP_STATE_Init,                   fnAPP_STATE_ENTER_Init,                 fnAPP_STATE_EXIT_Init                 },
		{  fnAPP_STATE_Check_Config,           fnAPP_STATE_ENTER_Check_Config,         fnAPP_STATE_EXIT_Check_Config         },
		{  fnAPP_STATE_Run,                    fnAPP_STATE_ENTER_Run,                  fnAPP_STATE_EXIT_Run                  },
		{  fnAPP_STATE_Detection,              fnAPP_STATE_ENTER_Detection,            fnAPP_STATE_EXIT_Detection            },
		{  fnAPP_STATE_Configuration,          fnAPP_STATE_ENTER_Configuration,        fnAPP_STATE_EXIT_Configuration        },
		{  fnAPP_STATE_Confirming_Detection,   fnAPP_STATE_ENTER_Confirming_Detection, fnAPP_STATE_EXIT_Confirming_Detection },
		{  fnAPP_STATE_Wait_Transmission,      fnAPP_STATE_ENTER_Wait_Transmission,    fnAPP_STATE_EXIT_Wait_Transmission    },

};

st_state_machine_desc_t st_app_state_machine_desc;
uint16_t u16_app_state_machine_data;


st_timer_index_t st_timer_confirm_detection;
st_timer_index_t st_timer_check_config_timeout;

st_timer_index_t st_timer_confirmation_frame;
st_timer_index_t st_timer_keep_alive;
st_timer_index_t st_timer_periodic_transmission;

uint8_t u8_detection_transmit_counter;



//en_module_type_t en_actual_module_type;

en_app_state_t en_next_app_state;

bool b_mag_trigger = false;
bool b_next_state = false;

uint8_t u8_actual_strong_max_sensivity = 9;

uint8_t u8_config_report_transmition_counter;
uint8_t u8_configuration_transmition_counter;

// valores em minutos
const uint16_t u16_config_report_timer_table[4] = {  
		00,                                                 //desabilita
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
	fnCOMM_SIGMAIS_Donwlink_Frame_Set_Callback ( fnAPP_STATE_System_Reconfiguration );

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
	print_device_data();
	printDevEUI();
	printDevAddr();

	return;
}

void fnAPP_STATE_ENTER_Check_Config ( void ) {

	fnDEBUG_Const_String("fnAPP_Check_Config\r\n");
	st_system_status.u8_state_machine_state = APP_STATE_CHECK_CONFIG;
	HAL_Init( );
	HAL_RCC_DeInit();
	SystemClockConfig_HighSpeed();
	//fltTime = AJUSTE_TIMER_CLOCK_RAPIDO;
	for (uint16_t atraso= 0; atraso < 15000; atraso++){
		__ASM volatile ("nop");
	}

	BoardDeInitPeriph();
	BoardInitPeriph();
	//fnDEBUG_Const_String("Reconfigurado\r\n");
	counterState = 0;
	timerTxSeconds = fnTIMESTAMP_Get_Timestamp_Counter_Seconds();
	fnCOMM_SIGMAIS_Send_Config_Report_Frame ( );


	fnTIMER_Start( &st_timer_check_config_timeout,
			//MS_TO_TICKS( 60000 ),  //
			MS_TO_TICKS( 120000 ),  //
			TIMER_TYPE_CONTINUOUS,
			fnAPP_STATE_State_Timeout,
			NULL );


	//variavel global somente no arquivo app_state_machine.c
	u8_config_report_transmition_counter = 1;

	return;
}

void fnAPP_STATE_ENTER_Run ( void ) {
	fnDEBUG_Const_String("fnAPP_Run\r\n");
	st_system_status.u8_state_machine_state = APP_STATE_RUN;
	//TODO: verificar se este estado deve ser mantido
	fnTIMESTAMP_Start_Horimetro( EN_IDLE_TIME );

	return;
}

void fnAPP_STATE_ENTER_Detection ( void ) {

	un_system_flags.flag.system_active = 1;

	st_system_status.u8_state_machine_state = APP_STATE_DETECTION;
	if(en_next_app_state == APP_STATE_DETECTION)
		return;

	st_system_status.u32_event_timestamp = fnTIMESTAMP_Get_Updated_Timestamp();
	timerTxSeconds = fnTIMESTAMP_Get_Timestamp_Counter_Seconds();

	fnTIMESTAMP_Start_Horimetro( EN_USED_TIME );

	// Atualiza o tempo com o tempo de "Confirming_Detection"
	fnTIMESTAMP_Update_Horimetro_Used_Counter(fnTIMESTAMP_Covert_Time_Bitfield_Into_Secs( (st_sigmais_time_byte_bitfield_t *) &st_system_status.st_sigmais_detection_debounce  ) + 5);

	// Incrementa o contador de contagem
	st_system_status.u32_machine_on++;
	fnDEBUG_32bit_Value("ligado = ", st_system_status.u32_machine_on , " vezes");


	fnDEBUG_Const_String("fnAPP_Detection\r\n");
	//st_system_status.b_detection_status = fnSENSORS_Has_Detection();
	st_system_status.b_detection_status = un_system_flags.flag.lsm303agr_act;
	if ( st_system_status.b_data_processed ){
		// transmissao por eventos
		if (first_package_of_day){
			first_package_of_day = false;
			fnCOMM_SIGMAIS_Send_Info_Frame_Horimetro();
		}
		else{
			fnCOMM_SIF_Send_Horimetro_Pulsimetro();
		}

		u8_detection_transmit_counter = 1;
		/*
      if( st_timer_confirmation_frame.b_busy ){
         fnTIMER_Stop( st_timer_confirmation_frame.u8_index );
      }
		 */
		//TODO: garantir um gap de 6s entre transmissoes, sob pena de ocorrer radio busy

		/*
      fnTIMER_Start( &st_timer_confirmation_frame,
      MS_TO_TICKS(  fnTIMESTAMP_Covert_Time_Bitfield_Into_Secs(  (st_sigmais_time_byte_bitfield_t *) &st_system_status.st_sigmais_confirmation_time ) * 1000 ),
      TIMER_TYPE_CONTINUOUS,
      fnSTATE_MACHINE_Confirmation,
      NULL );
		 */

	}
	return;
}


void fnAPP_STATE_ENTER_Configuration ( void ) {

	st_system_status.u8_state_machine_state = APP_STATE_CONFIGURATION;
	fnDEBUG_Const_String("fnAPP_Configuration\r\n");

	HAL_RCC_DeInit();
	SystemClockConfig_HighSpeed();
	//fltTime = AJUSTE_TIMER_CLOCK_RAPIDO;
	for (uint16_t atraso= 0; atraso < 15000; atraso++){
		__ASM volatile ("nop");
	}

	BoardDeInitPeriph();
	BoardInitPeriph();

	counterState = 0;
	timerTxSeconds = fnTIMESTAMP_Get_Timestamp_Counter_Seconds();
	fnCOMM_SIGMAIS_Request_Downlink_Frame ( );

	u8_configuration_transmition_counter = 1;

	return;
}

void fnAPP_STATE_ENTER_Confirming_Detection ( void ) {

	st_system_status.u8_state_machine_state = APP_STATE_CONFIRMING_DETECTION;
	fnDEBUG_Const_String("fnAPP_Confirming_detection\r\n");

	//st_system_status.b_detection_status = fnSENSORS_Has_Detection();
	//por interrupção
	st_system_status.b_detection_status = un_system_flags.flag.lsm303agr_act;
	fnTIMER_Start( &st_timer_confirm_detection,
			MS_TO_TICKS((fnTIMESTAMP_Covert_Time_Bitfield_Into_Secs( (st_sigmais_time_byte_bitfield_t *) &st_system_status.st_sigmais_detection_debounce  ) + 5) * 1000 ),
			TIMER_TYPE_SINGLE,
			fnAPP_STATE_Detection_Confirmed,
			NULL );

	return;
}

void fnAPP_STATE_ENTER_Wait_Transmission ( void ){
	st_system_status.u8_state_machine_state = APP_STATE_WAIT_TRANSMISSION;
	counterState = 0;
}

/*************************************************************************************************/
/*    STATE FUNCTIONS                                                                            */
/*************************************************************************************************/


uint8_t fnAPP_STATE_Init ( uint8_t event ) {
	en_next_app_state = APP_STATE_RUN;

	return APP_STATE_CHECK_CONFIG;
	//return APP_STATE_RUN;
}

uint8_t fnAPP_STATE_Check_Config ( uint8_t event ) {

	if( st_sigfox_events.flag.b_downlink_error ) {

		st_sigfox_events.flag.b_downlink_error = false;
		//if (!LoRaMacIsBusy()){
		if( u8_config_report_transmition_counter <= 1 ) {
			fnCOMM_SIGMAIS_Send_Config_Report_Frame ( );
			u8_config_report_transmition_counter++;
		} else {
			fnCOMM_SIGMAIS_Send_Error_Frame ( EN_SIGMAIS_ERROR_RECEIVE, EN_SIGMAIS_SERVER_RESPONSE );
			b_next_state = true;
		}
		//}
	}

	counterState++;
	if (counterState > 50){
		return APP_STATE_RUN;
	}
	/*
   if( st_sigfox_events.flag.b_config_frame_received ) {
	   fnDEBUG_Const_String("config frame recebido\r\n");
      st_sigfox_events.flag.b_config_frame_received = false;

 	 // if (!LoRaMacIsBusy()){
         if( u8_config_report_transmition_counter <= 1 ) {
            fnCOMM_SIGMAIS_Send_Config_Report_Frame ( );
            u8_config_report_transmition_counter++;
         } else {
            fnCOMM_SIGMAIS_Send_Error_Frame ( EN_SIGMAIS_ERROR_RECEIVE, EN_SIGMAIS_WRONG_FRAME );
            b_next_state = true;
         }
 	 // }
   }
	 */

	if( st_sigfox_events.flag.b_downlink_frame_received ) {
		fnDEBUG_Const_String("donwlink frame recebido\r\n");
		st_sigfox_events.flag.b_downlink_frame_received = false;
		fnCOMM_SIGMAIS_Decode_Downlink_Frame ( au8_downlink_frame );

		if( st_sigfox_events.flag.b_daily_update_received ) {
			fnDEBUG_Const_String("daily frame recebido\r\n");
			st_sigfox_events.flag.b_daily_update_received = false;
			if( st_system_status.b_configuration_pending ) {
				return APP_STATE_CONFIGURATION;
			}
			else {
				b_next_state = true;
			}
		}
		else{
			// ocorreu um erro no downlink
			return APP_STATE_RUN;
		}

	}

	/*
   if( st_sigfox_events.flag.b_daily_update_received ) {
	  fnDEBUG_Const_String("daily frame recebido\r\n");
      st_sigfox_events.flag.b_daily_update_received = false;
      if( st_system_status.b_configuration_pending ) {
         return APP_STATE_CONFIGURATION;
      }
      else {
         b_next_state = true;
      }
   }
	 */

	if( b_next_state == true ) {
		b_next_state = false;
		return en_next_app_state;
	}

	return APP_STATE_CHECK_CONFIG;

}


uint8_t fnAPP_STATE_Run ( uint8_t event ) {
	en_app_state_t  check_day_event_return;
	uint8_t u8_actual_hour = fnTIMESTAMP_Get_Day_Hour();

	switch ( event ) {

	case EVENT_MAG_THRESHOLD: {

		b_mag_trigger = true;
		en_next_app_state = APP_STATE_RUN;
		return APP_STATE_CHECK_CONFIG;
	}

	default: {
		break;
	}
	}

	bool b_is_working_hour = fnAPP_STATE_Is_At_Working_Hour( u8_actual_hour );

	//if( b_is_working_hour && fnSENSORS_Has_Detection() ) {
	//      return APP_STATE_CONFIRMING_DETECTION;
	//}

	// modo por interrupção do acelerometro.
	//b_is_working_hour = true;  //teste
	//printf("ligado = %d\r\n", un_system_flags.flag.lsm303agr_act);
	if( b_is_working_hour && (un_system_flags.flag.lsm303agr_act == 1) ) {
		return APP_STATE_CONFIRMING_DETECTION;; //return APP_STATE_DETECTION --> eliminar o debounce?;
	}

	check_day_event_return = fnAPP_STATE_Check_Day_Event ( APP_STATE_RUN );
	if (check_day_event_return == APP_STATE_RUN){
		return fnAPP_STATE_Check_Config_Report_Event ( APP_STATE_RUN);
	}

	return check_day_event_return;
}


uint8_t fnAPP_STATE_Detection ( uint8_t event ) {
	en_app_state_t  check_day_event_return;



	en_next_app_state = APP_STATE_WAIT_TRANSMISSION; //APP_STATE_INIT;

	switch ( event ) {

	case EVENT_MAG_THRESHOLD: {

		RtcDelayMs(20000);

		b_mag_trigger = true;

		if( st_timer_confirmation_frame.b_busy ){
			fnTIMER_Stop( st_timer_confirmation_frame.u8_index );
		}

		en_next_app_state = APP_STATE_DETECTION;
		return APP_STATE_CHECK_CONFIG;
	}

	default: {
		break;
	}

	}

	/*
   // DEBOUNCE DO SENSOR PARA O ESTADO DESLIGADO É FEITO EM SENSORS.C
   if( !fnSENSORS_Has_Detection( ) ) {
      if ( (fnTIMESTAMP_Get_Timestamp_Counter_Seconds() - timerTxSeconds)  >= TIME_TO_WAIT_NEW_TRANSMISSION){  //aguarda 15s antes de sair do estado detection
          return APP_STATE_WAIT_TRANSMISSION;
      }
   }
	 */
	if( un_system_flags.flag.lsm303agr_act == 0  ) {
		if ( (fnTIMESTAMP_Get_Timestamp_Counter_Seconds() - timerTxSeconds)  >= TIME_TO_WAIT_NEW_TRANSMISSION){  //aguarda 15s antes de sair do estado detection
			return APP_STATE_WAIT_TRANSMISSION;
		}
	}


	check_day_event_return = fnAPP_STATE_Check_Day_Event ( APP_STATE_DETECTION );
	if (check_day_event_return == APP_STATE_DETECTION){
		if ( (fnTIMESTAMP_Get_Timestamp_Counter_Seconds() - timerTxSeconds)  >= TIME_TO_WAIT_NEW_TRANSMISSION){  //aguarda 10s antes de permitir a saída do estado detection
			return fnAPP_STATE_Check_Config_Report_Event ( APP_STATE_DETECTION);
		}
	}
	return check_day_event_return;
}


uint8_t fnAPP_STATE_Configuration ( uint8_t event ) {

	if( st_sigfox_events.flag.b_downlink_error ) {

		st_sigfox_events.flag.b_downlink_error = false;

		if( u8_configuration_transmition_counter <= 1 ) {
			fnCOMM_SIGMAIS_Request_Downlink_Frame ( );
			u8_configuration_transmition_counter++;
		} else {
			fnCOMM_SIGMAIS_Send_Error_Frame ( EN_SIGMAIS_ERROR_RECEIVE, EN_SIGMAIS_SERVER_RESPONSE );
			b_next_state = true;
		}
	}

	counterState++;
	if (counterState > 50){
		return APP_STATE_RUN;
	}

	/*
   if( st_sigfox_events.flag.b_daily_update_received ) {

      st_sigfox_events.flag.b_daily_update_received = false;

      if( u8_configuration_transmition_counter <= 1 ) {
         fnCOMM_SIGMAIS_Request_Downlink_Frame ( );
         u8_configuration_transmition_counter++;
      } else {
         fnCOMM_SIGMAIS_Send_Error_Frame ( EN_SIGMAIS_ERROR_RECEIVE, EN_SIGMAIS_WRONG_FRAME );
         b_next_state = true; 
      }
   }
	 */

	if( st_sigfox_events.flag.b_downlink_frame_received ) {
		fnDEBUG_Const_String("donwlink frame recebido\r\n");
		st_sigfox_events.flag.b_downlink_frame_received = false;
		fnCOMM_SIGMAIS_Decode_Downlink_Frame ( au8_downlink_frame );
	}

	if( st_sigfox_events.flag.b_config_frame_received ) {
		fnDEBUG_Const_String("config frame recebido\r\n");
		st_sigfox_events.flag.b_config_frame_received = false;
		fnSENSORS_Config ( );

		u8_actual_strong_max_sensivity = st_system_status.u8_strong_mag_sensivity;
		//fnSENSORS_Mag_Threshold();

		//if( st_system_status.b_configuration_pending ) {
		//fnCOMM_SIGMAIS_Request_Downlink_Frame ( );
		//} else {
		b_next_state = true;
		//}
	}

	if( b_next_state == true ) {
		b_next_state = false;
		//if( st_system_status.b_calibration_authorized && b_mag_trigger ) {
		//   st_system_status.b_calibration_authorized = false;
		//  return APP_STATE_CALIBRATION;
		//} else {
		return en_next_app_state;
		//}
	}

	return APP_STATE_CONFIGURATION;
}

uint8_t fnAPP_STATE_Confirming_Detection ( uint8_t event ) {


	switch ( event ) {

	case EVENT_MAG_THRESHOLD: {

		b_mag_trigger = true;

		if( st_timer_confirm_detection.b_busy ){
			fnTIMER_Stop( st_timer_confirm_detection.u8_index );
		}

		en_next_app_state = APP_STATE_RUN;
		return APP_STATE_CHECK_CONFIG;

	}

	default: {
		break;
	}
	}

	if( st_sigfox_events.flag.b_detection_confirmed ) {
		st_sigfox_events.flag.b_detection_confirmed = false;

		return APP_STATE_DETECTION;
	}

	//if( !fnSENSORS_Has_Detection() ) {
		if (un_system_flags.flag.lsm303agr_act == 0){

			if( st_timer_confirm_detection.b_busy ){
				fnTIMER_Stop( st_timer_confirm_detection.u8_index );
			}

			return APP_STATE_RUN;

		} else {

			return APP_STATE_CONFIRMING_DETECTION;
		}

}

uint8_t fnAPP_STATE_Wait_Transmission ( uint8_t event ) {
	//fnDEBUG_Const_String("fnAPP_Wait_Transmission\n");

	counterState++;
	if (counterState > 15)
		return APP_STATE_RUN;
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

void fnAPP_STATE_EXIT_Run ( void ) {

	return;
}

void fnAPP_STATE_EXIT_Detection ( void ) {

	if(en_next_app_state == APP_STATE_DETECTION)
		return;

	// Interrompe o contador de em uso e inicia o contador de tempo ocioso
	fnTIMESTAMP_Start_Horimetro( EN_IDLE_TIME );
	// Elimina o tempo de debounce do sensor
	// TODO: verficar se é necessário ajustar o tempo de debounce para a unidade.
	//       a expectativa é que a unidade seja sempre em segundos, mas a variável permite outras configurações.
	fnTIMESTAMP_Update_Horimetro_Used_Counter(st_system_status.st_sigmais_detection_debounce.time_value);

	st_system_status.u32_event_timestamp = fnTIMESTAMP_Get_Updated_Timestamp();

	st_system_status.u32_timer_on = fnTIMESTAMP_Get_Horimetro( EN_USED_TIME );


	fnDEBUG_Const_String("fnAPP_No_Detection\r\n");
	//st_system_status.b_detection_status = fnSENSORS_Has_Detection();
	//por interrupção
	st_system_status.b_detection_status = un_system_flags.flag.lsm303agr_act;
	if ( st_system_status.b_data_processed ){
		// transmissao por eventos
		if (first_package_of_day){
			first_package_of_day = false;
			fnCOMM_SIGMAIS_Send_Info_Frame_Horimetro();
		}
		else{
			fnCOMM_SIF_Send_Horimetro_Pulsimetro();
		}
		u8_detection_transmit_counter = 1;

		if ( st_timer_confirmation_frame.b_busy ){
			fnTIMER_Stop( st_timer_confirmation_frame.u8_index );
		}

		// TODO: Bloquear as transmissões em menso de 6s, ou seja, o programa deverá garantir um gap de 6s entre transmissoes
		//       sob pena de dar radio busy
		/*
      fnTIMER_Start( &st_timer_confirmation_frame,
      MS_TO_TICKS(  fnTIMESTAMP_Covert_Time_Bitfield_Into_Secs(  (st_sigmais_time_byte_bitfield_t *) &st_system_status.st_sigmais_confirmation_time ) * 1000 ),
      TIMER_TYPE_CONTINUOUS,
      fnSTATE_MACHINE_Confirmation,
      NULL );
		 */
		//SpiInit( &SX1276.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
		//SX1276IoInit( );
	}

	// testes de reconfiguração de clock
	// só assim retorna a níveis de baixo consumo

	return;
}


void fnAPP_STATE_EXIT_Configuration ( void ) {
	return;
}

void fnAPP_STATE_EXIT_Confirming_Detection ( void ) {

	//st_system_status.b_detection_status = fnSENSORS_Has_Detection();
	st_system_status.b_detection_status = un_system_flags.flag.lsm303agr_act;
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
	} else if( en_sigmais_downlink_frame_type == EN_SIGMAIS_DOWNLINK_CONFIG_PARK_PARAMETERS ) {
		st_sigfox_events.flag.b_config_frame_received = true;
	}

	return;
}

void fnAPP_STATE_Detection_Confirmed ( void * pv_null ) {

	//modo por interrupção
	if (un_system_flags.flag.lsm303agr_act == 1){
		st_sigfox_events.flag.b_detection_confirmed = true;
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

		if( st_timer_confirm_detection.b_busy ){
			fnTIMER_Stop( st_timer_confirm_detection.u8_index );
		}

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

			if( st_timer_confirm_detection.b_busy ){
				fnTIMER_Stop( st_timer_confirm_detection.u8_index );
			}

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


void fnSTATE_MACHINE_Keep_Alive( void * pv_null ) {

	// fnCOMM_SIGMAIS_Send_Info_Frame( );

	return;
}

/*
void fnSTATE_MACHINE_Confirmation( void * pv_null ) {
  //TODO: voltar com os timers
   if( u8_detection_transmit_counter < ( 1 + st_system_status.u8_n_retransmitions ) ) {

      fnCOMM_SIGMAIS_Send_Info_Frame( );
      u8_detection_transmit_counter++;

   } else {
      if( st_timer_confirmation_frame.b_busy ) {
         fnTIMER_Stop( st_timer_confirmation_frame.u8_index );
      }
   }

   return;
}
 */

void fnSTATE_MACHINE_Periodic_Transmission( st_state_machine_desc_t * pst_desc ) {

	if (st_system_status.b_downlink_config_frame_received){
		st_system_status.b_downlink_config_frame_received = false;
		if (st_timer_periodic_transmission.b_busy){
			fnTIMER_Stop( st_timer_periodic_transmission.u8_index );
		}
	}

	if (!st_system_status.b_data_processed ){
		if( (pst_desc->current_state == APP_STATE_RUN ) || (pst_desc->current_state == APP_STATE_CONFIRMING_DETECTION) || (pst_desc->current_state == APP_STATE_DETECTION) || (pst_desc->current_state == APP_STATE_CONFIGURATION) ){

			if( !st_timer_periodic_transmission.b_busy ) {
				//    fnTIMER_Stop( st_timer_periodic_transmission.u8_index );
				// }

				fnDEBUG_Const_String("PERIODICO TIMER START\r\n");
				fnTIMER_Start( &st_timer_periodic_transmission,
						MS_TO_TICKS( fnTIMESTAMP_Covert_Time_Bitfield_Into_Secs(  (st_sigmais_time_byte_bitfield_t *) &st_system_status.st_sigmais_transmission_timer ) * 1000 ),
						TIMER_TYPE_CONTINUOUS,
						fnSTATE_MACHINE_Info_Frame_Horimetro,
						NULL );
			}
		}
	}
	else{
		if (st_timer_periodic_transmission.b_busy){
			fnTIMER_Stop( st_timer_periodic_transmission.u8_index );
		}
	}



	return;
}


void fnSTATE_MACHINE_Info_Frame_Horimetro( void * pv_null ) {

	//fnCOMM_SIGMAIS_Send_Info_Frame_Horimetro_Raw_Data();
	if (first_package_of_day){
		first_package_of_day = false;
		fnCOMM_SIGMAIS_Send_Info_Frame_Horimetro();
	}
	else{
		fnCOMM_SIF_Send_Horimetro_Pulsimetro();
	}
	return;
}


void print_device_data(void){

	fnDEBUG_Const_String( "\r\n");
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
