/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

/* TODO: ajustar este arquivo */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "libs/services/state_machine.h"
#include "libs/services/debug.h"
#include "libs/services/system_timer.h"
#include "system_flags.h"
#include "comm_sigmais.h"
#include "system_status.h"
#include "app.h"
#include "main.h"
#include "app_state_machine.h"
#include "sensors.h"
#include "timestamp.h"
#include "traffic.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/
#define OUT_OF_DAYTIME  0xFF
#define MAX_GROUPS 64

/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/

/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/

uint8_t fnAPP_STATE_Init ( uint8_t event );
uint8_t fnAPP_STATE_Configuration ( uint8_t event );
uint8_t fnAPP_STATE_Run ( uint8_t event );
uint8_t fnAPP_STATE_BLE_TX ( uint8_t event );
uint8_t fnAPP_STATE_BLE_RX ( uint8_t event );
uint8_t fnAPP_STATE_Send_BLE_Data ( uint8_t event );
uint8_t fnAPP_STATE_Wait ( uint8_t event );

void fnAPP_STATE_ENTER_Init ( void );
void fnAPP_STATE_ENTER_Configuration ( void );
void fnAPP_STATE_ENTER_Run ( void );
void fnAPP_STATE_ENTER_BLE_TX ( void );
void fnAPP_STATE_ENTER_BLE_RX ( void );
void fnAPP_STATE_ENTER_Send_BLE_Data ( void );
void fnAPP_STATE_ENTER_Wait ( void );

void fnAPP_STATE_EXIT_Init ( void );
void fnAPP_STATE_EXIT_Configuration ( void );
void fnAPP_STATE_EXIT_Run ( void );
void fnAPP_STATE_EXIT_BLE_TX ( void );
void fnAPP_STATE_EXIT_BLE_RX ( void );
void fnAPP_STATE_EXIT_Send_BLE_Data ( void );
void fnAPP_STATE_EXIT_Wait ( void );

uint8_t bufferSize( uint8_t *pbuffer );
void clearBuffer(uint8_t *pbuffer,  uint8_t bufferSize);

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/
volatile bool b_state_timer_timeout;
extern UART_HandleTypeDef huart1;

uint32_t timestamp;
uint32_t u32_seg;
uint8_t f_config = 1;;

uint8_t janela_BLE;
uint16_t timeOut_BLE;
uint16_t timer_Uplink;

uint8_t num_Groups;
uint8_t buffer_rx[MAX_GROUPS*8+5];
uint16_t size_buffer;

/* Based on en_app_state_t enumeration */ 
const st_state_machine_functions_t st_app_state_machine_functions[] = {

		{  fnAPP_STATE_Init,            fnAPP_STATE_ENTER_Init,             fnAPP_STATE_EXIT_Init           },
		{  fnAPP_STATE_Configuration,  	fnAPP_STATE_ENTER_Configuration,   	fnAPP_STATE_EXIT_Configuration	},
		{  fnAPP_STATE_Run,             fnAPP_STATE_ENTER_Run,              fnAPP_STATE_EXIT_Run            },
		{  fnAPP_STATE_BLE_TX,			fnAPP_STATE_ENTER_BLE_TX,    		fnAPP_STATE_EXIT_BLE_TX			},
		{  fnAPP_STATE_BLE_RX,      	fnAPP_STATE_ENTER_BLE_RX,    		fnAPP_STATE_EXIT_BLE_RX			},
		{  fnAPP_STATE_Send_BLE_Data,   fnAPP_STATE_ENTER_Send_BLE_Data,	fnAPP_STATE_EXIT_Send_BLE_Data	},
		{  fnAPP_STATE_Wait,   			fnAPP_STATE_ENTER_Wait,				fnAPP_STATE_EXIT_Wait			},
};

