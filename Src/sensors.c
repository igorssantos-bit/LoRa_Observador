/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include <stddef.h>
#include <math.h>
#include <stdlib.h>

//#include "asf.h"

//#include "libs_firmware_config.h"
//#include "libs_firmware/services/system_timer.h"
#include "libs/math/average.h"
//#include "libs_firmware/util/macros.h"
#include "libs/services/debug.h"

#include "sensors.h"
//#include "accelerometer.h"
//#include "magnetometer.h"
//#include "radio.h"

//#include "system_flags.h"
#include "system_status.h"

#include "libs/drivers/fxos8700cq.h"
//#include "ports/port_input_digital.h"
#include "system_flags.h"

/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/

#define AVERAGE_DETECTION_BUFFER_SIZE               16
#define AVERAGE_TEMPERATURE_BUFFER_SIZE             16

/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/

int16_t i16_avg_detection_mag_x[AVERAGE_DETECTION_BUFFER_SIZE];
int16_t i16_avg_detection_mag_y[AVERAGE_DETECTION_BUFFER_SIZE];
int16_t i16_avg_detection_mag_z[AVERAGE_DETECTION_BUFFER_SIZE];
int8_t i8_avg_temperature[AVERAGE_TEMPERATURE_BUFFER_SIZE];


typedef struct {
  
   float f_avg_x;
   float f_avg_y;
   float f_avg_z;
   
} st_raw_mag_axis_t;


typedef struct {
   
   st_average_desc_t st_x_average_desc;
   st_average_desc_t st_y_average_desc;
   st_average_desc_t st_z_average_desc;
   
} st_raw_detect_mag_axis_t;




int16_t i16_x_mag_test;
int16_t i16_y_mag_test;
int16_t i16_z_mag_test;

int16_t magdeteccaox;
int16_t magdeteccaoy;
int16_t magdeteccaoz;

int16_t correcaoAnteriorx = 0;
int16_t correcaoAnteriory = 0;
int16_t correcaoAnteriorz = 0;


typedef struct {
   
   uint8_t u8_threshold;
   uint8_t u8_detection_return;
   
} st_mag_sensivity_t;

/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/


bool fnSENSORS_Has_Steady_Threshold ( st_fxos8700cq_raw_data_t * pst_accel_raw_data );

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/

uint8_t u8_sensor_sensivity = 9;

st_raw_mag_axis_t st_steady_mag_axis;
st_raw_detect_mag_axis_t st_detection_mag_axis;
st_raw_mag_axis_t magnetometroSteadyDeteccao;

bool b_first_data = true;

bool b_mag_detected = false;
bool b_accel_detected = false;

bool b_calibrating = true; //false;

uint8_t u8_state_counter = 60;
uint8_t u8_state_detected_counter = 0;
int8_t temperaturaDeteccao;//int8_t temperaturaDeteccao;

st_average_desc_8bits_t st_temp_average_desc;

st_mag_sensivity_t st_mag_sensivity[8] = {
   { 10, 7 },  
   { 12, 8 },  
   { 15, 10 }, 
   { 20, 10 }, 
   { 25, 12 }, 
   { 25, 15 }, 
   { 30, 10 }, 
   { 30, 15 }, 
};

uint16_t u16_strong_thr[8] = {
   0x0300,
   0x0600,
   0x0900,
   0x1200,
   0x1500,
   0x1800,
   0x2100,
   0x2400,
};

const uint32_t u32_sensivity_table[8] = {
   0x00000400,
   0x00001000,
   0x00004000,
   0x00010000,
   0x0001B900,
   0x00040000,
   0x00100000,
   0x00400000,
};


/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnSENSORS_Init ( void ) {
   
   fnSENSORS_Config ( );
   
   return;
}

void fnSENSORS_Config ( void ) {
	//TODO: magnetometer initialization
/*
   fnFXOS8700CQ_Init ( false );
   fnFXOS8700CQ_Disable_Interrupts();
   fnFXOS8700CQ_Config ( st_system_status.en_module_type );
   fnMAGNETOMETER_Set_Mag_Threshold_Data( u16_strong_thr[st_system_status.u8_strong_mag_sensivity] );
   fnFXOS8700CQ_Enable_Interrupts();

   fnFXOS8700CQ_Enable();
*/

 // buffer initialization
    st_detection_mag_axis.st_x_average_desc.px_data = i16_avg_detection_mag_x;
    fnAVERAGE_Init ( &st_detection_mag_axis.st_x_average_desc, AVERAGE_DETECTION_BUFFER_SIZE, AVERAGE_DETECTION_BUFFER_SIZE*2 );

    st_detection_mag_axis.st_y_average_desc.px_data = i16_avg_detection_mag_y;
    fnAVERAGE_Init ( &st_detection_mag_axis.st_y_average_desc, AVERAGE_DETECTION_BUFFER_SIZE, AVERAGE_DETECTION_BUFFER_SIZE*2 );

    st_detection_mag_axis.st_z_average_desc.px_data = i16_avg_detection_mag_z;
    fnAVERAGE_Init ( &st_detection_mag_axis.st_z_average_desc, AVERAGE_DETECTION_BUFFER_SIZE, AVERAGE_DETECTION_BUFFER_SIZE*2 );

    st_temp_average_desc.px_data = i8_avg_temperature;
    fnAVERAGE_Init_8bits ( &st_temp_average_desc, AVERAGE_TEMPERATURE_BUFFER_SIZE, AVERAGE_TEMPERATURE_BUFFER_SIZE );

    b_mag_detected = false;

   return;
}   

