/*!
 * \file      board.c
 *
 * \brief     Target board general functions implementation
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

#include <stdio.h>
#include "stm32l0xx.h"
#include "utilities.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "timer.h"
#include "board-config.h"
#include "lpm-board.h"
#include "rtc-board.h"
#include "sx1276-board.h"
#include "lsm303agr_reg.h"
#include "board.h"
#include "accelerometer.h"
#include "ds18b20.h"
#include "si7021-a20.h"

/*!
 * Unique Devices IDs register set ( STM32L0xxx )
 */
#define         ID1                                 ( 0x1FF80050 )
#define         ID2                                 ( 0x1FF80054 )
#define         ID3                                 ( 0x1FF80064 )
#define         MAX_TRIES                           10
/*!
 * Hardware objects
 */
Gpio_t Led1;
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
ADC_HandleTypeDef hadc1;

volatile lsm303agr_ctx_t dev_ctx_xl = {
		.write_reg = accel_write,
		.read_reg = accel_read,
		.handle = (I2C_HandleTypeDef *) &hi2c1
};

volatile lsm303agr_ctx_t dev_ctx_mg = {
		.write_reg = magnet_write,
		.read_reg = magnet_read,
		.handle = (I2C_HandleTypeDef *) &hi2c1
};


/*!
 * Initializes the Debug mode in Stand by, Stop and Sleep mode
 */
static void BoardEnableDBG( void );


/*!
 * System Clock Re-Configuration when waking up from STOP mode
 */
static void SystemClockReConfig( void );

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

/*!
 * Flag to indicate if the SystemWakeupTime is Calibrated
 */
static volatile bool SystemWakeupTimeCalibrated = false;

/*!
 * Callback indicating the end of the system wake-up time calibration
 */

static void OnCalibrateSystemWakeupTimeTimerEvent( void* context )
{
	RtcSetMcuWakeUpTime( );
	SystemWakeupTimeCalibrated = true;
}

/*
 * Disable interruptions
 * */
void BoardCriticalSectionBegin( uint32_t *mask )
{
	*mask = __get_PRIMASK( );
	__disable_irq( );
}

/*
 * Enable interruptions
 * */
void BoardCriticalSectionEnd( uint32_t *mask )
{
	__set_PRIMASK( *mask );
}

/*
 * Initialize:
 *    RTC
 *    SPI
 *    I2C
 *    GPIO
 * */
void BoardInitPeriph( void )
{

	MX_GPIO_Init();
	MX_I2C1_Init( );
	MX_USART1_UART_Init( );
	MX_ADC_Init();

	SX1276IoDbgInit( );
	SX1276IoTcxoInit( );
	SpiInit( &SX1276.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
	SX1276IoInit( );

	if( McuInitialized == false ){
		// LSM303 inicialização desligado
		fnLsm303_Init(dev_ctx_xl, dev_ctx_mg);
		turnoff_Mag(dev_ctx_mg);

		// Temperature and Humidity sensor Init
		si7021_set_config(&hi2c1, SI7021_HEATER_OFF, SI7021_RESOLUTION_RH8_TEMP12);
	}

}

/*
 * DeInitialize:
 * 	  DEBUG
 * 	  SPI1
 *    I2C
 *    USART
 *    GPIO
 * */

void BoardDeInitPeriph( void )
{

	SpiDeInit(&SX1276.Spi);
	SX1276IoDeInit();

	MX_I2C1_DeInit( );
	MX_USART1_UART_DeInit( );

	MX_GPIO_DeInit();

}

void BoardInitMcu( void )
{
	if(McuInitialized == false){
		HAL_Init( );
	}

	SystemClockConfig_MSI();
	LpmSetOffMode( LPM_APPLI_ID, LPM_DISABLE );
	BoardEnableDBG();

	// Configure the hardware
	BoardInitPeriph();

	McuInitialized = true;

}

void BoardResetMcu( void )
{
	CRITICAL_SECTION_BEGIN( );

	//Restart system
	NVIC_SystemReset( );
}

void BoardDeInitMcu( void )
{
	SpiDeInit( &SX1276.Spi );
	SX1276IoDeInit( );
}

uint32_t BoardGetRandomSeed( void )
{
	return ( ( *( uint32_t* )ID1 ) ^ ( *( uint32_t* )ID2 ) ^ ( *( uint32_t* )ID3 ) );
}

void BoardGetUniqueId( uint8_t *id )
{
	id[7] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
	id[6] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
	id[5] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
	id[4] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
	id[3] = ( ( *( uint32_t* )ID2 ) ) >> 24;
	id[2] = ( ( *( uint32_t* )ID2 ) ) >> 16;
	id[1] = ( ( *( uint32_t* )ID2 ) ) >> 8;
	id[0] = ( ( *( uint32_t* )ID2 ) );
}

uint16_t BoardBatteryMeasureVoltage( void )
{
	return 0;
}

uint32_t BoardGetBatteryVoltage( void )
{
	return 0;
}

uint8_t BoardGetBatteryLevel( void )
{
	return 0;
}

uint8_t BoardGetPulse1Estado (void){
	return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3));
}

