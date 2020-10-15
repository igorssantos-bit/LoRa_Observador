
//#include <asf.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "traffic.h"
#include "libs/services/debug.h"
//#include "libs/services/delay_loop_port.h"
//#include "board_config.h"
#include "sensors.h"
//#include "magnetometer.h"
#include "timestamp.h"
//#include "radio.h"
//#include "libs_firmware/protocols/sigmais/framesSigtraffic.h"
//#include "libs_firmware/protocols/sigmais/bitfield.h"
//#include "libs_firmware/util/string_utils.h"
//#include "radio.h"
#include "app_state_machine.h"
#include "system_flags.h"
#include "sensors.h"


//#include "app.h"
//#include "config.h"
//#include "system_flags.h"
#include "system_status.h"
//#include "system_power.h"

#define VALOR_MAXIMO_AMPLITUDE 250
/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/

// para o filtro de Kalman
//float KalmanGain = 1.0;
float xantX = 1.0;
float pantX = 1.0;
//float Qw = 0.5;
//float Qv = 15;                 // informação do sensor
float xantY = 1.0;
float pantY = 1.0;

float xantZ = 1.0;
float pantZ = 1.0;


float xantX_steady = 1.0;
float pantX_steady = 1.0;
//float Qw = 0.5;
//float Qv = 15;                 // informação do sensor
float xantY_steady = 1.0;
float pantY_steady = 1.0;

float xantZ_steady = 1.0;
float pantZ_steady = 1.0;


/* Main control loop: wait in low power mode until some event needs to be processed */
/*
[19:07, 4/6/2018] Marcelo Fassarella: O algoritimo tá quase pronto. A ideia geral é a seguinte:
* criar dois estados, um de carroDetectado e outro de aguardandoDetecção;
* calcular a derivada entre dois pontos sequencias;
* no estado aguardandoDetecção deve-se comparar o resultado com um valor pré-fixado, tipo 20 (exemplo);
* incrementar o contador nesse momento e alterar o estado para carroDetectado
* criar uma histeresse para sair do estado carroDetectado, exemplo aguarda reduzir a derivada para um valor menor do que 10;

Testei isso com os dados que você enviou e funcionou. Claro que os resultados dos testes com o carro trarão confiança no algoritmo e talvez melhorias, mas já é uma base. Assim o tempo de deteção será reduzido.
[19:08, 4/6/2018] Marcelo Fassarella: cont = 0
estado = 0 //aguardandoDeteção
for i in range(len(difabs)):
...   if estado == 0:
...     if difabs[i] > 20:
...       estado = 1  //carro detectado
...       cont = cont + 1
...   elif estado == 1:
...     if difabs[i] <= 10:
...       estado = 0
*/



/*
*  verificaProcessaEventoThreshold
*  Descrição: Verifica se houve um evento de threshold (imã) e se houver coloca o equipamento em modo de calibração.
*/
/*
  void verificaProcessaEventoThreshold(unsigned int numCalibracao){
	  if (un_system_flags.flag.fxos_1_int_threshold == true) {
		  fnMAGNETOMETER_Read_Raw_Data (( st_fxos8700cq_raw_data_t * ) &st_system_status.st_mag_data );
		  un_system_flags.flag.fxos_1_int_threshold = false;
		  fnDEBUG_Const_String("evento de threshold\r\n");
		  //fnDEBUG_8bit_Value("m_ths_src = ",fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_THS_SRC )," ");
		  //fnDEBUG_8bit_Value("int src = ",fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_INT_SRC )," ");
		  //fnDEBUG_8bit_Value("dr reg = ",fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_DR_STATUS )," ");
		  //fnDEBUG_8bit_Value("mag reg1 = ",fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_CTRL_REG1 )," ");
		  //fnDEBUG_8bit_Value("mag reg2 = ",fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_CTRL_REG2 )," ");
		  //fnDEBUG_8bit_Value("mag reg3 = ",fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_CTRL_REG3 )," ");
		  calibracao(numCalibracao);
	  }
  }
  
  */

