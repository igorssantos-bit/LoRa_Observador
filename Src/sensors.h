#ifndef SENSORS_H_
#define SENSORS_H_


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <accelerometer.h>
#include <stdint.h>
#include <stdbool.h>


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
void fnSENSORS_Config ( void );
//void fnSENSORS_Mag_Threshold ( void );
//void fnSENSORS_Replace ( void );
//void fnSENSORS_Mag_Data_Ready ( st_accmag_raw_data_t * pst_accel_raw_data );
void fnSENSORS_Accel_Data_Ready ( st_accmag_raw_data_t * pst_accel_data);

//void fnSENSORS_Accel_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_accel_data, st_fxos8700cq_raw_data_t * pst_mag_data ) ;
//void fnSENSORS_Set_Accel_Sensivity ( uint8_t u8_new_horimetro_sensivity );
bool fnSENSORS_Has_Detection ( void );
bool fnSENSORS_Check_Magnetometer_Valid_Threshold ( void );
bool fnSENSORS_Is_Mag_Calibrating ( void );


#endif /* SENSORS_H_ */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/



