/*!
 * \file      sigmais_encoder.c
 *
 * \brief     Encoder data in a proprietary protocol, to be used with Lora and Sigfox nets.
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
                                                uint8_t u8_counter,
                                                bool b_battery_status,
                                                bool b_parking_status,
                                                uint8_t * au8_data, uint8_t * pu8_data_size ) {
   
   st_sigmais_uplink_header_bitfield_t * pst_sigmais_uplink_header_bitfield = ( st_sigmais_uplink_header_bitfield_t * ) au8_data;
   
   pst_sigmais_uplink_header_bitfield->en_sigmais_uplink_frame_type = en_sigmais_uplink_frame_type;
   pst_sigmais_uplink_header_bitfield->frame_counter = u8_counter;
   pst_sigmais_uplink_header_bitfield->battery_status = b_battery_status;
   pst_sigmais_uplink_header_bitfield->parking_status = b_parking_status;
   
   *pu8_data_size += sizeof( st_sigmais_uplink_header_bitfield_t );
   
   return;
}

void fnSIGMAIS_ENCODER_Uplink_Header2_Bitfield ( en_sigmais_uplink_frame_type_t en_sigmais_uplink_frame_type,
                                                uint8_t bateriaStatus,
												uint8_t ciclo1,
												uint8_t ciclo2,
												uint8_t fmon,
												uint8_t fton,
                                                uint8_t * au8_data, uint8_t * pu8_data_size ) {

   st_sigmais_uplink_header2_bitfield_t *pst_suh2 = ( st_sigmais_uplink_header2_bitfield_t * ) au8_data;

   pst_suh2->en_sigmais_uplink_frame_type = en_sigmais_uplink_frame_type;
   pst_suh2->flag1 = bateriaStatus;
   pst_suh2->ciclo1 = ciclo1;
   pst_suh2->ciclo2 = ciclo2;
   pst_suh2->ciclo3 = fmon;
   pst_suh2->ciclo4 = fton;

   *pu8_data_size += sizeof( st_sigmais_uplink_header2_bitfield_t );

   return;
}

void fnSIGMAIS_ENCODER_Downlink_Header_Bitfield ( en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type,
                                                  bool b_pending_configuration,
                                                  uint8_t * au8_data, uint8_t * pu8_data_size ) {
   
   st_sigmais_downlink_header_bitfield_t * pst_sigmais_downlink_header_bitfield = ( st_sigmais_downlink_header_bitfield_t * ) au8_data;

   pst_sigmais_downlink_header_bitfield->en_sigmais_downlink_frame_type = en_sigmais_downlink_frame_type;
   pst_sigmais_downlink_header_bitfield->pending_configuration = b_pending_configuration;
   
   *pu8_data_size += sizeof( st_sigmais_downlink_header_bitfield_t );
   
   return;
}

void fnSIGMAIS_ENCODER_Time_Byte_Bitfield ( st_sigmais_time_byte_bitfield_t pst_sigmais_time,
                                            uint8_t * au8_data, uint8_t * pu8_data_size ) {
   
   st_sigmais_time_byte_bitfield_t * pst_sigmais_time_byte_bitfield = ( st_sigmais_time_byte_bitfield_t * ) au8_data;   

   pst_sigmais_time_byte_bitfield->time_unity = pst_sigmais_time.time_unity;
   pst_sigmais_time_byte_bitfield->time_value = pst_sigmais_time.time_value;
   
   *pu8_data_size += sizeof( st_sigmais_time_byte_bitfield_t ) ;
   
   return;
}

void fnSIGMAIS_ENCODER_Working_Hour_Bitfield ( uint8_t u8_actual_day_starting_hour,
                                               uint8_t u8_actual_day_finishing_hour,
                                               uint8_t u8_next_day_starting_hour,
                                               uint8_t u8_next_day_finishing_hour,
                                               uint8_t u8_third_day_starting_hour,
                                               uint8_t u8_third_day_finishing_hour,
                                               uint8_t * au8_data, uint8_t * pu8_data_size ) {
   
   st_sigmais_working_hour_bitfield_t * pst_sigmais_working_hour_bitfield = ( st_sigmais_working_hour_bitfield_t * ) au8_data;   

   pst_sigmais_working_hour_bitfield->actual_day_starting_hour = u8_actual_day_finishing_hour;
   pst_sigmais_working_hour_bitfield->actual_day_finishing_hour = u8_actual_day_finishing_hour;
   pst_sigmais_working_hour_bitfield->next_day_starting_hour = u8_next_day_starting_hour;
   pst_sigmais_working_hour_bitfield->next_day_finishing_hour = u8_next_day_finishing_hour;
   pst_sigmais_working_hour_bitfield->third_day_starting_hour = u8_third_day_starting_hour;
   pst_sigmais_working_hour_bitfield->third_day_finishing_hour = u8_third_day_finishing_hour;
   
   *pu8_data_size += sizeof( st_sigmais_working_hour_bitfield_t );
   
   return;
}


/* UPLINK FRAMES */

