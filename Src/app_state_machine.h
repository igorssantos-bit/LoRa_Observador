#ifndef _APP_STATE_MACHINE_H
#define _APP_STATE_MACHINE_H


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include "events.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


typedef enum {
   APP_STATE_INIT = 0,
   APP_STATE_CONFIGURATION,
   APP_STATE_RUN,
   APP_STATE_BLE_TX,
   APP_STATE_BLE_RX,
   APP_STATE_SEND_BLE_DATA,
   APP_STATE_WAIT
   //APP_STATE_CHECK_CONFIG
} en_app_state_t;

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/

extern volatile bool b_state_timer_timeout;


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


void fnAPP_STATE_MACHINE_Init ( void );
void fnAPP_STATE_Machine ( en_event_t event );
en_app_state_t fnAPP_STATE_Machine_Get ( void );
void fnAPP_STATE_Machine_Set_Data ( uint16_t u16_data );

#endif  /* _APP_STATE_MACHINE_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/