uint8_t BoardGetPulse2Estado (void){
	return 0;
}

#if 1
void Debug_Init( void ){
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	//#if (DEBUG == true)
#if 1
	/* sw interface on*/

	/* Enable the GPIO_A Clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* Configure the GPIO pin */
	GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull   = GPIO_PULLUP;
	GPIO_InitStructure.Speed  = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pin    = (PA13_SWDIO_Pin | PA14_SWCLK_Pin);
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Reset debug Pins */
	HAL_GPIO_WritePin(PA13_SWDIO_Port, PA13_SWDIO_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PA14_SWCLK_Port, PA14_SWCLK_Pin, GPIO_PIN_RESET);

	__HAL_RCC_DBGMCU_CLK_ENABLE();

	//HAL_DBGMCU_EnableDBGSleepMode();
	//HAL_DBGMCU_EnableDBGStopMode();
	//HAL_DBGMCU_EnableDBGStandbyMode();

#else
	/* sw interface off*/

	/* Enable the GPIO_A Clock */
	__HAL_RCC_GPIOA_CLK_ENABLE() ;

	/* Configure the GPIO pin */
	GPIO_InitStructure.Mode   = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull   = GPIO_NOPULL;
	GPIO_InitStructure.Pin    = (PA13_SWDIO_Pin | PA14_SWCLK_Pin);
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Disable the GPIO_A Clock */
	__HAL_RCC_GPIOA_CLK_DISABLE() ;

	__HAL_RCC_DBGMCU_CLK_ENABLE();
	HAL_DBGMCU_DisableDBGSleepMode();
	HAL_DBGMCU_DisableDBGStopMode();
	HAL_DBGMCU_DisableDBGStandbyMode();
	__HAL_RCC_DBGMCU_CLK_DISABLE();
#endif
}
#endif

static void BoardEnableDBG( void )
{
	HAL_DBGMCU_EnableDBGSleepMode( );
	HAL_DBGMCU_EnableDBGStopMode( );
	HAL_DBGMCU_EnableDBGStandbyMode( );
}

void Debug_DeInit( void ){
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	// Enable the GPIO_A Clock
	__HAL_RCC_GPIOA_CLK_ENABLE() ;

	// Configure the GPIO pin
	GPIO_InitStructure.Mode   = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull   = GPIO_NOPULL;
	GPIO_InitStructure.Pin    = (PA13_SWDIO_Pin | PA14_SWCLK_Pin);
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Disable the GPIO_A Clock
	__HAL_RCC_GPIOA_CLK_DISABLE() ;

	__HAL_RCC_DBGMCU_CLK_ENABLE();
	HAL_DBGMCU_DisableDBGSleepMode();
	HAL_DBGMCU_DisableDBGStopMode();
	HAL_DBGMCU_DisableDBGStandbyMode();
	__HAL_RCC_DBGMCU_CLK_DISABLE();
}

#if 0
void SystemClockConfig( void )
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	__HAL_RCC_PWR_CLK_ENABLE( );

	__HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE3); //PWR_REGULATOR_VOLTAGE_SCALE1 );


	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
	RCC_OscInitStruct.HSIState = RCC_HSI_DIV4; //RCC_HSI_ON
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;

	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
	RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLLMUL_3;
	RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLLDIV_3;
	if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
	{
		assert_param( FAIL );
	}


	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; //RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;//RCC_SYSCLK_DIV8 (- 0.800mA) --> maior valor possivel; original: RCC_SYSCLK_DIV1
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 ) != HAL_OK )
	{
		assert_param( FAIL );
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
	if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK )
	{
		assert_param( FAIL );
	}

	//TODO: verificar pq o systic com HAL_RCC_GetHCLKFreq( ) / 1000 não deixa a placa funcionar
	//HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq( ) / 2000 ); // nao funciona
	HAL_SYSTICK_Config( 524 );

	HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

	// SysTick_IRQn interrupt configuration
	HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
}
#endif

