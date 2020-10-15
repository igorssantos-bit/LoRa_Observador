#ifndef _EVENTS_H
#define _EVENTS_H


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


typedef enum {

   EVENT_SYSTEM_INIT,
   EVENT_SYSTEM_RTC,

   EVENT_MAG_DATA_READY,
   EVENT_MAG_THRESHOLD,

   EVENT_INACTIVITY_TRIGGER,
   EVENT_CONFIRMATION_TRIGGER,

} en_event_t;


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


#endif  /* _EVENTS_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
