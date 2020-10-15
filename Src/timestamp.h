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
typedef enum {

   EN_IDLE_TIME,
   EN_USED_TIME,

} en_time_horimetro_t;


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


void fnTIMESTAMP_Init ( void );
void fnTIMESTAMP_Run_Time_Counter ( void );

uint8_t fnTIMESTAMP_Get_Day_Hour ( void );
uint32_t fnTIMESTAMP_Get_Updated_Timestamp ( void );
uint32_t fnTIMESTAMP_Get_Timestamp_Counter_Seconds ( void );
void fnTIMESTAMP_Store_Timestamp ( uint32_t u32_new_timestamp );
uint32_t fnTIMESTAMP_Covert_Time_Bitfield_Into_Secs ( st_sigmais_time_byte_bitfield_t * st_time_sigmais_bitfield );
void fnTIMESTAMP_Start_Horimetro ( en_time_horimetro_t en_time_horimetro );
uint32_t fnTIMESTAMP_Get_Horimetro ( en_time_horimetro_t en_time_horimetro );
uint32_t fnTIMESTAMP_Get_Horimetro_Seconds ( en_time_horimetro_t en_time_horimetro );
void fnTIMESTAMP_Update_Horimetro_Used_Counter(int16_t secondsToUpdate);

#endif /* TIMESTAMP_H_ */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/

