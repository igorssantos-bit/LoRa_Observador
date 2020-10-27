/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include "adc.h"
#include "stm32l0xx.h"


//#include "libs_firmware_config.h"
//#include "libs_firmware/services/system_timer.h"
//#include "libs_firmware/math/average.h"

//#include "board_config.h"
#include "input_analogic.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


#define ADC_REFERENCE                        (1.0/1.48)*2800.0       // ADC Reference in miliVolts
#define ADC_DIV_RES							 		(1.97*28.0)/(1.50)      // constante de convers�o A/D
#define ADC_RESOLUTION                       4096                    // 12 bits

#define ADC_BATTERY_OK_MIN_100MV             32
#define ADC_BATTERY_OK_MIN_3300mV            32
#define ADC_BATTERY_OK_MIN_3000mV            29


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/

extern ADC_HandleTypeDef hadc;

typedef struct {

	en_input_analogic_definition_t channel_name;
	uint8_t channel_input;

} st_adc_channel_config_t;


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/

//uint16_t adc_convert ( en_input_analogic_definition_t en_input_analogic_definition );
//void configure_adc(void);
//void fnINPUT_ANALOGIC_Read_Value_In_mV ( en_input_analogic_definition_t en_input, uint32_t * p_adc_raw, uint32_t * p_adc_mv );

/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/
extern ADC_HandleTypeDef hadc1;
/*
const st_adc_channel_config_t st_adc_channel_config[INPUT_ANALOGIC_DEFINITION_SIZE] = {

   { INPUT_ANALOGIC_DEFINITION_TEMP,          ADC_POSITIVE_INPUT_PIN0 },
   { INPUT_ANALOGIC_DEFINITION_VBAT,          ADC_POSITIVE_INPUT_PIN1 },
};

static struct adc_module adc_instance;
static en_input_analogic_definition_t en_channel_chosen = 0;
struct adc_config config_adc;

 */
/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


void fnINPUT_ANALOGIC_Init ( void ) {
	/*
   adc_get_config_defaults(&config_adc);

   config_adc.reference = ADC_REFCTRL_REFSEL_INTVCC0;

   adc_init(&adc_instance, ADC, &config_adc);

   adc_disable(&adc_instance);

   return;
	 */


}

bool fnINPUT_ANALOGIC_Get_Battery ( volatile uint8_t * u8_battery ) {
	/* TODO: implementar esta fun��o */

	uint16_t batteryRaw;

	HAL_ADC_Start( &hadc1 );
	HAL_ADC_PollForConversion( &hadc1, 10000 );

	batteryRaw = HAL_ADC_GetValue( &hadc1 );

	HAL_ADC_Stop( &hadc1 );

	// TODO: Corrigir falsos alarmes: �s vezes, esta fun��o retorna valor baixo da bateria, sem aparentemente estar baixo.
	*u8_battery = round( (float)( ADC_DIV_RES * batteryRaw ) / ADC_RESOLUTION );
	//*u8_battery = round( (float)( ADC_DIV_RES * batteryRaw ));
	if(*u8_battery > ADC_BATTERY_OK_MIN_3300mV) {
		return true; // battery ok
	}
	else {
		return false; // battery not ok
	}

}


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


uint16_t adc_convert ( en_input_analogic_definition_t en_input_analogic_definition ) {
	/*
   uint16_t u16_result;

   adc_enable(&adc_instance);

   en_channel_chosen = en_input_analogic_definition;
   config_adc.positive_input = st_adc_channel_config[en_channel_chosen].channel_input;
   adc_set_positive_input(&adc_instance, config_adc.positive_input );

   adc_start_conversion(&adc_instance);

   do {
   } while (adc_read(&adc_instance, &u16_result) == STATUS_BUSY);

   adc_disable(&adc_instance);

   return u16_result;
	 */
	return 0;

}


void fnINPUT_ANALOGIC_Read_Value_In_mV ( en_input_analogic_definition_t en_input, uint32_t * p_adc_raw, uint32_t * p_adc_mv ) {

	uint32_t u32_adc_raw;

	u32_adc_raw = adc_convert ( en_input );

	* p_adc_raw = u32_adc_raw;

	* p_adc_mv = u32_adc_raw * ( ADC_REFERENCE / ADC_RESOLUTION );

	return;

}



/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
