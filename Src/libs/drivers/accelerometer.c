/*!
 * \file      fxos8700cq.c
 *
 * \brief     Driver to work with the accelerometer
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



/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <accelerometer.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "system_status.h"
#include "lsm303agr_reg.h"
#include "stm32l0xx_hal.h"



/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/



/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/
extern I2C_HandleTypeDef hi2c1;

uint16_t u16_strong_threshold[8] = {
   0x01ff, //0x03E8,    //1000
   0x0622,    //1570
   0x0933,    //2355
   0x1244,    //4676
   0x1555,    //5461
   0x1866,    //6246
   0x2177,    //8567
   0x2488,    //9352
};

// profile 1 - hybrid, ODR: 25

/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

bool fnLsm303_Init(lsm303agr_ctx_t dev_ctx_xl, lsm303agr_ctx_t dev_ctx_mg){

	accmag_initialized = false;
	/*
	 *  Check device ID
	 */
	uint8_t whoamI = 0;
	lsm303agr_xl_device_id_get(&dev_ctx_xl, &whoamI);
	if ( whoamI != LSM303AGR_ID_XL ){
		return false;   //while(1); /*manage here device not found */
	}


	whoamI = 0;
	lsm303agr_mag_device_id_get(&dev_ctx_mg, &whoamI);
	if ( whoamI != LSM303AGR_ID_MG ){
		return false;  //while(1); /*manage here device not found */
	}


	/*
	 *  Restore default configuration for magnetometer
	 */
	uint8_t rst = 0;
	lsm303agr_mag_reset_set(&dev_ctx_mg, PROPERTY_ENABLE);
	do {
		lsm303agr_mag_reset_get(&dev_ctx_mg, &rst);
	} while (rst);

	/*
	 *  Enable Block Data Update
	 */
	lsm303agr_xl_block_data_update_set(&dev_ctx_xl, PROPERTY_ENABLE);
	lsm303agr_mag_block_data_update_set(&dev_ctx_mg, PROPERTY_ENABLE);

	/*
	 * Set Output Data Rate
	 */
	lsm303agr_xl_data_rate_set(&dev_ctx_xl, LSM303AGR_XL_POWER_DOWN);
	lsm303agr_mag_data_rate_set(&dev_ctx_mg, LSM303AGR_MG_ODR_10Hz);

	/*
	 * Set accelerometer full scale
	 */
	lsm303agr_xl_full_scale_set(&dev_ctx_xl, LSM303AGR_2g);

	/*
	 * Set / Reset magnetic sensor mode
	 */
	lsm303agr_mag_set_rst_mode_set(&dev_ctx_mg, LSM303AGR_SENS_OFF_CANC_EVERY_ODR);

	/*
	 * Enable temperature compensation on mag sensor
	 */
	lsm303agr_mag_offset_temp_comp_set(&dev_ctx_mg, PROPERTY_ENABLE);

	/*
	 * Enable temperature sensor
	 */
	lsm303agr_temperature_meas_set(&dev_ctx_xl, LSM303AGR_TEMP_DISABLE);

	/*
	 * Set device in continuos mode
	 */
	lsm303agr_xl_operating_mode_set(&dev_ctx_xl, LSM303AGR_HR_12bit);


	/*
	 * Set magnetometer in continuos mode
	 */
	lsm303agr_mag_operating_mode_set(&dev_ctx_mg, LSM303AGR_POWER_DOWN);

	/*
	 * Set magnetometer in low power mode:
	 */
	lsm303agr_mag_power_mode_set(&dev_ctx_mg, LSM303AGR_LOW_POWER);
	/*
	 * Set magnetometer threshold
	 */
	lsm303agr_mag_int_gen_treshold_set(&dev_ctx_mg, &u16_strong_threshold[st_system_status.u8_strong_mag_sensivity] );

	uint16_t au16;
	lsm303agr_mag_int_gen_treshold_get(&dev_ctx_mg, &au16);

	if (au16 != u16_strong_threshold[st_system_status.u8_strong_mag_sensivity]){
		lsm303agr_mag_int_gen_treshold_get(&dev_ctx_mg, &au16);
	}
	/*
	 * Disable DRDY and Enable Threshold interrupt event
	 */
	lsm303agr_mag_drdy_on_pin_set(&dev_ctx_mg, 0);
	lsm303agr_mag_int_on_pin_set(&dev_ctx_mg,1);

	/*
	 * Read/Clear generator configuration register
	 */
	uint8_t res;
	lsm303agr_mag_int_gen_conf_get(&dev_ctx_mg,&res);

	/*
	 * Enable interrupt event
	 */
	lsm303agr_int_crtl_reg_m_t crtl_reg_bits_conf = {
			.ien = 1,
			.iel = 0,                            //pulsado
			.iea = 1,
			.not_used_01 = 0,
			.zien = 1,
			.yien = 1,
			.xien = 1,
	};
	lsm303agr_mag_int_gen_conf_set(&dev_ctx_mg,&crtl_reg_bits_conf);

	accmag_initialized = true;
	return true;
}


