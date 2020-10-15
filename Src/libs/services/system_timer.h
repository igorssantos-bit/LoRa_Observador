#ifndef _SYSTEM_TIMER_H
#define _SYSTEM_TIMER_H


/*************************************************************************************************/
/*    CHECK                                                                                      */
/*************************************************************************************************/


//#ifndef LIBS_FIRMWARE_CONFIG
//#error "Configure libs"
//#endif


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/

#define  SYSTEM_TICK_PERIOD_MS      1000U         /* 1U Miliseconds */
#define  timer_size_t               uint32_t
#define  MS_TO_TICKS( ms )          ((timer_size_t)( ms / SYSTEM_TICK_PERIOD_MS ))
#define  NUMBER_OF_COUNTERS         8


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


typedef struct {

   uint8_t  u8_hours;
   uint8_t  u8_minutes;
   uint8_t  u8_seconds;
   uint16_t u16_mili_seconds;

} st_timer_clock_t;


typedef enum {

   TIMER_TYPE_SINGLE,
   TIMER_TYPE_CONTINUOUS,

} en_timer_type_t;


typedef void ( * fnptr_timer_callback_t )( void * );
typedef void ( * pfn_system_tick_callback_t ) ( void );


typedef struct {

   bool b_busy;
   uint8_t u8_index;

} st_timer_index_t;


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


void fnTIMER_System_Tick_Init ( void );
void fnTIMER_System_Tick      ( void );

bool fnTIMER_Start ( st_timer_index_t * pst_timer_index, timer_size_t ticks, en_timer_type_t type, fnptr_timer_callback_t callback, void * pv_parameter );
void fnTIMER_Stop ( uint8_t u8_index );
void fnTIMER_Reload ( uint8_t u8_index );

void fnTIMER_Set_System_Tick ( timer_size_t );
timer_size_t fnTIMER_Get_System_Tick ( void );

void fnTIMER_HAL_System_Tick_Init ( void );
void fnSYSTEM_TICK_INT_Event_Callback ( void );


#endif   /* _SYSTEM_TIMER_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
