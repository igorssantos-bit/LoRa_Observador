/*!
 * \file      comm_sigmais.c
 *
 * \brief     Functions that are used to do communication
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

#include "comm_sigmais.h"

#include "libs/protocols/sigmais/sigmais_encoder.h"
#include "libs/protocols/sigmais/sigmais_decoder.h"
#include "libs/protocols/sigmais/sigmais_protocol.h"
#include "libs/services/debug.h"

#include "system_status.h"
#include "timestamp.h"
#include "input_analogic.h"
#include "LmHandler.h"
#include "LoraMac.h"
#include "app.h"
#include "board.h"
#include "ds18b20.h"
#include "si7021-a20.h"

/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/

#define SIGMAIS_FRAME_COUNTER_MAX_VALUE   7
/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE         LORAMAC_HANDLER_UNCONFIRMED_MSG//LORAMAC_HANDLER_CONFIRMED_MSG


/*************************************************************************************************/
/*    VARIABLES                                                                                   */
/*************************************************************************************************/
uint8_t lora_busy_counter = 0;
/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/

uint8_t fnCOMM_SIGMAIS_Get_Frame_Counter ( void );

static bool isToReset(uint8_t thresholdCounter);
static void resetBusyCounter();
static void incBusyCounter();
static void saveHorimetro(void);
uint8_t calculateCiclo(uint32_t counter, uint32_t counter_size);

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/

uint8_t u8_sigmais_frame_counter = SIGMAIS_FRAME_COUNTER_MAX_VALUE;

pfn_comm_sigmais_downlink_frame_callback_t pfn_comm_sigmais_downlink_frame_callback = 0;

/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnCOMM_SIGMAIS_Donwlink_Frame_Set_Callback ( pfn_comm_sigmais_downlink_frame_callback_t pfn_downlink_frame_callback ) {
   
   pfn_comm_sigmais_downlink_frame_callback = pfn_downlink_frame_callback;   
   
   return;
}


/* UPLINK FRAMES */
#if 0
void fnCOMM_SIGMAIS_Send_Info_Frame_Raw_Data ( void ) {

   uint8_t au8_data[12];
   uint8_t u8_data_size = 0;

   st_system_status.b_battery_status = fnINPUT_ANALOGIC_Get_Battery ( &st_system_status.u8_battery );
   
   /* Mount packet */
   fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( EN_SIGMAIS_UPLINK_INFO_FRAME_RAW_DATA,
                                              fnCOMM_SIGMAIS_Get_Frame_Counter ( ),
                                              st_system_status.b_battery_status,
                                              st_system_status.b_detection_status,
                                              &au8_data[u8_data_size], &u8_data_size );
                                              
   fnSIGMAIS_ENCODER_Info_Frame_Raw_Data ( (uint8_t) (st_system_status.st_mag_data.i16_raw_x >> 8),
                                           (uint8_t)  st_system_status.st_mag_data.i16_raw_x,
                                           (uint8_t) (st_system_status.st_mag_data.i16_raw_y >> 8),  
                                           (uint8_t)  st_system_status.st_mag_data.i16_raw_y,
                                           (uint8_t) (st_system_status.st_mag_data.i16_raw_z >> 8),
                                           (uint8_t)  st_system_status.st_mag_data.i16_raw_z,
                                           (int8_t)   st_system_status.i8_temperature,
                                                      st_system_status.u8_battery,
                                                      fnTIMESTAMP_Get_Updated_Timestamp(),
                                                      &au8_data[u8_data_size], &u8_data_size );


   fnDEBUG_Const_String("TX = ");
   fnDEBUG_String_Size((char *)au8_data,u8_data_size);
   fnDEBUG_Const_String("\r\n");

   /* Transmit */
   /* TODO: esta função  */
   //fnRADIO_Send_Message ( au8_data, u8_data_size, false );
  if( LmHandlerIsBusy( ) == true )
   {
	   fnDEBUG_Const_String("Radio Busy\r\n");
       return;
   }

   LmHandlerAppData_t local_AppData =
   {
       .Buffer = &au8_data,
       .BufferSize = u8_data_size,
       .Port = 2
   };
   LmHandlerSend( &local_AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE );


}
#endif

