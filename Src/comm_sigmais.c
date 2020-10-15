/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include "comm_sigmais.h"

#include "libs/protocols/sigmais/sigmais_encoder.h"
#include "libs/protocols/sigmais/sigmais_decoder.h"
#include "libs/protocols/sigmais/sigmais_protocol.h"
#include "libs/services/debug.h"
//#include "libs_firmware/services/delay_loop.h"

#include "system_status.h"
//#include "radio.h"
#include "timestamp.h"
#include "input_analogic.h"
//#include "board_config.h"
#include "LmHandler.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/

#define SIGMAIS_FRAME_COUNTER_MAX_VALUE   			7
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE  			242
/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE         LORAMAC_HANDLER_UNCONFIRMED_MSG



/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/

uint8_t u8_sigmais_frame_counter = SIGMAIS_FRAME_COUNTER_MAX_VALUE;

pfn_comm_sigmais_downlink_frame_callback_t pfn_comm_sigmais_downlink_frame_callback = 0;

/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnCOMM_SIGMAIS_Send_Frame_Tabela ( void ){
	uint8_t au8_data[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];
	uint8_t u8_data_size = 0;

	st_system_status.b_battery_status = fnINPUT_ANALOGIC_Get_Battery ( &st_system_status.u8_battery );

	// Counter
	bool pend_bit = 0;
	// data_size é uint16_t, por isso multiplicamos por 2
	if( st_system_status.data_size > LORAWAN_APP_DATA_BUFFER_MAX_SIZE ){
		pend_bit = 1;
	}

	/* Mount packet */
	fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( 	EN_SIGMAIS_UPLINK_TABELA_DEVICES,
												pend_bit,
												st_system_status.b_battery_status,
												0,
												&au8_data[u8_data_size], &u8_data_size );

	// pula o cabeçalho
	st_system_status.data_prt += 5;
	while( u8_data_size < st_system_status.data_size-1 ){
		fnSIGMAIS_ENCODER_Info_Frame_Tabela ( st_system_status.data_prt, &au8_data[u8_data_size], &u8_data_size );
		st_system_status.data_prt += 8;
	}

	for (uint8_t ii = 0; ii < LORAWAN_APP_DATA_BUFFER_MAX_SIZE-u8_data_size; ii++ ){
		au8_data[u8_data_size+ii] = 0;
	}

	/* Transmit */
	if( LmHandlerIsBusy( ) == true ) {
	   return;
	}

	LmHandlerAppData_t local_AppData =
	{
	   .Buffer = &au8_data,
	   .BufferSize = u8_data_size,
	   .Port = 2
	};

	LmHandlerSend( &local_AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE );

	return;

}

/* DOWNLINK FRAMES */


void fnCOMM_SIGMAIS_Decode_Downlink_Frame ( uint8_t * pu8_data ) {

	en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type;
	en_sigmais_downlink_frame_type = fnSIGMAIS_DECODER_Downlink( pu8_data );

	if( pfn_comm_sigmais_downlink_frame_callback != 0 ) {
		pfn_comm_sigmais_downlink_frame_callback( en_sigmais_downlink_frame_type );
	}


	if (en_sigmais_downlink_frame_type == EN_SIGMAIS_DOWNLINK_CONFIG_FRAME) {
		st_system_status.b_downlink_config_frame_received = true;
	}

	return;
}

/* VOID FRAMES */


void fnCOMM_SIGMAIS_Request_Downlink_Frame ( void ) {

	uint8_t au8_data[1];
	uint8_t u8_data_size;

	au8_data[0] = 0;
	u8_data_size = 1;

	/* Transmit */

	if( LmHandlerIsBusy( ) == true )
	{
		return;
	}

	LmHandlerAppData_t local_AppData =
	{
		.Buffer = &au8_data,
		.BufferSize = u8_data_size,
		.Port = 2
	};
	LmHandlerSend( &local_AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE );


	return;
}

/* GENERIC FRAMES */

/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
