/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include <stddef.h>

#include "timestamp.h"
#include "rtc-board.h"

//#include "libs_firmware_config.h"


#include "events.h"
#include "app_state_machine.h"
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

uint32_t u32_time_counter_s = 0;
uint32_t u32_last_timestamp = 0;

//en_time_trigger_t en_time_trigger;
bool b_time_trigger_started = false;
uint32_t u32_time_trigger_counter = 0;
uint32_t u32_time_trigger_value = 0;

uint32_t u32_horimetro_idle_counter_s = 0;
uint32_t u32_horimetro_used_counter_s = 0;

bool b_idle_time_start = false;
bool b_used_time_start = false;


/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/
// ok
void fnTIMESTAMP_Init ( void ) {

   u32_time_counter_s = 0;
   u32_last_timestamp = st_system_status.u32_timestamp;
   
   return;  
}   

//TODO:
uint8_t fnTIMESTAMP_Get_Day_Hour ( void ) {

   float f_days_since_2017 = ( st_system_status.u32_timestamp + (u32_time_counter_s/60) ) / ( 60.0 * 24.0 );
   
   return ( f_days_since_2017 - (uint16_t) f_days_since_2017 ) * 24;
}

// ok
uint32_t fnTIMESTAMP_Get_Updated_Timestamp ( void ) {

   //uint16_t miliseconds;
   //uint32_t seconds;
   //seconds = RtcGetCalendarTime(&miliseconds);

   return st_system_status.u32_timestamp + (u32_time_counter_s/60);
   //return st_system_status.u32_timestamp + seconds;
   
}

uint32_t fnTIMESTAMP_Get_Timestamp_Counter_Seconds ( void ) {
	//uint16_t miliseconds;
	//uint32_t seconds;

	//seconds = RtcGetCalendarTime(&miliseconds);
	//return seconds;
	return u32_time_counter_s;
}



uint32_t fnTIMESTAMP_Covert_Time_Bitfield_Into_Secs ( st_sigmais_time_byte_bitfield_t * st_time_sigmais_bitfield ) {

   uint32_t u32_time_in_secs;
   
   switch ( st_time_sigmais_bitfield->time_unity ) {
      
      case EN_SIGMAIS_TIME_SECONDS: {
         
         u32_time_in_secs = st_time_sigmais_bitfield->time_value;
         
         break;
      }
      
      case EN_SIGMAIS_TIME_MINUTES:  {

         u32_time_in_secs = st_time_sigmais_bitfield->time_value * 60;

         break;
      }

      case EN_SIGMAIS_TIME_HOURS:  {

         u32_time_in_secs = st_time_sigmais_bitfield->time_value * 3600;

         break;
      }

      case EN_SIGMAIS_TIME_DAYS:  {

         u32_time_in_secs = st_time_sigmais_bitfield->time_value * 3600 * 24;
         
         break;
      }
   }
   
   return u32_time_in_secs;
}



void fnTIMESTAMP_Start_Horimetro ( en_time_horimetro_t en_time_horimetro ) {

   if( en_time_horimetro ==  EN_IDLE_TIME ) {
      b_idle_time_start = true;
      b_used_time_start = false;
   } else {
      b_idle_time_start = false;
      b_used_time_start = true;
   }

   return;
}

uint32_t fnTIMESTAMP_Get_Horimetro ( en_time_horimetro_t en_time_horimetro ) {

   if( en_time_horimetro ==  EN_IDLE_TIME ) {
      return u32_horimetro_idle_counter_s / 60 ;
   } else {
      return u32_horimetro_used_counter_s / 60 ;
   }

}


uint32_t fnTIMESTAMP_Get_Horimetro_Seconds ( en_time_horimetro_t en_time_horimetro ) {

   if( en_time_horimetro ==  EN_IDLE_TIME ) {
      return u32_horimetro_idle_counter_s ;
   } else {
      return u32_horimetro_used_counter_s ;
   }

}

/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnTIMESTAMP_Run_Time_Counter ( void ) {
	 uint16_t miliseconds;
	 uint32_t elapsedTime;
	 //  seconds = RtcGetCalendarTime(&miliseconds);

   if( u32_last_timestamp != st_system_status.u32_timestamp ) {
      u32_last_timestamp = st_system_status.u32_timestamp;
      //u32_time_counter_s = 0;
      u32_time_counter_s = RtcGetCalendarTime(&miliseconds);
      elapsedTime = 0;
   } else {
	   elapsedTime = RtcGetCalendarTime(&miliseconds) - u32_time_counter_s;
	   u32_time_counter_s += elapsedTime;
      //u32_time_counter_s++;
   }

   if( b_idle_time_start ) {
      //u32_horimetro_idle_counter_s++;
	   u32_horimetro_idle_counter_s += elapsedTime;
   }

   if( b_used_time_start ) {
      //u32_horimetro_used_counter_s++;
	   u32_horimetro_used_counter_s += elapsedTime;
   }

   if( b_time_trigger_started ) {
      //u32_time_trigger_counter++;
	   u32_time_trigger_counter += elapsedTime;
   }
   
   return;
}

/*
 * Atualiza o contador de segundos do horímetro u32_horimetro_used_counter_s
 * */
void fnTIMESTAMP_Update_Horimetro_Used_Counter(int16_t secondsToUpdate){
	u32_horimetro_used_counter_s += secondsToUpdate;
	u32_horimetro_idle_counter_s -= secondsToUpdate;
}



/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