void SystemClockConfig_HighSpeed( void )
{
	//RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;//RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;//RCC_SYSCLK_DIV8 (- 0.800mA) --> maior valor possivel; original: RCC_SYSCLK_DIV1
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 ) != HAL_OK )
	{
		assert_param( FAIL );
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK; //RCC_RTCCLKSOURCE_LSE; //RCC_USART1CLKSOURCE_HSI;
	if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK )
	{
		assert_param( FAIL );
	}

	HAL_SYSTICK_Config(524 );

	HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

	// SysTick_IRQn interrupt configuration
	HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
}


void SystemClockConfig_MSI( void )
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	__HAL_RCC_PWR_CLK_ENABLE( );

	__HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE3); //PWR_REGULATOR_VOLTAGE_SCALE1 );


	RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI ;
	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;//RCC_HSI_DIV4; //
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
	RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLLMUL_3;
	RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLLDIV_3;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = 0;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
	if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
	{
		assert_param( FAIL );
	}

	//TODO: no RCC_MSIRANGE_2 ocorre um erro ao inicializar essa estrutura
	//      contudo ela o sistema funcionou perfeitamente sem ela.
	//      no Range 2 o consumo foi para 230uA
#if 1
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI; //RCC_SYSCLKSOURCE_HSI; //RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;//RCC_SYSCLK_DIV8 -> no MSI da erro de sincronismo com o RTC
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	//if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 ) != HAL_OK )
	if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_0 ) != HAL_OK )
	{
		assert_param( FAIL );
	}
#endif

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;//RCC_RTCCLKSOURCE_LSE;
	if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK )
	{
		assert_param( FAIL );
	}


	// Systick intitialize
	//TODO: ENTENDER PQ A CONFIG ABAIXO GERA PROBLEMA
	HAL_SYSTICK_Config( (uint32_t )HAL_RCC_GetHCLKFreq() / 1000 );
	// HAL_SYSTICK_Config( 1048 );

	HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

	// SysTick_IRQn interrupt configuration
	HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
}


/*
void CalibrateSystemWakeupTime( void )
{
    if( SystemWakeupTimeCalibrated == false )
    {
        TimerInit( &CalibrateSystemWakeupTimeTimer, OnCalibrateSystemWakeupTimeTimerEvent );
        TimerSetValue( &CalibrateSystemWakeupTimeTimer, 1000 );
        TimerStart( &CalibrateSystemWakeupTimeTimer );
        while( SystemWakeupTimeCalibrated == false )
        {

        }
    }
}
 */
void SystemClockReConfig( void )
{
	__HAL_RCC_PWR_CLK_ENABLE( );
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3); //PWR_REGULATOR_VOLTAGE_SCALE1 );

	// Enable HSI
	__HAL_RCC_HSI_CONFIG( RCC_HSI_ON );

	// Wait till HSI is ready
	while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSIRDY ) == RESET )
	{
	}

	// Enable PLL
	__HAL_RCC_PLL_ENABLE( );

	// Wait till PLL is ready
	while( __HAL_RCC_GET_FLAG( RCC_FLAG_PLLRDY ) == RESET )
	{
	}

	// Select PLL as system clock source
	__HAL_RCC_SYSCLK_CONFIG ( RCC_SYSCLKSOURCE_PLLCLK );

	// Wait till PLL is used as system clock source
	while( __HAL_RCC_GET_SYSCLK_SOURCE( ) != RCC_SYSCLKSOURCE_STATUS_PLLCLK )
	{
	}
}

void SysTick_Handler( void )
{
	HAL_IncTick( );
	HAL_SYSTICK_IRQHandler( );
}

/**
 * \brief Enters Low Power Stop Mode
 *
 * \note ARM exists the function when waking up
 */
void LpmEnterStopMode( void)
{

	CRITICAL_SECTION_BEGIN( );

	BoardDeInitPeriph();

	// Disable the Power Voltage Detector
	HAL_PWR_DisablePVD( );

	// Clear wake up flag
	SET_BIT( PWR->CR, PWR_CR_CWUF );

	// Enable Ultra low power mode
	HAL_PWREx_EnableUltraLowPower( );

	// Enable the fast wake up from Ultra low power mode
	HAL_PWREx_EnableFastWakeUp( );

	CRITICAL_SECTION_END( );

	// Enter Stop Mode
	HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
}