#if 0
void fnCOMM_SIGMAIS_Send_Error_Frame_Raw_Data ( void ) {

   uint8_t au8_data[9];
   uint8_t u8_data_size = 0;
   
   /* Mount packet */
   fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( EN_SIGMAIS_UPLINK_ERROR_FRAME,
                                              fnCOMM_SIGMAIS_Get_Frame_Counter ( ),
                                              st_system_status.b_battery_status,
                                              st_system_status.b_detection_status,
                                              &au8_data[u8_data_size], &u8_data_size );
                                              
                                              
      fnSIGMAIS_ENCODER_Error_Frame_Raw_Data ( 0x03, 
                                             (uint8_t) (st_system_status.st_mag_data.i16_raw_x >> 8),
                                             (uint8_t)  st_system_status.st_mag_data.i16_raw_x,
                                             (uint8_t) (st_system_status.st_mag_data.i16_raw_y >> 8),
                                             (uint8_t)  st_system_status.st_mag_data.i16_raw_y,
                                             (uint8_t) (st_system_status.st_mag_data.i16_raw_z >> 8),
                                             (uint8_t)  st_system_status.st_mag_data.i16_raw_z,
                                             &au8_data[u8_data_size], &u8_data_size );                                             

      fnDEBUG_Const_String("TX = ");
      fnDEBUG_String_Size((char *)au8_data,u8_data_size);
      fnDEBUG_Const_String("\r\n");

   /* Transmit */
      /* TODO: esta função  */
   //fnRADIO_Send_Message ( au8_data, u8_data_size, false );
   if( LmHandlerIsBusy( ) == true )
   {
	    fnDEBUG_Const_String("Radio Busy\r\n");
        return;
   }

   LmHandlerAppData_t local_AppData =
   {
      .Buffer = &au8_data,
      .BufferSize = u8_data_size,
      .Port = 2
   };
   LmHandlerSend( &local_AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE );


}
#endif