/*
*  verificarRegistradores
*  Descrição: Verificar registradores por conta de bug detectado.
*             Retestar e confirmar se há o problema realmente, ou se foi algum outro erro durante os testes
*/
/*
void verificarRegistradores(void){
	   if (fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_INT_SRC ) > 0){
          fnMAGNETOMETER_Read_Raw_Data (( st_fxos8700cq_raw_data_t * ) &st_system_status.st_mag_data );
		  fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_INT_SRC );
		  fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_THS_SRC );
		  //fnDEBUG_8bit_Value("int src = ",fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_INT_SRC )," ");   
 		  //fnDEBUG_8bit_Value("m_ths_src = ",fnFXOS8700CQ_Read_Register(FXOS8700CQ_REG_M_THS_SRC )," ");   
		  
	   }
}
*/
/*
*  Entrada: 
*	       magnetometroValor: valor da leitura do magnetômetro. O mais adequado é que este valor venha filtrado.
*		   linhaDeCorte: valor que será usado como limiar de detecção.
*          pstEstado: ponteiro para o estado atual da detecção. 
*  Altera valor: 
*				pstEstado: ponteiro para o estado atual da contagem
*               pignoreSamples: irá manter o sensor  em modo detectado pelo número de leituras indicadas nessa variável
*                               Verificou-se que a influência da passagem de um veículo sobre o sensor dura de 200 a 350 amostras.
*                               Bons resultados foram obtidos com a variável pignoreSamples com valor de 200 amostras p/ amostragem de 800Hz. 
*			    
*  Retorna:
*		    1: se contou um veículo 
*		    0: caso não tenha contado
*
* OSB: Espera-se que o sinal comporte-se da seguinte forma:  ...00011110000....  ou  ... 0001< n_amostras = x >0000... ou 
*                                                            ... 0001< n_amostras = x >1100 ...
*                                                            onde zero representa um valor abaixo da linha de corte e
*                                                            um representa valores superiores da linha de corte. 
*      O eixo Z terá um sinal + e ao passar um veículo o sinal é reduzido, mas não chega a valores menores que zero.
* Modo de Instalação: 
*	 eixo Z fica vertical ao solo de forma que o sinal dele seja +.
* O algoritmo agora serve para qualquer um dos outros eixos também
**/

uint8_t algoritmoContagemEixo (int16_t magnetometroValor, int16_t valorReferencia, uint8_t *pEstado, uint16_t linhaDeCorteSuperior, uint16_t linhaDeCorteInferior ,uint16_t *pignoreSamples, uint8_t *pDetectadoCount){
	uint8_t contadorTrafego=0;
	uint16_t diferenca = 0;
	
	diferenca = abs (abs(magnetometroValor) - abs(valorReferencia));
	contadorTrafego = 0;
	if (diferenca > VALOR_MAXIMO_AMPLITUDE ){
	   (*pEstado) = ESTADO_ERRO_AMPLITUDE;
	   return contadorTrafego;
	}
	
	if  (*pEstado == ESTADO_AGUARDANDO_DETECCAO){
		if ( (*pignoreSamples) > 0){
			(*pignoreSamples)--;
			(*pDetectadoCount) = 0;							//reset no contador
		}
		else{
		  if (abs(linhaDeCorteSuperior) < abs(diferenca)){  
			//fnDELAY_LOOP_ms(2);  // original era sem delay
			(*pDetectadoCount) = (*pDetectadoCount) + 1;
			if (*pDetectadoCount >= st_system_status.u8_npsat){  //if (*pDetectadoCount >=  NUM_AMOSTRAS_COUNT){
			   contadorTrafego = 1;				
			   (*pEstado) = ESTADO_VEICULO_DETECTADO;
			}
		  }
		  else{
			contadorTrafego = 0;
		    (*pDetectadoCount) = 0;								//reset no contador
		  }
		}
	}
	else{
		if ( (*pignoreSamples) > 0){
			(*pignoreSamples)--;
    		(*pDetectadoCount) = 0;								//reset no contador
		}
		else{
			if ( abs(linhaDeCorteInferior) > abs(diferenca)){			//  linhaDeCorte/2 = linhaDeCorteInferior -> p/ criar uma  histerese para sair da detecção
			   (*pDetectadoCount) = (*pDetectadoCount) + 1;
			   if ( (*pDetectadoCount) >= st_system_status.u8_npsat){ //if ( (*pDetectadoCount) >= NUM_AMOSTRAS_COUNT){
				  (*pEstado) = ESTADO_AGUARDANDO_DETECCAO;
			   }
			}
			else{
    		   (*pDetectadoCount) = 0;							//reset no contador				
			}
		}
		contadorTrafego = 0;
	}
	
	return contadorTrafego;
}
/*
*  Entrada:
*	       magnetometroValor: valor da leitura do magnetômetro. O mais adequado é que este valor venha filtrado.
*		   linhaDeCorte: valor que será usado como limiar de detecção.
*          pstEstado: ponteiro para o estado atual da detecção.
*  Altera valor:
*				pstEstado: ponteiro para o estado atual da contagem
*
*  Retorna:
*		    1: se contou um veículo
*		    0: caso não tenha contado
*
* OSB: Espera-se que o sinal comporte-se da seguinte forma:  ...00011110000....  ,
*                                                            onde zero representa um valor abaixo da linha de corte e
*                                                            um representa valores superiores da linha de corte.
*      O eixo Z terá um sinal + e ao passar um veículo o sinal é reduzido, mas não chega a valores menores que zero.
* Modo de Instalação:
*	 eixo Z fica vertical ao solo de forma que o sinal dele seja +.
* O algoritmo agora serve para qualquer um dos outros eixos também
**/

