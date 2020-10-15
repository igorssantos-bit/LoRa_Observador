/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//#include "libs_config.h"
#include "libs/drivers/fxos8700cq.h"
//#include "libs/drivers/fxos8700cq_port.h"
#include "system_status.h"


#include "lsm303agr_reg.h"
//#include <string.h>
#include <stdio.h>
#include <math.h>


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/

void fnFXOS8700CQ_Hard_Reset ( void );
static int32_t accel_write(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);
static int32_t accel_read(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);
static int32_t magnet_write(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);
static int32_t magnet_read(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/

#define FXOS8700CQ_NUMBER_OF_PARAMETERS 29
#define FXOS8700CQ_NUMBER_OF_PROFILES 2


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

/*const uint8_t au8_fxos8700cq_profile_config[FXOS8700CQ_NUMBER_OF_PARAMETERS][FXOS8700CQ_NUMBER_OF_PROFILES + 1] = {
//
///              REG       PROFILE:  0,    1,
   // 25Hz ODR Hybrid mode
   { FXOS8700CQ_REG_CTRL_REG1,      0xF8, 0xB0 },
      //{ FXOS8700CQ_REG_CTRL_REG1,      0xF8, 0x18 },
         
   // Debounce counter
   { FXOS8700CQ_REG_A_VECM_CNT,     0x00, 0x00 },
   
   // Acc-cell Config
   { FXOS8700CQ_REG_XYZ_DATA_CFG,   0x00, 0x00 },
   
   // M-cell Config
   { FXOS8700CQ_REG_M_CTRL_REG1,    0x59, 0x5B }, // hybrid mode
   { FXOS8700CQ_REG_M_CTRL_REG2,    0x30, 0x30 }, 
   { FXOS8700CQ_REG_M_CTRL_REG3,    0x80, 0x80 }, 

   // Sleep mode + wake mode config + interrupt
   { FXOS8700CQ_REG_CTRL_REG2,      0x05, 0x05 }, 
   { FXOS8700CQ_REG_CTRL_REG3,      0x00, 0x00 }, 
   { FXOS8700CQ_REG_CTRL_REG4,      0x01, 0x01 },
   { FXOS8700CQ_REG_CTRL_REG5,      0x03, 0x03 },  

   { FXOS8700CQ_REG_ASLP_COUNT,     0x00, 0x00 },  
   { FXOS8700CQ_REG_TRIG_CFG,       0x00, 0x02 },  

   // Accel Vector Magnitude config
   { FXOS8700CQ_REG_A_VECM_CFG,      0x00, 0x00 },
   { FXOS8700CQ_REG_A_VECM_THS_MSB,  0x00, 0x02 },
   { FXOS8700CQ_REG_A_VECM_THS_LSB,  0x00, 0x40 },
      
   // Magnetic Threshold Configuration
   { FXOS8700CQ_REG_M_THS_COUNT,     0x02, 0x02 },  
      
   { FXOS8700CQ_REG_M_THS_CFG,       0xFE, 0xFE },
      
   // Hardcoded Hard-Iron Calibration
   { FXOS8700CQ_REG_MAX_X_MSB,       0x02, 0x02 },  
   { FXOS8700CQ_REG_MAX_X_LSB,       0xA3, 0xA3 },  
   { FXOS8700CQ_REG_MAX_Y_MSB,       0x00, 0x00 },  
   { FXOS8700CQ_REG_MAX_Y_LSB,       0xCF, 0xCF },  
   { FXOS8700CQ_REG_MAX_Z_MSB,       0x03, 0x03 },  
   { FXOS8700CQ_REG_MAX_Z_LSB,       0x3E, 0x3E },  
   { FXOS8700CQ_REG_MIN_X_MSB,       0x00, 0x00 },  
   { FXOS8700CQ_REG_MIN_X_LSB,       0x00, 0x00 },  
   { FXOS8700CQ_REG_MIN_Y_MSB,       0x00, 0x00 },  
   { FXOS8700CQ_REG_MIN_Y_LSB,       0x00, 0x00 },  
   { FXOS8700CQ_REG_MIN_Z_MSB,       0x00, 0x00 },  
   { FXOS8700CQ_REG_MIN_Z_LSB,       0x00, 0x00 },       
            
      
};
*/

// Parking config
// profile 0 - only mag, magnetometer int1 data ready, int2 threshold detection, hard-iron enabled, ODR 1.58Hz
//           - threshold for magnetic data above 255 for 2 cycles ( 1,2 secs at 1.56Hz )


// profile 1 - hybrid, ODR: 25

/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

bool fnLsm303_Init(lsm303agr_ctx_t dev_ctx_xl, lsm303agr_ctx_t dev_ctx_mg ){
    // accelerometer
	/*
    lsm303agr_ctx_t dev_ctx_xl;
    dev_ctx_xl.write_reg = accel_write;
    dev_ctx_xl.read_reg = accel_read;
    dev_ctx_xl.handle = (void*)LSM303AGR_I2C_ADD_XL;

    // magnetometer
    lsm303agr_ctx_t dev_ctx_mg;
	dev_ctx_mg.write_reg = magnet_write;
	dev_ctx_mg.read_reg = magnet_read;
    dev_ctx_mg.handle = (void*)LSM303AGR_I2C_ADD_MG;
*/
        /*
	     *  Check device ID
	     */
    uint8_t whoamI = 0;
    lsm303agr_xl_device_id_get(&dev_ctx_xl, &whoamI);
    if ( whoamI != LSM303AGR_ID_XL )
       while(1); /*manage here device not found */

    whoamI = 0;
    lsm303agr_mag_device_id_get(&dev_ctx_mg, &whoamI);
    if ( whoamI != LSM303AGR_ID_MG )
       while(1); /*manage here device not found */

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
    lsm303agr_xl_data_rate_set(&dev_ctx_xl, LSM303AGR_XL_ODR_10Hz);
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
    lsm303agr_temperature_meas_set(&dev_ctx_xl, LSM303AGR_TEMP_ENABLE);
	    /*
	     * Set device in continuos mode
	     */
    lsm303agr_xl_operating_mode_set(&dev_ctx_xl, LSM303AGR_HR_12bit);
	    /*
	     * Set magnetometer in continuos mode
	     */
    lsm303agr_mag_operating_mode_set(&dev_ctx_mg, LSM303AGR_CONTINUOUS_MODE);
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
    return true;
}

bool fnFXOS8700CQ_Init ( bool b_toggle_rst_line ) {

	/* TODO: ESTA FUNCAO */
/* Toggle RST Line */
	/*
   if ( b_toggle_rst_line == true ) {
      fnFXOS8700CQ_Hard_Reset();
   }

   fnFXOS8700CQ_PORT_Init_I2C();
*/

/* Read who i am register - Check reading error */
	/*
   if ( fnFXOS8700CQ_Read_Register( FXOS8700CQ_REG_WHO_AM_I ) != FXOS8700CQ_DEVICE_ID ) {
      return false;
   }
*/
/* Set standby mode */
  // fnFXOS8700CQ_Disable();

   return true;
}


void fnFXOS8700CQ_Enable_Interrupts ( void ) {
/* TODO: ESTA FUNCAO */
   //fnFXOS8700CQ_PORT_Enable_Interrupts_1();

   return;
}


void fnFXOS8700CQ_Disable_Interrupts ( void ) {
	/* TODO: ESTA FUNCAO */
  // fnFXOS8700CQ_PORT_Disable_Interrupts_1();

   return;
}


void fnFXOS8700CQ_Config ( uint8_t u8_profile ) {
	/* TODO: ESTA FUNCAO */
   //uint8_t i;

/* Make sure standby */

   //fnFXOS8700CQ_Disable();

/* Send commands */
   /*
   for ( i = 0; i < FXOS8700CQ_NUMBER_OF_PARAMETERS; i++ ) {

     fnFXOS8700CQ_Write_Register( au8_fxos8700cq_profile_config[i][0],
                                  au8_fxos8700cq_profile_config[i][1+u8_profile] );

   }
*/
   return;
}


void fnFXOS8700CQ_Enable ( void ) {
	/* TODO: ESTA FUNCAO */
  // uint8_t u8_fxos_reg;

//   u8_fxos_reg = fnFXOS8700CQ_Read_Register( FXOS8700CQ_REG_CTRL_REG1 ) | 0x01;

/* Send command */
//   fnFXOS8700CQ_Write_Register ( FXOS8700CQ_REG_CTRL_REG1, u8_fxos_reg );

   return;
}


void fnFXOS8700CQ_Disable ( void ) {
	/* TODO: ESTA FUNCAO */

   //uint8_t u8_fxos_reg;

   //u8_fxos_reg = fnFXOS8700CQ_Read_Register( FXOS8700CQ_REG_CTRL_REG1 ) & ~0x01;

/* Send command */
   //fnFXOS8700CQ_Write_Register ( FXOS8700CQ_REG_CTRL_REG1, u8_fxos_reg );

   return;
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

/*
void fnLSM303_Read_Acel_Raw_Data ( lsm303agr_ctx_t dev_ctx_mg,  st_fxos8700cq_raw_data_t *pst_accel_data) {
    lsm303agr_reg_t reg;
    volatile axis3bit16_t data_raw_magnetic;

    lsm303agr_mag_status_get(&dev_ctx_mg, &reg.status_reg_m);
    if (reg.status_reg_m.zyxda)
    {

	// Read magnetic field data
       memset(data_raw_magnetic.u8bit, 0x00, 3*sizeof(int16_t));
       lsm303agr_magnetic_raw_get(&dev_ctx_mg, data_raw_magnetic.u8bit);
       pst_accel_data->i16_raw_x =  data_raw_acceleration.i16bit[0];
       pst_accel_data->i16_raw_y = data_raw_acceleration.i16bit[1];
       pst_accel_data->i16_raw_z = data_raw_acceleration.i16bit[2];
    }

}
*/

void fnLSM303_Read_Mag_Raw_Data ( lsm303agr_ctx_t dev_ctx_xl, st_fxos8700cq_raw_data_t * st_mag_raw_data ) {
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

void fnLSM303_Read_Temp_Raw_Data ( lsm303agr_ctx_t dev_ctx_xl, st_fxos8700cq_raw_data_t * st_mag_raw_data ) {
   lsm303agr_reg_t reg;
   axis1bit16_t data_raw_temperature;

   lsm303agr_temp_data_ready_get(&dev_ctx_xl, &reg.byte);
   if (reg.byte)
   {
	/* Read temperature data */
      memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
      uint8_t au8;
      //lsm303agr_read_reg(&dev_ctx_xl, LSM303AGR_OUT_TEMP_L_A, &au8, 1);
      //lsm303agr_read_reg(&dev_ctx_xl, LSM303AGR_OUT_TEMP_H_A, &au8, 1);
      lsm303agr_temperature_raw_get(&dev_ctx_xl, data_raw_temperature.u8bit);
      au8 = data_raw_temperature.u8bit[1];
      st_mag_raw_data->u8_temp = au8 + 23;
      //st_system_status.st_mag_data.u8_temp = au8 + 20; // + 25;
   }

}


bool fnLSM303_Read_Threshold_Source ( lsm303agr_ctx_t dev_ctx_mg) {
   lsm303agr_reg_t reg;
   bool ret;

   ret = lsm303agr_mag_int_gen_source_get(&dev_ctx_mg, &reg.byte);
   if (ret == 1)
   {
	   return true;  // one or more source found
   }
   else{
	   return false; // erro
   }

}

void fnFXOS8700CQ_Read_Accel_Mag_Raw_Data ( st_fxos8700cq_raw_data_t * pst_accel_data, st_fxos8700cq_raw_data_t * pst_mag_data ) {
	/* TODO: ESTA FUNCAO */
//   Note that this function assumes that the hyb_autoinc_mode bit has been set to enable the
//   reading of all accelerometer and magnetometer data in a single-burst, read operation

   //uint8_t au8_fxos8700cq_read_buffer[13];

/* Read data */
   //fnFXOS8700CQ_Read_Register_Array ( FXOS8700CQ_REG_STATUS, au8_fxos8700cq_read_buffer, 13 );

/* Check status */

/* Convert */
	/*
   pst_accel_data->i16_raw_x = (int16_t)( (au8_fxos8700cq_read_buffer[1] << 8) | au8_fxos8700cq_read_buffer[2] ) >> 2;
   pst_accel_data->i16_raw_y = (int16_t)( (au8_fxos8700cq_read_buffer[3] << 8) | au8_fxos8700cq_read_buffer[4] ) >> 2;
   pst_accel_data->i16_raw_z = (int16_t)( (au8_fxos8700cq_read_buffer[5] << 8) | au8_fxos8700cq_read_buffer[6] ) >> 2;

   pst_mag_data->i16_raw_x = (int16_t)( (au8_fxos8700cq_read_buffer[7]  << 8) | au8_fxos8700cq_read_buffer[8]  ) >> 2;
   pst_mag_data->i16_raw_y = (int16_t)( (au8_fxos8700cq_read_buffer[9]  << 8) | au8_fxos8700cq_read_buffer[10] ) >> 2;
   pst_mag_data->i16_raw_z = (int16_t)( (au8_fxos8700cq_read_buffer[11] << 8) | au8_fxos8700cq_read_buffer[12] ) >> 2;
*/
   return;
}




void fnFXOS8700CQ_Write_Register ( uint8_t reg, uint8_t value ) {
	/* TODO: ESTA FUNCAO */
   //fnFXOS8700CQ_PORT_Write_1 ( reg, &value, 1 );

   return;
}


uint8_t fnFXOS8700CQ_Read_Register ( uint8_t reg ) {
/*
   uint8_t au8_data[8];
   au8_data[0] = 0;

   fnFXOS8700CQ_PORT_Read_1( reg, au8_data, 1 );

   return au8_data[0];
   */
	return 0;
}


void fnFXOS8700CQ_Read_Register_Array ( uint8_t reg, uint8_t * pu8_data, uint8_t u8_size ) {

   //fnFXOS8700CQ_PORT_Read_1( reg, pu8_data, u8_size );

   return;
}

/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnFXOS8700CQ_Hard_Reset ( void ) {
	/* TODO: ESTA FUNCAO */
	/*

   fnFXOS8700CQ_PORT_Set_RST_Line(true);

   fnFXOS8700CQ_PORT_Delay_Ms(5);

   fnFXOS8700CQ_PORT_Set_RST_Line(false);

   fnFXOS8700CQ_PORT_Delay_Ms(5);
*/
   return;
}

#if 0
static int32_t accel_write(void *handle, uint8_t Reg, uint8_t *Bufp,
                              uint16_t len)
{
  uint32_t i2c_add = (uint32_t)handle;
  if (i2c_add == LSM303AGR_I2C_ADD_XL)
  {
    /* enable auto incremented in multiple read/write commands */
    Reg |= 0x80;
  }
  HAL_I2C_Mem_Write(&hi2c1, i2c_add, Reg,
                    I2C_MEMADD_SIZE_8BIT, Bufp, len, 1000);
  return 0;
}

static int32_t accel_read(void *handle, uint8_t Reg, uint8_t *Bufp,
                             uint16_t len)
{
  uint32_t i2c_add = (uint32_t)handle;
  if (i2c_add == LSM303AGR_I2C_ADD_XL)
  {
    /* enable auto incremented in multiple read/write commands */
    Reg |= 0x80;
  }
  HAL_I2C_Mem_Read(&hi2c1, (uint8_t) i2c_add, Reg,
                   I2C_MEMADD_SIZE_8BIT, Bufp, len, 1000);
  return 0;
}

static int32_t magnet_write(void *handle, uint8_t Reg, uint8_t *Bufp,
                              uint16_t len)
{
  uint32_t i2c_add = (uint32_t)handle;
  if (i2c_add == LSM303AGR_I2C_ADD_MG)
  {
    /* enable auto incremented in multiple read/write commands */
    Reg |= 0x80;
  }
  HAL_I2C_Mem_Write(&hi2c1, i2c_add, Reg,
                    I2C_MEMADD_SIZE_8BIT, Bufp, len, 1000);
  return 0;
}

static int32_t magnet_read(void *handle, uint8_t Reg, uint8_t *Bufp,
                             uint16_t len)
{
  uint32_t i2c_add = (uint32_t)handle;
  if (i2c_add == LSM303AGR_I2C_ADD_MG)
  {
    /* enable auto incremented in multiple read/write commands */
    Reg |= 0x80;
  }
  HAL_I2C_Mem_Read(&hi2c1, (uint8_t) i2c_add, Reg,
                   I2C_MEMADD_SIZE_8BIT, Bufp, len, 1000);
  return 0;
}

#endif
/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
