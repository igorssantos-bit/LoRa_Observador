/*!
 * \file      accelerometer.h
 *
 * \brief     LSM API definition
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

#ifndef _ACCEL_H
#define _ACCEL_H

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>
#include "lsm303agr_reg.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


typedef struct {

   int16_t i16_raw_x;
   int16_t i16_raw_y;
   int16_t i16_raw_z;
   int8_t i8_temp;
   uint8_t u8_temp;

} st_accmag_raw_data_t;
// TODO: trocar o nome st_fxos8700cq_raw_data_t para st_accmag_raw_data_t. Talvez até trocar o nome do arquivo para accmag.c e accmag.h

#ifdef _ACCEL_H
bool accmag_initialized;
#else
extern bool accmag_initialized;
#endif

/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


/*************************************************************************************************/
/*    Accelerometer and Magnetometer Functions                                                                                */
/*************************************************************************************************/
bool fnLsm303_Init(lsm303agr_ctx_t dev_ctx_xl, lsm303agr_ctx_t dev_ctx_mg );
void fnLSM303_Read_Mag_Raw_Data ( lsm303agr_ctx_t dev_ctx_mg, st_accmag_raw_data_t * st_mag_raw_data );
void fnLSM303_Read_Temp_Raw_Data ( lsm303agr_ctx_t dev_ctx_xl, st_accmag_raw_data_t * st_mag_raw_data );
bool fnLSM303_Mag_Ready ( lsm303agr_ctx_t *plocal_dev_ctx);
void fnLSM303_Read_Accel_Raw_Data ( lsm303agr_ctx_t dev_ctx_accel,  st_accmag_raw_data_t *pst_accel_data);
bool fnLSM303_Accel_Ready ( lsm303agr_ctx_t *plocal_dev_ctx);

bool is_Mag_Threshold_Event ( lsm303agr_ctx_t dev_ctx_mg);
void turnoff_Mag(lsm303agr_ctx_t dev_ctx_mg);
void turnon_Mag(lsm303agr_ctx_t dev_ctx_mg);

void fnLSM303_Accel_LPMode(lsm303agr_ctx_t dev_ctx_xl);
bool is_Accel_Event ( lsm303agr_ctx_t dev_ctx_xl );

#if defined(__cplusplus)
}
#endif /* __cplusplus */


#endif   /* _FXOS8700CQ_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