uint8_t algoritmoContagemEixoXYZ (int16_t magX, int16_t referenciaX, int16_t magY, int16_t referenciaY, int16_t magZ, int16_t referenciaZ, uint8_t *pEstado, uint16_t linhaDeCorte){
	uint8_t contadorTrafego=0;
	int16_t diferencaX = 0;
	int16_t diferencaY = 0;
	int16_t diferencaZ = 0;
	int16_t diferenca = 0;
	
	diferencaX = abs(abs(magX) - abs(referenciaX));
	diferencaY = abs(abs(magY) - abs(referenciaY));
	diferencaZ = abs(abs(magZ) - abs(referenciaZ));
	diferenca = diferencaX + diferencaY + diferencaZ;
	
	if  ( (*pEstado) == ESTADO_AGUARDANDO_DETECCAO){
		if (abs(linhaDeCorte) < abs(diferenca)){
			contadorTrafego = 1;
			(*pEstado) = ESTADO_VEICULO_DETECTADO;
		}
	}
	else{
		if ( abs(linhaDeCorte) > abs(diferenca)){
			(*pEstado) = ESTADO_AGUARDANDO_DETECCAO;
			contadorTrafego = 0;
		}
	}

	return contadorTrafego;
}

uint8_t algoritmoContagemEixoXYZ_2(uint8_t contx, uint8_t conty, uint8_t contz, uint8_t estadox, uint8_t estadoy, uint8_t estadoz){

	  
	  if (contx && conty && contz){
		  return 1;
	  }
	  else{
    	  if (contx && (conty || estadoy) && (contz || estadoz)){
			  return 1;
		  }
		  else{
        	  if (conty && (contx || estadox) && (contz || estadoz)){
	        	  return 1;
    	      }
			  else{
		      	  if (contz && (conty || estadoy) && (contx || estadox)){
			      	  return 1;
		      	  }

			  }
			  
		  }

	  }
	  return 0;
	  
   
}

//ideia para algoritmos de contagem 3 e 4
// uint8_t algoritmoContagemEixoXYZ_3(amplitudeMaximaX, timeQOcorreuAmplitudeMaximaX, amplitudeMaximaY, timeQOcorreuAmplitudeMaximaY,amplitudeMaximaZ, timeQOcorreuAmplitudeMaximaZ)
// a implementação deverá verificar se a ocorrência da amplitude máxima acima de um threshold 2 nos tres eixos e se ela ocorreu mais ou menos no mesmo periodo.
// a ideia é que exista um threshold menor para considerar a contagem e depois, o algoritmo possa determinar se a amplitude máxima atingiu as expectativas reais ao comparar com o threshold2.
// ou 
// uint8_t algoritmoContagemEixoXYZ_3( timeQOcorreuAmplitudeMaximaX, amplitudeMaximaY, timeQOcorreuAmplitudeMaximaY,amplitudeMaximaZ, timeQOcorreuAmplitudeMaximaZ)
// numa versão simplificada o algoritmo pode apenas verificar se as amplitudes máximas ocorreram no mesmo tempo (colocar uma pequena margem de erro no tempo se necessário)
// TODO: capaturar o momento em que acontece as amplitudes máximas

