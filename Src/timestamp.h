#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>

#include "libs/protocols/sigmais/sigmais_protocol.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


void fnTIMESTAMP_Init ( void );
void fnTIMESTAMP_Run_Time_Counter ( void );

uint8_t fnTIMESTAMP_Get_Day_Hour ( void );
uint32_t fnTIMESTAMP_Get_Updated_Timestamp ( void );
uint32_t fnTIMESTAMP_Get_Updated_Timestamp_Seconds ( void );
void fnTIMESTAMP_Store_Timestamp ( uint32_t u32_new_timestamp );
uint32_t fnTIMESTAMP_Covert_Time_Bitfield_Into_Secs ( st_sigmais_time_byte_bitfield_t * st_time_sigmais_bitfield );


#endif /* TIMESTAMP_H_ */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/