st_state_machine_desc_t st_app_state_machine_desc;
uint16_t u16_app_state_machine_data;
uint8_t au8_downlink_frame[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
st_timer_index_t st_timer_check_config_timeout;

/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnAPP_STATE_MACHINE_Init ( void ) {

	st_app_state_machine_desc.pst_functions = (st_state_machine_functions_t*)&st_app_state_machine_functions[0];

	fnSTATE_Machine_Init( &st_app_state_machine_desc );  // executa fnAPP_STATE_ENTER_Init

	st_sigfox_events.u16_all_flags = 0;

	b_state_timer_timeout = false;

	return;

}


void fnAPP_STATE_Machine ( en_event_t event ) {

	fnSTATE_Machine( &st_app_state_machine_desc, event );

	return;

}

en_app_state_t fnAPP_STATE_Machine_Get ( void ) {

	return st_app_state_machine_desc.current_state;

}

void fnAPP_STATE_Machine_Set_Data ( uint16_t u16_data ) {

	u16_app_state_machine_data = u16_data;

	return;

}


/*************************************************************************************************/
/*    STATE ENTER FUNCTIONS                                                                      */
/*************************************************************************************************/

void fnAPP_STATE_ENTER_Init ( void ) {
	// Salva referencia pra calculo de tempo
	u32_seg = fnTIMESTAMP_Get_Updated_Timestamp_Seconds();
	fnCOMM_SIGMAIS_Request_Downlink_Frame();

	fnDEBUG_Const_String("STATE_ENTER_APP_Init\r\n");
	return;
}

void fnAPP_STATE_ENTER_Configuration ( void ) {
	fnDEBUG_Const_String("STATE_ENTER_APP_Configuration\r\n");
	return;
}

void fnAPP_STATE_ENTER_Run ( void ) {
	fnDEBUG_Const_String("STATE_ENTER_APP_Run\r\n");
	return;
}

void fnAPP_STATE_ENTER_BLE_TX ( void ) {
	return;
}

void fnAPP_STATE_ENTER_BLE_RX ( void ) {
	fnDEBUG_Const_String("STATE_ENTER_BLE_RX\r\n");
	return;
}

void fnAPP_STATE_ENTER_Send_BLE_Data ( void ) {
	fnDEBUG_Const_String("fnAPP_STATE_ENTER_Send_BLE_Data\r\n");
	return;
}

void fnAPP_STATE_ENTER_Wait ( void ) {
	// Salva referencia pra calculo de tempo
	u32_seg = fnTIMESTAMP_Get_Updated_Timestamp_Seconds();

	fnDEBUG_Const_String("STATE_ENTER_Wait\r\n");
	return;
}

/*************************************************************************************************/
/*    STATE FUNCTIONS                                                                            */
/*************************************************************************************************/


uint8_t fnAPP_STATE_Init ( uint8_t event ) {
	timestamp = fnTIMESTAMP_Get_Updated_Timestamp_Seconds();

	// Verifica se a flag chegou
	if(st_sigfox_events.flag.b_downlink_frame_received == true){
		return APP_STATE_CONFIGURATION;
	}

	// Espera até 7s pela flag
	if ( abs( timestamp - u32_seg ) >= 7 ){
		// Se deu timeout configura tempo padrão e segue o barco
		st_system_status.u8_op_code = 7;
		st_system_status.u8_janela_BLE = 5;
		st_system_status.u16_timeOut_BLE = 60;
		st_system_status.u16_timer_Uplink = 60;

		// Padrão procurar por todos grupos
		num_Groups = MAX_GROUPS;

		return APP_STATE_RUN;
	}

	return APP_STATE_INIT;
}

uint8_t fnAPP_STATE_Configuration ( uint8_t event ) {

	uint8_t * pu8_data;
	pu8_data = &au8_downlink_frame[0];

	// calib pend op_code frame_type
	//     0 	0     111        001
	// Config frame = 39 05 3C 00 3C 00
	fnCOMM_SIGMAIS_Decode_Downlink_Frame(pu8_data);

	// fazer switch case aqui dps
	if ( st_system_status.u8_op_code == EN_SIGMAIS_OP_CODE_ALL ){
		num_Groups = MAX_GROUPS;
	}

	st_sigfox_events.flag.b_downlink_frame_received = false;

	f_config = 1;

	return APP_STATE_RUN;
}

uint8_t fnAPP_STATE_Run ( uint8_t event ) {

	// Verifica se a flag chegou
	if(st_sigfox_events.flag.b_downlink_frame_received == true){
		return APP_STATE_CONFIGURATION;
	}

	// Limpa o buffer uart_rx
	uint8_t * prt = &buffer_rx[0];
	clearBuffer( prt, bufferSize( prt ));
	huart1.pRxBuffPtr = &buffer_rx[0];

	return APP_STATE_BLE_TX;
}

uint8_t fnAPP_STATE_BLE_TX ( uint8_t event ) {
	// Acorda o BLE pelo Pino PA8
	HAL_GPIO_WritePin(WKUP_BLE_GPIO_Port, WKUP_BLE, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

	// Verificar no osciloscopio a quantidade de tempo antre acordar o BLE e passar na UART
	HAL_Delay(30);

	if(f_config == 1){
		fnDEBUG_Const_String("[H:2");
		fnDEBUG_8bit_Hex(",", st_system_status.u8_op_code, "");
		fnDEBUG_8bit_Hex(",", st_system_status.u8_janela_BLE, "");
		fnDEBUG_16bit_Hex(",", st_system_status.u16_timeOut_BLE, "]\r\n");
		f_config = 0;
	}

	HAL_Delay(30);

	return APP_STATE_BLE_RX;
}

uint8_t fnAPP_STATE_BLE_RX ( uint8_t event ) {

	size_buffer = 5+num_Groups*8;
	// [H:1,1F:0001,20:0001,21:0001]
	// 5B 48 3A 31 2C 31 46 3A 30 30 30 31 2C 32 30 3A 30 30 30 31 2C 32 31 3A 30 30 30 31 5D
	// [  H	 :  1  ,  1  F  :  0  0  0  1  ,  2  0  :  0  0  0  1  ,  2  0  :  0  0  0  1  ]
	HAL_UART_Receive_IT(&huart1, &buffer_rx[0], size_buffer);

	char header1[] = "[H:1";
	if ( memcmp(&buffer_rx[0], header1, 4) != 0 ){
		uint8_t * prt = &buffer_rx[0];
		clearBuffer( prt, bufferSize( prt ));
		huart1.pRxBuffPtr = &buffer_rx[0];
		HAL_Delay(30);
		return APP_STATE_BLE_RX;
	}

	uint8_t * prt = &buffer_rx[0];
	num_Groups = ( bufferSize( prt ) - 5)/8 ;

	// dorme o ble
	HAL_GPIO_WritePin(WKUP_BLE_GPIO_Port, WKUP_BLE, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

	return APP_STATE_SEND_BLE_DATA;

}

uint8_t fnAPP_STATE_Send_BLE_Data ( uint8_t event ) {

	st_system_status.data_prt = &buffer_rx[0];
	st_system_status.data_size = num_Groups*6+1; // num de bytes a serem enviados

	fnCOMM_SIGMAIS_Send_Frame_Tabela();

	return APP_STATE_WAIT;
}

uint8_t fnAPP_STATE_Wait ( uint8_t event ) {
	timestamp = fnTIMESTAMP_Get_Updated_Timestamp_Seconds();

	// Time Check
	if ( abs( timestamp - u32_seg ) >= st_system_status.u16_timer_Uplink ){
		return APP_STATE_RUN;
	}
	return APP_STATE_WAIT;
}

/*************************************************************************************************/
/*    STATE EXIT FUNCTIONS                                                                       */
/*************************************************************************************************/

void fnAPP_STATE_EXIT_Init ( void ) {
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

void fnAPP_STATE_EXIT_Wait ( void ) {
	return;
}

/*************************************************************************************************/
/*    GENERAL FUNCTIONS                                                                          */
/*************************************************************************************************/

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

/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
