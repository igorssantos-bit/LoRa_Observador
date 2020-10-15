/*!
 * \file      sensors.c
 *
 * \brief     API to use accelerometer and magnetometer MEMs
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

#include <accelerometer.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>

#include "libs/math/average.h"
#include "libs/services/debug.h"

#include "sensors.h"
#include "system_status.h"

/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/

#define AVERAGE_DETECTION_BUFFER_SIZE               16
#define AVERAGE_TEMPERATURE_BUFFER_SIZE             16

/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/

int16_t i16_avg_detection_mag_x[AVERAGE_DETECTION_BUFFER_SIZE];
int16_t i16_avg_detection_mag_y[AVERAGE_DETECTION_BUFFER_SIZE];
int16_t i16_avg_detection_mag_z[AVERAGE_DETECTION_BUFFER_SIZE];
int8_t i8_avg_temperature[AVERAGE_TEMPERATURE_BUFFER_SIZE];


typedef struct {
  
   float f_avg_x;
   float f_avg_y;
   float f_avg_z;
   
} st_raw_mag_axis_t;


typedef struct {
   
   st_average_desc_t st_x_average_desc;
   st_average_desc_t st_y_average_desc;
   st_average_desc_t st_z_average_desc;
   
} st_raw_detect_mag_axis_t;




int16_t i16_x_mag_test;
int16_t i16_y_mag_test;
int16_t i16_z_mag_test;

int16_t magdeteccaox;
int16_t magdeteccaoy;
int16_t magdeteccaoz;

int16_t correcaoAnteriorx = 0;
int16_t correcaoAnteriory = 0;
int16_t correcaoAnteriorz = 0;

/*
typedef struct {
   
   uint8_t u8_threshold;
   uint8_t u8_detection_return;
   
} st_mag_sensivity_t;
*/

/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/


bool fnSENSORS_Has_Steady_Threshold ( st_accmag_raw_data_t * pst_accel_raw_data );

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/

//uint8_t u8_sensor_sensivity = 9;

st_raw_mag_axis_t st_steady_mag_axis;
st_raw_detect_mag_axis_t st_detection_mag_axis;
st_raw_mag_axis_t magnetometroSteadyDeteccao;

//bool b_first_data = true;

bool b_mag_detected = false;
bool b_accel_detected = false;

bool b_calibrating = true;

//uint8_t u8_state_counter = 60;
//uint8_t u8_state_detected_counter = 0;
//int8_t temperaturaDeteccao;

st_average_desc_8bits_t st_temp_average_desc;
/*
st_mag_sensivity_t st_mag_sensivity[8] = {
   { 10, 7 },  
   { 12, 8 },  
   { 15, 10 }, 
   { 20, 10 }, 
   { 25, 12 }, 
   { 25, 15 }, 
   { 30, 10 }, 
   { 30, 15 }, 
};
*/

uint16_t u16_strong_thr[8] = {
   0x0300,
   0x0600,
   0x0900,
   0x1200,
   0x1500,
   0x1800,
   0x2100,
   0x2400,
};

// Tabela de sensibilidade
// Nível de ruído parado: Pico: 52000 = 0xCB20
const uint32_t u32_sensivity_table[8] = {
		0x0000D6D8, // 0 - (55000) muito sensivel, detecta ar cond com fan nv 2
		0x0001C900, // 1 - (116992) muito sensivel, detecta ar cond sem problema com fan no max
		0x0006C900, // 2 - (444672)
		0x0009C900, // 3 - bom pra detectar o ventilador na parte de baixo
		0x0011C900, // 4 - fica alterando quando acoplado na parte de baixo do ventilador
		0x001AC900, // 5 - filtra o ventilador quando acoplado na parte de baixo
		0x002AC900, // 6 -
		0x003AC900, // 7 - detecta batidas fortes na mesa
};

/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnSENSORS_Init ( void ) {
   
   fnSENSORS_Config ( );
   
   return;
}

void fnSENSORS_Config ( void ) {
	//TODO: initialization

   st_temp_average_desc.px_data = i8_avg_temperature;
   fnAVERAGE_Init_8bits ( &st_temp_average_desc, AVERAGE_TEMPERATURE_BUFFER_SIZE, AVERAGE_TEMPERATURE_BUFFER_SIZE );

   b_accel_detected = false;
   //fnSENSORS_Set_Accel_Sensivity ( st_system_status.u8_sensor_sensivity );

   return;
}   

//

/*
void fnSENSORS_Mag_Threshold ( void ) {
   //TODO: ver a necessidade desta funcao
   return;
}   
*/

/*
void fnSENSORS_Replace ( void ) {
   
   u8_state_counter = 61;
	//u8_state_counter = 250;
   
   b_first_data = true;
   b_mag_detected = false;
   b_calibrating = true;
 
   return;
}
*/

/*
void fnSENSORS_Mag_Data_Ready ( st_accmag_raw_data_t * pst_accel_raw_data ) {
   return;
}   
*/

