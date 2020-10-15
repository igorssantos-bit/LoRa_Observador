/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
//#include <asf.h>


//#include "libs_firmware_config.h"
#include "libs/services/debug.h"
//#include "libs/services/system_timer.h"
#include "libs/services/delay_loop.h"
//#include "libs/services/debug_port.h"

//#include "libse/hal/peripherals/hal_wdt.h"
//#include "libs/hal/peripherals/hal_i2c.h"
#include "libs/drivers/fxos8700cq.h"


//#include "uart_sigfox.h"
#include "app.h"
#include "app_state_machine.h"
#include "system_flags.h"
#include "system_status.h"
//#include "system_power.h"
#include "events.h"
//#include "port_input_digital.h"
//#include "board_config.h"
//#include "accelerometer.h"
#include "input_analogic.h"
#include "sensors.h"
//#include "timestamp.h"
//#include "rtc.h"
#include "radio.h"
//#include "uart_external.h"
//#include "eeprom_atsam.h"



/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/


/* Process events */
void fnAPP_Process_Event_System_Tick ( void );

void fnAPP_Process_Event_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_mag_data );
void fnAPP_Process_Event_Accel_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_accel_data, st_fxos8700cq_raw_data_t * pst_mag_data );
void fnAPP_Process_Event_Mag_Threshold ( void );

/* APP Timer - Heartbeat */
void fnAPP_TIMER_Heartbeat ( void );

void fnAPP_Init_Standard_Values ( void );


/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/


/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


void fnAPP_Init ( void ) {
   //TODO: esta funcao
  // fnRTC_Init();

   fnTIMER_System_Tick_Init();

   fnAPP_Init_Standard_Values();
   
   fnSENSORS_Init();

   //fnRADIO_Init();
   //fnINPUT_ANALOGIC_Init();
   un_system_flags.flag.fxos_1_int_threshold = false;

   fnAPP_STATE_MACHINE_Init( );
   fnAPP_STATE_Machine( EVENT_SYSTEM_INIT );

   //TODO: acertar o timestamp_init()
   //fnTIMESTAMP_Init();
   
   return;

}

// TODO: esta funcao
void fnAPP_Process_Events ( void ) {
/* Clear invalid flags */
  // un_system_flags.flag.reserved = 0;

/* Check event -> Clear flag, process */
/*
   if ( un_system_flags.flag.radio_timer_int == true ) {
      un_system_flags.flag.radio_timer_int = false;
      fnRADIO_Process_Timeout();
   }

   if ( un_system_flags.flag.radio_packet_received == true ) {
      un_system_flags.flag.radio_packet_received = false;
      fnRADIO_Process_Packet_Received();
   }
   */
   //if ( un_system_flags.flag.fxos_1_int_data_ready == true ) {
    if (fnLSM303_Mag_Ready((lsm303agr_ctx_t *) &dev_ctx_mg)){
      // un_system_flags.flag.fxos_1_int_data_ready = false; // usado no caso da interrupcao
      fnAPP_Process_Event_Mag_Data_Ready ( ( st_fxos8700cq_raw_data_t * ) &st_system_status.st_mag_data );
   }

   if ( un_system_flags.flag.fxos_1_int_threshold == true ) {
      un_system_flags.flag.fxos_1_int_threshold = false;
      fnAPP_Process_Event_Mag_Threshold();
   }

   if ( un_system_flags.flag.rtc_tick == true ) {
      un_system_flags.flag.rtc_tick = false;
//      fnAPP_Process_Event_RTC();
   }

   return;

}


void fnAPP_Check_Events ( void ) {

   return;

}

//TODO: ajustar os modos de LOW Power
void fnAPP_Enter_Low_Power ( void ) {
/*
   un_system_power.flag.reserved = 0;

   if ( un_system_power.u16_all_flags == 0 ) {
      system_set_sleepmode( SYSTEM_SLEEPMODE_STANDBY );
   }
   else {
      system_set_sleepmode( SYSTEM_SLEEPMODE_IDLE_2 );
   }

   system_sleep();
*/
   return;

}


