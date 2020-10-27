/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include <string.h>

#include "sigmais_encoder.h"

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

/* BITFIELDS */

void fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( en_sigmais_uplink_frame_type_t en_sigmais_uplink_frame_type,
                                                bool pending,
                                                bool b_battery_status,
												uint8_t empity,
                                                uint8_t * au8_data, uint8_t * pu8_data_size ) {
   
   st_sigmais_uplink_header_bitfield_t * pst_sigmais_uplink_header_bitfield = ( st_sigmais_uplink_header_bitfield_t * ) au8_data;
   
   pst_sigmais_uplink_header_bitfield->en_sigmais_uplink_frame_type = en_sigmais_uplink_frame_type;
   pst_sigmais_uplink_header_bitfield->pending = pending;
   pst_sigmais_uplink_header_bitfield->battery_status = b_battery_status;
   pst_sigmais_uplink_header_bitfield->empity = empity;
   
   *pu8_data_size += sizeof( st_sigmais_uplink_header_bitfield_t );
   
   return;
}

void fnSIGMAIS_ENCODER_Info_Frame_Tabela ( uint8_t * u8_data_prt, uint8_t * au8_data, uint8_t * pu8_data_size ){
   
	st_info_frame_tabela_t * pst_info_frame_tabela_t = ( st_info_frame_tabela_t * ) au8_data;

	pst_info_frame_tabela_t->major_b1 = * (st_system_status.data_prt);
	pst_info_frame_tabela_t->major_b2 = * (st_system_status.data_prt+1);
	// pula a virgula
	pst_info_frame_tabela_t->counter_b1 = * (st_system_status.data_prt+3);
	pst_info_frame_tabela_t->counter_b2 = * (st_system_status.data_prt+4);
	pst_info_frame_tabela_t->counter_b3 = * (st_system_status.data_prt+5);
	pst_info_frame_tabela_t->counter_b4 = * (st_system_status.data_prt+6);

   *pu8_data_size += sizeof( st_info_frame_tabela_t );
   
   return;
}

/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
