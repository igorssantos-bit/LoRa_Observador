/* project_main.c file */

#include "main.h"

#include "utilities.h"
#include "board.h"
#include "gpio.h"

#include "state_machine.h"

#include "Commissioning.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "CayenneLpp.h"
#include "LmHandlerMsgDisplay.h"
//#include "comm_sigmais.c"

#include "lsm303agr_reg.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "fxos8700cq.h"
#include "system_status.h"
#include "app.h"
#include "system_flags.h"
#include "debug.h"
#include "traffic.h"

#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_EU868

#endif

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
 * \remark Please note that LORAWAN_DEFAULT_DATARATE is used only when ADR is disabled 
 */
#define LORAWAN_DEFAULT_DATARATE                    DR_5

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            53

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

/* MAX_DELAY usado em uart1 */
//#define MAX_DELAY		1000

/* TX_BUF_DIM usado em uart1 */
#define TX_BUF_DIM      40

//static axis3bit16_t data_raw_acceleration;
//static axis3bit16_t data_raw_magnetic;
//static axis1bit16_t data_raw_temperature;
//static float acceleration_mg[3];
//static float magnetic_mG[3];
//static float temperature_degC;
//static uint8_t whoamI, rst;
//static uint8_t txBuffer[TX_BUF_DIM];

volatile un_system_flags_t un_system_flags;
volatile st_system_status_t st_system_status;
volatile lsm303agr_ctx_t dev_ctx_xl;
volatile lsm303agr_ctx_t dev_ctx_mg;
volatile st_sigfox_events_t st_sigfox_events;
uint8_t au8_downlink_frame2[16];
uint8_t au8_downlink_frame[8];

volatile uint8_t bufferTX[33]; //12 original

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
/*
static LmHandlerAppData_t AppData =
{
    .Buffer = AppDataBuffer,
    .BufferSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE,
    .Port = 0
};
 */
/*!
 * Specifies the state of the application LED
 */
//static bool AppLedStateOn = false;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxTimer;

/*!
 * Timer to handle the state of LED1
 */
static TimerEvent_t Led1Timer;

/*!
 * Timer to handle the state of LED2
 */
//static TimerEvent_t Led2Timer;

/*!
 * Timer to handle the state of LED3
 */
//static TimerEvent_t Led3Timer;

/*!
 * Timer to handle the state of LED beacon indicator
 */
static TimerEvent_t LedBeaconTimer;

static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC_Init(void);
void MX_GPIO_Init(void);


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

static void StartTxProcess( LmHandlerTxEvents_t txEvent );

/*!
 * Function executed on TxTimer event
 */
//static void OnTxTimerEvent( void* context );

/*!
 * Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent( void* context );

/*!
 * Function executed on Led 2 Timeout event
 */
//static void OnLed2TimerEvent( void* context );

/*!
 * \brief Function executed on Led 3 Timeout event
 */
//static void OnLed3TimerEvent( void* context );

/*!
 * \brief Function executed on Beacon timer Timeout event
 */
static void OnLedBeaconTimerEvent( void* context );

static int32_t accel_write(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);

static int32_t accel_read(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);

static int32_t magnet_write(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);

static int32_t magnet_read(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);

//static void readSensors( void );
/*
 *  Function to print messages
 */
//void tx_com( uint8_t *s, uint16_t len );

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

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1; // Tx
extern Gpio_t Led2; // Blinks every 5 seconds when beacon is acquired
extern Gpio_t Led3; // Rx
extern Gpio_t Led4; // App

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
ADC_HandleTypeDef hadc;

// traffic variables

uint16_t ignoreSamplesX = 80; // IGNORE_SAMPLES;
uint16_t ignoreSamplesY = 80; //IGNORE_SAMPLES;
uint16_t ignoreSamplesZ = 80; //IGNORE_SAMPLES;
uint8_t  waitNumAmostrasX = 0;
uint8_t  waitNumAmostrasY = 0;
uint8_t  waitNumAmostrasZ = 0;

uint32_t timestamp = 0;