/*!
 * \brief Exists Low Power Stop Mode
 */
void LpmExitStopMode( void )
{
	// Disable IRQ while the MCU is not running on HSI
	CRITICAL_SECTION_BEGIN( );

	BoardInitMcu( );

	CRITICAL_SECTION_END( );
}

/*!
 * \brief Enters Low Power Sleep Mode
 *
 * \note ARM exits the function when waking up
 */
void LpmEnterSleepMode( void)
{
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void BoardLowPowerHandler( void )
{
	__disable_irq( );
	/*!
	 * If an interrupt has occurred after __disable_irq( ), it is kept pending
	 * and cortex will not enter low power anyway
	 */

	LpmEnterLowPower( );

	__enable_irq( );
}


void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	/* Configuração dos pinos GPIOA.
	 *
	 * Mapeamento dos Pinos:
	 * PA0	| INTERRUPÇÃO XL	| FAIL - Não pode ser configurado pois conflita com DIO2 (ACELEROMETRO)
	 * PA1	| ANT_SWITCH_RX		| A CONFIGURAR (RADIO)
	 * PA2	| BATERIA INTERNA   | ANALOGICA
	 * PA3	| INTERRUPÇÃO2 ACEL | OK
	 * PA4	| OneWire1			| OK  (usando como entrada onewire) [DIO5 --> disabled interrupt mode]
	 * PA5	| OneWire2			| OK  (influencia no dio irq4, mas DIO4 Lora está desabilitado)
	 * PA6	| MISO				| OK
	 * PA7	| MOSI				| OK
	 * PA8	| NC				| -
	 * PA9	| NC     			| OK  | UART_TX QND USAR BLE
	 * PA10	| NC    			| OK  | UART_RX QND USAR BLE
	 * PA11	| NC				| -
	 * PA12	| RADIO_TCXO_POWER 	| OK
	 * PA13	| SWDIO				| OK
	 * PA14	| SWCLK				| OK
	 * PA15	| NSS				| OK */


	/* Configuração do parametro ANT_Switch_Rx */
	HAL_GPIO_WritePin(PA1_ANT_SW_RX_Port, PA1_ANT_SW_RX_Pin, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = PA1_ANT_SW_RX_Pin;  //RADIO_TCXO_POWER
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PA1_ANT_SW_RX_Port, &GPIO_InitStruct);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(WKUP_BLE_GPIO_Port,WKUP_BLE, GPIO_PIN_RESET);

	/*Configure GPIO pins : WKUP_BLE */
	GPIO_InitStruct.Pin = WKUP_BLE;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(WKUP_BLE_GPIO_Port, &GPIO_InitStruct);

	/* Configuração dos pinos não utilizados (NC) e pinos Analogicos */
	GPIO_InitStruct.Pin =  EXTI0_XL_Pin | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_9| GPIO_PIN_10| GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configuração dos pinos GPIOB
	 *
	 * Mapeamento dos Pinos:
	 * PB0	| DIO2				| OK
	 * PB1	| DIO1				| OK
	 * PB2	| INTERRRUPÇÃO MAG	| OK
	 * PB3	| SCK				| OK
	 * PB4	| DIO0				| OK INTERRRUPÇÃO
	 * PB5	| LED1				| A CONFIGURAR (LED)
	 * PB6	| NC				| -
	 * PB7	| NC				| -
	 * PB8	| SLC				| OK
	 * PB9	| SDA				| OK
	 * PB10	| NC				| -
	 * PB11	| NC				| -
	 * PB12	| NC        		| -
	 * PB13	| RADIO_DBG_TX (NC)	| -
	 * PB14	| PULSE1         	| ????? RADIO_DBG_RX (NC)
	 * PB15	| PULSE2    		| OK
	 *
	 */

	/* Configuração dos pinos não utilizados (NC) */
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
			| GPIO_PIN_13 | GPIO_PIN_15 | GPIO_PIN_14 | EXTI2_MAGNET_Pin | GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Configuração dos pinos GPIOC
	 *
	 * Mapeamento dos Pinos:
	 * PC0	| RADIO_RESET		| A CONFIGURAR (RADIO)
	 * PC1	| ANT_SW_TX_BOOST	| A CONFIGURAR (RADIO)
	 * PC2	| ANT_SW_TX_RFO		| A CONFIGURAR (RADIO)
	 * PC3	| NC				| -
	 * PC4	| NC				| -
	 * PC5	| NC				| -
	 * PC6	| NC				| -
	 * PC7	| NC				| -
	 * PC8	| NC				| -
	 * PC9	| NC				| -
	 * PC10	| NC				| -
	 * PC11	| NC				| -
	 * PC12	| NC				| -
	 * PC13	| RADIO_DIO_3		| OK
	 * PC14	| NC				| -
	 * PC15	| NC				| - */

	/* Configuração dos parametros ANT_Switch_TX_BOOST e ANT_Switch_TX_RFO */
	HAL_GPIO_WritePin(GPIOC, PC1_ANT_SW_TX_BOOST_Pin|PC2_ANT_SW_TX_RFO_Pin, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = PC1_ANT_SW_TX_BOOST_Pin|PC2_ANT_SW_TX_RFO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Configuração do RADIO_RESET */
	HAL_GPIO_WritePin(PC0_RADIO_RESET_Port, PC0_RADIO_RESET_Pin, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = PC0_RADIO_RESET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(PC0_RADIO_RESET_Port, &GPIO_InitStruct);

	/* Configuração dos pinos não utilizados (NC) */
	GPIO_InitStruct.Pin = GPIO_PIN_All & (~(GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_13));
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Configuração dos pinos GPIOH (Nenhum pino) */
	GPIO_InitStruct.Pin = GPIO_PIN_All;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

	/* Disable Unused GPIOs clock */
	__HAL_RCC_GPIOH_CLK_DISABLE();

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void MX_GPIO_DeInit(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	/* A */
	GPIO_InitStruct.Pin = EXTI0_XL_Pin | PA1_ANT_SW_RX_Pin | WKUP_BLE;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* B */
	GPIO_InitStruct.Pin = PB5_LED1_Pin | EXTI2_MAGNET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* C */
	GPIO_InitStruct.Pin = PC1_ANT_SW_TX_BOOST_Pin | PC2_ANT_SW_TX_RFO_Pin | PC0_RADIO_RESET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Disable Unused GPIOs clock */
	__HAL_RCC_GPIOB_CLK_DISABLE();
	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOH_CLK_DISABLE();
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */

void MX_I2C1_Init(void)
{

	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00300F38; //0x00707CBB;  //
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	// Initialize pinout
	// see more at HAL_I2C_MspInit in stm32l0xx_hal_msp.c
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}

	// Configure Analog filter
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		Error_Handler();
	}

	// Configure Digital filter
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		Error_Handler();
	}
}


void MX_I2C1_DeInit(void){
	HAL_I2C_DeInit(&hi2c1);
}

void MX_USART1_UART_Init(void)
{

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	//huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
//	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
//	HAL_NVIC_EnableIRQ(USART1_IRQn);
	/* USER CODE END USART1_Init 1 */

}


void MX_USART1_UART_DeInit(void){
	HAL_UART_DeInit(&huart1);
}



/**
 * @brief ADC Initialization Function
 * @param None
 * @retval None
 */
//https://visualgdb.com/tutorials/arm/stm32/adc/
/**
 * @brief ADC Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC_Init(void)
{


	ADC_ChannelConfTypeDef sConfig = {0};

	/* USER CODE BEGIN ADC_Init 1 */

	/* USER CODE END ADC_Init 1 */
	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.OversamplingMode = DISABLE;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;//ADC_SAMPLETIME_79CYCLES_5;
	hadc1.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.LowPowerFrequencyMode = DISABLE;
	hadc1.Init.LowPowerAutoPowerOff = DISABLE;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure for the selected ADC regular channel to be converted.
	 */
	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
#if 0
	/** Configure for the selected ADC regular channel to be converted.
	 */
	sConfig.Channel = ADC_CHANNEL_5;
	if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel to be converted.
	 */
	sConfig.Channel = ADC_CHANNEL_VREFINT;
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
#endif

}


