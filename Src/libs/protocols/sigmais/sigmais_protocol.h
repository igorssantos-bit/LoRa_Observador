#ifndef SIGMAIS_PROTOCOL_H_
#define SIGMAIS_PROTOCOL_H_


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

/* AUXILIARY DEFINITIONS */

typedef enum {

	EN_SIGMAIS_TIME_SECONDS = 0,
	EN_SIGMAIS_TIME_MINUTES = 1,
	EN_SIGMAIS_TIME_HOURS = 2,
	EN_SIGMAIS_TIME_DAYS = 3,

} en_time_unity_t;


typedef enum {

	EN_SIGMAIS_ERROR_TRANSMIT = 0,
	EN_SIGMAIS_ERROR_RECEIVE = 1,

} en_error_code_t;

typedef enum {

	EN_SIGMAIS_WRONG_FRAME = 0,
	EN_SIGMAIS_SIGFOX_RESPONSE = 1,
	EN_SIGMAIS_SERVER_RESPONSE = 2,

} en_error_value_t;




/* FRAMES */

typedef enum {

	EN_SIGMAIS_UPLINK_REQUEST_CONFIG = 0,
	EN_SIGMAIS_UPLINK_TABELA_DEVICES = 1,
	EN_SIGMAIS_UPLINK_DEVICE_STATUS = 2,

} en_sigmais_uplink_frame_type_t;


typedef enum {

	EN_SIGMAIS_DOWNLINK_DAILY_UPDATE = 0,
	EN_SIGMAIS_DOWNLINK_CONFIG_FRAME = 1,
	EN_SIGMAIS_DOWNLINK_SIZE = 2

}en_sigmais_downlink_frame_type_t;

typedef enum {

	EN_SIGMAIS_OP_CODE_IGUAL = 0,
	EN_SIGMAIS_OP_CODE_ADD = 1,
	EN_SIGMAIS_OP_CODE_SUB = 2,
	EN_SIGMAIS_OP_CODE_ALL = 7,

}en_sigmais_downlink_op_code_t;


/* BITFIELDS */
/*
 ***************************************************************************************
	a = versao da arquitetura  (4 bits)  [manual]
	b = novas funcionalidades  (4 bits)  [manual]
	c = correcao de bugs (4) [manual]
	d = melhoria de performance, manutencao do codigo (12 bits)  [automatico]



Regras:
- mudanca no a zera b,c
- mudanca no b zera c

 ****************************************************************************************
 */

typedef struct {
	//versionamento -> 3 bytes (a.b.c.d)

	uint8_t fwa               :4;
	uint8_t fwb               :4;
	uint8_t fwc               :4;         	
	uint16_t fwd              :12;

}  __attribute__ ((packed)) st_sigmais_fwVersion_bitfield_t;


typedef struct {

	en_sigmais_uplink_frame_type_t en_sigmais_uplink_frame_type    	:4;
	uint8_t pending                                          		:1;
	uint8_t battery_status                                         	:1;
	uint8_t empity                                         			:2;

}  __attribute__ ((packed)) st_sigmais_uplink_header_bitfield_t;


typedef struct {

	en_sigmais_downlink_frame_type_t en_sigmais_downlink_frame_type    :3;
	uint8_t op_code                                               	 :3;
	uint8_t pending                                      			 :1;
	uint8_t calibration_authorized                                   :1;

}  __attribute__ ((packed)) st_sigmais_downlink_header_bitfield_t;


typedef struct {

	uint8_t time_value               :6;
	en_time_unity_t time_unity       :2;

}  __attribute__ ((packed)) st_sigmais_time_byte_bitfield_t;


typedef struct {

	uint8_t actual_day_starting_hour  :5;
	uint8_t actual_day_finishing_hour :5;
	uint8_t next_day_starting_hour    :5;
	uint8_t next_day_finishing_hour   :5;
	uint8_t third_day_starting_hour   :5;
	uint8_t third_day_finishing_hour  :5;
	uint8_t reserved                  :2;

} __attribute__ ((packed)) st_sigmais_working_hour_bitfield_t;


/* UPLINK FRAMES */

typedef struct {

	uint8_t major_b1                                           				:8;
	uint8_t major_b2                                           				:8;
	uint8_t counter_b1                                           			:8;
	uint8_t counter_b2                                           			:8;
	uint8_t counter_b3                                           			:8;
	uint8_t counter_b4                                           			:8;

} __attribute__ ((packed)) st_info_frame_tabela_t ;

typedef struct {

	uint8_t error_code                                                      :8;
	uint8_t mag_raw_data_x_msb                                              :8;
	uint8_t mag_raw_data_x_lsb                                              :8;
	uint8_t mag_raw_data_y_msb                                              :8;
	uint8_t mag_raw_data_y_lsb                                              :8;
	uint8_t mag_raw_data_z_msb                                              :8;
	uint8_t mag_raw_data_z_lsb                                              :8;

} st_error_frame_raw_data_t ;

typedef struct {

	int8_t  temperature                                                   :8;
	uint8_t battery                                                       :8;
	uint32_t timestamp                                                    :24;

} __attribute__ ((packed)) st_sigmais_info_frame_parking_processed_t;


