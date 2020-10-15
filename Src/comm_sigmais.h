/*!
 * \file      comm_sigmais.h
 *
 * \brief     Header of Functions that are used to do communication
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

#ifndef COMM_SIGMAIS_H_
#define COMM_SIGMAIS_H_


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include "libs/protocols/sigmais/sigmais_protocol.h"

#include <stdint.h>
#include <stdbool.h>


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/




/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/

typedef void ( * pfn_comm_sigmais_downlink_frame_callback_t ) ( en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type );

/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/

void fnCOMM_SIGMAIS_Donwlink_Frame_Set_Callback ( pfn_comm_sigmais_downlink_frame_callback_t pfn_downlink_frame_callback );

//void fnCOMM_SIGMAIS_Send_Info_Frame_Parking_Processed ( void ) ;
//void fnCOMM_SIGMAIS_Send_Info_Frame_Raw_Data ( void );
//void fnCOMM_SIGMAIS_Send_Error_Frame_Raw_Data ( void );
void fnCOMM_SIGMAIS_Send_Config_Report_Frame ( void );
void fnCOMM_SIGMAIS_Send_Error_Frame ( uint8_t u8_error_code, uint8_t u8_error_data );
void fnCOMM_SIGMAIS_Send_Info_Frame_Horimetro ( void ) ;
void fnCOMM_SIGMAIS_Send_Info_Frame_Horimetro_Raw_Data(void);
void fnCOMM_SIGMAIS_Decode_Downlink_Frame ( uint8_t * pu8_data );
void fnCOMM_SIGMAIS_Request_Downlink_Frame ( void );
void fnCOMM_SIGMAIS_Send_Info_Frame ( void );
void fnCOMM_SIF_Send_Horimetro_Pulsimetro ( void );

//void fnCOMM_SIGMAIS_Send_Empyt_Average_Data ( void );

//bool xSemaphoreTake(void);
//void xSemaphoreGive(void);
uint8_t getBusyCounter(void);

#endif /* COMM_SIGMAIS_H_ */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
