#ifndef SENSORS_H_
#define SENSORS_H_


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>

#include "libs/drivers/fxos8700cq.h"

/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/

void fnSENSORS_Init ( void );
void fnSENSORS_Check_Data ( void );

void fnSENSORS_Replace ( void );
void fnSENSORS_Mag_Threshold ( void );
void fnSENSORS_Config ( void );
void fnSENSORS_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_accel_raw_data );
void fnSENSORS_Accel_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_accel_data, st_fxos8700cq_raw_data_t * pst_mag_data ) ;
void fnSENSORS_Set_Accel_Sensivity ( uint8_t u8_new_horimetro_sensivity );
bool fnSENSORS_Has_Detection ( void );
bool fnSENSORS_Check_Magnetometer_Valid_Threshold ( void );
bool fnSENSORS_Is_Mag_Calibrating ( void );
bool fnSENSORS_Confirm_Calibration_Detection_Status ( void ) ;
void fnSENSORS_b_Mag_Detected_Set ( bool status );
bool fnSENSORS_Has_Detection_Stopped ( void );

#endif /* SENSORS_H_ */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/