/*
*  Calcula o valor medio de um buffer após a inserção de um novo valor e a exclusão do valor mais antigo.
*  Entrada: 
*        pbuff: ponteiro para o buffer onde estão armazenados os valores para o filtro
*        numAmostras: numero de amostras utilizado no filtro. Espera-se que esse número seja igual ao tamanho do buffer de amostras
*        valorNovo: novo valor a ser inserido no buffer de amostras e com o qual será calculado o novo valor médio
* 
*/
// passar para int16
int16_t filtroMedia( int16_t valorNovo, int16_t *pBuff, uint16_t numAmostras){
	int32_t valorMedio = 0;

     // Atualiza o buffer de amostras, sendo que 0 é o elemento mais antigo
	 // OBS: TROCAR ESSA MODO DE FAZER POR PONTEIROS PARA O ELEMENTO MAIS ANTIGO
	 // EVITANDO ASSIM PERDA DE TEMPO COM REALOCAÇÃO DE VALORES NO BUFFER
     for (uint16_t i = 0; i < (numAmostras - 1); i++){
		 pBuff[i] = pBuff[i+1];                                 // TROCAR CONFORME DESCRITO NA OBS ACIMA
		 valorMedio += pBuff[i];
	 }
	 pBuff[numAmostras-1] = valorNovo;
	 valorMedio += pBuff[numAmostras-1];
	 valorMedio /= numAmostras;
	 
	return valorMedio;
}

int16_t filtroMediaSt( int16_t valorNovo, st_traffic_buffer *pStBuffer){
	int32_t valorMedio = 0;


	pStBuffer->p16_bufAddress[pStBuffer->next_pos] = valorNovo;      //*(pStBuffer->p16_bufAddress + pStBuffer->next_pos) = valorNovo;
    for (uint16_t i = 0; i < pStBuffer->bufferSize; i++){
	   valorMedio += pStBuffer->p16_bufAddress[i];
    }
	valorMedio /= pStBuffer->bufferSize;
		
    pStBuffer->media = valorMedio;
	pStBuffer->next_pos = pStBuffer->next_pos + 1;
    if ( ( pStBuffer->next_pos % pStBuffer->bufferSize ) == 0 ){
	   pStBuffer->next_pos = 0;
    }	
    

	return valorMedio;
}

		 // algoritmo 1
		 /*
		 dadoMagAnterior.i16_raw_x = dadoMagNovo.i16_raw_x;
		 dadoMagAnterior.i16_raw_y = dadoMagNovo.i16_raw_y;
		 dadoMagAnterior.i16_raw_z = dadoMagNovo.i16_raw_z;
		 
		 dadoMagNovo.i16_raw_x = abs(st_system_status.st_mag_data.i16_raw_x);
		 dadoMagNovo.i16_raw_y = abs(st_system_status.st_mag_data.i16_raw_y);
		 dadoMagNovo.i16_raw_z = abs(st_system_status.st_mag_data.i16_raw_z);
		 if (estado == ESTADO_INICIAL){
    		 dadoMagAnterior.i16_raw_x = dadoMagNovo.i16_raw_x;
	    	 dadoMagAnterior.i16_raw_y = dadoMagNovo.i16_raw_y;
		     dadoMagAnterior.i16_raw_z = dadoMagNovo.i16_raw_z;
			 estado = ESTADO_AGUARDANDO_DETECCAO;
		 }
		 derivadaMag.i16_raw_x = dadoMagNovo.i16_raw_x - dadoMagAnterior.i16_raw_x;
		 derivadaMag.i16_raw_y = dadoMagNovo.i16_raw_y - dadoMagAnterior.i16_raw_y;
		 derivadaMag.i16_raw_z = dadoMagNovo.i16_raw_z - dadoMagAnterior.i16_raw_z;
		 
		 if (estado == ESTADO_AGUARDANDO_DETECCAO){
	        if (derivadaMag.i16_raw_y > SENSIBILIDADE_DETECCAO){
			  estado = ESTADO_VEICULO_DETECTADO;
			  contadorTrafego++;
			  fnDEBUG_32bit_Value("contador = ", contadorTrafego, " \r\n");
			}
		 }
		 else if (estado == ESTADO_VEICULO_DETECTADO){
	        if (derivadaMag.i16_raw_y < SENSIBILIDADE_LIBERACAO){
		        estado = ESTADO_AGUARDANDO_DETECCAO;
			}
		 }
		 */


  // Filtro de Kalman: retorna o valor corrigido
  float kalmanFilterAtualizar(float *pant, float *xant, float valorMedido, float Qw, float Qv){
	  float pprox = 0;
	  float xprox = 0;
	  //float Qw = 0.2; float Qv = 20;     -> rápido
	  //float Qw = 0.002; float Qv = 1500; -> lento
	  float KalmanGain = 0;
	  
	  //
	  //
	  //    Equação de Propagação
	  //    xprox(i)=A*xant(i-1);                     % do estado
	  //    pprox(i)=A*pant(i-1)*A + B*Qw*B;          % da covariancia do estado
	  //
	  //
	  xprox = (*xant);
	  pprox = (*pant) + Qw;
	  
	  //
	  //      Equações de Atualização:
	  //      K(i)=pprox(i)*C/(C*pprox(i)*C + Qv);       % do ganho de Kalman
	  //      xant(i)=xprox(i) + K(i)*(y(i)-C*xprox(i)); % do estado
	  //      pant(i)=(1 - K(i)*C)*pprox(i);             % da covariancia do estado
	  //
	  KalmanGain = pprox / (pprox + Qv);
	  (*xant) = xprox + KalmanGain * (valorMedido - xprox);
	  (*pant) = (1 - KalmanGain) * pprox;
	  
	  return *xant; // valor calculado
	  
  }

  
  /*
  *  filtraDadoMagnetometro
  *  Lê os dados do magnetômetro e efetua a filtragem do mesmo.
  *  Atualiza o valor medio em cada um dos eixos
  *  Retorna verdadeiro quando a leitura foi feita e falso caso contrário
  */
