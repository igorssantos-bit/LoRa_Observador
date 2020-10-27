/*!
 * \file      project_main.c
 *
 * \brief     Works with Lora states, some Lora configurations and the application
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

#include <accelerometer.h>
#include "main.h"

#include "utilities.h"
#include "board.h"
#include "gpio.h"

#include "state_machine.h"

#include "Commissioning.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "LmHandlerMsgDisplay.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "system_status.h"
#include "app.h"
#include "system_flags.h"
#include "debug.h"
#include "sx1276.h"
#include "sx1276-board.h"
#include "board-config.h"
#include "comm_sigmais.h"
#include "timestamp.h"
#include "rtc-board.h"
#include "app_state_machine.h"

#include "si7021-a20.h"
#include "ds18b20.h"

#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_EU868

#endif

/*!
 * LoRa States
 */
#define LORA_IDLE                                  0
#define LORA_PROCESSING                            1
#define LORA_TIMEOUT                               2

/*!
 * LoRaWAN default end-device class
 */
#define LORAWAN_DEFAULT_CLASS                       CLASS_A

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            5000

/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND                        1000

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE                           LORAMAC_HANDLER_ADR_OFF

/*!
 * Default datarate
 *
 * ATCBR: testes realizados
 *   DR_0 erro: no channel found
 *   DR_1 erro: no channel found
 *   DR_2 ok       spreading 10
 *   DR_3 ok       spreading 9   "delay": 0.048966169357299805,
 *   DR_4 falha: alta perda de pacotes
 *   DR_5 falha: alta perda de pacotes
 *   DR_6 erro: no channel found
 *
 * OBS: RSSI -118dB e SNR -13 está próximo do limiar máximo de transmissão
 * \remark Please note that LORAWAN_DEFAULT_DATARATE is used only when ADR is disabled 
 */

#define LORAWAN_DEFAULT_DATARATE                    DR_3
//#define LORAWAN_DEFAULT_DATARATE                    DR_2




/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            242 //32 // 32 = 4 * 8 (au8_downlink_frame) -> antes 242

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON                        false

/*!
 * LoRaWAN application port
 * @remark The allowed port range is from 1 up to 223. Other values are reserved.
 */
#define LORAWAN_APP_PORT                            2

volatile un_system_flags_t un_system_flags;
volatile st_system_status_t st_system_status;
volatile st_sigfox_events_t st_sigfox_events;
uint8_t au8_downlink_frame2[16];
uint8_t au8_downlink_frame[8];

/*!
 *
 */
typedef enum
{
	LORAMAC_HANDLER_TX_ON_TIMER,
	LORAMAC_HANDLER_TX_ON_EVENT,
}LmHandlerTxEvents_t;

/*!
 * User application data
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * User application data structure
 */
static LmHandlerAppData_t AppData =
{
		.Buffer = AppDataBuffer,
		.BufferSize = 0,
		.Port = 0
};


static void OnMacProcessNotify( void );
static void OnNvmContextChange( LmHandlerNvmContextStates_t state );
static void OnNetworkParametersChange( CommissioningParams_t* params );
static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq );
static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq );
static void OnJoinRequest( LmHandlerJoinParams_t* params );
static void OnTxData( LmHandlerTxParams_t* params );
static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params );
static void OnClassChange( DeviceClass_t deviceClass );
static void OnBeaconStatusChange( LoRaMAcHandlerBeaconParams_t* params );
void GetDeviceSN(void);

static LmHandlerCallbacks_t LmHandlerCallbacks =
{
		.GetBatteryLevel = BoardGetBatteryLevel,
		.GetTemperature = NULL,
		.GetUniqueId = BoardGetUniqueId,
		.GetRandomSeed = BoardGetRandomSeed,
		.OnMacProcess = OnMacProcessNotify,
		.OnNvmContextChange = OnNvmContextChange,
		.OnNetworkParametersChange = OnNetworkParametersChange,
		.OnMacMcpsRequest = OnMacMcpsRequest,
		.OnMacMlmeRequest = OnMacMlmeRequest,
		.OnJoinRequest = OnJoinRequest,
		.OnTxData = OnTxData,
		.OnRxData = OnRxData,
		.OnClassChange= OnClassChange,
		.OnBeaconStatusChange = OnBeaconStatusChange
};

static LmHandlerParams_t LmHandlerParams =
{
		.Region = ACTIVE_REGION,
		.AdrEnable = LORAWAN_ADR_STATE,
		.TxDatarate = LORAWAN_DEFAULT_DATARATE,
		.PublicNetworkEnable = LORAWAN_PUBLIC_NETWORK,
		.DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
		.DataBufferMaxSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE,
		.DataBuffer = AppDataBuffer
};