//

void fnSENSORS_Mag_Threshold ( void ) {
   //TODO: esta funcao
  // fnMAGNETOMETER_Set_Mag_Threshold_Data( u16_strong_thr[st_system_status.u8_strong_mag_sensivity] );
   
   return;
}   

void fnSENSORS_Replace ( void ) {
   
   u8_state_counter = 61;
	//u8_state_counter = 250;
   
   b_first_data = true;
   b_mag_detected = false;
   b_calibrating = true;
 
   return;
}


//      ESTE eh O PRINCIPAL CoDIGO DO PARK. ELE PARTICIPA DO CONTROLE DA ENTRADA NO ESTADO DE DETECcaO E DESDETECcaO
void fnSENSORS_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_accel_raw_data ) {

   //fnMAGNETOMETER_Read_Raw_Data (pst_accel_raw_data );
   fnLSM303_Read_Mag_Raw_Data ( dev_ctx_mg, ( st_fxos8700cq_raw_data_t * ) &st_system_status.st_mag_data);
   fnLSM303_Read_Temp_Raw_Data (dev_ctx_xl, ( st_fxos8700cq_raw_data_t * ) &st_system_status.st_mag_data );

   fnAVERAGE_Update_8bits(&st_temp_average_desc,st_system_status.st_mag_data.u8_temp);//fnAVERAGE_Update_8bits(&st_temp_average_desc,pst_accel_raw_data->i8_temp);
   st_system_status.i8_temperature =  fnAVERAGE_Get_Value_8bits(&st_temp_average_desc); // para eliminar a leitura do sensor analogico que está em comm_sigmais
   un_system_flags.flag.fxos_1_int_data_ready = true;

#if 0
  if( u8_state_counter > 0 )
      u8_state_counter--;
   else if( b_calibrating == true ) {
      b_calibrating = false;
      b_first_data = true;
	  /*
	  * Armazena a media calculada para vaga vazia aposs a calibracao.
	  * Este valor ficara constante e sera utilizado para impedir a inversao do estado vazio com o estado ocupado.
	  */
      st_system_status.st_steady_calibration_mag_data.i16_raw_x = st_system_status.st_steady_mag_data.i16_raw_x;
	  st_system_status.st_steady_calibration_mag_data.i16_raw_y = st_system_status.st_steady_mag_data.i16_raw_y;
	  st_system_status.st_steady_calibration_mag_data.i16_raw_z = st_system_status.st_steady_mag_data.i16_raw_z;
	  st_system_status.st_steady_calibration_mag_data.i8_temp = (uint8_t *)fnAVERAGE_Get_Value_8bits(&st_temp_average_desc);

	  st_system_status.st_steady_calibration_mag_data_2.i16_raw_x = st_system_status.st_steady_mag_data.i16_raw_x;
	  st_system_status.st_steady_calibration_mag_data_2.i16_raw_y = st_system_status.st_steady_mag_data.i16_raw_y;
	  st_system_status.st_steady_calibration_mag_data_2.i16_raw_z = st_system_status.st_steady_mag_data.i16_raw_z;
	  st_system_status.st_steady_calibration_mag_data_2.i8_temp =  st_system_status.st_steady_calibration_mag_data.i8_temp;

	  fnDEBUG_16bit_Int_Value("CALIBRACAO (magx;magy;magz,temp) = ",st_system_status.st_steady_calibration_mag_data.i16_raw_x," ");
	  fnDEBUG_16bit_Int_Value("; ",st_system_status.st_steady_calibration_mag_data.i16_raw_y," ");
	  fnDEBUG_16bit_Int_Value("; ",st_system_status.st_steady_calibration_mag_data.i16_raw_z," ");
	  fnDEBUG_8bit_Value("; ",st_system_status.st_steady_calibration_mag_data.i8_temp,"\r\n");
      fnDEBUG_Const_String("READY\r\n"); 

	  //delay_cycles(1000);

   }
   
   if( b_first_data ) {
      
      b_first_data = false;
      
      st_steady_mag_axis.f_avg_x = pst_accel_raw_data->i16_raw_x;
      st_steady_mag_axis.f_avg_y = pst_accel_raw_data->i16_raw_y;
      st_steady_mag_axis.f_avg_z = pst_accel_raw_data->i16_raw_z;
    
   }

  // delay_cycles(1500);
   fnDEBUG_16bit_Int_Value("RAW (x,y,z,temp): ",pst_accel_raw_data->i16_raw_x," ");
 //  delay_cycles(1000);
   fnDEBUG_16bit_Int_Value(";  ", pst_accel_raw_data->i16_raw_y," ");
 //  delay_cycles(1000);
   fnDEBUG_16bit_Int_Value("; ",pst_accel_raw_data->i16_raw_z," ");
  // delay_cycles(1000);
   //fnDEBUG_8bit_Value("; ",st_system_status.st_mag_data.u8_temp,"\r\n");
   fnDEBUG_8bit_Value("; ",st_system_status.i8_temperature,"\r\n");

   //fnINPUT_ANALOGIC_Get_Battery(&st_system_status.u8_battery);
   //fnDEBUG_8bit_Value("bat = ",st_system_status.u8_battery,"\r\n");

   /*
   if (fnINPUT_Get_TX_Activity()){
	   // fnDEBUG_Const_String("transmitindo\r\n\r\n\r\n");
		return;
   }
   if (   Get_Tx_Activity_bySoftware()){
	   //fnDEBUG_Const_String("transmitindo por software\r\n\r\n\r\n");
	   Dec_Tx_Activity();
	   return;
   }
   */

   //fnDEBUG_8bit_Value("counter = ", u8_state_counter, " \r\n");
   /*
   fnDEBUG_16bit_Int_Value("media (x,y,z): ", st_system_status.st_steady_mag_data.i16_raw_x ," ");

   fnDEBUG_16bit_Int_Value("; ", st_system_status.st_steady_mag_data.i16_raw_y ," ");

   fnDEBUG_16bit_Int_Value("; ", st_system_status.st_steady_mag_data.i16_raw_z ,"\r\n");
*/
/*
   fnDEBUG_16bit_Int_Value("avgsteady (x,y,z): ", st_steady_mag_axis.f_avg_x ," ");
   delay_cycles(1000);
   fnDEBUG_16bit_Int_Value("; ", st_steady_mag_axis.f_avg_y ," ");
   delay_cycles(1000);
   fnDEBUG_16bit_Int_Value("; ", st_steady_mag_axis.f_avg_z ,"\r\n");
   delay_cycles(1000);
   fnDEBUG_16bit_Int_Value("sysdetected (x,y,z): ", st_system_status.st_detected_mag_data.i16_raw_x ," ");
   delay_cycles(1000);
   fnDEBUG_16bit_Int_Value("; ", st_system_status.st_detected_mag_data.i16_raw_y ," ");
   delay_cycles(1000);
   fnDEBUG_16bit_Int_Value("; ", st_system_status.st_detected_mag_data.i16_raw_z ,"\r\n");
   delay_cycles(1000);
  */ 
   
   if( b_mag_detected == false && ( u8_state_counter == 0 ) && fnSENSORS_Has_Steady_Threshold( pst_accel_raw_data ) ) {

      u8_state_detected_counter++;
      if (u8_state_detected_counter >= st_system_status.u8_numAmostrasAcimaDoLimiarParaDeteccao){
        b_mag_detected = true;
        fnDEBUG_Const_String("DETECTED\r\n");
	     u8_state_counter = 33;

		  temperaturaDeteccao = fnAVERAGE_Get_Value_8bits(&st_temp_average_desc); 
	      magnetometroSteadyDeteccao.f_avg_x = st_steady_mag_axis.f_avg_x;
		  magnetometroSteadyDeteccao.f_avg_y = st_steady_mag_axis.f_avg_y;
		  magnetometroSteadyDeteccao.f_avg_z = st_steady_mag_axis.f_avg_z;
		  correcaoAnteriorx = 0;
		  correcaoAnteriory = 0;
		  correcaoAnteriorz = 0;
	  }
   
    } else{ if( b_mag_detected == true && ( u8_state_counter == 0) && fnSENSORS_Has_Detection_Stopped()  ) {
				 
                 u8_state_detected_counter = 0;
                 b_mag_detected = false;
                 fnDEBUG_Const_String("STEADY\r\n");
                 u8_state_counter = 0; //37
            }
            else{ 
	            if ( b_mag_detected == false && ( u8_state_counter == 0 ) && (fnSENSORS_Has_Steady_Threshold( pst_accel_raw_data ) == false) ) {
	                u8_state_detected_counter = 0; 
		        }
	            //fnDEBUG_Const_String(" LIMBO\n\r");
            }
   
   }
   if( b_mag_detected == false ) {
		 
      float f_avg_value;
      
	  //fnDEBUG_Const_String("AVG STEADY\n\r");
	  //fnDEBUG_8bit_Value(" temp = ", pst_accel_raw_data->i8_temp , " ");
	  //fnDEBUG_8bit_Value(" tempmedio = ", fnAVERAGE_Get_Value_8bits(&st_temp_average_desc) , "\r\n");
	  
	  if (!fnSENSORS_Has_Steady_Threshold( pst_accel_raw_data ) ) {
       f_avg_value = (float)( pst_accel_raw_data->i16_raw_x + ( (st_system_status.u16_numAmostrasMediaVagaVazia - 1) *  st_steady_mag_axis.f_avg_x ) )/st_system_status.u16_numAmostrasMediaVagaVazia;
	    if (b_calibrating == false){
		  // ajustando para a variacao de temperatura
		  // 
	       if (abs(st_system_status.st_steady_calibration_mag_data_2.i8_temp - fnAVERAGE_Get_Value_8bits(&st_temp_average_desc)) >= 2 ){
		       st_system_status.st_steady_calibration_mag_data_2.i16_raw_x += ((st_system_status.st_steady_mag_data.i16_raw_x - st_system_status.st_steady_calibration_mag_data_2.i16_raw_x));
			   st_system_status.st_steady_calibration_mag_data_2.i16_raw_y += ((st_system_status.st_steady_mag_data.i16_raw_y - st_system_status.st_steady_calibration_mag_data_2.i16_raw_y));
			   st_system_status.st_steady_calibration_mag_data_2.i16_raw_z += ((st_system_status.st_steady_mag_data.i16_raw_z - st_system_status.st_steady_calibration_mag_data_2.i16_raw_z));
			   st_system_status.st_steady_calibration_mag_data_2.i8_temp = fnAVERAGE_Get_Value_8bits(&st_temp_average_desc);

			   //fnDEBUG_16bit_Int_Value("CALIBRACAO_REF (magx;magy;magz,temp) = ",st_system_status.st_steady_calibration_mag_data_2.i16_raw_x," ");
			   //fnDEBUG_16bit_Int_Value("; ",st_system_status.st_steady_calibration_mag_data_2.i16_raw_y," ");
			   //fnDEBUG_16bit_Int_Value("; ",st_system_status.st_steady_calibration_mag_data_2.i16_raw_z," ");
			   //fnDEBUG_8bit_Value("; ",st_system_status.st_steady_calibration_mag_data_2.i8_temp,"\r\n");
			  
		   }
		   else{
			  // se estiver na mesma temperatura da calibracao original, retorna a media original
			   if (abs(st_system_status.st_steady_calibration_mag_data_2.i8_temp - st_system_status.st_steady_calibration_mag_data.i8_temp) <= 1 ){
     		      st_system_status.st_steady_calibration_mag_data_2.i16_raw_x = st_system_status.st_steady_calibration_mag_data.i16_raw_x;
	     	      st_system_status.st_steady_calibration_mag_data_2.i16_raw_y = st_system_status.st_steady_calibration_mag_data.i16_raw_y;
		          st_system_status.st_steady_calibration_mag_data_2.i16_raw_z = st_system_status.st_steady_calibration_mag_data.i16_raw_z;
			  }
			  
		   }

	     //vendo se esta dentro dos limites permitidos
	     if ((f_avg_value < (st_system_status.st_steady_calibration_mag_data_2.i16_raw_x + st_system_status.u8_emptyParkAmplitudeControl)) && 
	        (f_avg_value > (st_system_status.st_steady_calibration_mag_data_2.i16_raw_x - st_system_status.u8_emptyParkAmplitudeControl)) ){
		     st_steady_mag_axis.f_avg_x = f_avg_value;  // pode alterar o valor da mï¿½dia
	     }
	     //else{
		    // fnDEBUG_Const_String("RECALCULO MEDIA X BLOQUEADA\n\r");
			//se a temperatura != temperatura da calibracao --> 
	     //}
	  }
	  else{
         st_steady_mag_axis.f_avg_x = f_avg_value;		  
	  }
	  
	  

      
      f_avg_value = (float) ( pst_accel_raw_data->i16_raw_y + ( (st_system_status.u16_numAmostrasMediaVagaVazia - 1) *  st_steady_mag_axis.f_avg_y ) )/st_system_status.u16_numAmostrasMediaVagaVazia;
  	   if (b_calibrating == false){
	
		  //vendo se esta dentro dos limites permitidos
		  if ((f_avg_value < (st_system_status.st_steady_calibration_mag_data_2.i16_raw_y + st_system_status.u8_emptyParkAmplitudeControl)) &&
		  (f_avg_value > (st_system_status.st_steady_calibration_mag_data_2.i16_raw_y - st_system_status.u8_emptyParkAmplitudeControl)) ){
			  st_steady_mag_axis.f_avg_y = f_avg_value;  // pode alterar o valor da media
		  }
		  //else{
			//  fnDEBUG_Const_String("RECALCULO MEDIA Y BLOQUEADA\n\r");
		  //}
	   }
	   else{
		  st_steady_mag_axis.f_avg_y = f_avg_value;
	   }
	  
      
      f_avg_value = (float) ( pst_accel_raw_data->i16_raw_z + ( (st_system_status.u16_numAmostrasMediaVagaVazia-1) *  st_steady_mag_axis.f_avg_z ) )/st_system_status.u16_numAmostrasMediaVagaVazia;
      if (b_calibrating == false){
		  
		  //vendo se esta dentro dos limites permitidos
		  if ((f_avg_value < (st_system_status.st_steady_calibration_mag_data_2.i16_raw_z + st_system_status.u8_emptyParkAmplitudeControl)) &&
		  (f_avg_value > (st_system_status.st_steady_calibration_mag_data_2.i16_raw_z - st_system_status.u8_emptyParkAmplitudeControl)) ){
			  st_steady_mag_axis.f_avg_z = f_avg_value;  // pode alterar o valor da media
		  }
		  //else{
		  // 	  fnDEBUG_Const_String("RECALCULO MEDIA Z BLOQUEADA\n\r");
		  //}
	  }
	  else{
		  st_steady_mag_axis.f_avg_z = f_avg_value;
	  }
	  
   
	  }
   } 
   else { // detection
	   b_mag_detected = true;
      //fnDEBUG_Const_String("AVG DETECTION\n\r");
	   int8_t deltatemperatura = temperaturaDeteccao - fnAVERAGE_Get_Value_8bits(&st_temp_average_desc);

	   /*
       if (deltatemperatura < 0){
			fnDEBUG_8bit_Value(" deltatemp = -", abs(deltatemperatura), ";");
		}	
		else{
		   fnDEBUG_8bit_Value(" deltatemp = ", abs(deltatemperatura), ";");
		}
		fnDEBUG_8bit_Value(" tempdetecao = ", temperaturaDeteccao, ";");
		fnDEBUG_8bit_Value(" temp = ", pst_accel_raw_data->i8_temp , " ");
		fnDEBUG_8bit_Value(" tempmedio = ", fnAVERAGE_Get_Value_8bits(&st_temp_average_desc) , "\r\n");
*/
	    volatile int16_t correcaox = 0;
	    volatile int16_t correcaoy = 0;
	    volatile int16_t correcaoz = 0;
	    volatile int16_t magdtx = 0;
	    volatile int16_t magdty = 0;
	    volatile int16_t magdtz = 0;

	  if (u8_state_counter > 0){
		  magdeteccaox = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_x_average_desc ); //st_detection_mag_axis.st_x_average_desc.x_data_sum;
		  magdeteccaoy = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_y_average_desc ); //st_detection_mag_axis.st_y_average_desc.x_data_sum;
		  magdeteccaoz = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_z_average_desc ); //st_detection_mag_axis.st_z_average_desc.x_data_sum;
		  correcaoAnteriorx = 0;
		  correcaoAnteriory = 0;
		  correcaoAnteriorz = 0;
	  }
	  else{  //a
        if (abs(deltatemperatura) < 1 ){ 
		  // nao faz nada enquanto a temperatura estiver dentro da faixa aceitavel de variacao
		    magdeteccaox = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_x_average_desc ); //st_detection_mag_axis.st_x_average_desc.x_data_sum;
		    magdeteccaoy = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_y_average_desc ); //st_detection_mag_axis.st_y_average_desc.x_data_sum;
		    magdeteccaoz = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_z_average_desc ); //st_detection_mag_axis.st_z_average_desc.x_data_sum;
		    correcaoAnteriorx = 0;
		    correcaoAnteriory = 0;
		    correcaoAnteriorz = 0;

          magdtx = magdeteccaox;
	       magdty = magdeteccaoy;
	       magdtz = magdeteccaoz;
	     }
	     else{//b
		    magdtx = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_x_average_desc );
		    magdty = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_y_average_desc );
		    magdtz = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_z_average_desc );
		    correcaox = magdtx - magdeteccaox;
		    correcaoy = magdty - magdeteccaoy;
		    correcaoz = magdtz - magdeteccaoz;  

	      if (correcaox >= 4){
            correcaox = 4;
		   } 
		   else{
		     if (correcaox <= -4){
			    correcaox = -4;
			  }
		   }

	      if (correcaoy >= 4){
		     correcaoy = 4;
	      }
	      else{
		     if (correcaoy <= -4){
			    correcaoy = -4;
		     }
	      }
	      if (correcaoz >= 4){
		     correcaoz = 4;
	      }
	      else{
		     if (correcaoz <= -4){
			    correcaoz = -4;
		     }
	      }


         //-------

           if (abs(pst_accel_raw_data->i16_raw_x - magdtx) >= st_system_status.u8_sensibilityToEmptyDueTemperature || abs(pst_accel_raw_data->i16_raw_y - magdty) >= st_system_status.u8_sensibilityToEmptyDueTemperature  ){			  
	          correcaox = correcaoAnteriorx;
	          correcaoy = correcaoAnteriory;
	          correcaoz = correcaoAnteriorz;
//	          fnDEBUG_Const_String(" mantem correcao\r\n");
			
			    b_mag_detected = true;
                fnDEBUG_Const_String("DETECTED 2\r\n");
			    u8_state_counter = 23;
			    temperaturaDeteccao = fnAVERAGE_Get_Value_8bits(&st_temp_average_desc); //st_system_status.st_steady_mag_data.i8_temp;
			    magnetometroSteadyDeteccao.f_avg_x = st_steady_mag_axis.f_avg_x;
			    magnetometroSteadyDeteccao.f_avg_y = st_steady_mag_axis.f_avg_y;
			    magnetometroSteadyDeteccao.f_avg_z = st_steady_mag_axis.f_avg_z;
			    correcaoAnteriorx = 0;
			    correcaoAnteriory = 0;
			    correcaoAnteriorz = 0;
           }
		     else{
			    correcaoAnteriorx = correcaox;
			    correcaoAnteriory = correcaoy;
			    correcaoAnteriorz = correcaoz;
		     }

		// -----------------------------------

		   if (abs(deltatemperatura) > 3 ){
			 // if (abs(correcaox) || abs(correcaoy) || abs(correcaoz)){
             if (abs(correcaox) || abs(correcaoy)){				 
            	   temperaturaDeteccao = fnAVERAGE_Get_Value_8bits(&st_temp_average_desc);
                   magdeteccaox = magdtx;
     		       magdeteccaoy = magdty;
     		       magdeteccaoz = magdtz;
			   
                  st_steady_mag_axis.f_avg_x = magnetometroSteadyDeteccao.f_avg_x + (correcaox * 1.0); //sobe a media de vaga vazia
		          st_steady_mag_axis.f_avg_y = magnetometroSteadyDeteccao.f_avg_y + (correcaoy * 1.0); //sobe a media de vaga vazia
		          st_steady_mag_axis.f_avg_z = magnetometroSteadyDeteccao.f_avg_z + (correcaoz * 1.0);

		          magnetometroSteadyDeteccao.f_avg_x = st_steady_mag_axis.f_avg_x;
		          magnetometroSteadyDeteccao.f_avg_y = st_steady_mag_axis.f_avg_y;
		          magnetometroSteadyDeteccao.f_avg_z = st_steady_mag_axis.f_avg_z;
			   
			  
			       correcaox = 0;
			       correcaoy = 0;
			       correcaoz = 0;
			   
			     }
		   }

		   //------------------
		}


	  }
	

     st_steady_mag_axis.f_avg_x = magnetometroSteadyDeteccao.f_avg_x + (correcaox * 1.0); //sobe a media de vaga vazia
	  st_steady_mag_axis.f_avg_y = magnetometroSteadyDeteccao.f_avg_y + (correcaoy * 1.0); //sobe a media de vaga vazia
	  st_steady_mag_axis.f_avg_z = magnetometroSteadyDeteccao.f_avg_z + (correcaoz * 1.0);
	  /*
	  fnDEBUG_16bit_Int_Value("magDetection = ", magdeteccaox , ";" );
	  fnDEBUG_16bit_Int_Value(" ",  magdeteccaoy , ";" );
	  fnDEBUG_16bit_Int_Value(" ",  magdeteccaoz , "\r\n" );

      delay_cycles(1000);
	  fnDEBUG_16bit_Int_Value("magdt = ", magdtx , ";" );
	  fnDEBUG_16bit_Int_Value(" ",  magdty , ";" );
	  fnDEBUG_16bit_Int_Value(" ",  magdtz , "\r\n" );
	  
	     delay_cycles(1000);
	  fnDEBUG_16bit_Int_Value("correcaoAnterior = ", correcaoAnteriorx, ";" );
	  fnDEBUG_16bit_Int_Value(" ", correcaoAnteriory , ";" );
	  fnDEBUG_16bit_Int_Value(" ", correcaoAnteriorz , "\r\n" );
		   delay_cycles(1000);
	     delay_cycles(1000);
     fnDEBUG_16bit_Int_Value("correcao = ", correcaox, ";" );
     fnDEBUG_16bit_Int_Value(" ", correcaoy , ";" );
     fnDEBUG_16bit_Int_Value(" ", correcaoz , "\r\n" );
		   
      fnDEBUG_16bit_Int_Value("magSteadyDetecao = ", magnetometroSteadyDeteccao.f_avg_x , ";" );
      fnDEBUG_16bit_Int_Value(" ",  magnetometroSteadyDeteccao.f_avg_y , ";" );
	  fnDEBUG_16bit_Int_Value(" ",  magnetometroSteadyDeteccao.f_avg_z , "\r\n" );
		   delay_cycles(1000);
	  fnDEBUG_16bit_Int_Value("mediaSteadyCorrigida = ", st_steady_mag_axis.f_avg_x , ";" );
	  fnDEBUG_16bit_Int_Value(" ",  st_steady_mag_axis.f_avg_y , ";" );
	  fnDEBUG_16bit_Int_Value(" ",  st_steady_mag_axis.f_avg_z , "\r\n" );
	     delay_cycles(1000);
	  fnDEBUG_Const_String("\r\n");
	  */

      fnAVERAGE_Update( &st_detection_mag_axis.st_x_average_desc, pst_accel_raw_data->i16_raw_x );
      fnAVERAGE_Update( &st_detection_mag_axis.st_y_average_desc, pst_accel_raw_data->i16_raw_y );
      fnAVERAGE_Update( &st_detection_mag_axis.st_z_average_desc, pst_accel_raw_data->i16_raw_z );
   }          
   
   st_system_status.st_steady_mag_data.i16_raw_x = roundf( st_steady_mag_axis.f_avg_x );
   st_system_status.st_steady_mag_data.i16_raw_y = roundf( st_steady_mag_axis.f_avg_y );
   st_system_status.st_steady_mag_data.i16_raw_z = roundf( st_steady_mag_axis.f_avg_z );
   st_system_status.st_steady_mag_data.i8_temp = fnAVERAGE_Get_Value_8bits(&st_temp_average_desc);

   st_system_status.st_detected_mag_data.i16_raw_x = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_x_average_desc );
   st_system_status.st_detected_mag_data.i16_raw_y = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_y_average_desc );
   st_system_status.st_detected_mag_data.i16_raw_z = fnAVERAGE_Get_Value( &st_detection_mag_axis.st_z_average_desc );
