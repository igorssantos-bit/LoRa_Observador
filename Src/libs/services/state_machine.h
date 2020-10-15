#ifndef _STATE_MACHINE_H
#define _STATE_MACHINE_H


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


//typedef en_app_state_t ( * pfn_current_state_t )( en_event_t event );
typedef uint8_t ( * pfn_current_state_t )( uint8_t event );
typedef void ( * pfn_on_enter_t )( void );
typedef void ( * pfn_on_exit_t )( void );


typedef struct {

   pfn_current_state_t pfn_current_state;
   pfn_on_enter_t pfn_on_enter;
   pfn_on_exit_t pfn_on_exit;

} st_state_machine_functions_t;


typedef struct _st_state_machine_desc_t {
   
   uint8_t current_state;
   uint8_t previous_state;

   st_state_machine_functions_t * pst_functions;
   
} st_state_machine_desc_t;


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


void fnSTATE_Machine_Init ( st_state_machine_desc_t * pst_desc );
void fnSTATE_Machine ( st_state_machine_desc_t * pst_desc, uint8_t event );

void fnSTATE_MACHINE_On_State_Changed( st_state_machine_desc_t * pst_desc );
void fnSTATE_MACHINE_Periodic_Transmission( st_state_machine_desc_t * pst_desc );

#endif   /* _STATE_MACHINE_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
