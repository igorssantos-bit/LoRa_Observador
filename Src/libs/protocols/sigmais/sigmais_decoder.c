/*!
 * \file      sigmais_decoder.c
 *
 * \brief     Decode downlink packets
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

#include <string.h>

#include "libs/protocols/sigmais/sigmais_protocol.h"
#include "libs/protocols/sigmais/sigmais_decoder.h"

#include "app.h"
#include "system_status.h"
#include "board.h"

/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/


en_sigmais_downlink_frame_type_t fnSIGMAIS_DECODER_Downlink ( uint8_t * au8_data ) {
//enum status_code ret;
uint8_t u8_aux = 0;
   
   en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type;   
   st_sigmais_downlink_header_bitfield_t * pst_downlink_header_bitfield = ( st_sigmais_downlink_header_bitfield_t * ) au8_data;
   
   st_system_status.b_configuration_pending = pst_downlink_header_bitfield->pending_configuration;
   st_system_status.b_calibration_authorized = pst_downlink_header_bitfield->calibration_authorized;
   
   switch ( pst_downlink_header_bitfield->en_sigmais_downlink_frame_type ) {
      
      case EN_SIGMAIS_DOWNLINK_DAILY_UPDATE: {
         
         st_sigmais_daily_update_t * pst_sigmais_daily_update = ( st_sigmais_daily_update_t * ) &au8_data[1];
         
         st_system_status.u32_timestamp = pst_sigmais_daily_update->timestamp; 
         
         st_system_status.u8_first_day_start_hour = pst_sigmais_daily_update->time_configuration.actual_day_starting_hour;
         st_system_status.u8_first_day_end_hour = pst_sigmais_daily_update->time_configuration.actual_day_finishing_hour;
         st_system_status.u8_second_day_start_hour = pst_sigmais_daily_update->time_configuration.next_day_starting_hour;
         st_system_status.u8_second_day_end_hour = pst_sigmais_daily_update->time_configuration.next_day_finishing_hour;
         st_system_status.u8_third_day_start_hour = pst_sigmais_daily_update->time_configuration.third_day_starting_hour;
         st_system_status.u8_third_day_end_hour = pst_sigmais_daily_update->time_configuration.third_day_finishing_hour;
         
         if (pst_downlink_header_bitfield->reset_counter1 == 1){
        	 st_system_status.u32_number_of_pulsos_1 = 0;
         }

         if (pst_downlink_header_bitfield->reset_counter2 == 1){
        	 st_system_status.u32_number_of_pulsos_2 = 0;
         }

         if (pst_downlink_header_bitfield->reset_mon_ton == 1){
        	 st_system_status.u32_machine_on = 0;
        	 st_system_status.u32_timer_on = 0;
        	 // TODO: alterar o u32_timer_off para ser um contador de tempo total de dispositivo em funcionamento
         }

         en_sigmais_downlink_frame_type = EN_SIGMAIS_DOWNLINK_DAILY_UPDATE;
         
         break;
      }

      case EN_SIGMAIS_DOWNLINK_CONFIG_FRAME: {
         
         st_sigmais_config_frame_t * pst_sigmais_config_frame = ( st_sigmais_config_frame_t * ) &au8_data[1];
                        
         st_system_status.b_data_processed = pst_sigmais_config_frame->configuration_data_processed;
         st_system_status.u8_sensor_sensivity = pst_sigmais_config_frame->configuration_sensivity;

         st_system_status.st_sigmais_confirmation_time = pst_sigmais_config_frame->time_byte_confirmation;
         st_system_status.st_sigmais_keep_alive_time = pst_sigmais_config_frame->time_byte_inactivity;
         st_system_status.st_sigmais_detection_debounce = pst_sigmais_config_frame->time_byte_detection_debounce;

         st_system_status.u8_strong_mag_sensivity = pst_sigmais_config_frame->magnet_sensivity ;
         st_system_status.u8_n_retransmitions = pst_sigmais_config_frame->retransmition_counter ;
		 st_system_status.st_sigmais_transmission_timer = pst_sigmais_config_frame->time_byte_transmission_timer;   

		 // teste de validade dos dados
		 /*
		 if (st_system_status.st_sigmais_transmission_timer.time_value == 0){
			 st_system_status.st_sigmais_transmission_timer.time_value = 1;
			 if ( (st_system_status.st_sigmais_transmission_timer.time_unity == EN_SIGMAIS_TIME_SECONDS) && (st_system_status.st_sigmais_transmission_timer.time_value < 60) ){
				 st_system_status.st_sigmais_transmission_timer.time_value = 60;
			 }
		 }
		 */
		 validateTransmissionTimer();

		 //saving configurations
		 uint8_t flags = readFromEEPROM(ADD_FLAGS);
		 flags &= ~0x01;
		 writeByteToEEPROM(ADD_FLAGS,flags);

		 uint32_t valor;
		 valor = (st_system_status.st_sigmais_transmission_timer.time_value << 2) | (st_system_status.st_sigmais_transmission_timer.time_unity);
		 valor <<= 8;
		 valor |= (st_system_status.st_sigmais_detection_debounce.time_value << 2) | (st_system_status.st_sigmais_detection_debounce.time_unity);
		 valor <<= 8;
         valor |=  st_system_status.u8_sensor_sensivity;
         valor <<= 8;
         valor |= st_system_status.b_data_processed;
		 writeWordToEEPROM(ADD_DATA_TYPE,valor);

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


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