void fnSIGMAIS_ENCODER_Info_Frame_Raw_Data ( uint8_t u8_mag_raw_data_x_msb,
                                             uint8_t u8_mag_raw_data_x_lsb,
                                             uint8_t u8_mag_raw_data_y_msb,
                                             uint8_t u8_mag_raw_data_y_lsb,
                                             uint8_t u8_mag_raw_data_z_msb,
                                             uint8_t u8_mag_raw_data_z_lsb,
                                             int8_t  i8_temperature,
                                             uint8_t u8_battery,
                                             uint32_t u32_timestamp,
                                             uint8_t * au8_data, uint8_t * pu8_data_size ) {
   
   st_info_frame_raw_data_t * pst_info_frame_raw_data = ( st_info_frame_raw_data_t * ) au8_data;
   
   pst_info_frame_raw_data->mag_raw_data_x_msb = u8_mag_raw_data_x_msb;
   pst_info_frame_raw_data->mag_raw_data_x_lsb = u8_mag_raw_data_x_lsb;
   pst_info_frame_raw_data->mag_raw_data_y_msb = u8_mag_raw_data_y_msb;
   pst_info_frame_raw_data->mag_raw_data_y_lsb = u8_mag_raw_data_y_lsb;
   pst_info_frame_raw_data->mag_raw_data_z_msb = u8_mag_raw_data_z_msb;
   pst_info_frame_raw_data->mag_raw_data_z_lsb = u8_mag_raw_data_z_lsb;
   // TODO: efetuar a conversao de 16bits para 8 bits na temperatura
   pst_info_frame_raw_data->temperature = (int8_t) i8_temperature;

   pst_info_frame_raw_data->battery = u8_battery;
   pst_info_frame_raw_data->timestamp = u32_timestamp;
   
   *pu8_data_size += sizeof( st_info_frame_raw_data_t );
   
   return;
}


void fnSIGMAIS_ENCODER_Info_Frame_Parking_Processed ( int8_t i8_temperature,
                                                      uint8_t u8_battery,
                                                      uint32_t u32_timestamp,
                                                      uint8_t * au8_data, uint8_t * pu8_data_size ) {
   
   st_sigmais_info_frame_parking_processed_t * pst_sigmais_info_frame_parking_processed = ( st_sigmais_info_frame_parking_processed_t * ) au8_data; 

   // TODO: efetuar a conversao de 16bits para 8 bits na temperatura
   pst_sigmais_info_frame_parking_processed->temperature = i8_temperature;


   pst_sigmais_info_frame_parking_processed->battery = u8_battery;
   pst_sigmais_info_frame_parking_processed->timestamp = u32_timestamp;

   *pu8_data_size += sizeof( st_sigmais_info_frame_parking_processed_t );
   
   return;
}


void fnSIGMAIS_ENCODER_Keep_Alive_Frame ( st_sigmais_uplink_header_bitfield_t un_sigmais_uplink_header_bitfield,
                                                      uint8_t * au8_data, uint8_t * pu8_data_size ) {
   
   st_sigmais_keep_alive_frame_t * pst_sigmais_keep_alive_frame_t = ( st_sigmais_keep_alive_frame_t * ) au8_data;
   
   pst_sigmais_keep_alive_frame_t->un_sigmais_uplink_header_bitfield = un_sigmais_uplink_header_bitfield;
   
   *pu8_data_size += sizeof( st_sigmais_keep_alive_frame_t );
   
   return;
}


void fnSIGMAIS_ENCODER_Config_Report_Frame ( uint8_t u8_configuration_sensivity,
                                             uint8_t u8_configuration_data_processed,
                                             uint8_t u8_module_type,
                                             st_sigmais_working_hour_bitfield_t un_sigmais_working_hour,
                                             uint32_t u32_timestamp,
                                             uint8_t * au8_data, uint8_t * pu8_data_size ) {
                                          
   st_sigmais_config_report_frame_t * pst_sigmais_config_report_frame = ( st_sigmais_config_report_frame_t * ) au8_data;                                          

   pst_sigmais_config_report_frame->configuration_sensivity = u8_configuration_sensivity;
   pst_sigmais_config_report_frame->configuration_data_processed = u8_configuration_data_processed;
   pst_sigmais_config_report_frame->module_type = u8_module_type;
   pst_sigmais_config_report_frame->configuration_reserved = 0;
   pst_sigmais_config_report_frame->sigmais_working_hour = un_sigmais_working_hour;
   pst_sigmais_config_report_frame->timestamp = u32_timestamp;

   *pu8_data_size += sizeof( st_sigmais_config_report_frame_t );
   
   return;
}


