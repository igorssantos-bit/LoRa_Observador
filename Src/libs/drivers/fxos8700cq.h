#ifndef _FXOS8700CQ_H
#define _FXOS8700CQ_H

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


//#define FXOS8700CQ_I2C_BUS_ADDRESS     0x1E  // with pins SA0=0, SA1=0

//#define FXOS8700CQ_DEVICE_ID           0xC7


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


typedef struct {

   int16_t i16_raw_x;
   int16_t i16_raw_y;
   int16_t i16_raw_z;
   int8_t i8_temp;
   uint8_t u8_temp;

} st_fxos8700cq_raw_data_t;


typedef enum {

   FXOS8700CQ_REG_STATUS           = 0x00,
   FXOS8700CQ_REG_DR_STATUS        = 0x00,
   FXOS8700CQ_REG_F_STATUS         = 0x00,
   FXOS8700CQ_REG_OUT_X_MSB        = 0x01,
   FXOS8700CQ_REG_OUT_X_LSB        = 0x02,
   FXOS8700CQ_REG_OUT_Y_MSB        = 0x03,
   FXOS8700CQ_REG_OUT_Y_LSB        = 0x04,
   FXOS8700CQ_REG_OUT_Z_MSB        = 0x05,
   FXOS8700CQ_REG_OUT_Z_LSB        = 0x06,
   FXOS8700CQ_REG_F_SETUP          = 0x09,
   FXOS8700CQ_REG_TRIG_CFG         = 0x0A,
   FXOS8700CQ_REG_SYSMOD           = 0x0B,
   FXOS8700CQ_REG_INT_SOURCE       = 0x0C,
   FXOS8700CQ_REG_WHO_AM_I         = 0x0D,
   FXOS8700CQ_REG_XYZ_DATA_CFG     = 0x0E,
   FXOS8700CQ_REG_HP_FILTER_CUTOFF = 0x0F,
   FXOS8700CQ_REG_PL_STATUS        = 0x10,
   FXOS8700CQ_REG_PL_CFG           = 0x11,
   FXOS8700CQ_REG_PL_COUNT         = 0x12,
   FXOS8700CQ_REG_PL_BF_ZCOMP      = 0x13,
   FXOS8700CQ_REG_PL_THS_REG       = 0x14,
   FXOS8700CQ_REG_A_FFMT_CFG       = 0x15,
   FXOS8700CQ_REG_A_FFMT_SRC       = 0x16,
   FXOS8700CQ_REG_A_FFMT_THS       = 0x17,
   FXOS8700CQ_REG_A_FFMT_COUNT     = 0x18,
   FXOS8700CQ_REG_TRANSIENT_CFG    = 0x1D,
   FXOS8700CQ_REG_TRANSIENT_SRC    = 0x1E,
   FXOS8700CQ_REG_TRANSIENT_THS    = 0x1F,
   FXOS8700CQ_REG_TRANSIENT_COUNT  = 0x20,
   FXOS8700CQ_REG_PULSE_CFG        = 0x21,
   FXOS8700CQ_REG_PULSE_SRC        = 0x22,
   FXOS8700CQ_REG_PULSE_THSX       = 0x23,
   FXOS8700CQ_REG_PULSE_THSY       = 0x24,
   FXOS8700CQ_REG_PULSE_THSZ       = 0x25,
   FXOS8700CQ_REG_PULSE_TMLT       = 0x26,
   FXOS8700CQ_REG_PULSE_LTCY       = 0x27,
   FXOS8700CQ_REG_PULSE_WIND       = 0x28,
   FXOS8700CQ_REG_ASLP_COUNT       = 0x29,
   FXOS8700CQ_REG_CTRL_REG1        = 0x2A,
   FXOS8700CQ_REG_CTRL_REG2        = 0x2B,
   FXOS8700CQ_REG_CTRL_REG3        = 0x2C,
   FXOS8700CQ_REG_CTRL_REG4        = 0x2D,
   FXOS8700CQ_REG_CTRL_REG5        = 0x2E,
   FXOS8700CQ_REG_OFF_X            = 0x2F,
   FXOS8700CQ_REG_OFF_Y            = 0x30,
   FXOS8700CQ_REG_OFF_Z            = 0x31,
   FXOS8700CQ_REG_M_DR_STATUS      = 0x32,
   FXOS8700CQ_REG_M_OUT_X_MSB      = 0x33,
   FXOS8700CQ_REG_M_OUT_X_LSB      = 0x34,
   FXOS8700CQ_REG_M_OUT_Y_MSB      = 0x35,
   FXOS8700CQ_REG_M_OUT_Y_LSB      = 0x36,
   FXOS8700CQ_REG_M_OUT_Z_MSB      = 0x37,
   FXOS8700CQ_REG_M_OUT_Z_LSB      = 0x38,
   FXOS8700CQ_REG_CMP_OUT_X_MSB    = 0x39,
   FXOS8700CQ_REG_CMP_OUT_X_LSB    = 0x3A,
   FXOS8700CQ_REG_CMP_OUT_Y_MSB    = 0x3B,
   FXOS8700CQ_REG_CMP_OUT_Y_LSB    = 0x3C,
   FXOS8700CQ_REG_CMP_OUT_Z_MSB    = 0x3D,
   FXOS8700CQ_REG_CMP_OUT_Z_LSB    = 0x3E,
   FXOS8700CQ_REG_M_OFF_X_MSB      = 0x3F,
   FXOS8700CQ_REG_M_OFF_X_LSB      = 0x40,
   FXOS8700CQ_REG_M_OFF_Y_MSB      = 0x41,
   FXOS8700CQ_REG_M_OFF_Y_LSB      = 0x42,
   FXOS8700CQ_REG_M_OFF_Z_MSB      = 0x43,
   FXOS8700CQ_REG_M_OFF_Z_LSB      = 0x44,
   FXOS8700CQ_REG_MAX_X_MSB        = 0x45,
   FXOS8700CQ_REG_MAX_X_LSB        = 0x46,
   FXOS8700CQ_REG_MAX_Y_MSB        = 0x47,
   FXOS8700CQ_REG_MAX_Y_LSB        = 0x48,
   FXOS8700CQ_REG_MAX_Z_MSB        = 0x49,
   FXOS8700CQ_REG_MAX_Z_LSB        = 0x4A,
   FXOS8700CQ_REG_MIN_X_MSB        = 0x4B,
   FXOS8700CQ_REG_MIN_X_LSB        = 0x4C,
   FXOS8700CQ_REG_MIN_Y_MSB        = 0x4D,
   FXOS8700CQ_REG_MIN_Y_LSB        = 0x4E,
   FXOS8700CQ_REG_MIN_Z_MSB        = 0x4F,
   FXOS8700CQ_REG_MIN_Z_LSB        = 0x50,
   FXOS8700CQ_REG_TEMP             = 0x51,
   FXOS8700CQ_REG_M_THS_CFG        = 0x52,
   FXOS8700CQ_REG_M_THS_SRC        = 0x53,
   FXOS8700CQ_REG_M_THS_X_MSB      = 0x54,
   FXOS8700CQ_REG_M_THS_X_LSB      = 0x55,
   FXOS8700CQ_REG_M_THS_Y_MSB      = 0x56,
   FXOS8700CQ_REG_M_THS_Y_LSB      = 0x57,
   FXOS8700CQ_REG_M_THS_Z_MSB      = 0x58,
   FXOS8700CQ_REG_M_THS_Z_LSB      = 0x59,
   FXOS8700CQ_REG_M_THS_COUNT      = 0x5A,
   FXOS8700CQ_REG_M_CTRL_REG1      = 0x5B,
   FXOS8700CQ_REG_M_CTRL_REG2      = 0x5C,
   FXOS8700CQ_REG_M_CTRL_REG3      = 0x5D,
   FXOS8700CQ_REG_M_INT_SRC        = 0x5E,
   FXOS8700CQ_REG_A_VECM_CFG       = 0x5F,
   FXOS8700CQ_REG_A_VECM_THS_MSB   = 0x60,
   FXOS8700CQ_REG_A_VECM_THS_LSB   = 0x61,
   FXOS8700CQ_REG_A_VECM_CNT       = 0x62,
   FXOS8700CQ_REG_A_VECM_INITX_MSB = 0x63,
   FXOS8700CQ_REG_A_VECM_INITX_LSB = 0x64,
   FXOS8700CQ_REG_A_VECM_INITY_MSB = 0x65,
   FXOS8700CQ_REG_A_VECM_INITY_LSB = 0x66,
   FXOS8700CQ_REG_A_VECM_INITZ_MSB = 0x67,
   FXOS8700CQ_REG_A_VECM_INITZ_LSB = 0x68,
   FXOS8700CQ_REG_M_VECM_CFG       = 0x69,
   FXOS8700CQ_REG_M_VECM_THS_MSB   = 0x6A,
   FXOS8700CQ_REG_M_VECM_THS_LSB   = 0x6B,
   FXOS8700CQ_REG_M_VECM_CNT       = 0x6C,
   FXOS8700CQ_REG_M_VECM_INITX_MSB = 0x6D,
   FXOS8700CQ_REG_M_VECM_INITX_LSB = 0x6E,
   FXOS8700CQ_REG_M_VECM_INITY_MSB = 0x6F,
   FXOS8700CQ_REG_M_VECM_INITY_LSB = 0x70,
   FXOS8700CQ_REG_M_VECM_INITZ_MSB = 0x71,
   FXOS8700CQ_REG_M_VECM_INITZ_LSB = 0x72,
   FXOS8700CQ_REG_A_FFMT_THS_X_MSB = 0x73,
   FXOS8700CQ_REG_A_FFMT_THS_X_LSB = 0x74,
   FXOS8700CQ_REG_A_FFMT_THS_Y_MSB = 0x75,
   FXOS8700CQ_REG_A_FFMT_THS_Y_LSB = 0x76,
   FXOS8700CQ_REG_A_FFMT_THS_Z_MSB = 0x77,
   FXOS8700CQ_REG_A_FFMT_THS_Z_LSB = 0x78,

} en_fxos8700cq_reg_t;