#endif
   return;
}   


void fnSENSORS_Accel_Mag_Data_Ready ( st_fxos8700cq_raw_data_t * pst_accel_data, st_fxos8700cq_raw_data_t * pst_mag_data ) {
   /*
   static uint8_t u8_debounce = 0;
   bool b_curr_accel_detected = false;
      
   fnFXOS8700CQ_Read_Accel_Mag_Raw_Data ( pst_accel_data, pst_mag_data );

   uint32_t u32_vector_accel_pow;
   int32_t i32_axys_pow;
   
   i32_axys_pow = st_system_status.st_accel_data.i16_raw_x - st_system_status.st_accel_ref.i16_raw_x; 
   u32_vector_accel_pow = i32_axys_pow * i32_axys_pow;
   
   i32_axys_pow = st_system_status.st_accel_data.i16_raw_y - st_system_status.st_accel_ref.i16_raw_y;
   u32_vector_accel_pow += i32_axys_pow * i32_axys_pow;
   
   i32_axys_pow = st_system_status.st_accel_data.i16_raw_z - st_system_status.st_accel_ref.i16_raw_z;
   u32_vector_accel_pow += i32_axys_pow * i32_axys_pow;
   
   b_curr_accel_detected = u32_vector_accel_pow > u32_sensivity_table[ st_system_status.u8_sensor_sensivity ];     
   
   if( b_accel_detected != b_curr_accel_detected ) {
      u8_debounce++;
      if( u8_debounce >= ( b_accel_detected ? 6: 2 ) ) {
         b_accel_detected = b_curr_accel_detected;
         u8_debounce = 0;
      }
   } else {
      u8_debounce = 0;
   }
   
   st_system_status.st_accel_ref.i16_raw_x = st_system_status.st_accel_data.i16_raw_x;         
   st_system_status.st_accel_ref.i16_raw_y = st_system_status.st_accel_data.i16_raw_y;
   st_system_status.st_accel_ref.i16_raw_z = st_system_status.st_accel_data.i16_raw_z;
   */
   return;
}