void fnSENSORS_Accel_Data_Ready ( st_accmag_raw_data_t * pst_accel_data ) {

   static uint8_t u8_debounce = 0;
   bool b_curr_accel_detected = false;

   //fnFXOS8700CQ_Read_Accel_Mag_Raw_Data ( pst_accel_data, pst_mag_data );
   fnLSM303_Read_Accel_Raw_Data ( dev_ctx_xl, ( st_accmag_raw_data_t * ) &st_system_status.st_accel_data);
   fnLSM303_Read_Temp_Raw_Data (dev_ctx_xl, ( st_accmag_raw_data_t * ) &st_system_status.st_mag_data );

   fnAVERAGE_Update_8bits(&st_temp_average_desc,st_system_status.st_mag_data.i8_temp);//fnAVERAGE_Update_8bits(&st_temp_average_desc,pst_accel_raw_data->i8_temp);
   st_system_status.u8_temperature_internal =  fnAVERAGE_Get_Value_8bits(&st_temp_average_desc); // para eliminar a leitura do sensor analogico que está em comm_sigmais


   uint32_t u32_vector_accel_pow;
   int32_t i32_axys_pow;

   i32_axys_pow = st_system_status.st_accel_data.i16_raw_x - st_system_status.st_accel_ref.i16_raw_x;
   u32_vector_accel_pow = i32_axys_pow * i32_axys_pow;

   i32_axys_pow = st_system_status.st_accel_data.i16_raw_y - st_system_status.st_accel_ref.i16_raw_y;
   u32_vector_accel_pow += i32_axys_pow * i32_axys_pow;

   i32_axys_pow = st_system_status.st_accel_data.i16_raw_z - st_system_status.st_accel_ref.i16_raw_z;
   u32_vector_accel_pow += i32_axys_pow * i32_axys_pow;

   //fnDEBUG_32bit_Hex(" ", u32_vector_accel_pow, " \r\n");

   b_curr_accel_detected = u32_vector_accel_pow > u32_sensivity_table[ st_system_status.u8_sensor_sensivity % 8 ];

   // DEBOUNCE DO SENSOR
   if( b_accel_detected != b_curr_accel_detected ) {
      u8_debounce++;
      if( u8_debounce >= ( b_accel_detected ? 15:4 ) ) { //6:2
         b_accel_detected = b_curr_accel_detected;
         u8_debounce = 0;
      }
   } else {
      u8_debounce = 0;
   }

   st_system_status.st_accel_ref.i16_raw_x = st_system_status.st_accel_data.i16_raw_x;
   st_system_status.st_accel_ref.i16_raw_y = st_system_status.st_accel_data.i16_raw_y;
   st_system_status.st_accel_ref.i16_raw_z = st_system_status.st_accel_data.i16_raw_z;

   return;
}

/*
void fnSENSORS_Accel_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_accel_data, st_fxos8700cq_raw_data_t * pst_mag_data ) {
   
   static uint8_t u8_debounce = 0;
   bool b_curr_accel_detected = false;
      
   //fnFXOS8700CQ_Read_Accel_Mag_Raw_Data ( pst_accel_data, pst_mag_data );
   fnLSM303_Read_Acel_Raw_Data ( dev_ctx_mg, ( st_fxos8700cq_raw_data_t * ) &st_system_status.st_accel_data);
   fnLSM303_Read_Temp_Raw_Data (dev_ctx_xl, ( st_fxos8700cq_raw_data_t * ) &st_system_status.st_mag_data );

   fnAVERAGE_Update_8bits(&st_temp_average_desc,st_system_status.st_mag_data.i8_temp);//fnAVERAGE_Update_8bits(&st_temp_average_desc,pst_accel_raw_data->i8_temp);
   st_system_status.i8_temperature =  fnAVERAGE_Get_Value_8bits(&st_temp_average_desc); // para eliminar a leitura do sensor analogico que está em comm_sigmais


   uint32_t u32_vector_accel_pow;
   int32_t i32_axys_pow;
   
   i32_axys_pow = st_system_status.st_accel_data.i16_raw_x - st_system_status.st_accel_ref.i16_raw_x; 
   u32_vector_accel_pow = i32_axys_pow * i32_axys_pow;
   
   i32_axys_pow = st_system_status.st_accel_data.i16_raw_y - st_system_status.st_accel_ref.i16_raw_y;
   u32_vector_accel_pow += i32_axys_pow * i32_axys_pow;
   
   i32_axys_pow = st_system_status.st_accel_data.i16_raw_z - st_system_status.st_accel_ref.i16_raw_z;
   u32_vector_accel_pow += i32_axys_pow * i32_axys_pow;
   
   b_curr_accel_detected = u32_vector_accel_pow > u32_sensivity_table[ st_system_status.u8_sensor_sensivity ];     
   
   if( b_accel_detected != b_curr_accel_detected ) {
      u8_debounce++;
      if( u8_debounce >= ( b_accel_detected ? 6: 2 ) ) {
         b_accel_detected = b_curr_accel_detected;
         u8_debounce = 0;
      }
   } else {
      u8_debounce = 0;
   }
   
   st_system_status.st_accel_ref.i16_raw_x = st_system_status.st_accel_data.i16_raw_x;         
   st_system_status.st_accel_ref.i16_raw_y = st_system_status.st_accel_data.i16_raw_y;
   st_system_status.st_accel_ref.i16_raw_z = st_system_status.st_accel_data.i16_raw_z;
   
   return;
}
*/
/*
void fnSENSORS_Set_Accel_Sensivity ( uint8_t u8_new_horimetro_sensivity ) {
//TODO: this function
	return;
}
*/

bool fnSENSORS_Has_Detection ( void ) {
   return b_accel_detected;
}

bool fnSENSORS_Check_Magnetometer_Valid_Threshold ( void ) {
//TODO: esta funcao
   uint8_t u8_mag_threshold_source = 0;

   //fnMAGNETOMETER_Read_Mag_Threshold_Data ( &u8_mag_threshold_source );

   return ( u8_mag_threshold_source > 1 );
}


bool fnSENSORS_Is_Mag_Calibrating ( void ) {

   return b_calibrating;
   
}


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