typedef union {

   uint8_t u8_value;

   struct {

      uint8_t b_x_data_ready        : 1;
      uint8_t b_y_data_ready        : 1;
      uint8_t b_z_data_ready        : 1;
      uint8_t b_xyz_data_ready      : 1;
      
      uint8_t b_x_data_overwrite    : 1;
      uint8_t b_y_data_overwrite    : 1;
      uint8_t b_z_data_overwrite    : 1;
      uint8_t b_xyz_data_overwrite  : 1;

   } bitfield;

} un_fxos8700cq_reg_dr_status_t;


typedef union {

   uint8_t u8_value;

   struct {

      uint8_t b_active              : 1;  /* Operating mode selection */
      uint8_t b_trigger_measurement : 1;  /* Trigger immediate measurement. */
      uint8_t b_fast_read           : 1;  /* Fast Read selection */
      uint8_t b_over_sampling       : 2;  /* This register configures the over sampling ratio or measurement integration time. */
      uint8_t b_data_rate           : 3;  /* Data rate selection. */

   } bitfield;
   
} un_fxos8700cq_reg_ctrl_reg1_t;


typedef union {

   uint8_t u8_value;

   struct {

      uint8_t b_reserved_4             : 4;
      uint8_t b_mag_reset              : 1;  /* Magnetic Sensor Reset (One-Shot). */
      uint8_t b_raw_data               : 1;  /* Data output correction */
      uint8_t b_reserved_1             : 1;
      uint8_t b_auto_mag_reset_enable  : 1;  /* Automatic Magnetic Sensor Reset. */

   } bitfield;

} un_fxos8700cq_reg_ctrl_reg2_t;


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/