static LmhpComplianceParams_t LmhpComplianceParams =
{
		.AdrEnabled = LORAWAN_ADR_STATE,
		.DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
		.StopPeripherals = NULL,
		.StartPeripherals = NULL,
};

/*!
 * Indicates if LoRaMacProcess call is pending.
 * 
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

static volatile uint8_t IsTxFramePending = 0;

uint32_t timeOutMacBusy = 0;


//extern Gpio_t Led1;
/*!
 * Main application entry point.
 */
int ProjectMain( void )
{
	// Inicialização da placa e dos periféricos
	BoardInitMcu();
	RtcInit();
	RtcDelayMs(3000);

	// Inicialização da rede LoRa
	GetDeviceSN();
	LmHandlerInit( &LmHandlerCallbacks, &LmHandlerParams );
	LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams );
	LmHandlerJoin();

	// Inicia os processos utilizados na placa
	fnAPP_Init();

	IsMacProcessPending = 0;

	while( 1 )
	{

		LmHandlerProcess( );
		fnAPP_Process_Events();

		switch (IsMacProcessPending){

		// Modo normal: pode dormir o processador
		case LORA_IDLE:{
			if (!LmHandlerIsBusy()){
				if (st_system_status.u8_state_machine_state == APP_STATE_WAIT_TRANSMISSION){
					LpmEnterStopMode();
					LpmExitStopMode();
				}
			}
		}
		break;

		// Modo MAC process: não pode entrar em sleep
		case LORA_PROCESSING:{
			//CRITICAL_SECTION_BEGIN( );
		}
		break;

		// Modo Timeout: wait timeout
		case LORA_TIMEOUT:{
			if (fnTIMESTAMP_Get_Timestamp_Counter_Seconds() - timeOutMacBusy > 6){
				IsMacProcessPending = 0;

				// Reconfigure clock to low power
				HAL_RCC_DeInit();
				for (uint16_t atraso= 0; atraso < 10000; atraso++){
					__ASM volatile ("nop");
				}

				SystemClockConfig_MSI();
				for (uint16_t atraso= 0; atraso < 10000; atraso++){
					__ASM volatile ("nop");
				}
				BoardDeInitPeriph();
				BoardInitPeriph();

			}
		}
		break;

		// Modo erro
		default:{
			// goes to Mode Timeout
			IsMacProcessPending = 2;
			timeOutMacBusy = fnTIMESTAMP_Get_Timestamp_Counter_Seconds();
		}
		break;
		}
	}
}


/*
 * OnMacProcessNotify()
 * 		Sinaliza quando há um processo ocorrendo na camada MAC.
 * 		Percebeu-se que durante um processo de transmissão, com ou sem downlink,
 * 		ocorrem de duas (normalmente com downlink) a três (sem downlink) notificações MAC.
 * 		A primeira notificação ocorre logo após o processo de transmissão ser chamado, essa
 * 		notificação é do tipo MCPS REQUEST, e a segunda notificação ocorre entre 5 a 8s
 * 		após a primeira.
 * **/
static void OnMacProcessNotify( void )
{
	IsMacProcessPending = 1;
	fnDEBUG_Const_String("############### MAC process Notify  ###############\r\n");
	LmHandlerProcess();
}

static void OnNvmContextChange( LmHandlerNvmContextStates_t state )
{
	DisplayNvmContextChange( state );
	//fnDEBUG_Const_String("\r\n------ nvmcontextchange ---- \r\n");
}

static void OnNetworkParametersChange( CommissioningParams_t* params )
{
	DisplayNetworkParametersUpdate( params );
	//fnDEBUG_Const_String("\r\n------ networkparameterschange ---- \r\n");
}


/*
 * OnMacMcpsRequest()
 * 		Sinaliza que recebeu uma requisição de processamento na camada MAC
 * */
static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq )
{
	IsMacProcessPending = 1;
	fnDEBUG_Const_String("\r\n------ macmcpsrequest ---- \r\n");
	DisplayMacMcpsRequestUpdate( status, mcpsReq );

}

static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq )
{
	fnDEBUG_Const_String("\r\n------ MACMLMEREQUEST ---- \r\n");
	DisplayMacMlmeRequestUpdate( status, mlmeReq );

}

static void OnJoinRequest( LmHandlerJoinParams_t* params )
{
	//DisplayJoinRequestUpdate( params );
	if( params->Status == LORAMAC_HANDLER_ERROR )
	{
		LmHandlerJoin( );
	}
	else
	{
		LmHandlerRequestClass( LORAWAN_DEFAULT_CLASS );
	}
}

