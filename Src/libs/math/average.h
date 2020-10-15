#ifndef AVERAGE_H
#define AVERAGE_H


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>

//#include "libs_firmware_config.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/
#define x_average_data_type_t          int16_t
#define x_average_data_sum_type_t      int32_t
#define AVERAGE_USE_ROTATE_TO_DIVIDE   0



typedef struct {

   bool b_measure_done;
   uint8_t u8_max_index;
   uint8_t u8_current_index;
   x_average_data_sum_type_t x_data_sum;
   x_average_data_type_t data;
   volatile x_average_data_type_t* px_data;

} st_average_desc_t;




typedef struct {

	bool b_measure_done;
	uint8_t u8_max_index;
	uint8_t u8_current_index;
	x_average_data_sum_type_t x_data_sum;
	int8_t* px_data;

} st_average_desc_8bits_t;

/*
typedef struct {

	bool b_measure_done;
	uint8_t u8_max_index;
	uint8_t u8_current_index;
	x_average_data_sum_type_t x_data_sum;
	volatile x_average_data_type_t* px_data;

} st_average_desc_8bits_t;
*/
/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


void fnAVERAGE_Init ( st_average_desc_t * pst_desc, uint8_t u8_max_index, uint16_t u16_buffer_size );
void fnAVERAGE_Update ( volatile st_average_desc_t * pst_average_desc, x_average_data_type_t x_value );
x_average_data_type_t fnAVERAGE_Get_Value ( volatile st_average_desc_t * pst_average_desc );

/*
void fnAVERAGE_Init_8bits ( st_average_desc_8bits_t * pst_desc, uint8_t u8_max_index, uint16_t u16_buffer_size );
void fnAVERAGE_Update_8bits (volatile st_average_desc_8bits_t * pst_desc, uint8_t x_value );
x_average_data_type_t fnAVERAGE_Get_Value_8bits ( volatile st_average_desc_8bits_t * pst_desc );
*/

void fnAVERAGE_Init_8bits ( st_average_desc_8bits_t * pst_desc, uint8_t u8_max_index, uint8_t u8_buffer_size );
void fnAVERAGE_Update_8bits (volatile st_average_desc_8bits_t * pst_desc, int8_t newValue );
int8_t fnAVERAGE_Get_Value_8bits ( volatile st_average_desc_8bits_t * pst_desc );

#endif	/* AVERAGE_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