st_fxos8700cq_raw_data_t dadoMagAnterior, dadoMagNovo, derivadaMag;
//uint8_t flagTX = false;

//int16_t magnetometroBufferEixoZ[NUMERO_AMOSTRAS_BUFFER];
int16_t valorMedioX = 0;
int16_t valorMedioY = 0;
int16_t valorMedioZ = 0;

int16_t amplitudeMaxX = 0;
int16_t amplitudeMaxY = 0;
int16_t amplitudeMaxZ = 0;

uint32_t periodoDetectadoX = 0;
uint32_t periodoDetectadoY = 0;
uint32_t periodoDetectadoZ = 0;



/*!
 * Main application entry point.
 */
int ProjectMain( void )
{
	uint16_t cont=0;
	//uint8_t contX, contY, contZ;

	BoardInitMcu( );
	MX_I2C1_Init( );
	MX_USART1_UART_Init( );
	MX_GPIO_Init();
	MX_ADC_Init();

	fnDEBUG_Const_String( "\r\n");
	fnDEBUG_Const_String( "\r\nSIGTRACK v0.0\r\n");

	TimerInit( &Led1Timer, OnLed1TimerEvent );
	TimerSetValue( &Led1Timer, 16 );


	TimerInit( &LedBeaconTimer, OnLedBeaconTimerEvent );
	TimerSetValue( &LedBeaconTimer, 5000 );

	/*
	 *  Initialize mems driver interface
	 */
	// accelerometer
	dev_ctx_xl.write_reg = accel_write;
	dev_ctx_xl.read_reg = accel_read;
	dev_ctx_xl.handle = (void*)LSM303AGR_I2C_ADD_XL;

	// magnetometer
	dev_ctx_mg.write_reg = magnet_write;
	dev_ctx_mg.read_reg = magnet_read;
	dev_ctx_mg.handle = (void*)LSM303AGR_I2C_ADD_MG;


	// traffic

	st_system_status.u32_cnt_trafegoX = 0;
	st_system_status.u32_cnt_trafegoY = 0;
	st_system_status.u32_cnt_trafegoZ = 0;
	st_system_status.u32_cnt_trafegoXYZ = 0;
	st_system_status.u8_traffic_reset_counters = 0;
	//
	LmHandlerInit( &LmHandlerCallbacks, &LmHandlerParams );

	// The LoRa-Alliance Compliance protocol package should always be
	// initialized and activated.
	LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams );

	LmHandlerJoin( );

	//StartTxProcess( LORAMAC_HANDLER_TX_ON_TIMER );

	fnAPP_Init();

	fnLsm303_Init(dev_ctx_xl, dev_ctx_mg);

	TimerStart( &Led1Timer );


	while( 1 )
	{

		//    fnINPUT_ANALOGIC_Get_Battery( &st_system_status.u8_battery);
		cont++;

		// fim traffic
		LmHandlerProcess( );


		fnAPP_Process_Events();

		CRITICAL_SECTION_BEGIN( );
		if( IsMacProcessPending == 1 )
		{
			// Clear flag and prevent MCU to go into low power modes.
			IsMacProcessPending = 0;
		}
		else
		{
			// The MCU wakes up through events
			BoardLowPowerHandler( );
		}
		CRITICAL_SECTION_END( );

	}
}


static void OnMacProcessNotify( void )
{
	IsMacProcessPending = 1;
}

static void OnNvmContextChange( LmHandlerNvmContextStates_t state )
{
	DisplayNvmContextChange( state );
}

static void OnNetworkParametersChange( CommissioningParams_t* params )
{
	DisplayNetworkParametersUpdate( params );
}

static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq )
{
	DisplayMacMcpsRequestUpdate( status, mcpsReq );
}

static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq )
{
	DisplayMacMlmeRequestUpdate( status, mlmeReq );
}

static void OnJoinRequest( LmHandlerJoinParams_t* params )
{
	DisplayJoinRequestUpdate( params );
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
	DisplayTxUpdate( params );
}