/*************************************************************************************************/
/*    SYSTEM EVENT                                                                               */
/*************************************************************************************************/


void fnAPP_Process_Event_RTC ( void ) {

   fnTIMER_System_Tick();

  // fnSENSORS_Check_Data();

   fnTIMESTAMP_Run_Time_Counter();

/*
   fnWDT_Reset_Counter();



   fnAPP_TIMER_Heartbeat();


*/
   fnAPP_STATE_Machine( EVENT_SYSTEM_RTC );

   return;

}


void fnAPP_Process_Event_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_mag_data ) {

   fnSENSORS_Mag_Data_Ready ( pst_mag_data );

   fnAPP_STATE_Machine( EVENT_MAG_DATA_READY );

   return;

}


void fnAPP_Process_Event_Accel_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_accel_data, st_fxos8700cq_raw_data_t * pst_mag_data ) {

   fnSENSORS_Accel_Mag_Data_Ready ( pst_accel_data, pst_mag_data );

   return;

}

void fnAPP_Process_Event_Mag_Threshold ( void ) {

   //fnSENSORS_Check_Magnetometer_Valid_Threshold( );
   if (fnLSM303_Read_Threshold_Source (dev_ctx_mg) ){
	   fnAPP_STATE_Machine( EVENT_MAG_THRESHOLD );
   }
   return;

}


/*************************************************************************************************/
/*    TIMER                                                                                      */
/*************************************************************************************************/


void fnAPP_TIMER_Heartbeat ( void ) {
	//TODO: esta função
/*
   if (Get_Uart_Stop_State() == false){

     if ( fnSENSORS_Is_Mag_Calibrating() ) {

        LED_ON;
        fnDELAY_LOOP_ms(10);
        LED_OFF;
        fnDELAY_LOOP_ms(80);
        LED_ON;
        fnDELAY_LOOP_ms(10);
        LED_OFF;

     } else if ( fnSENSORS_Has_Detection() ) {      

        LED_ON;

      } else {
      
        LED_OFF;
      
      }
	  
   }
*/
   return;

}


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