typedef struct {

	st_sigmais_uplink_header_bitfield_t un_sigmais_uplink_header_bitfield;

} st_sigmais_keep_alive_frame_t;


typedef struct {

	uint8_t configuration_sensivity                                       :3;
	uint8_t configuration_data_processed                                  :1;
	uint8_t module_type                                                   :1;
	uint8_t configuration_reserved                                        :3;
	st_sigmais_working_hour_bitfield_t sigmais_working_hour               ;
	uint32_t timestamp                                                    :24;

} __attribute__ ((packed)) st_sigmais_config_report_frame_t;


typedef struct {

	uint8_t error_code                                                    :8;
	uint8_t error_data                                                    :8;

} st_sigmais_error_frame_t;


typedef struct {

	int8_t  temperature                                                   :8;
	uint8_t battery                                                       :8;
	uint32_t timestamp                                                    :24;
	uint32_t usage_time                                                   :24;
	uint32_t number_of_times_timer_on                                     :24;
	//uint16_t inactivity_time                                              ;

} __attribute__ ((packed)) st_sigmais_info_frame_horimetro_t;


typedef struct {

	uint8_t mag_raw_data_x_msb                                              :8;
	uint8_t mag_raw_data_x_lsb                                              :8;
	uint8_t mag_raw_data_y_msb                                              :8;
	uint8_t mag_raw_data_y_lsb                                              :8;
	uint8_t mag_raw_data_z_msb                                              :8;
	uint8_t mag_raw_data_z_lsb                                              :8;
	uint16_t number_of_times_timer_on                                       :16;
	uint32_t usage_time                                                     :24;
} __attribute__ ((packed)) st_info_frame_horimetro_raw_data_t;


typedef struct {

	uint32_t contadorTrafego1                                                :24;
	uint32_t contadorTrafego2                                                :24;
	uint32_t timestamp                                                       :24;
	uint8_t periodo_transmissao1                                             :8;
	uint8_t periodo_transmissao2                                             :8;

} __attribute__ ((packed)) st_info_traffic_frame_t ;

/* DOWNLINK FRAMES */

typedef struct {

	uint32_t timestamp                                                         :24;
	st_sigmais_working_hour_bitfield_t time_configuration                      ;

} __attribute__ ((packed)) st_sigmais_daily_update_t;


typedef struct {

	uint8_t janela_BLE                                                       :8;
	uint16_t timeOut_BLE                                                     :16;
	uint16_t timer_Uplink													 :16;

} __attribute__ ((packed)) st_sigmais_config_frame_t;


typedef struct {

	uint8_t emptyParkAmplitudeControl                                         :8;   // vaga vazia -> valor para o controle de amplitude da m�dia do valor do magnet�metro 
	uint16_t numAmostrasMediaVagaVazia                                        :16;  // n�mero de amostras para calcular a m�dia do valor do magnet�metro quando a vaga est� vazia
	uint8_t sensibilityToDetection                                            :8;   // valor do offset para considerar vaga ocupada
	uint8_t sensibilityToEmpty                                                :8;   // valor do offset para considerar vaga liberada
	uint8_t sensibilityToEmptyDueTemperature                                  :5;   // LSB
	uint8_t numAmostrasAcimaDoLimiarParaDeteccao                              :3;   // MSB: numero de amostras acima do limiar para detec��o.  
	uint8_t mag_enable                                                        :3;    // x, y, z enable to detection e undetection
	uint8_t deltat_auto_correction                                            :2;    // 
	uint8_t config_report_timer_periodico                                     :2;    // valor da configura��o do timer periodico do config report
	uint8_t reserved                                                          :1; 

} __attribute__ ((packed)) st_sigmais_config_park_parameters_t;


typedef struct {

	uint8_t configuration_traffic_threshold                                   :8;
	uint8_t configuration_time2transmission_1                                 :6;
	uint8_t unit_time2transmission_1                                          :2;
	uint8_t configuration_time2transmission_2                                 :6;
	uint8_t unit_time2transmission_2                                          :2;
	uint8_t npsat                                                             :3; // 0 a 7
	uint8_t sample_freq                                                       :2; // 0 = 800Hz, 1 = 400Hz, 2 = 200Hz, 3 = 100Hz
	uint8_t tmh_buf                                                           :3; // 0 = 50 amostras, 1 = 100 amostras, ..., 7 = 400 amostras
	uint8_t nptd                                                              :3; // 0 = 0 amostras, 1 = 40 amostras, ... , 7 = 280 amostras
	uint8_t vatumedia_detectado                                               :3; // 0 a 7, 0 = desabilita, 1 = 50, 2 = 100, ... 7 = 350 amostras
	uint8_t reserved_2bits                                                    :2; // 2 bits reservados
	uint8_t configuration_traffic_threshold_inferior                          :8; // 0 a 255
	uint8_t reserved                                                          :8;
} __attribute__ ((packed)) st_sigmais_config_traffic_frame_t;


#pragma GCC diagnostic pop

/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


#endif /* SIGMAIS_PROTOCOL_H_ */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/