static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params )
{
	DisplayRxUpdate( appData, params );

	switch( appData->Port )
	{
	case 1:
	case LORAWAN_APP_PORT:
	{
		//AppLedStateOn = appData->Buffer[0] & 0x01;
		/// PROCESSAR DADO QUE CHEGA: teste everynet, pacote = 1c05fa00120dae07 e na base64 = MWMwNWZhMDAxMjBkYWUwNw==

		st_sigfox_events.flag.b_downlink_frame_received = true;
		memcpy(au8_downlink_frame,appData->Buffer,8);
		//1c05fa00120dae07 --> testar esse frame
		//au8_downlink_frame2[0] = 0x30; au8_downlink_frame2[0] = 0x;
		//             for (uint8_t cnt=0; cnt<8; cnt++){
		//            	 au8_downlink_frame[cnt] = fnConvert_Ascii_To_4bitHex (au8_downlink_frame2[cnt*2]) << 4;
		//		         au8_downlink_frame[cnt] |= fnConvert_Ascii_To_4bitHex (au8_downlink_frame2[(cnt*2) + 1]);
		//             }

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
		TimerStart( &LedBeaconTimer );
		break;
	}
	case LORAMAC_HANDLER_BEACON_LOST:
	case LORAMAC_HANDLER_BEACON_NRX:
	{
		TimerStop( &LedBeaconTimer );
		break;
	}
	default:
	{
		break;
	}
	}

	DisplayBeaconUpdate( params );
}

/*!
 * Prepares the payload of the frame and transmits it.
 */
/*
static void PrepareTxFrame( void )
{
    if( LmHandlerIsBusy( ) == true )
    {
        return;
    }

    //uint8_t channel = 0;

    //AppData.Port = LORAWAN_APP_PORT;

    //CayenneLppReset( );
    //CayenneLppAddDigitalInput( channel++, AppLedStateOn );
    //CayenneLppAddAnalogInput( channel++, BoardGetBatteryLevel( ) * 100 / 254 );
    //AppData.BufferSize = 8;

    /*
    AppData.Buffer[0] = 0xAA;
    AppData.Buffer[1] = 0xBB;
    AppData.Buffer[2] = 0xCC;
    AppData.Buffer[3] = 0xDD;
    AppData.Buffer[4] = 0xEE;
    AppData.Buffer[5] = 0xFF;
    AppData.Buffer[6] = 0x00;
    AppData.Buffer[7] = 0x99;
    AppData.Buffer[8] = 0x33;
    AppData.Buffer[9] = 0x22;
    AppData.Buffer[10] = 0x11;
    AppData.Buffer[11] = 0x05;
    AppData.Buffer[12] = 0x45;
    AppData.Buffer[13] = 0xAA;
    AppData.Buffer[14] = 0xBB;
    AppData.Buffer[15] = 0xCC;
    AppData.Buffer[16] = 0xDD;
    AppData.Buffer[17] = 0xEE;
    AppData.Buffer[18] = 0xFF;
    AppData.Buffer[19] = 0x00;
    AppData.Buffer[20] = 0x99;
    AppData.Buffer[21] = 0x33;
    AppData.Buffer[22] = 0x22;
    AppData.Buffer[22] = 0x11;
    AppData.Buffer[23] = 0x05;
    AppData.Buffer[24] = 0x45;


    //CayenneLppCopy( AppData.Buffer );
    //AppData.BufferSize = CayenneLppGetSize( );
    AppData.BufferSize = 12;

    if( LmHandlerSend( &AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE ) == LORAMAC_HANDLER_SUCCESS )
    {
        // Switch LED 1 ON
        GpioWrite( &Led1, 1 );
        TimerStart( &Led1Timer );
    }

}
 */

/*
static void StartTxProcess( LmHandlerTxEvents_t txEvent )
{
    switch( txEvent )
    {
    default:
        // Intentional fall through
    case LORAMAC_HANDLER_TX_ON_TIMER:
        {
            // Schedule 1st packet transmission
            TimerInit( &TxTimer, OnTxTimerEvent );
            TimerSetValue( &TxTimer, APP_TX_DUTYCYCLE  + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
            OnTxTimerEvent( NULL );
        }
        break;
    case LORAMAC_HANDLER_TX_ON_EVENT:
        {
        }
        break;
    }
}
 */