bool fnFXOS8700CQ_Init ( bool b_toggle_rst_line );
void fnFXOS8700CQ_Config ( uint8_t u8_profile );
void fnFXOS8700CQ_Enable ( void );
void fnFXOS8700CQ_Disable ( void );
void fnFXOS8700CQ_Enable_Interrupts ( void );
void fnFXOS8700CQ_Disable_Interrupts ( void );

void fnFXOS8700CQ_Read_Accel_Mag_Raw_Data ( st_fxos8700cq_raw_data_t * pst_mag_data, st_fxos8700cq_raw_data_t * pst_accel_data );

uint8_t fnFXOS8700CQ_Read_Register ( uint8_t reg );
void fnFXOS8700CQ_Write_Register ( uint8_t reg, uint8_t value ) ;
void fnFXOS8700CQ_Read_Register_Array ( uint8_t reg, uint8_t * pu8_data, uint8_t u8_size );

/*************************************************************************************************/
/*    LORA BOARD                                                                                 */
/*************************************************************************************************/
bool fnLsm303_Init(lsm303agr_ctx_t dev_ctx_xl, lsm303agr_ctx_t dev_ctx_mg );
void fnLSM303_Read_Mag_Raw_Data ( lsm303agr_ctx_t dev_ctx_mg, st_fxos8700cq_raw_data_t * st_mag_raw_data );
void fnLSM303_Read_Temp_Raw_Data ( lsm303agr_ctx_t dev_ctx_xl, st_fxos8700cq_raw_data_t * st_mag_raw_data );
bool fnLSM303_Mag_Ready ( lsm303agr_ctx_t *plocal_dev_ctx);
bool fnLSM303_Read_Threshold_Source ( lsm303agr_ctx_t dev_ctx_mg);

#if defined(__cplusplus)
}
#endif /* __cplusplus */


#endif   /* _FXOS8700CQ_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