static void OnTxData( LmHandlerTxParams_t* params )
{
	fnDEBUG_Const_String("\r\n------ TRANSMITINDO ---- \r\n");
	DisplayTxUpdate( params );
	/*
	fnDEBUG_8bit_Hex("MCPS Confirm: 0x", params->IsMcpsConfirm, "\r\n");
	fnDEBUG_8bit_Hex("MAC Event Status: 0x", params->Status, "\r\n");
	fnDEBUG_8bit_Hex("Canal: 0x", params->Channel, "\r\n");
	fnDEBUG_8bit_Hex("DR: 0x", params->Datarate, "\r\n");
	fnDEBUG_8bit_Hex("TXPWR: 0x", params->TxPower, "\r\n");
	fnDEBUG_8bit_Hex("ACK de MSG: 0x", params->MsgType, "\r\n"); //0 sem confirmação
	fnDEBUG_8bit_Hex("ACK: 0x", params->AckReceived, "\r\n");
	fnDEBUG_32bit_Hex("Counter: 0x",params->UplinkCounter, "\r\n");
	 */
	fnDEBUG_Const_String("\r\n------ ------ ---------- \r\n");
	// Indicador de que a camada MAC finalizando a sua tarefa
	// Necessario temporizar para permitir a completa finalização
	IsMacProcessPending = 2;  //
	timeOutMacBusy = fnTIMESTAMP_Get_Timestamp_Counter_Seconds();


}

static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params )
{
	//DisplayRxUpdate2( appData, params );
	DisplayRxUpdate( appData, params );
	switch( appData->Port )
	{
	case 1:
	case LORAWAN_APP_PORT:
	{
		// PROCESSAR DADO QUE CHEGA
		st_sigfox_events.flag.b_downlink_frame_received = true;
		memcpy(au8_downlink_frame2,appData->Buffer,16);
		//1c05fa00120dae07 --> testar esse frame
		//au8_downlink_frame2[0] = 0x30; au8_downlink_frame2[0] = 0x;
		fnDEBUG_Const_String("RX = ");
		for (uint8_t cnt=0; cnt<8; cnt++){
			au8_downlink_frame[cnt] = fnConvert_Ascii_To_4bitHex (au8_downlink_frame2[cnt*2]) << 4;
			au8_downlink_frame[cnt] |= fnConvert_Ascii_To_4bitHex (au8_downlink_frame2[(cnt*2) + 1]);
			fnDEBUG_8bit_Hex("", au8_downlink_frame[cnt], " ");
		}
		fnDEBUG_Const_String("\r\n");
	}
	break;
	default:
		break;
	}

}

static void OnClassChange( DeviceClass_t deviceClass )
{
	DisplayClassUpdate( deviceClass );

	// Inform the server as soon as possible that the end-device has switched to ClassB
	LmHandlerAppData_t appData =
	{
			.Buffer = NULL,
			.BufferSize = 0,
			.Port = 0
	};
	LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
}

static void OnBeaconStatusChange( LoRaMAcHandlerBeaconParams_t* params )
{
	switch( params->State )
	{
	case LORAMAC_HANDLER_BEACON_RX:
	{
		//TimerStart( &LedBeaconTimer );
		break;
	}
	case LORAMAC_HANDLER_BEACON_LOST:
	case LORAMAC_HANDLER_BEACON_NRX:
	{
		//TimerStop( &LedBeaconTimer );
		break;
	}
	default:
	{
		break;
	}
	}

	DisplayBeaconUpdate( params );
}



/*
 * GetDeviceSN: Generates a DevEUI and a DevAddr automatically
 * */
void GetDeviceSN(void)
{
	uint32_t	dev_id_word0 = HAL_GetUIDw0();
	uint32_t	dev_id_word1 = HAL_GetUIDw1();
	uint32_t	dev_id_word2 = HAL_GetUIDw2();
	//007b00643068380f
	//3068380f
	CommissioningParams.DevEui[0] = (uint8_t)(dev_id_word2>>24);
	CommissioningParams.DevEui[1] = (uint8_t)(dev_id_word2>>16)+(dev_id_word0>>16);
	CommissioningParams.DevEui[2] = (uint8_t)(dev_id_word2>>8);
	CommissioningParams.DevEui[3] = (uint8_t)(dev_id_word2 + dev_id_word0);
	CommissioningParams.DevEui[4] = (uint8_t)(dev_id_word1>>24);
	CommissioningParams.DevEui[5] = (uint8_t)(dev_id_word1>>16)+(dev_id_word1>>8);
	CommissioningParams.DevEui[6] = (uint8_t)(dev_id_word1);
	CommissioningParams.DevEui[7] = (uint8_t)(dev_id_word0>>24);

	CommissioningParams.DevAddr = dev_id_word2;
	/*
	for (int cnt = 0; cnt < 4; cnt++){
	   CommissioningParams.DevAddr <<= 8;
	   CommissioningParams.DevAddr |= CommissioningParams.DevEui[cnt] ;
	}
	 */
}


