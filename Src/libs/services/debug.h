#ifndef _DEBUG_H
#define _DEBUG_H


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stm32l0xx_hal.h"

/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/
#define MAX_DELAY		1000

/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/

extern UART_HandleTypeDef huart1;
/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


void fnDEBUG_Const_String ( const char * p_char );
void fnDEBUG_String_Size ( char * p_char, uint16_t size );
void fnDEBUG_8bit_Value ( const char * p_char , const uint8_t u8_data, const char * p_char_after );
void fnDEBUG_16bit_Value ( const char * p_char , const uint16_t u16_data, const char * p_char_after );
void fnDEBUG_16bit_Int_Value ( const char * p_char_before, const int16_t i16_data, const char * p_char_after );
void fnDEBUG_32bit_Value ( const char * p_char , const uint32_t u32_data, const char * p_char_after );
uint8_t fnConvert_Ascii_To_4bitHex ( uint8_t u8_value );
void fnDEBUG_8bit_Hex ( const char * p_char_before, uint8_t u8_data, const char * p_char_after );
void fnDEBUG_32bit_Hex ( const char * p_char_before, uint32_t u32_data, const char * p_char_after );


#endif   /* _DEBUG_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