//TODO: esta função corretamente
  bool filtraDadoMagnetometro(int16_t *valorMedioX, int16_t *valorMedioY, int16_t *valorMedioZ){
#if 1
	 if ( un_system_flags.flag.fxos_1_int_data_ready == true){
		/* Clear flag */
 		//EIC->INTFLAG.reg = ( 1UL << ( FXOS8700_DATA_READY_EIC_LINE_1 % 32 ) );
		un_system_flags.flag.fxos_1_int_data_ready = false;
		/* lê os dados */
		//fnMAGNETOMETER_Read_Raw_Data (( st_fxos8700cq_raw_data_t * ) &st_system_status.st_mag_data );

		//fnDEBUG_32bit_Value("time = ", fnTIMESTAMP_Get_Interrupt_Counter_Timestamp(), " ; ");
		//fnDEBUG_16bit_Int_Value(" mag(x;y;z) = ",st_system_status.st_mag_data.i16_raw_x,";");
		//fnDEBUG_16bit_Int_Value("",st_system_status.st_mag_data.i16_raw_y,";");
		//fnDEBUG_16bit_Int_Value("",st_system_status.st_mag_data.i16_raw_z," ");
		//fnDEBUG_Const_String("\r\n");

		
		//valorMedio = filtroMedia(st_system_status.st_mag_data.i16_raw_z, &magnetometroBufferEixoZ, NUMERO_AMOSTRAS_BUFFER);
		(*valorMedioX) = (int16_t) kalmanFilterAtualizar(&pantX, &xantX, st_system_status.st_mag_data.i16_raw_x,0.2,20.0);
		(*valorMedioY) = (int16_t) kalmanFilterAtualizar(&pantY, &xantY, st_system_status.st_mag_data.i16_raw_y,0.2,20.0);
		(*valorMedioZ) = (int16_t) kalmanFilterAtualizar(&pantZ, &xantZ, st_system_status.st_mag_data.i16_raw_z,0.2,20.0);

		//fnDEBUG_16bit_Int_Value("valorMedio (X;Y;Z) = ", *valorMedioX, " ; ");
		//fnDEBUG_16bit_Int_Value("", *valorMedioY," ; ");
		//fnDEBUG_16bit_Int_Value("",  *valorMedioZ," \r\n");

		return true;
		         
	 }
	 else{
		 return false;
	 }
#endif
}
  
  /*
  * Processo de calibração
  * considera calibrado em qnt amostras
  */
  void calibracao(int16_t *valorRefX, int16_t *valorRefY, int16_t *valorRefZ, uint16_t qnt){
	  uint16_t i;
	  int16_t valorMedioX, valorMedioY, valorMedioZ = 0;
	  
	  xantX = 1.0;
	  pantX = 1.0;
	  xantY = 1.0;
	  pantY = 1.0;
	  xantZ = 1.0;
	  pantZ = 1.0;

	  fnDEBUG_Const_String("Calibrando\r\n");
	  
	  for (i = 0; i < qnt; i++){
		   if (fnLSM303_Mag_Ready((lsm303agr_ctx_t *) &dev_ctx_mg)){
		      // un_system_flags.flag.fxos_1_int_data_ready = false; // usado no caso da interrupcao
		      fnSENSORS_Mag_Data_Ready( ( st_fxos8700cq_raw_data_t * ) &st_system_status.st_mag_data );
		   }

		   if ( !filtraDadoMagnetometro(&valorMedioX,&valorMedioY,&valorMedioZ) ){
			   HAL_Delay(1);//delay_cycles(500);
		   }
		   //fnDEBUG_16bit_Int_Value("\r\ncontador = ", i , "\r\n");
  	  }
	  
	  (*valorRefX) = valorMedioX;
	  (*valorRefY) = valorMedioY;
	  (*valorRefZ) = valorMedioZ;
	  st_system_status.st_steady_mag_data.i16_raw_x = valorMedioX;
	  st_system_status.st_steady_mag_data.i16_raw_y = valorMedioY;
	  st_system_status.st_steady_mag_data.i16_raw_z = valorMedioZ;
	  // backup do xant e pant de cada eixo após a calibração
	  xantX_steady = xantX;
	  pantX_steady = pantX;
	  xantY_steady = xantY;
	  pantY_steady = pantY;	  
	  xantZ_steady = xantZ;
	  pantZ_steady = pantZ;	  
	  fnDEBUG_Const_String("Terminado\r\n");
#if 1
      fnDEBUG_16bit_Int_Value(" Valor Medio(x;y;z) = ",abs(valorMedioX),";");
      fnDEBUG_16bit_Int_Value("",abs(valorMedioY),";");
      fnDEBUG_16bit_Int_Value("",abs(valorMedioZ),"\r\n ");

#endif 	  
  }
  
  
  /*
  * Atualiza medias 
  * Mantém a media de calibração atualizada 
  */
  void atualizaMedias(int16_t *valorRefX, int16_t *valorRefY, int16_t *valorRefZ){

	st_system_status.st_steady_mag_data.i16_raw_x = (int16_t) kalmanFilterAtualizar(&pantX_steady, &xantX_steady, st_system_status.st_mag_data.i16_raw_x,0.0000002,15000.0);
	st_system_status.st_steady_mag_data.i16_raw_y = (int16_t) kalmanFilterAtualizar(&pantY_steady, &xantY_steady, st_system_status.st_mag_data.i16_raw_y,0.0000002,15000.0);
	st_system_status.st_steady_mag_data.i16_raw_z = (int16_t) kalmanFilterAtualizar(&pantZ_steady, &xantZ_steady, st_system_status.st_mag_data.i16_raw_z,0.0000002,15000.0);

    (*valorRefX) = st_system_status.st_steady_mag_data.i16_raw_x;
	(*valorRefY) = st_system_status.st_steady_mag_data.i16_raw_y;
	(*valorRefZ) = st_system_status.st_steady_mag_data.i16_raw_z;
#if 0
      fnDEBUG_16bit_Int_Value(" Valor Medio(x;y;z) = ",st_system_status.st_steady_mag_data.i16_raw_x,";");
      fnDEBUG_16bit_Int_Value("",st_system_status.st_steady_mag_data.i16_raw_y,";");
      fnDEBUG_16bit_Int_Value("",st_system_status.st_steady_mag_data.i16_raw_z,"\r\n ");

      fnDEBUG_16bit_Int_Value(" Valor RAW(x;y;z) = ",st_system_status.st_mag_data.i16_raw_x,";");
      fnDEBUG_16bit_Int_Value("",st_system_status.st_mag_data.i16_raw_y,";");
      fnDEBUG_16bit_Int_Value("",st_system_status.st_mag_data.i16_raw_z,"\r\n ");

#endif 	  
  }
  
/*
  * Atualiza media 
  * Mantém a media de calibração atualizada individualmente
  */
  void atualizaMedia(int16_t *valorRef, int16_t rawData, uint8_t eixo){
  float valorRaw = 0.0;
  
    valorRaw = (float)rawData * 1.0;
    if (eixo == 0){
	   (*valorRef) = (int16_t) kalmanFilterAtualizar(&pantX_steady, &xantX_steady, valorRaw,0.001,15000.0);
	}
	else{
		if (eixo == 1){
	       (*valorRef) = (int16_t) kalmanFilterAtualizar(&pantY_steady, &xantY_steady, valorRaw,0.001,15000.0);			
		}
		else{
			if (eixo == 2){
	            (*valorRef) = (int16_t) kalmanFilterAtualizar(&pantZ_steady, &xantZ_steady, valorRaw,0.001,15000.0);							
			}
		}
	}


  }  


/*
 * Zera todas as posições de zero a len de um buffer
  */

//void clearBuffer(uint8_t *pbuf, uint8_t len){
//
//  	for (uint8_t cont = 0; cont<len; cont++){
//  		pbuf[cont] = 0;
//  	}
//}

/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