/*
 * @brief Function used to save data on internal EEPROM
 * @param address is the address from DATA_EEPROM_BASE
 * @param value is the data to be saved
 * @retval true if the data was saved with success
 *
 * */
bool writeByteToEEPROM (uint32_t address, uint8_t value){

	HAL_StatusTypeDef flash_ok = HAL_ERROR;
	uint8_t timeout = MAX_TRIES;
	while (flash_ok != HAL_OK && timeout--)
	{
		flash_ok = HAL_FLASHEx_DATAEEPROM_Unlock();
		//HAL_Delay(1);
	}
	if (flash_ok == HAL_ERROR)
		return false;

	flash_ok = HAL_ERROR;
	timeout = MAX_TRIES;
	while (flash_ok != HAL_OK && timeout--)
	{
		flash_ok = HAL_FLASHEx_DATAEEPROM_Program (FLASH_TYPEPROGRAMDATA_BYTE, DATA_EEPROM_BASE + address, value);
		//HAL_Delay(1);
	}
	if (flash_ok == HAL_ERROR)
		return false;

	flash_ok = HAL_ERROR;
	timeout = MAX_TRIES;
	while (flash_ok != HAL_OK  && timeout--)
	{
		flash_ok = HAL_FLASHEx_DATAEEPROM_Lock ();
		//HAL_Delay(1);
	}

	return  true;
}