void fnSIGMAIS_ENCODER_Error_Frame ( uint8_t u8_error_code,
                                     uint8_t u8_error_data,
                                     uint8_t * au8_data, uint8_t * pu8_data_size ) {
                                              
   st_sigmais_error_frame_t * pst_sigmais_error_frame = ( st_sigmais_error_frame_t * ) au8_data;                                                 
                                                         
   pst_sigmais_error_frame->error_code = u8_error_code;
   pst_sigmais_error_frame->error_data = u8_error_data;
   
   *pu8_data_size += sizeof( st_sigmais_error_frame_t );
   
   return;
}

void fnSIGMAIS_ENCODER_Error_Frame_Raw_Data (  uint8_t u8_error_code,
                                             uint8_t u8_mag_raw_data_x_msb,
                                             uint8_t u8_mag_raw_data_x_lsb,
                                             uint8_t u8_mag_raw_data_y_msb,
                                             uint8_t u8_mag_raw_data_y_lsb,
                                             uint8_t u8_mag_raw_data_z_msb,
                                             uint8_t u8_mag_raw_data_z_lsb,
                                             uint8_t * au8_data, uint8_t * pu8_data_size ) {
   
   st_error_frame_raw_data_t * pst_error_frame_raw_data = ( st_error_frame_raw_data_t * ) au8_data;

   pst_error_frame_raw_data->error_code = u8_error_code;
   pst_error_frame_raw_data->mag_raw_data_x_msb = u8_mag_raw_data_x_msb;
   pst_error_frame_raw_data->mag_raw_data_x_lsb = u8_mag_raw_data_x_lsb;
   pst_error_frame_raw_data->mag_raw_data_y_msb = u8_mag_raw_data_y_msb;
   pst_error_frame_raw_data->mag_raw_data_y_lsb = u8_mag_raw_data_y_lsb;
   pst_error_frame_raw_data->mag_raw_data_z_msb = u8_mag_raw_data_z_msb;
   pst_error_frame_raw_data->mag_raw_data_z_lsb = u8_mag_raw_data_z_lsb;

   *pu8_data_size += sizeof( st_error_frame_raw_data_t );
   
   return;
}

void fnSIGMAIS_ENCODER_Info_Frame_Horimetro ( int8_t  i8_temperature,      
                                              uint8_t u8_battery,
                                              uint32_t u32_timestamp,
                                              uint32_t u32_usage_time,
											  uint32_t u32_number_of_times_timer_on,
                                              uint8_t * au8_data, uint8_t * pu8_data_size ) {
                                                       
   st_sigmais_info_frame_horimetro_t * pst_sigmais_info_frame_horimetro = ( st_sigmais_info_frame_horimetro_t * ) au8_data;                                                 

   pst_sigmais_info_frame_horimetro->temperature = i8_temperature;
   pst_sigmais_info_frame_horimetro->battery = u8_battery;
   pst_sigmais_info_frame_horimetro->timestamp =  u32_timestamp;
   pst_sigmais_info_frame_horimetro->usage_time = u32_usage_time;
   pst_sigmais_info_frame_horimetro->number_of_times_timer_on = u32_number_of_times_timer_on;
   
   *pu8_data_size += sizeof( st_sigmais_info_frame_horimetro_t );
   
   return;
}

void fnSIGMAIS_ENCODER_Info_Frame_Horimetro_Raw_Data ( uint8_t u8_mag_raw_data_x_msb,
                                                       uint8_t u8_mag_raw_data_x_lsb,
                                                       uint8_t u8_mag_raw_data_y_msb,
                                                       uint8_t u8_mag_raw_data_y_lsb,
                                                       uint8_t u8_mag_raw_data_z_msb,
                                                       uint8_t u8_mag_raw_data_z_lsb,
													   uint32_t u32_number_of_times_timer_on,
                                                       uint32_t u32_usage_time,
                                                       uint8_t * au8_data, uint8_t * pu8_data_size ) {
	
	st_info_frame_horimetro_raw_data_t * pst_info_frame_horimetro_raw_data = ( st_info_frame_horimetro_raw_data_t * ) au8_data;
	
	pst_info_frame_horimetro_raw_data->mag_raw_data_x_msb = u8_mag_raw_data_x_msb;
	pst_info_frame_horimetro_raw_data->mag_raw_data_x_lsb = u8_mag_raw_data_x_lsb;
	pst_info_frame_horimetro_raw_data->mag_raw_data_y_msb = u8_mag_raw_data_y_msb;
	pst_info_frame_horimetro_raw_data->mag_raw_data_y_lsb = u8_mag_raw_data_y_lsb;
	pst_info_frame_horimetro_raw_data->mag_raw_data_z_msb = u8_mag_raw_data_z_msb;
	pst_info_frame_horimetro_raw_data->mag_raw_data_z_lsb = u8_mag_raw_data_z_lsb;
	
    pst_info_frame_horimetro_raw_data->number_of_times_timer_on = (uint16_t) u32_number_of_times_timer_on; 
    pst_info_frame_horimetro_raw_data->usage_time = u32_usage_time;

	
	*pu8_data_size += sizeof( st_info_frame_horimetro_raw_data_t );
	
	return;
}