void fnSENSORS_Set_Accel_Sensivity ( uint8_t u8_new_horimetro_sensivity ) {
//TODO: esta funcao
   if( u8_sensor_sensivity != u8_new_horimetro_sensivity ) {
      
     // fnACCELOMETER_Vector_Magnitude_Set_Sensivity ( u8_new_horimetro_sensivity );
      u8_sensor_sensivity = u8_new_horimetro_sensivity;
   
   }
   
   return;
}

bool fnSENSORS_Has_Detection ( void ) {
   return b_mag_detected;
}

bool fnSENSORS_Check_Magnetometer_Valid_Threshold ( void ) {
//TODO: esta funcao
   uint8_t u8_mag_threshold_source = 0;

   //fnMAGNETOMETER_Read_Mag_Threshold_Data ( &u8_mag_threshold_source );

   return ( u8_mag_threshold_source > 1 );
}


bool fnSENSORS_Is_Mag_Calibrating ( void ) {

   return b_calibrating;
   
}


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/

void fnSENSORS_Check_Data ( void ) {
	//TODO: esta funcao  --> aparentemente dá pra viver sem ela
/*
   if (( un_system_flags.flag.fxos_1_int_data_ready == false ) && ( fnINPUT_Get_S1_EXTINT1 ( ) == 0 )) {
      un_system_flags.flag.fxos_1_int_data_ready = true;

   }

   if (( un_system_flags.flag.fxos_1_int_threshold == false ) && ( fnINPUT_Get_S1_EXTINT2 ( ) == 0 )) {
      un_system_flags.flag.fxos_1_int_threshold = true;
   }
*/

   return;
}


