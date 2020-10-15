/*************************************************************************************************/
/*    INFO                                                                                       */
/*************************************************************************************************/

/*
   Calc average from a buffer
*/

/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <string.h>

#include "average.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    INTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/


/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


void fnAVERAGE_Init ( st_average_desc_t * pst_desc, uint8_t u8_max_index, uint16_t u16_buffer_size ) {

   pst_desc->b_measure_done   = false;
   pst_desc->u8_max_index     = u8_max_index;
   pst_desc->u8_current_index = 0;
   pst_desc->x_data_sum       = 0;
   memset( (void*)pst_desc->px_data, 0, u16_buffer_size );
   
   return;

}


void fnAVERAGE_Update ( volatile st_average_desc_t * pst_desc, x_average_data_type_t x_value ) {

/* Sum new data, subtract oldest data */
   pst_desc->x_data_sum -= pst_desc->px_data[ pst_desc->u8_current_index ];
   pst_desc->x_data_sum += x_value;

/* Save new data, discard oldest data */
   pst_desc->px_data[ pst_desc->u8_current_index++ ] = x_value;

/* Check index */
   if( pst_desc->u8_current_index >= pst_desc->u8_max_index ) {

      pst_desc->u8_current_index = 0;
      pst_desc->b_measure_done = true;
   }
   
   return;
   
}


x_average_data_type_t fnAVERAGE_Get_Value ( volatile st_average_desc_t * pst_desc ) {

   x_average_data_type_t x_return = 0;

   if ( pst_desc->b_measure_done == true ) {
      
#if ( AVERAGE_USE_ROTATE_TO_DIVIDE == 0 )
      x_return = (x_average_data_type_t)((pst_desc->x_data_sum)/(pst_desc->u8_max_index));
#else
      x_return = (x_average_data_type_t)((pst_desc->x_data_sum)>>AVERAGE_USE_ROTATE_TO_DIVIDE);
#endif
   }

   return x_return;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0

void fnAVERAGE_Init_8bits ( st_average_desc_8bits_t * pst_desc, uint8_t u8_max_index, uint16_t u16_buffer_size ) {

	pst_desc->b_measure_done   = false;
	pst_desc->u8_max_index     = u8_max_index;
	pst_desc->u8_current_index = 0;
	pst_desc->x_data_sum       = 0;
	memset( (void*)pst_desc->px_data, 0, u16_buffer_size );

	return;

}

void fnAVERAGE_Update_8bits (volatile st_average_desc_8bits_t * pst_desc, uint8_t x_value ) {


	/* Sum new data, subtract oldest data */
	uint8_t valor =  pst_desc->px_data[pst_desc->u8_current_index] ;
	pst_desc->x_data_sum -= valor;
	pst_desc->x_data_sum += x_value;

	/* Save new data, discard oldest data */
	pst_desc->px_data[ pst_desc->u8_current_index++ ] = x_value;

	/* Check index */
	if( pst_desc->u8_current_index >= pst_desc->u8_max_index ) {

		pst_desc->u8_current_index = 0;
		pst_desc->b_measure_done = true;
	}
	
	return;
	
}


x_average_data_type_t fnAVERAGE_Get_Value_8bits ( volatile st_average_desc_8bits_t * pst_desc ) {

	x_average_data_type_t x_return = 0;

	if ( pst_desc->b_measure_done == true ) {
		
		#if ( AVERAGE_USE_ROTATE_TO_DIVIDE == 0 )
		x_return = (x_average_data_type_t)((pst_desc->x_data_sum)/(pst_desc->u8_max_index));
		#else
		x_return = (x_average_data_type_t)((pst_desc->x_data_sum)>>AVERAGE_USE_ROTATE_TO_DIVIDE);
		#endif
	}

	return x_return;

}

#endif




void fnAVERAGE_Init_8bits ( st_average_desc_8bits_t * pst_desc, uint8_t u8_max_index, uint8_t u8_buffer_size ) {

	pst_desc->b_measure_done   = false;
	pst_desc->u8_max_index     = u8_max_index;
	pst_desc->u8_current_index = 0;
	pst_desc->x_data_sum       = 0;
	memset( pst_desc->px_data, 0, u8_buffer_size );
	return;

}


void fnAVERAGE_Update_8bits (volatile st_average_desc_8bits_t * pst_desc, int8_t newValue ) {


	/* Sum new data, subtract oldest data */
	int8_t oldestValue =  pst_desc->px_data[pst_desc->u8_current_index] ;
	pst_desc->x_data_sum -= oldestValue;
	pst_desc->x_data_sum += newValue;

	/* Save new data, discard oldest data */
	pst_desc->px_data[ pst_desc->u8_current_index++ ] = newValue;

	/* Check index */
	if( pst_desc->u8_current_index >= pst_desc->u8_max_index ) {
		pst_desc->u8_current_index = 0;
		pst_desc->b_measure_done = true;
	}

	return;

}

int8_t fnAVERAGE_Get_Value_8bits ( volatile st_average_desc_8bits_t * pst_desc ) {

	int8_t x_return = 0;

	if ( pst_desc->b_measure_done == true ) {
		x_return = (int8_t)((pst_desc->x_data_sum)/(pst_desc->u8_max_index));
	}
	return x_return;

}


/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
