#ifndef _SYSTEM_STATUS_H
#define _SYSTEM_STATUS_H


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/
//#include <asf.h>

//#include "libs_firmware_config.h"
#include <accelerometer.h>
#include "libs/protocols/sigmais/sigmais_protocol.h"
#include "lsm303agr_reg.h"

//#include "port_input_digital.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/
#define MAGX_ENABLE  0x04
#define MAGY_ENABLE  0x02
#define MAGZ_ENABLE  0x01

/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/

typedef struct {


/* ACCMAG Data */
	/* TODO: implementar algo similar a estrutura do fxos8700cq_raw_data_t */
	st_accmag_raw_data_t st_accel_data;
	st_accmag_raw_data_t st_accel_ref;
	st_accmag_raw_data_t st_mag_data;
	//st_accmag_raw_data_t st_steady_mag_data;
	//st_accmag_raw_data_t st_detected_mag_data;
	//st_accmag_raw_data_t st_steady_calibration_mag_data;   //contem o valor da media do estado estacionario calculado na calibracao
	//st_accmag_raw_data_t st_steady_calibration_mag_data_2; //contem o valor da media do estado estacionario usado para recalculo da media por temperatura

/* System Data */
   uint8_t u8_battery;
   bool b_battery_status;
   uint8_t u8_strong_mag_sensivity;
   uint8_t u8_temperature_internal;

   //TODO: rever a necessidade da variavel b_detection_status e u8_n_retransmitions
   bool b_detection_status;
   uint8_t u8_n_retransmitions;
   

/* Times */
   st_sigmais_time_byte_bitfield_t st_sigmais_confirmation_time;
   st_sigmais_time_byte_bitfield_t st_sigmais_keep_alive_time;
   st_sigmais_time_byte_bitfield_t st_sigmais_detection_debounce;
   
   uint8_t u8_first_day_start_hour;
   uint8_t u8_first_day_end_hour;
   
   uint8_t u8_second_day_start_hour;
   uint8_t u8_second_day_end_hour;
   
   uint8_t u8_third_day_start_hour; 
   uint8_t u8_third_day_end_hour;
   
   uint32_t u32_timestamp;
   uint32_t u32_event_timestamp;

   st_sigmais_time_byte_bitfield_t st_sigmais_transmission_timer;       
 
/* Parquimetro */   
   uint8_t u8_config_report_timer_periodico;

   /* Horimetro */
   // Sensibilidade
   uint8_t u8_sensor_sensivity;
   // Contadores
   uint32_t u32_number_of_pulsos_1;
   uint32_t u32_last_timer_pulsos_1;
   uint32_t u32_number_of_pulsos_2;
   uint32_t u32_last_timer_pulsos_2;
   uint32_t u32_machine_on;
   uint32_t u32_timer_on;
   uint32_t u32_timer_off;
   // Temperatura
   uint16_t u16_temperatura1;
   uint16_t u16_temperatura2;
   bool b_probe1_enabled;
   bool b_probe2_enabled;
   // Flags das entradas digitais
   bool fea_fmon;
   bool fea_pulso1;
   bool fea_pulso2;

   /* System */
   bool b_data_processed;
   bool b_configuration_pending;
   bool b_calibration_authorized;
   st_sigmais_fwVersion_bitfield_t fwVersion;
   uint8_t u8_state_machine_state;
   bool b_downlink_config_frame_received;




} st_system_status_t;

extern volatile st_system_status_t st_system_status;




/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/

extern volatile lsm303agr_ctx_t dev_ctx_xl;
extern volatile lsm303agr_ctx_t dev_ctx_mg;

#endif  /* _SYSTEM_STATUS_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