bool fnSENSORS_Has_Steady_Threshold ( st_fxos8700cq_raw_data_t * pst_accel_raw_data ) {
  
   uint8_t u8_counter = 0;
   
   uint8_t u8_thr = st_system_status.u8_sensibilityToDetection;
   
   if (st_system_status.u8_mag_enable  & MAGX_ENABLE){
      if( abs( st_system_status.st_steady_mag_data.i16_raw_x - pst_accel_raw_data->i16_raw_x ) >= u8_thr ) {
         u8_counter++;
      }
   }

   if (st_system_status.u8_mag_enable  & MAGY_ENABLE){
      if( abs( st_system_status.st_steady_mag_data.i16_raw_y - pst_accel_raw_data->i16_raw_y ) >= u8_thr ) {
         u8_counter++;
      }
   }

   if (st_system_status.u8_mag_enable  & MAGZ_ENABLE){
      if( abs( st_system_status.st_steady_mag_data.i16_raw_z - pst_accel_raw_data->i16_raw_z ) >= u8_thr ) {
         u8_counter++;
      }
   }

   return ( u8_counter > 0 );

}   

bool fnSENSORS_Has_Detection_Stopped ( void ) {
   
   uint8_t u8_counter = 0;
   uint8_t mag_enable = 0;

   uint8_t u8_ret = st_system_status.u8_sensibilityToEmpty; 
   
   if (st_system_status.u8_mag_enable  & MAGX_ENABLE){
	   mag_enable++;
      if( abs( st_system_status.st_detected_mag_data.i16_raw_x - st_system_status.st_steady_mag_data.i16_raw_x ) <= u8_ret ) {
         u8_counter++;
      }
   }

   if (st_system_status.u8_mag_enable  & MAGY_ENABLE){
	   mag_enable++;
      if( abs( st_system_status.st_detected_mag_data.i16_raw_y - st_system_status.st_steady_mag_data.i16_raw_y ) <= u8_ret ) {
         u8_counter++;
      }
   }


   if (st_system_status.u8_mag_enable  & MAGZ_ENABLE){   
	   mag_enable++;
      if( abs( st_system_status.st_detected_mag_data.i16_raw_z - st_system_status.st_steady_mag_data.i16_raw_z ) <= u8_ret ) {
         u8_counter++;
      }
   }

   return ( u8_counter >= mag_enable);
          
} 

