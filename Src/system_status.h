#ifndef _SYSTEM_STATUS_H
#define _SYSTEM_STATUS_H


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/
//#include <asf.h>

//#include "libs_firmware_config.h"
#include "libs/drivers/fxos8700cq.h"
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


/* FXOS Data */
	/* TODO: implementar algo similar a estrutura do fxos8700cq_raw_data_t */

   st_fxos8700cq_raw_data_t st_mag_data;
   st_fxos8700cq_raw_data_t st_steady_mag_data;
   st_fxos8700cq_raw_data_t st_detected_mag_data;
   st_fxos8700cq_raw_data_t st_steady_calibration_mag_data;   //contem o valor da media do estado estacionario calculado na calibracao
   st_fxos8700cq_raw_data_t st_steady_calibration_mag_data_2; //contem o valor da media do estado estacionario usado para recalculo da media por temperatura

/* System Data */
   int8_t i8_temperature;
   uint8_t u8_temperature;
   uint8_t u8_battery;
   
   bool b_battery_status;
   bool b_detection_status;
   
   uint8_t u8_strong_mag_sensivity;
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
   uint8_t u8_emptyParkAmplitudeControl;
   uint16_t u16_numAmostrasMediaVagaVazia;
   uint8_t u8_sensibilityToDetection;
   uint8_t u8_sensibilityToEmpty;
   uint8_t u8_sensibilityToEmptyDueTemperature;
   uint8_t u8_numAmostrasAcimaDoLimiarParaDeteccao;
   uint8_t u8_mag_enable; // bit2 -> x, bit1 -> y e bit0 -> z
   uint8_t u8_deltat_auto_correction; // 4 valores 00, 01, 10, 11
   uint8_t u8_config_report_timer_periodico;
    
   /* Horimetro */
      uint8_t u8_sensor_sensivity;
      uint32_t u32_horimetro_used_time_accumulated;

   /* Traffic Variables */
   	uint8_t u8_traffic_threshold;
   	uint8_t u8_traffic_time2transmit_first_day_hour;
   	uint8_t u8_unit_time2transmit_first_day_hour;
   	uint8_t u8_traffic_time2transmit_second_day_hour;
   	uint8_t u8_unit_time2transmit_second_day_hour;
   	uint8_t u8_traffic_type; // 0 -> MagX e MagY, 1 -> MagX e MagZ, 2 -> MagY e MagZ, 3 -> MagXYZ e MagZ (2 bits apenas)
   	uint8_t u8_traffic_reset_counters;
   	uint32_t u32_cnt_trafegoX;
   	uint32_t u32_cnt_trafegoY;
   	uint32_t u32_cnt_trafegoZ;
   	uint32_t u32_cnt_trafegoXYZ;
   	uint8_t u8_traffic_erro;					// indica quando detectar um erro ou alarme no processo de contagem
   	uint8_t u8_npsat;							// controla a quantidade de pontos sequenciais acima do threshold para entrar/sair da detecção
   	uint8_t u8_sfreq;                           // controla a frequencia da transmissão dos dados pelo magnetômetr
   	uint8_t u8_tmhbuf;                          // controla o tamanho do buffer da média
   	uint8_t u8_nptd;                            // controla a quantidade de amostras a esperar após entrar em um estado antes de aceitar novas variações no estado
   	uint8_t u8_vatualizacao_media_detectado;    // controla a velocidade de atualização da média quando no estado detectado. Vai de 0 a 7 com incrementos de 50 em 50.
    uint8_t u8_traffic_threshold_inferior;      // threshold para sair do estado detectado

    /* Tracker */
    uint8_t u8_op_code;
    uint8_t u8_janela_BLE;
    uint16_t u16_timeOut_BLE;
    uint16_t u16_timer_Uplink;
    uint8_t* data_prt;
    uint16_t data_size;

    /* System */
   bool b_data_processed;
   bool b_configuration_pending;
   //en_module_type_t en_module_type;  /* TODO: ver a necessidade dessa variavel*/
   bool b_calibration_authorized;
   st_sigmais_fwVersion_bitfield_t fwVersion;
   uint8_t u8_radioMode; //RC2 ou RC4
   uint8_t u8_dongleLigado; // 0 -> desligado e 1 -> ligado
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
