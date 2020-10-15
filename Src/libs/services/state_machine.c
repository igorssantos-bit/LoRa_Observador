/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/

/*
   State machine control
*/

/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "state_machine.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/


void fnSTATE_Machine_Update ( st_state_machine_desc_t * pst_desc, uint8_t next_state );


/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/


/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


void fnSTATE_Machine_Init ( st_state_machine_desc_t * pst_desc ) {

/* Set initial state */
   pst_desc->current_state  = 0;
   pst_desc->previous_state = 0;

/* Execute enter function */
   if ( pst_desc->pst_functions[pst_desc->current_state].pfn_on_enter != NULL ) {
      pst_desc->pst_functions[pst_desc->current_state].pfn_on_enter();
   }

   return;

}


void fnSTATE_Machine ( st_state_machine_desc_t * pst_desc, uint8_t event ) {

   uint8_t state;

   state = pst_desc->pst_functions[ pst_desc->current_state ].pfn_current_state( event );
   
   while ( state != pst_desc->current_state ) {

      fnSTATE_Machine_Update( pst_desc, state );
      event = 0;
      state = pst_desc->pst_functions[ pst_desc->current_state ].pfn_current_state( event );

   }
   
   return;

}


void fnSTATE_Machine_Update ( st_state_machine_desc_t * pst_desc, uint8_t next_state ) {

/* Execute exit function */
   if( pst_desc->pst_functions[ pst_desc->current_state ].pfn_on_exit != NULL ) {
      pst_desc->pst_functions[ pst_desc->current_state ].pfn_on_exit( );
   }

/* Update state */
   pst_desc->previous_state = pst_desc->current_state;
   pst_desc->current_state = next_state;

/* Execute enter function */
   if( pst_desc->pst_functions[ pst_desc->current_state ].pfn_on_enter != NULL ) {
      pst_desc->pst_functions[ pst_desc->current_state ].pfn_on_enter( );
   }
   
   fnSTATE_MACHINE_On_State_Changed( pst_desc );
   fnSTATE_MACHINE_Periodic_Transmission(pst_desc);

   return;

}


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

__attribute__ ((weak)) void fnSTATE_MACHINE_On_State_Changed( st_state_machine_desc_t * pst_desc )  {
   
   return;
}

/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