void fnAPP_Init_Standard_Values ( void ) {
const char *pdate = __DATE__;
const char *ptime = __TIME__;
const char *ptimestamp = __TIMESTAMP__;
uint16_t fwVersao = 0;

   st_system_status.u8_strong_mag_sensivity = 5;
   st_system_status.u8_n_retransmitions = 1;
   
   st_system_status.i8_temperature = 25;
   st_system_status.u8_battery = 33;
   
   st_system_status.b_battery_status = true;
   st_system_status.b_detection_status = false;

   st_system_status.st_sigmais_confirmation_time.time_unity = EN_SIGMAIS_TIME_SECONDS;
   st_system_status.st_sigmais_confirmation_time.time_value = 30;

   st_system_status.st_sigmais_keep_alive_time.time_unity = EN_SIGMAIS_TIME_MINUTES;
   st_system_status.st_sigmais_keep_alive_time.time_value = 5;

   //st_system_status.st_sigmais_keep_alive_time.time_unity = EN_SIGMAIS_TIME_HOURS;
   //st_system_status.st_sigmais_keep_alive_time.time_value = 2;
      
   st_system_status.st_sigmais_detection_debounce.time_unity = EN_SIGMAIS_TIME_SECONDS;
   //st_system_status.st_sigmais_detection_debounce.time_value = 20;
   st_system_status.st_sigmais_detection_debounce.time_value = 5;

   st_system_status.u8_first_day_start_hour = 0;
   st_system_status.u8_first_day_end_hour = 24;
   
   st_system_status.u8_second_day_start_hour = 0;
   st_system_status.u8_second_day_end_hour = 24;
   
   st_system_status.u8_third_day_start_hour = 0; 
   st_system_status.u8_third_day_end_hour = 24;
   
   st_system_status.u32_timestamp = 540; // 9AM 01/01/2017
      
//   st_system_status.u8_sensor_sensivity = 2;
//   st_system_status.u32_horimetro_used_time_accumulated = 0;
//   st_system_status.u32_horimetro_number_of_times_timer_on = 0;
   st_system_status.st_sigmais_transmission_timer.time_unity = EN_SIGMAIS_TIME_MINUTES;
   st_system_status.st_sigmais_transmission_timer.time_value = 5; //15;

   st_system_status.b_data_processed = false;    // park = raw data e sense = transmissao periodica
   //st_system_status.b_data_processed = true;       // sense = transmissao por eventos 
   st_system_status.b_configuration_pending = false;

   st_system_status.u8_emptyParkAmplitudeControl = 15;  //5
   st_system_status.u16_numAmostrasMediaVagaVazia = 2048; //100
   st_system_status.u8_sensibilityToDetection = 40;      //15
   st_system_status.u8_sensibilityToEmpty = 20;           //10
   st_system_status.u8_sensibilityToEmptyDueTemperature = 12;
   st_system_status.u8_numAmostrasAcimaDoLimiarParaDeteccao = 5;

// versionamento
// Arquitetura: 02 -> STM + LSM
// Funcionalidades: 01 -> firmware original Sigfox + Lora
//                  02 -> modo park
// bugs:
//                  01.00 -> nenhum bug detectado

   st_system_status.fwVersion.fwa = 2; //arquitetura
   st_system_status.fwVersion.fwb = 2; // funcionalidades
   st_system_status.fwVersion.fwc = 0; //correcao de bugs 
   
	for (int cont = 0; cont< (strlen(pdate) - 1); cont++){
		fwVersao ^= pdate[cont] | (pdate[cont+1] << 8);
	}

	for (int cont = 0; cont<(strlen(ptime) - 1); cont++){
		fwVersao ^= ptime[cont] | (ptime[cont+1] << 8);
	}

	for (int cont = 0; cont<(strlen(ptimestamp) - 1); cont++){
		fwVersao ^= ptimestamp[cont] | (ptimestamp[cont+1] << 8);
	}
   
   st_system_status.fwVersion.fwd = fwVersao;
   //st_system_status.u8_dongleLigado = 0;		             // sigfox
   //st_system_status.u8_radioMode = 0x02;	                 // 0x02 = RC2 e 0x04 = RC4
   st_system_status.u8_mag_enable = 0x07;		             // x,y,z enable
   st_system_status.u8_deltat_auto_correction =	0;           // 0 -> menor valor = 3
   st_system_status.u8_config_report_timer_periodico = 0;    // 0, 1, 2, 3
   
   st_system_status.b_calibration_authorized = true;
   st_system_status.b_downlink_config_frame_received = false;

   
   st_system_status.u8_traffic_time2transmit_first_day_hour = 1;			// 30
   st_system_status.u8_unit_time2transmit_first_day_hour = 1;				//minutos
   st_system_status.u8_traffic_time2transmit_second_day_hour = 1;           // 30
   st_system_status.u8_unit_time2transmit_second_day_hour = 1;				//minutos
   st_system_status.u8_traffic_type  = 0;									// X e Y
   st_system_status.u8_traffic_reset_counters = 0;                          //
   st_system_status.u8_traffic_threshold = 25;                              //
   st_system_status.u8_traffic_threshold_inferior = 15;                     //
   st_system_status.u32_cnt_trafegoX = 0;
   st_system_status.u32_cnt_trafegoY = 0;
   st_system_status.u32_cnt_trafegoZ = 0;
   st_system_status.u32_cnt_trafegoXYZ = 0;
   st_system_status.u8_traffic_erro = 0;
   st_system_status.u8_npsat = 1; // 3 amostras
   st_system_status.u8_sfreq = 1;  //0 = 800, 1 = 400Hz, 2 = 200, 3 = 100
   st_system_status.u8_tmhbuf = 6; //350 amostras
   st_system_status.u8_nptd = 1;   //40 amostras
   st_system_status.u8_vatualizacao_media_detectado = 7;  // 0 a 7 sendo que 0 -> desabilita e depois incrementa em multiplos de 50

   return;

}


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
