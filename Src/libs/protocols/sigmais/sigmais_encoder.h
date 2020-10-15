#ifndef SIGMAIS_ENCODER_H_
#define SIGMAIS_ENCODER_H_


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>

#include "sigmais_protocol.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/



/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/

/* BITFIELDS */

void fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( en_sigmais_uplink_frame_type_t en_sigmais_uplink_frame_type,
                                                bool pending,
                                                bool b_battery_status,
												uint8_t empity,
                                                uint8_t * au8_data, uint8_t * pu8_data_size );

void fnSIGMAIS_ENCODER_Info_Frame_Tabela ( uint8_t * data_prt, uint8_t * au8_data, uint8_t * pu8_data_size );


#endif /* SIGMAIS_ENCODER_H_ */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/

