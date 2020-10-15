/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include <string.h>

#include "libs/protocols/sigmais/sigmais_protocol.h"
//#include "libs/protocols/sigmais/sigmais_decoder.h"
#include "sigmais_decoder.h"

#include "system_status.h"

/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/


/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

en_sigmais_downlink_frame_type_t fnSIGMAIS_DECODER_Downlink ( uint8_t * au8_data ) {

   st_sigmais_downlink_header_bitfield_t * pst_sigmais_downlink_header_bitfield = ( st_sigmais_downlink_header_bitfield_t *) au8_data;

   en_sigmais_downlink_frame_type_t  en_sigmais_downlink_frame_type = (en_sigmais_downlink_frame_type_t) pst_sigmais_downlink_header_bitfield->en_sigmais_downlink_frame_type;
   st_system_status.u8_op_code = pst_sigmais_downlink_header_bitfield->op_code;
   st_system_status.b_configuration_pending = pst_sigmais_downlink_header_bitfield->pending;
   st_system_status.b_calibration_authorized = pst_sigmais_downlink_header_bitfield->calibration_authorized;
   
   switch ( pst_sigmais_downlink_header_bitfield->en_sigmais_downlink_frame_type ) {
//   switch ( en_sigmais_downlink_frame_type ) {

   case EN_SIGMAIS_DOWNLINK_CONFIG_FRAME:
   {
	   st_sigmais_config_frame_t * pst_sigmais_config_frame = ( st_sigmais_config_frame_t * ) &au8_data[1];

	   st_system_status.u8_janela_BLE = pst_sigmais_config_frame->janela_BLE;
	   st_system_status.u16_timeOut_BLE = pst_sigmais_config_frame->timeOut_BLE;
	   st_system_status.u16_timer_Uplink = pst_sigmais_config_frame->timer_Uplink;

	   en_sigmais_downlink_frame_type = EN_SIGMAIS_DOWNLINK_CONFIG_FRAME;

	   break;
   }

   default: {

	   en_sigmais_downlink_frame_type = EN_SIGMAIS_DOWNLINK_SIZE;

	   break;
   }
   }
   
   return en_sigmais_downlink_frame_type;
}


/*
* en_sigmais_timebyte_to_minute
* Converte um TimeByte para minutos.
* Parametros: primeiros 5 bits ( 0 - 63), que é o tempo, e os últimos 3 bits (segundos, minutos, horas, dias),
*             que é a unidade do timebyte;
* Retorna: o número inteiro de minutos
*          quando a configuração estiver em segundos e o número for menor do que 60, retorna 0
*/

uint32_t en_sigmais_timebyte_to_minute(uint8_t tempo, uint8_t unidade ){
uint32_t valor = 0;
   unidade &= 0x03;

   switch(unidade){
   case 0:
   {
	      valor = (uint32_t) (tempo & 0x1f)/60;   // segundos -> minutos
   }
   break;

   case 1:
   {
		valor = (uint32_t) (tempo & 0x1f);    // minutos -> minutos
   }
   break;

   case 2:
   {
	   valor = (uint32_t) (tempo & 0x1f) * 60;    // horas -> minutos
   }
   break;
   case 3:
   {
	   valor = (uint32_t) (tempo & 0x1f) * 24 * 60;    // dias -> minutos
   }
   break;
   default:
   {
   }
   break;
   }
   return valor;
}


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