/*!
 * Function executed on TxTimer event
 */
/*
static void OnTxTimerEvent( void* context )
{
    TimerStop( &TxTimer );

    IsTxFramePending = 1;

    // Schedule next transmission
    TimerSetValue( &TxTimer, APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
    TimerStart( &TxTimer );
}
 */

/*!
 * Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent( void* context )
{
	TimerStop( &Led1Timer );
	// Switch LED 1 OFF
	GpioWrite( &Led1, 0 );
	fnAPP_Process_Event_RTC();
	TimerStart(&Led1Timer);
}


/*!
 * \brief Function executed on Beacon timer Timeout event
 */
static void OnLedBeaconTimerEvent( void* context )
{
	GpioWrite( &Led2, 1 );
	//    TimerStart( &Led2Timer );

	TimerStart( &LedBeaconTimer );
}

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

/*
 *  Function to print messages
 */
/*
void tx_com( uint8_t *s, uint16_t len )
{
  HAL_UART_Transmit( &huart1, s, len, MAX_DELAY );
}
 */

//static void readSensors( void ) {

//}

static void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00707CBB;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

static void MX_USART1_UART_Init(void)
{

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}




/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, PA15_RESERVED_Pin|PA12_RESERVED_Pin|PA1_RESERVED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, PC1_RESERVED_Pin|PC0_RESERVED_Pin|PC2_RESERVED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : PA15_RESERVED_Pin PA12_RESERVED_Pin PA1_RESERVED_Pin */
	GPIO_InitStruct.Pin = PA15_RESERVED_Pin|PA12_RESERVED_Pin|PA1_RESERVED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : WKUP_BLE */
	GPIO_InitStruct.Pin = WKUP_BLE;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(WKUP_BLE_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(WKUP_BLE_GPIO_Port,WKUP_BLE, GPIO_PIN_SET);

	/*Configure GPIO pin : LED1_Pin */
	GPIO_InitStruct.Pin = LED1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PB4_RESERVED_Pin PB1_RESERVED_Pin EXTI2_MAGNET_Pin PB0_RESERVED_Pin */
	GPIO_InitStruct.Pin = PB4_RESERVED_Pin|EXTI2_MAGNET_Pin|PB1_RESERVED_Pin|PB0_RESERVED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : PB2 */
	/*
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	 */

	/*Configure GPIO pin : PC13_RESERVED_Pin */
	GPIO_InitStruct.Pin = PC13_RESERVED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(PC13_RESERVED_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PC1_RESERVED_Pin PC0_RESERVED_Pin PC2_RESERVED_Pin */
	GPIO_InitStruct.Pin = PC1_RESERVED_Pin|PC0_RESERVED_Pin|PC2_RESERVED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/**
 * @brief ADC Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC_Init(void)
{

	/* USER CODE BEGIN ADC_Init 0 */

	/* USER CODE END ADC_Init 0 */

	ADC_ChannelConfTypeDef sConfig = {0};

	/* USER CODE BEGIN ADC_Init 1 */

	/* USER CODE END ADC_Init 1 */
	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc.Instance = ADC1;
	hadc.Init.OversamplingMode = DISABLE;
	hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	hadc.Init.Resolution = ADC_RESOLUTION_12B;
	hadc.Init.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
	hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc.Init.ContinuousConvMode = DISABLE;
	hadc.Init.DiscontinuousConvMode = DISABLE;
	hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc.Init.DMAContinuousRequests = DISABLE;
	hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc.Init.LowPowerAutoWait = DISABLE;
	hadc.Init.LowPowerFrequencyMode = DISABLE;
	hadc.Init.LowPowerAutoPowerOff = DISABLE;
	if (HAL_ADC_Init(&hadc) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure for the selected ADC regular channel to be converted.
	 */
	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure for the selected ADC regular channel to be converted.
	 */
	sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
	if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC_Init 2 */

	/* USER CODE END ADC_Init 2 */

}