/*
 * @brief Function used to save data on internal EEPROM
 * @param address is the address from DATA_EEPROM_BASE
 * @param value is the data to be saved
 * @retval true if the data was saved with success
 *
 * */
bool writeWordToEEPROM (uint32_t address, uint32_t value){

	HAL_StatusTypeDef flash_ok = HAL_ERROR;
	uint8_t timeout = MAX_TRIES;
	while (flash_ok != HAL_OK && timeout--)
	{
		flash_ok = HAL_FLASH_Unlock();
		//HAL_Delay(1);
	}
	if (flash_ok == HAL_ERROR)
		return false;
	/*
    flash_ok = HAL_ERROR;
    timeout = MAX_TRY;
    while (flash_ok != HAL_OK && timeout--)
    {
      flash_ok = HAL_FLASHEx_DATAEEPROM_Erase (FLASH_TYPEERASEDATA_WORD, address);
      HAL_Delay(5);
    }
    if (flash_ok == HAL_ERROR)
      	return HAL_ERROR;
	 */

	flash_ok = HAL_ERROR;
	timeout = MAX_TRIES;
	while (flash_ok != HAL_OK && timeout--)
	{
		flash_ok = HAL_FLASH_Program (FLASH_TYPEPROGRAMDATA_WORD, DATA_EEPROM_BASE + address, value);
		//HAL_Delay(1);
	}
	if (flash_ok == HAL_ERROR)
		return false;


	while (flash_ok != HAL_OK  && timeout--)
	{
		flash_ok = HAL_FLASH_Lock ();
		//HAL_Delay(1);
	}

	return  true;
}


/*
 * @brief Function used to restore data on internal EEPROM
 * @param address is the address from DATA_EEPROM_BASE
 * @retval true if the data was saved with success
 *
 * */
uint32_t readFromEEPROM(uint32_t address){
	return ((*(__IO uint32_t *)(DATA_EEPROM_BASE + address)));
}



#if !defined ( __CC_ARM )

/*
 * Function to be used by stdout for printf etc
 */

int _write( int fd, const void *buf, size_t count )
{
	//while( UartPutBuffer( &Uart2, ( uint8_t* )buf, ( uint16_t )count ) != 0 ){ };
	for (int i = 0; i<count; i++){
		HAL_UART_Transmit( &huart1, ( uint8_t* )buf + i, 1, 100 );
	}
	return count;
}

#if 0
/*
 * Function to be used by stdin for scanf etc
 */
int _read( int fd, const void *buf, size_t count )
{
	size_t bytesRead = 0;
	while( UartGetBuffer( &Uart2, ( uint8_t* )buf, count, ( uint16_t* )&bytesRead ) != 0 ){ };
	// Echo back the character
	while( UartPutBuffer( &Uart2, ( uint8_t* )buf, ( uint16_t )bytesRead ) != 0 ){ };
	return bytesRead;
}
#endif
#else

// Keil compiler
int fputc( int c, FILE *stream )
{
	while( UartPutChar( &Uart2, ( uint8_t )c ) != 0 );
	return c;
}

int fgetc( FILE *stream )
{
	uint8_t c = 0;
	while( UartGetChar( &Uart2, &c ) != 0 );
	// Echo back the character
	while( UartPutChar( &Uart2, c ) != 0 );
	return ( int )c;
}

#endif

#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %lu\r\n", file, line) */

	printf( "Wrong parameters value: file %s on line %lu\r\n", ( const char* )file, line );
	/* Infinite loop */
	while( 1 )
	{
	}
}
#endif