// Retorna: true se o dado RAW esta proximo do valor da calibracao original
//        : false caso contrario

bool fnSENSORS_Confirm_Calibration_Detection_Status ( void ) {
	
	uint8_t u8_counter = 0;
   uint8_t deltat = 3;


	deltat = deltat + (2 * (st_system_status.u8_deltat_auto_correction & 0x03)  ); // 3, 5, 7, 9
	uint8_t u8_ret = st_system_status.u8_sensibilityToEmpty;

	
	if (abs(st_system_status.st_steady_calibration_mag_data.i8_temp - st_system_status.st_mag_data.i8_temp) <= deltat ){
		
	   if (st_system_status.u8_mag_enable  & MAGX_ENABLE){		
		   //mag_enable++;
	      if( abs( st_system_status.st_steady_calibration_mag_data.i16_raw_x - st_system_status.st_mag_data.i16_raw_x ) <= u8_ret ) {
		     u8_counter |= MAGX_ENABLE; // x position
	      }
	   }

	   if (st_system_status.u8_mag_enable  & MAGY_ENABLE){
		 //mag_enable++;		   
	     if( abs( st_system_status.st_steady_calibration_mag_data.i16_raw_y - st_system_status.st_mag_data.i16_raw_y ) <= u8_ret ) {
		    u8_counter |= MAGY_ENABLE; //y position
	     }
	   }
	
	   if (st_system_status.u8_mag_enable  & MAGZ_ENABLE){
		   //mag_enable++;		   
	     if( abs( st_system_status.st_steady_calibration_mag_data.i16_raw_z - st_system_status.st_mag_data.i16_raw_z ) <= u8_ret ) {
		    u8_counter |= MAGZ_ENABLE; 
	     }
	   }
	}

	return ( u8_counter == st_system_status.u8_mag_enable );
	
}


void fnSENSORS_b_Mag_Detected_Set ( bool status ){

	if (u8_state_counter <=5){
	  b_mag_detected = status;
     u8_state_counter = 5; // efetua uma recalculo na media de vaga vazia por N amostras

     st_steady_mag_axis.f_avg_x = st_system_status.st_steady_calibration_mag_data.i16_raw_x;
	  st_steady_mag_axis.f_avg_y = st_system_status.st_steady_calibration_mag_data.i16_raw_y;
	  st_steady_mag_axis.f_avg_z = st_system_status.st_steady_calibration_mag_data.i16_raw_z;
	}

}

/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
