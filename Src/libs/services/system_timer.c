/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/

/*
   Software timer.
   Generate system tick.
   Generate timer callbacks. (multiple of system tick)
*/

/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//#include "libs_firmware_config.h"
#include "system_timer.h"
#include "libs/services/debug.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


typedef struct {

   bool            b_busy;
   timer_size_t    x_counter_value;
   timer_size_t    x_reload_value;
   en_timer_type_t en_timer_type;
   fnptr_timer_callback_t fnptr_timer_callback;
   st_timer_index_t * pst_timer_index;
   void * pv_parameter;

} st_timer_t;


/*************************************************************************************************/
/*    PROTOTYPES                                                                                 */
/*************************************************************************************************/


void fnTIMER_Update_Clock     ( void );
void fnTIMER_Update_Counters  ( void );


/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/


timer_size_t x_system_tick_counter;

st_timer_t st_timer[NUMBER_OF_COUNTERS];


/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*-----------------------------------------------------------------------------------------------*/
/*!
 *    \brief         Init System Tick.
 */
/*-----------------------------------------------------------------------------------------------*/

void fnTIMER_System_Tick_Init ( void ) {

   uint8_t i;
     
/* Clear tick counter */
   x_system_tick_counter = 0;
   
/* Clear timer struct */
   for ( i = 0; i < NUMBER_OF_COUNTERS; i++ ) {
      fnTIMER_Stop(i);
   }

/* Config System Tick */
   //fnTIMER_HAL_System_Tick_Init();

   return;

}


/*-----------------------------------------------------------------------------------------------*/
/*!
 *    \brief         This function updates general timers / counters / clock.
 */
/*-----------------------------------------------------------------------------------------------*/

void fnTIMER_System_Tick ( void ) {

/* Update System Tick Counter */
   x_system_tick_counter++;

/* Update Timers */
   fnTIMER_Update_Counters();
   //if ((x_system_tick_counter %5000) == 0)
   //   fnDEBUG_Const_String("timer ");
   return;

}


/*-----------------------------------------------------------------------------------------------*/
/*!
 *    \brief         .
 */
/*-----------------------------------------------------------------------------------------------*/

bool fnTIMER_Start ( st_timer_index_t * pst_timer_index, timer_size_t ticks, en_timer_type_t type, fnptr_timer_callback_t callback, void * pv_parameter ) {

   uint8_t i;

   for ( i = 0; i < NUMBER_OF_COUNTERS; i++ ) {

      if ( st_timer[i].b_busy == false ) {
         break;  
      }

   }

   if ( i >= NUMBER_OF_COUNTERS ) {
      return -1;
   }

   if ( pst_timer_index ) {
      pst_timer_index->b_busy = true;
      pst_timer_index->u8_index = i;
      st_timer[i].pst_timer_index = pst_timer_index;
   }

   st_timer[i].b_busy = true;
   st_timer[i].x_counter_value = ticks;
   st_timer[i].x_reload_value = ticks;
   st_timer[i].en_timer_type = type;
   st_timer[i].fnptr_timer_callback = callback;
   st_timer[i].pv_parameter = pv_parameter;
   
   return i;  
   
}


/*-----------------------------------------------------------------------------------------------*/
/*!
 *    \brief         .
 */
/*-----------------------------------------------------------------------------------------------*/

void fnTIMER_Stop ( uint8_t u8_index ) {
 
   if( u8_index >= NUMBER_OF_COUNTERS ) {
      return;
   }

   if ( st_timer[ u8_index ].pst_timer_index ) {
      st_timer[ u8_index ].pst_timer_index->b_busy = false;
      st_timer[ u8_index ].pst_timer_index->u8_index = 0xFF;
      st_timer[ u8_index ].pst_timer_index = NULL;
   }

   st_timer[ u8_index ].b_busy = false;
 
   return;

}


/*-----------------------------------------------------------------------------------------------*/
/*!
 *    \brief         TIMER Restart.
 */
/*-----------------------------------------------------------------------------------------------*/

void fnTIMER_Reload ( uint8_t u8_index ) {

   st_timer[u8_index].x_counter_value = st_timer[u8_index].x_reload_value;

   return;

}


/*-----------------------------------------------------------------------------------------------*/
/*!
 *    \brief         This function reads the system tick counter.
 *
 *    \return        u32_system_tick_counter.
 */
/*-----------------------------------------------------------------------------------------------*/

void fnTIMER_Set_System_Tick ( timer_size_t value ) {
   
   x_system_tick_counter = value;
   
   return;
   
}


/*-----------------------------------------------------------------------------------------------*/
/*!
 *    \brief         This function reads the system tick counter.
 *
 *    \return        u32_system_tick_counter.
 */
/*-----------------------------------------------------------------------------------------------*/

timer_size_t fnTIMER_Get_System_Tick ( void ) {

   return x_system_tick_counter;

}


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*-----------------------------------------------------------------------------------------------*/
/*!
 *    \brief         This function updates system counters.
 */
/*-----------------------------------------------------------------------------------------------*/

void fnTIMER_Update_Counters ( void ) {

   uint8_t i;

   for ( i = 0; i < NUMBER_OF_COUNTERS; i++ ) {

      if ( st_timer[i].b_busy == true ) {

         if ( st_timer[i].x_counter_value ) {
            st_timer[i].x_counter_value--;
         }
         if ( st_timer[i].x_counter_value == 0 ) {

            if ( st_timer[i].en_timer_type == TIMER_TYPE_CONTINUOUS ) {
               st_timer[i].x_counter_value = st_timer[i].x_reload_value;
            }
            else {
               fnTIMER_Stop(i);
            }

            if ( st_timer[i].fnptr_timer_callback != NULL ) {
               st_timer[i].fnptr_timer_callback( st_timer[i].pv_parameter );
            }
         }
      }
   }
 
   return;

}


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