#if 0
void fnCOMM_SIGMAIS_Send_Info_Frame_Parking_Processed ( void ) {

   uint8_t au8_data[6];
   uint8_t u8_data_size = 0;

   st_system_status.b_battery_status = fnINPUT_ANALOGIC_Get_Battery ( &st_system_status.u8_battery );

   /* Mount packet */
   fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( EN_SIGMAIS_UPLINK_INFO_FRAME_PARKING_PROCESSED,
                                              fnCOMM_SIGMAIS_Get_Frame_Counter ( ),
                                              st_system_status.b_battery_status,
                                              st_system_status.b_detection_status,
                                              &au8_data[u8_data_size], &u8_data_size );
  
   fnSIGMAIS_ENCODER_Info_Frame_Parking_Processed ( ( int8_t ) st_system_status.i8_temperature,
                                                    st_system_status.u8_battery,
                                                    st_system_status.u32_event_timestamp,
                                                    &au8_data[u8_data_size], &u8_data_size );

   fnDEBUG_Const_String("TX = ");
   fnDEBUG_String_Size((char *)au8_data,u8_data_size);
   fnDEBUG_Const_String("\r\n");

   /* Transmit */
   /* TODO: esta função  */
   //fnRADIO_Send_Message ( au8_data, u8_data_size, false );
   if( LmHandlerIsBusy( ) == true )
   {
	   fnDEBUG_Const_String("Radio Busy\r\n");
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
#endif


void fnCOMM_SIGMAIS_Send_Config_Report_Frame ( void ) {

   uint8_t au8_data[11];
   uint8_t u8_data_size = 0;


   st_system_status.b_battery_status = fnINPUT_ANALOGIC_Get_Battery ( &st_system_status.u8_battery );

   /* Mount packet */
   fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( EN_SIGMAIS_UPLINK_CONFIG_REPORT_FRAME,
                                              fnCOMM_SIGMAIS_Get_Frame_Counter ( ),
                                              st_system_status.b_battery_status,
                                              st_system_status.b_detection_status,
                                              &au8_data[u8_data_size], &u8_data_size );
  
   st_sigmais_working_hour_bitfield_t st_sigmais_working_hour_bitfield;
   st_sigmais_working_hour_bitfield.actual_day_starting_hour = st_system_status.u8_first_day_start_hour;
   st_sigmais_working_hour_bitfield.actual_day_finishing_hour = st_system_status.u8_first_day_end_hour;
   st_sigmais_working_hour_bitfield.next_day_starting_hour = st_system_status.u8_second_day_start_hour;
   st_sigmais_working_hour_bitfield.next_day_finishing_hour = st_system_status.u8_second_day_end_hour;
   st_sigmais_working_hour_bitfield.third_day_starting_hour = st_system_status.u8_third_day_start_hour;
   st_sigmais_working_hour_bitfield.third_day_finishing_hour = st_system_status.u8_third_day_end_hour;
  
   fnSIGMAIS_ENCODER_Config_Report_Frame ( 0, //st_system_status.u8_sensor_sensivity,
                                           st_system_status.b_data_processed ,
                                           0, //st_system_status.en_module_type,
                                           st_sigmais_working_hour_bitfield,
                                           fnTIMESTAMP_Get_Updated_Timestamp(),
                                           &au8_data[u8_data_size], &u8_data_size );
  
  fnSIGMAIS_ENCODER_Time_Byte_Bitfield ( st_system_status.st_sigmais_confirmation_time,
                                         &au8_data[u8_data_size], &u8_data_size );

  fnSIGMAIS_ENCODER_Time_Byte_Bitfield ( st_system_status.st_sigmais_keep_alive_time,
                                         &au8_data[u8_data_size], &u8_data_size );

  fnDEBUG_Const_String("TX = ");
  fnDEBUG_String_Size((char *)au8_data,u8_data_size);
  fnDEBUG_Const_String("\r\n");

   /* Transmit */
  /* TODO: esta função  */
   //fnRADIO_Send_Message ( au8_data, u8_data_size, true );
  if( LmHandlerIsBusy( ) == true )
  {
	  fnDEBUG_Const_String("Radio Busy\r\n");
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


void fnCOMM_SIGMAIS_Send_Error_Frame ( uint8_t u8_error_code, uint8_t u8_error_data ) {

   uint8_t au8_data[3];
   uint8_t u8_data_size = 0;


   st_system_status.b_battery_status = fnINPUT_ANALOGIC_Get_Battery ( &st_system_status.u8_battery );

   /* Mount packet */
   fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( EN_SIGMAIS_UPLINK_ERROR_FRAME,
                                              fnCOMM_SIGMAIS_Get_Frame_Counter ( ),
                                              st_system_status.b_battery_status,
                                              st_system_status.b_detection_status,
                                              &au8_data[u8_data_size], &u8_data_size );

   fnSIGMAIS_ENCODER_Error_Frame ( u8_error_code, u8_error_data, &au8_data[u8_data_size], &u8_data_size );
   
   fnDEBUG_Const_String("TX = ");
   fnDEBUG_String_Size((char *)au8_data,u8_data_size);
   fnDEBUG_Const_String("\r\n");

   /* Transmit */
   /* TODO: esta função  */
   //fnRADIO_Send_Message ( au8_data, u8_data_size, false );
   if( LmHandlerIsBusy( ) == true )
    {
	    fnDEBUG_Const_String("Radio Busy\r\n");
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


void fnCOMM_SIGMAIS_Send_Info_Frame_Horimetro ( void ) {

   uint8_t au8_data[12];
   uint8_t u8_data_size = 0;
   uint8_t u8_temperatura;

   st_system_status.b_battery_status = fnINPUT_ANALOGIC_Get_Battery ( &st_system_status.u8_battery );
   st_system_status.u32_timer_on = fnTIMESTAMP_Get_Horimetro( EN_USED_TIME );
   st_system_status.u32_timer_off = fnTIMESTAMP_Get_Horimetro( EN_IDLE_TIME );

   u8_temperatura = (uint8_t) st_system_status.st_mag_data.i8_temp;

   // Mount packet
   fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( EN_SIGMAIS_UPLINK_INFO_FRAME_HORIMETRO,
                                              fnCOMM_SIGMAIS_Get_Frame_Counter ( ),
                                              st_system_status.b_battery_status,
                                              st_system_status.b_detection_status,
                                              &au8_data[u8_data_size], &u8_data_size );

   fnSIGMAIS_ENCODER_Info_Frame_Horimetro ( u8_temperatura,
                                            st_system_status.u8_battery,
                                            st_system_status.u32_event_timestamp,
                                            st_system_status.u32_timer_on,
											st_system_status.u32_machine_on,
                                            &au8_data[u8_data_size], &u8_data_size );
   
   fnDEBUG_Const_String("TX = ");
   fnDEBUG_String_Size((char *)au8_data,u8_data_size);
   fnDEBUG_Const_String("\r\n");

   // Transmit
 	if( LmHandlerIsBusy( ) == true )
	{
 	   fnDEBUG_Const_String("Radio Busy\r\n");
	   incBusyCounter();
	   if ((getBusyCounter() % 5) == 0){
	      flag_rejoin = 1;
	   }
	     // save data and reset
	   if (isToReset(MAX_MSG_BUSY)){
	     BoardResetMcu();
	   }
	   return;
	}

	resetBusyCounter();
	saveHorimetro();
	LmHandlerAppData_t local_AppData =
	{
	    .Buffer = &au8_data,
	    .BufferSize = u8_data_size,
	    .Port = 2
	};
	LmHandlerSend( &local_AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE );
   
   return;
}


void fnCOMM_SIGMAIS_Send_Info_Frame_Horimetro_Raw_Data ( void ) {

	uint8_t au8_data[12];
	uint8_t u8_data_size = 0;

	st_system_status.b_battery_status = fnINPUT_ANALOGIC_Get_Battery ( &st_system_status.u8_battery );
	st_system_status.u32_timer_on = fnTIMESTAMP_Get_Horimetro( EN_USED_TIME );
	st_system_status.u32_timer_off = fnTIMESTAMP_Get_Horimetro( EN_IDLE_TIME );
	// Mount packet
	fnSIGMAIS_ENCODER_Uplink_Header_Bitfield ( EN_SIGMAIS_UPLINK_INFO_FRAME_HORIMETRO_RAW_DATA,
	fnCOMM_SIGMAIS_Get_Frame_Counter ( ),
	st_system_status.b_battery_status,
	st_system_status.b_detection_status,
	&au8_data[u8_data_size], &u8_data_size );

   fnSIGMAIS_ENCODER_Info_Frame_Horimetro_Raw_Data ( (uint8_t) (st_system_status.st_accel_data.i16_raw_x >> 8),
   (uint8_t)  st_system_status.st_accel_data.i16_raw_x,
   (uint8_t) (st_system_status.st_accel_data.i16_raw_y >> 8),
   (uint8_t)  st_system_status.st_accel_data.i16_raw_y,
   (uint8_t) (st_system_status.st_accel_data.i16_raw_z >> 8),
   (uint8_t)  st_system_status.st_accel_data.i16_raw_z,
   (uint32_t) st_system_status.u32_machine_on,
   (uint32_t) st_system_status.u32_timer_on,
   &au8_data[u8_data_size], &u8_data_size );

	
	// Transmit
    fnDEBUG_Const_String("TX = ");
    fnDEBUG_String_Size((char *)au8_data,u8_data_size);
    fnDEBUG_Const_String("\r\n");

	if( LmHandlerIsBusy( ) == true )
	{
	   fnDEBUG_Const_String("Radio Busy\r\n");
	   incBusyCounter();
       if ((getBusyCounter() % 5) == 0){
          flag_rejoin = 1;
       }
      // save data and reset
       if (isToReset(MAX_MSG_BUSY)){
          BoardResetMcu();
       }
       return;
	}

	// save data and reset counter
	resetBusyCounter();
	saveHorimetro();

	LmHandlerAppData_t local_AppData =
	{
	    .Buffer = &au8_data,
	    .BufferSize = u8_data_size,
	    .Port = 2
	};
	LmHandlerSend( &local_AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE );
	
	return;
}


void fnCOMM_SIF_Send_Horimetro_Pulsimetro ( void ) {
#define TEMP_BUFFER 3
   uint8_t au8_data[12];
   uint8_t u8_data_size = 0;
   uint8_t fciclo1, fciclo2, fciclo3, fciclo4;
   float temperatura[TEMP_BUFFER];


   st_system_status.b_battery_status = fnINPUT_ANALOGIC_Get_Battery ( &st_system_status.u8_battery );
   st_system_status.u32_timer_on = fnTIMESTAMP_Get_Horimetro( EN_USED_TIME );
   st_system_status.u32_timer_off = fnTIMESTAMP_Get_Horimetro( EN_IDLE_TIME );

   // Calcula o ciclo atual do contador
   fciclo1 = calculateCiclo(st_system_status.u32_number_of_pulsos_1, PULSE1_COUNTER_SIZE);
   fciclo2 = calculateCiclo(st_system_status.u32_number_of_pulsos_2, PULSE2_COUNTER_SIZE);
   fciclo3 = calculateCiclo(st_system_status.u32_machine_on, MACHINE_ON_COUNTER_SIZE);
   fciclo4 = calculateCiclo(st_system_status.u32_timer_on, TIMER_ON_COUNTER_SIZE);

   // Calcula os flags de estado atual
   st_system_status.fea_fmon = st_system_status.b_detection_status;
   st_system_status.fea_pulso1 = BoardGetPulse1Estado();
   st_system_status.fea_pulso2 = BoardGetPulse2Estado();

   // Captura as temperaturas
   if ( st_system_status.b_probe1_enabled){
      st_system_status.u16_temperatura1 = (uint16_t) ((getTemperatureMedian(ONE_WIRE_PORT_PIN_1,ONE_WIRE_PIN_1 ) + 56.0) * 10);
   }
   else{
	   uint16_t aux = (uint16_t) (si7021_measure_temperature(&hi2c1)/10);
	   st_system_status.u16_temperatura1 = aux+560;
	   printf("sitemp %x ", st_system_status.u16_temperatura1 );
   }

   if ( st_system_status.b_probe2_enabled){
      st_system_status.u16_temperatura2 = (uint16_t) ((getTemperatureMedian(ONE_WIRE_PORT_PIN_2,ONE_WIRE_PIN_2 ) + 56.0) * 10);
   }
   else{
	      uint16_t aux8 = (uint16_t) si7021_measure_humidity(&hi2c1);
	      st_system_status.u16_temperatura2 = (aux8+56)*10;
	      printf("siumd %x\r\n", st_system_status.u16_temperatura2 );
   }

   // Mount packet
   fnSIGMAIS_ENCODER_Uplink_Header2_Bitfield ( EN_SIGMAIS_UPLINK_INFO_FRAME_HORIMETRO_PULSIMETRO,
		                                       st_system_status.b_battery_status,
											   fciclo1,
											   fciclo2,
											   fciclo3,
											   fciclo4, //flags ciclo
                                              &au8_data[u8_data_size], &u8_data_size );


   fnSIGMAIS_ENCODER_Info_Frame_Horimetro_Pulsimetro (
		                                    st_system_status.fea_fmon,
											st_system_status.fea_pulso1,
											st_system_status.fea_pulso2,
											st_system_status.u32_number_of_pulsos_1,
											st_system_status.u32_number_of_pulsos_2,
											st_system_status.u32_machine_on,
                                            st_system_status.u16_temperatura1,
											st_system_status.u16_temperatura2,
                                            st_system_status.u32_timer_on,

                                            &au8_data[u8_data_size], &u8_data_size );

   fnDEBUG_Const_String("TX = ");
   fnDEBUG_String_Size((char *)au8_data,u8_data_size);
   fnDEBUG_Const_String("\r\n");

   // Transmit
 	if( LmHandlerIsBusy( ) == true )
	{
 	   fnDEBUG_Const_String("Radio Busy\r\n");
	   incBusyCounter();
	   if ((getBusyCounter() % 5) == 0){
	      flag_rejoin = 1;
	   }
	     // save data and reset
	   if (isToReset(MAX_MSG_BUSY)){
	     BoardResetMcu();
	   }
	   return;
	}

	resetBusyCounter();
	saveHorimetro();
	LmHandlerAppData_t local_AppData =
	{
	    .Buffer = &au8_data,
	    .BufferSize = u8_data_size,
	    .Port = 2
	};
	LmHandlerSend( &local_AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE );

   return;
}

/*************************************************************************************************/
/*    DOWNLINK FRAMES                                                                         */
/*************************************************************************************************/


void fnCOMM_SIGMAIS_Decode_Downlink_Frame ( uint8_t * pu8_data ) {
en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type;
   
   en_sigmais_downlink_frame_type = fnSIGMAIS_DECODER_Downlink( pu8_data );

   if( pfn_comm_sigmais_downlink_frame_callback != 0 ) {
      pfn_comm_sigmais_downlink_frame_callback( en_sigmais_downlink_frame_type );
   }   
   

   if (en_sigmais_downlink_frame_type == EN_SIGMAIS_DOWNLINK_DAILY_UPDATE){
      saveHorimetro();
   }
   else if (en_sigmais_downlink_frame_type == EN_SIGMAIS_DOWNLINK_CONFIG_FRAME){
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
	      fnDEBUG_Const_String("Radio Busy\r\n");
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

void fnCOMM_SIGMAIS_Send_Info_Frame ( void ) {
   
      fnCOMM_SIGMAIS_Send_Info_Frame_Horimetro ( );

   
   return;
}

/*************************************************************************************************/
/*    OTHERS
/*************************************************************************************************/
static void incBusyCounter(){
	lora_busy_counter++;
}

static void resetBusyCounter(){
	lora_busy_counter = 0;
}

// salva dados na memória se ultrapassar determinado numero de insucessos de transmissão
static bool isToReset(uint8_t thresholdCounter){
	if (lora_busy_counter > thresholdCounter){
		//save critical data and reset
		uint8_t flags = readFromEEPROM(ADD_FLAGS);
		flags &= ~0x02;

		writeByteToEEPROM(ADD_FLAGS,flags);
		writeWordToEEPROM(ADD_HORIMETRO_COUNTER,st_system_status.u32_machine_on);
		writeWordToEEPROM(ADD_HORIMETRO_TIME_ON, st_system_status.u32_timer_on);
		writeWordToEEPROM(ADD_HORIMETRO_TIME_OFF, st_system_status.u32_timer_off);
		writeWordToEEPROM(ADD_COUNTER_1, st_system_status.u32_number_of_pulsos_1);
		writeWordToEEPROM(ADD_COUNTER_2, st_system_status.u32_number_of_pulsos_2);
		return true;
	}
	return false;
}

static void saveHorimetro(void){
        // printf("write");
		//save critical data and reset
		uint8_t flags = readFromEEPROM(ADD_FLAGS);
		flags &= ~0x02;

		writeByteToEEPROM(ADD_FLAGS,flags);
		writeWordToEEPROM(ADD_HORIMETRO_COUNTER,st_system_status.u32_machine_on);
		writeWordToEEPROM(ADD_HORIMETRO_TIME_ON, st_system_status.u32_timer_on);
		writeWordToEEPROM(ADD_HORIMETRO_TIME_OFF, st_system_status.u32_timer_off);
		writeWordToEEPROM(ADD_COUNTER_1, st_system_status.u32_number_of_pulsos_1);
		writeWordToEEPROM(ADD_COUNTER_2, st_system_status.u32_number_of_pulsos_2);
        //printf("sai\r\n");
}

uint8_t getBusyCounter(void){
	return lora_busy_counter;
}



/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

uint8_t fnCOMM_SIGMAIS_Get_Frame_Counter ( void ) {
   
   u8_sigmais_frame_counter = ( u8_sigmais_frame_counter < SIGMAIS_FRAME_COUNTER_MAX_VALUE ) ? (u8_sigmais_frame_counter + 1) : 0;
   
   return u8_sigmais_frame_counter;
}


/*
 * Calcula em qual ciclo o contador está.
 * Se o quociente da disão do contador pelo seu tamanho for par  o ciclo é 0 e
 * e se for impar é 1.
 *
 * EX: contador de 11 bits, contador_size = 2048
 *     contador/2048  --> 0, 1, 2, 3, 4, 5, ..., 100
 *     ciclo          --> 0, 1, 0, 1, 0, 1, ...,  0
 *
 *     return: 0 ou 1
 * */
uint8_t calculateCiclo(uint32_t counter, uint32_t counter_size){
	uint32_t quociente = 0;

    quociente = counter/counter_size;
    return (quociente % 2);
  }
/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
