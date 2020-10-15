/*!
 * \file      app_state_machine.h
 *
 * \brief     State Machine Definitions
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

#ifndef _APP_STATE_MACHINE_H
#define _APP_STATE_MACHINE_H


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include "events.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/
#ifndef DEVEL_UDEV
#define DEVEL_UDEV 1  //parqu�metro
#endif


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


typedef enum {

   APP_STATE_INIT = 0,
   APP_STATE_CHECK_CONFIG,
   APP_STATE_RUN,
   APP_STATE_DETECTION,
   APP_STATE_CONFIGURATION,
   APP_STATE_CONFIRMING_DETECTION,
   APP_STATE_WAIT_TRANSMISSION,
   
} en_app_state_t;


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


void fnAPP_STATE_MACHINE_Init ( void );
void fnAPP_STATE_Machine ( en_event_t event );

#endif  /* _APP_STATE_MACHINE_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