/*
 * Horimetro and Pulsimetro Byte Field
 *
 * Byte  | Nome   | Descrição
 * ----- | -------| ----------------
 * 0     | Header | Bitfield header uplink
 * 1 LSB | Bat (4 bits)    | Valor da bateria. Valores de 2.5 a 4.0V (basta multiplicar por 10 o valor analógico e subtrair o resultado de 25
 * 1 MSB | TempF  | Parte fracionaria da temperatura de 0 a 9 (4 bits)
 * 2     | Temp   | Temperatura parte inteira -127°C a +128°C
 * 3 - 5 | N_Pulso| Numero acumulado de pulsos (24 bits)
 * 6 - 8 | Tempo  | Tempo acumulado de máquina ligada (24 bits)
 * 9 - 11| Counter| Quantidade de vezes que a máquina foi ligada
 *
 * */

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
                                              uint8_t * au8_data, uint8_t * pu8_data_size )
{
   st_sif_horimetro_pulsimetro_t * pst_horimetro_pulsimetro_protocol = ( st_sif_horimetro_pulsimetro_t * ) au8_data;
   pst_horimetro_pulsimetro_protocol->u8_fmon = u8_fmon;
   pst_horimetro_pulsimetro_protocol->u8_fpulse1 = u8_fpulse1;
   pst_horimetro_pulsimetro_protocol->u8_fpulse2 = u8_fpulse2;
   pst_horimetro_pulsimetro_protocol->u32_pulse_counter1 = u32_pulse_counter1;
   pst_horimetro_pulsimetro_protocol->u32_pulse_counter2 = u32_pulse_counter2;
   pst_horimetro_pulsimetro_protocol->u16_counter_machine_on = u16_counter_machine_on;
   pst_horimetro_pulsimetro_protocol->u16_temperature1 = u16_temperature1;
   pst_horimetro_pulsimetro_protocol->u16_temperature2 = u16_temperature2;
   pst_horimetro_pulsimetro_protocol->u16_timer_on = u16_timer_on;
   *pu8_data_size += sizeof( st_sigmais_info_frame_horimetro_t );
   return;
}


/* DOWNLINK FRAMES */

void fnSIGMAIS_ENCODER_Daily_Update ( uint32_t u32_timestamp,                                   
                                      st_sigmais_working_hour_bitfield_t un_time_configuration,
                                      uint8_t * au8_data, uint8_t * pu8_data_size ) {
                                                         
   st_sigmais_daily_update_t * pst_sigmais_daily_update = ( st_sigmais_daily_update_t * ) au8_data;
                                                         
   pst_sigmais_daily_update->timestamp = u32_timestamp;
   pst_sigmais_daily_update->time_configuration = un_time_configuration;
   
   *pu8_data_size += sizeof( st_sigmais_daily_update_t );
   
   return;
}


void fnSIGMAIS_ENCODER_Config_Frame ( uint8_t u8_configuration_sensivity,                      
                                      uint8_t u8_configuration_data_processed,
                                      uint8_t u8_module_type,                                                       
                                      uint8_t u8_configuration_reserved,                       
                                      uint8_t * au8_data, uint8_t * pu8_data_size ) {
                          
   st_sigmais_config_frame_t * pst_sigmais_config_frame = ( st_sigmais_config_frame_t * ) au8_data;
                             
   pst_sigmais_config_frame->configuration_sensivity = u8_configuration_sensivity;
   pst_sigmais_config_frame->configuration_data_processed = u8_configuration_data_processed;
   pst_sigmais_config_frame->module_type = u8_module_type;
   pst_sigmais_config_frame->configuration_reserved = u8_configuration_reserved;
   pst_sigmais_config_frame->reserved = 0;                                   
   
   *pu8_data_size += sizeof( st_sigmais_config_frame_t );
   
   return;
}


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