bool fnLSM303_Mag_Ready ( lsm303agr_ctx_t *plocal_dev_ctx) {
    lsm303agr_reg_t reg;

    lsm303agr_mag_status_get(plocal_dev_ctx, &reg.status_reg_m);
    if (reg.status_reg_m.zyxda)
    {
    	return true;
    }
    else{
    	return false;
    }
}


bool fnLSM303_Accel_Ready ( lsm303agr_ctx_t *plocal_dev_ctx) {
    lsm303agr_reg_t reg;

    lsm303agr_xl_data_ready_get(plocal_dev_ctx, &reg.status_reg_a);
    if (reg.status_reg_a.zyxda  || reg.status_reg_a.xda)
    {
    	return true;
    }
    else{
    	return false;
    }
}


void fnLSM303_Read_Accel_Raw_Data ( lsm303agr_ctx_t dev_ctx_accel,  st_accmag_raw_data_t *pst_accel_data) {
    lsm303agr_reg_t reg;
    volatile axis3bit16_t data_raw_acceleration;

    lsm303agr_xl_data_ready_get(&dev_ctx_accel, &reg.status_reg_a);
    if (reg.status_reg_a.zyxda || reg.status_reg_a.xda)
    {

	// Read magnetic field data
       memset(data_raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
       lsm303agr_acceleration_raw_get(&dev_ctx_accel, data_raw_acceleration.u8bit);
       pst_accel_data->i16_raw_x =  data_raw_acceleration.i16bit[0];
       pst_accel_data->i16_raw_y = data_raw_acceleration.i16bit[1];
       pst_accel_data->i16_raw_z = data_raw_acceleration.i16bit[2];
    }

}


void fnLSM303_Read_Mag_Raw_Data ( lsm303agr_ctx_t dev_ctx_xl, st_accmag_raw_data_t * st_mag_raw_data ) {
    lsm303agr_reg_t reg;
    volatile axis3bit16_t data_raw_magnetic;

    lsm303agr_mag_status_get(&dev_ctx_xl, &reg.status_reg_m);
    if (reg.status_reg_a.zyxda)
    {
	/* Read magnetic field data */
       memset(data_raw_magnetic.u8bit, 0x00, 3*sizeof(int16_t));
       lsm303agr_magnetic_raw_get(&dev_ctx_xl, data_raw_magnetic.u8bit);
       st_mag_raw_data->i16_raw_x =  data_raw_magnetic.i16bit[0];
       st_mag_raw_data->i16_raw_y = data_raw_magnetic.i16bit[1];
       st_mag_raw_data->i16_raw_z = data_raw_magnetic.i16bit[2];

       st_system_status.st_mag_data.i16_raw_x = st_mag_raw_data->i16_raw_x;
       st_system_status.st_mag_data.i16_raw_y = st_mag_raw_data->i16_raw_y;
       st_system_status.st_mag_data.i16_raw_z = st_mag_raw_data->i16_raw_z;
    }

}

void fnLSM303_Read_Temp_Raw_Data ( lsm303agr_ctx_t dev_ctx_xl, st_accmag_raw_data_t * st_mag_raw_data ) {
   lsm303agr_reg_t reg;
   axis1bit16_t data_raw_temperature;

   lsm303agr_temp_data_ready_get(&dev_ctx_xl, &reg.byte);
   if (reg.byte)
   {
	/* Read temperature data */
      memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
      int8_t au8;
      //lsm303agr_read_reg(&dev_ctx_xl, LSM303AGR_OUT_TEMP_L_A, &au8, 1);
      //lsm303agr_read_reg(&dev_ctx_xl, LSM303AGR_OUT_TEMP_H_A, &au8, 1);
      lsm303agr_temperature_raw_get(&dev_ctx_xl, data_raw_temperature.u8bit);
      au8 = data_raw_temperature.u8bit[1];
      st_mag_raw_data->i8_temp = au8 + 23;
      //st_system_status.st_mag_data.u8_temp = au8 + 20; // + 25;
   }

}


void fnLSM303_Accel_LPMode(lsm303agr_ctx_t dev_ctx_xl){

	/* THS = 5 * 32 mg = 160 mg */
	lsm303agr_act_threshold_set(&dev_ctx_xl,0x05);

	/* DUR = ( 8 * 2 + 1 ) / ODR =  */
	lsm303agr_act_timeout_set(&dev_ctx_xl,0x03);

	// CTRL_REG6_A: Click
	lsm303agr_ctrl_reg6_a_t ctrl_reg6_bits_conf = {
			.i2_clicken = 0,
			.i2_int1 = 0,
			.i2_int2 = 0,
			.boot_i2 = 0,
			.p2_act = 1,
			.not_used_02 = 0,
			.h_lactive = 0,
			.not_used_01 = 0
	};
	lsm303agr_xl_pin_int2_config_set(&dev_ctx_xl, &ctrl_reg6_bits_conf);

}

bool is_Accel_Event ( lsm303agr_ctx_t dev_ctx_xl ){

	lsm303agr_reg_t reg;

	lsm303agr_xl_int1_gen_source_get(&dev_ctx_xl, &reg.int1_src_a);

	if( reg.int1_src_a.ia ){
		/*
		printf("xh: %u, xl: %u, yh: %u, yl: %u, zh: %u, zl: %u\r\n",
				reg.int1_src_a.xh, reg.int1_src_a.xl, reg.int1_src_a.yh, reg.int1_src_a.yl,
				reg.int1_src_a.zh, reg.int1_src_a.zl);
				*/
		return true;
	}

	return false;

}

/**
  * @brief  get true when an magnetometer threshold event is detected in any axys
  *
  * @param  ctx    Read / write interface definitions.(ptr)
  * @retval        return true -> Event Detected
  *
  */
bool is_Mag_Threshold_Event ( lsm303agr_ctx_t dev_ctx_mg) {
   lsm303agr_reg_t reg;

   lsm303agr_mag_int_gen_source_get(&dev_ctx_mg, &reg.byte);
   if (reg.byte)
   {
	   return true;  // one or more source found
   }
   else{
	   return false; // erro
   }

}

/**
  * @brief  Put the magnetometer in low power mode
  *
  * @param  ctx    Read / write interface definitions.(ptr)
  * @retval        none
  *
  */
void turnoff_Mag(lsm303agr_ctx_t dev_ctx_mg){
	/*
	 * Set magnetometer mode to low power
	 */
	lsm303agr_mag_operating_mode_set(&dev_ctx_mg, LSM303AGR_POWER_DOWN);
}

/**
  * @brief  Put the magnetometer in continuous mode
  *
  * @param  ctx    Read / write interface definitions.(ptr)
  * @retval        none
  *
  */
void turnon_Mag(lsm303agr_ctx_t dev_ctx_mg){
	/*
	 * Set magnetometer mode to continous
	 */
	lsm303agr_mag_operating_mode_set(&dev_ctx_mg, LSM303AGR_CONTINUOUS_MODE);
}
/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
