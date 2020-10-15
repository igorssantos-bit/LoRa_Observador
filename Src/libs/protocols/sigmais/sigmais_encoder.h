/*!
 * \file      sigmais_encoder.h
 *
 * \brief     Prototypes for encoder.c
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
                                                uint8_t u8_counter,
                                                bool b_battery_status,
                                                bool b_parking_status,
                                                uint8_t * au8_data, uint8_t * pu8_data_size );

void fnSIGMAIS_ENCODER_Uplink_Header2_Bitfield ( en_sigmais_uplink_frame_type_t en_sigmais_uplink_frame_type,
                                                uint8_t bateriaStatus,
												uint8_t ciclo1,
												uint8_t ciclo2,
												uint8_t fmon,
												uint8_t fton,
                                                uint8_t * au8_data, uint8_t * pu8_data_size );


void fnSIGMAIS_ENCODER_Downlink_Header_Bitfield ( en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type,
                                                  bool b_pending_configuration,
                                                  uint8_t * au8_data, uint8_t * pu8_data_size );

void fnSIGMAIS_ENCODER_Time_Byte_Bitfield ( st_sigmais_time_byte_bitfield_t pst_sigmais_time,
                                            uint8_t * au8_data, uint8_t * pu8_data_size );



void fnSIGMAIS_ENCODER_Working_Hour_Bitfield ( uint8_t u8_actual_day_starting_hour,
                                               uint8_t u8_actual_day_finishing_hour,
                                               uint8_t u8_next_day_starting_hour,
                                               uint8_t u8_next_day_finishing_hour,
                                               uint8_t u8_third_day_starting_hour,
                                               uint8_t u8_third_day_finishing_hour,
                                               uint8_t * au8_data, uint8_t * pu8_data_size );

/* UPLINK FRAMES */
/*
void fnSIGMAIS_ENCODER_Info_Frame_Raw_Data ( uint8_t u8_mag_raw_data_x_msb,
                                             uint8_t u8_mag_raw_data_x_lsb,
                                             uint8_t u8_mag_raw_data_y_msb,
                                             uint8_t u8_mag_raw_data_y_lsb,
                                             uint8_t u8_mag_raw_data_z_msb,
                                             uint8_t u8_mag_raw_data_z_lsb,
                                             int8_t  i8_temperature,
                                             uint8_t u8_battery,
                                             uint32_t u32_timestamp,
                                             uint8_t * au8_data, uint8_t * pu8_data_size );
*/

/*
void fnSIGMAIS_ENCODER_Info_Frame_Parking_Processed ( int8_t i8_temperature,
                                                      uint8_t u8_battery,
                                                      uint32_t u32_timestamp,
                                                      uint8_t * au8_data, uint8_t * pu8_data_size );
*/
void fnSIGMAIS_ENCODER_Keep_Alive_Frame ( st_sigmais_uplink_header_bitfield_t un_sigmais_uplink_header_bitfield,
                                          uint8_t * au8_data, uint8_t * pu8_data_size );

void fnSIGMAIS_ENCODER_Config_Report_Frame ( uint8_t u8_configuration_sensivity,
                                             uint8_t u8_configuration_data_processed,
                                             uint8_t u8_module_type,
                                             st_sigmais_working_hour_bitfield_t un_sigmais_working_hour,
                                             uint32_t u32_timestamp,
                                             uint8_t * au8_data, uint8_t * pu8_data_size );

void fnSIGMAIS_ENCODER_Error_Frame ( uint8_t u8_error_code,
                                     uint8_t u8_error_data,
                                     uint8_t * au8_data, uint8_t * pu8_data_size );

void fnSIGMAIS_ENCODER_Info_Frame_Horimetro ( int8_t  i8_temperature,
                                              uint8_t u8_battery,
                                              uint32_t u32_timestamp,
                                              uint32_t u32_usage_time,
                                              uint32_t u32_number_of_times_timer_on,
                                              uint8_t * au8_data, uint8_t * pu8_data_size );
											  
void fnSIGMAIS_ENCODER_Error_Frame_Raw_Data (  uint8_t u8_error_code,
                                             uint8_t u8_mag_raw_data_x_msb,
                                             uint8_t u8_mag_raw_data_x_lsb,
                                             uint8_t u8_mag_raw_data_y_msb,
                                             uint8_t u8_mag_raw_data_y_lsb,
                                             uint8_t u8_mag_raw_data_z_msb,
                                             uint8_t u8_mag_raw_data_z_lsb,
                                             uint8_t * au8_data, uint8_t * pu8_data_size );
											 
void fnSIGMAIS_ENCODER_Info_Frame_Horimetro_Raw_Data ( uint8_t u8_mag_raw_data_x_msb,
											 uint8_t u8_mag_raw_data_x_lsb,
											 uint8_t u8_mag_raw_data_y_msb,
											 uint8_t u8_mag_raw_data_y_lsb,
											 uint8_t u8_mag_raw_data_z_msb,
											 uint8_t u8_mag_raw_data_z_lsb,
											 uint32_t u32_number_of_times_timer_on,
											 uint32_t u32_usage_time,
											 uint8_t * au8_data, uint8_t * pu8_data_size );


void fnSIGMAIS_ENCODER_Info_Frame_Horimetro_Pulsimetro (
		                                      uint8_t u8_fmon,
											  uint8_t u8_fpulse1,
											  uint8_t u8_fpulse2,
											  uint32_t u32_pulse_counter1,
											  uint32_t u32_pulse_counter2,
											  uint16_t u16_counter_machine_on,
											  uint16_t u16_temperature1,
											  uint16_t u16_temperature2,
                                              uint16_t u16_timer_on,
                                              uint8_t * au8_data, uint8_t * pu8_data_size );
/* DOWNLINK FRAMES */

void fnSIGMAIS_ENCODER_Daily_Update ( uint32_t u32_timestamp,                                   
                                      st_sigmais_working_hour_bitfield_t un_time_configuration,
                                      uint8_t * au8_data, uint8_t * pu8_data_size );

void fnSIGMAIS_ENCODER_Config_Frame ( uint8_t u8_configuration_sensivity,                      
                                      uint8_t u8_configuration_data_processed,
                                      uint8_t u8_module_type,                                                       
                                      uint8_t u8_configuration_reserved,                       
                                      uint8_t * au8_data, uint8_t * pu8_data_size );



#endif /* SIGMAIS_ENCODER_H_ */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/


