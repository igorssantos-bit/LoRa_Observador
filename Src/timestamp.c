/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include <stddef.h>

#include "timestamp.h"

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


/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnTIMESTAMP_Init ( void ) {

   u32_time_counter_s = 0;
   u32_last_timestamp = st_system_status.u32_timestamp;
   
   return;  
}   


uint8_t fnTIMESTAMP_Get_Day_Hour ( void ) {

   float f_days_since_2017 = ( st_system_status.u32_timestamp + (u32_time_counter_s/60) ) / ( 60.0 * 24.0 );
   
   return ( f_days_since_2017 - (uint16_t) f_days_since_2017 ) * 24;
}

uint32_t fnTIMESTAMP_Get_Updated_Timestamp ( void ) {

   return st_system_status.u32_timestamp + (u32_time_counter_s/60);
   
}

//retorna o valor em segundos
uint32_t fnTIMESTAMP_Get_Updated_Timestamp_Seconds ( void ) {
	uint32_t valorAjuste;

   valorAjuste = u32_time_counter_s / 64; //RTC period = 16
   //return st_system_status.u32_timestamp + (u32_time_counter_s/60);
   return st_system_status.u32_timestamp + (valorAjuste);  //converte para segundos

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



/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnTIMESTAMP_Run_Time_Counter ( void ) {
   
   if( u32_last_timestamp != st_system_status.u32_timestamp ) {
      u32_last_timestamp = st_system_status.u32_timestamp;
      u32_time_counter_s = 0;
   } else {
      u32_time_counter_s++;   
   }
   
   return;
}



/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
