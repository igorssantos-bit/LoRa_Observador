/*!
 * \file      debug.c
 *
 * \brief     Functions to debug data over serial port
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

/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/

#include <string.h>
#include "debug.h"


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/
#define PRODUCTION_RELEASE false

/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


/*************************************************************************************************/
/*    PRIVATE PROTOTYPES                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    VARIABLES                                                                                  */
/*************************************************************************************************/


/*************************************************************************************************/
/*    EXTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


uint8_t fnConvert_4bitHex_To_Ascii ( uint8_t u8_value ) {

   u8_value &= 0x0F;

   if ( u8_value <= 9 ) {
      return ( u8_value + 0x30 );
   }
   else {
      return ( u8_value + 0x37 );
   }

}


uint8_t fnConvert_Ascii_To_4bitHex ( uint8_t u8_value ) {
uint8_t valor = 0;

   valor = u8_value - 0x30;

   if ( u8_value >= 'a' &&  u8_value <= 'f') {
      valor -= 0x27;
   }
   else{
	   if ( u8_value >= 'A' &&  u8_value <= 'F') {
		 valor -= 0x07;
	   }
   }

   return valor;

}

void fnConvert_8bitHex_To_Decimal_Ascii_String ( uint8_t u8_value, uint8_t * pu8_dst ) {

   pu8_dst[0] = u8_value/100 + 0x30;     /* hundred       */
   u8_value  %= 100;
   pu8_dst[1] = u8_value/10 + 0x30;      /* ten           */
   u8_value  %= 10;
   pu8_dst[2] = u8_value + 0x30;         /* unity         */
   pu8_dst[3] = 0;

   return;

}



void fnConvert_16bitHex_To_Decimal_Ascii_String ( uint16_t u16_value, uint8_t * pu8_dst ) {

   pu8_dst[0] = u16_value/10000 + 0x30;   /* tens of thousand   */
   u16_value %= 10000;
   pu8_dst[1] = u16_value/1000 + 0x30;    /* thousand			*/
   u16_value %= 1000;
   pu8_dst[2] = u16_value/100 + 0x30;     /* hundred			*/
   u16_value %= 100;
   pu8_dst[3] = u16_value/10 + 0x30;      /* ten				*/
   u16_value %= 10;
   pu8_dst[4] = u16_value + 0x30;         /* unity				*/
   pu8_dst[5] = 0;

   return;

}



void fnConvert_32bitHex_To_Decimal_Ascii_String ( uint32_t u32_value, uint8_t * pu8_dst ) {

   pu8_dst[0] = u32_value/1000000000 + 0x30;
   u32_value %= 1000000000;
   pu8_dst[1] = u32_value/100000000 + 0x30;
   u32_value %= 100000000;
   pu8_dst[2] = u32_value/10000000 + 0x30;
   u32_value %= 10000000;
   pu8_dst[3] = u32_value/1000000 + 0x30;
   u32_value %= 1000000;
   pu8_dst[4] = u32_value/100000 + 0x30;
   u32_value %= 100000;
   pu8_dst[5] = u32_value/10000 + 0x30;   /* 10x thousand  */
   u32_value %= 10000;
   pu8_dst[6] = u32_value/1000 + 0x30;    /* thousand      */
   u32_value %= 1000;
   pu8_dst[7] = u32_value/100 + 0x30;     /* hundred       */
   u32_value %= 100;
   pu8_dst[8] = u32_value/10 + 0x30;      /* ten           */
   u32_value %= 10;
   pu8_dst[9] = u32_value + 0x30;         /* unity         */
   pu8_dst[10] = 0;
   return;

}

void fnDEBUG_Const_String ( const char * p_char ) {


#if ( PRODUCTION_RELEASE == true )
   return;
#endif

   uint16_t u16_str_len = strlen(p_char);
   HAL_UART_Transmit( &huart1, p_char, u16_str_len, MAX_DELAY ); //fnDEBUG_PORT_Send_String( p_char, u8_str_len );
   
   return ;
   
}


void fnDEBUG_String_Size ( char * p_char, uint16_t size ) {

#if ( PRODUCTION_RELEASE == true )
   return;
#endif

  uint16_t cont;
  char c_buffer[4];
  unsigned char var, lower, upper;

  for (cont = 0; cont < size; cont++){
	  var = *(p_char + cont);
	  upper = (var >> 4) & 0x0f;
	  lower = var & 0x0f;
      upper =  fnConvert_4bitHex_To_Ascii(upper);
      lower =  fnConvert_4bitHex_To_Ascii(lower);
     //fnConvert_8bitHex_To_Decimal_Ascii_String ( p_char[cont], (uint8_t*)c_buffer );
     HAL_UART_Transmit( &huart1,  &upper, 1, MAX_DELAY );
     HAL_UART_Transmit( &huart1,  &lower, 1, MAX_DELAY );
     HAL_UART_Transmit( &huart1,  " ", 1, MAX_DELAY );
  }



/* TODO: implementar a função de debug pela porta serial */
   
   return;
   
}


void fnDEBUG_8bit_Value ( const char * p_char_before, const uint8_t u8_data, const char * p_char_after ) {

#if ( PRODUCTION_RELEASE == true )
   return;
#endif


	char c_buffer[4];

   if ( p_char_before ) {
	   HAL_UART_Transmit( &huart1, p_char_before, strlen(p_char_before), MAX_DELAY );
   }

	fnConvert_8bitHex_To_Decimal_Ascii_String ( u8_data, (uint8_t*)c_buffer );
	HAL_UART_Transmit( &huart1, c_buffer, 3, MAX_DELAY );
	
   if ( p_char_after ) {
	   HAL_UART_Transmit( &huart1, p_char_after, strlen(p_char_after), MAX_DELAY );
   }
      

	return;
	
}


void fnDEBUG_16bit_Value ( const char * p_char_before, const uint16_t u16_data, const char * p_char_after ) {

#if ( PRODUCTION_RELEASE == true )
   return;
#endif


  char c_buffer[6];

  if ( p_char_before ) {
	   HAL_UART_Transmit( &huart1, p_char_before, strlen(p_char_before), MAX_DELAY );
   }
	fnConvert_16bitHex_To_Decimal_Ascii_String ( u16_data, (uint8_t*)c_buffer);
	HAL_UART_Transmit( &huart1, c_buffer, 5, MAX_DELAY );

    if ( p_char_after ) {
 	   HAL_UART_Transmit( &huart1, p_char_after, strlen(p_char_after), MAX_DELAY );
    }

	return;
	
}

void fnDEBUG_16bit_Int_Value ( const char * p_char_before, const int16_t i16_data, const char * p_char_after ) {

#if ( PRODUCTION_RELEASE == true )
   return;
#endif


   char c_buffer[6];
   uint16_t u16_data;
   
   if ( p_char_before ) {
 	   HAL_UART_Transmit( &huart1, p_char_before, strlen(p_char_before), MAX_DELAY ); //Send_String_befor( p_char_before, strlen(p_char_before) );
    }


   if( i16_data < 0 ) {
	   HAL_UART_Transmit( &huart1, "-", 1, MAX_DELAY ); //fnDEBUG_PORT_Send_String( "-", 1 );
      u16_data = i16_data * -1;
   } else {
      u16_data = i16_data;
   }
   
   fnConvert_16bitHex_To_Decimal_Ascii_String ( u16_data, (uint8_t*)c_buffer);
   HAL_UART_Transmit( &huart1, c_buffer, 5, MAX_DELAY ); //fnDEBUG_PORT_Send_String( c_buffer, 5);

   if ( p_char_after ) {
	   HAL_UART_Transmit( &huart1, p_char_after, strlen(p_char_after), MAX_DELAY );       //send_String_after( p_char_after, strlen(p_char_after) );
   }
   return;
   
}


void fnDEBUG_32bit_Value ( const char * p_char_before, const uint32_t u32_data, const char * p_char_after ) {

#if ( PRODUCTION_RELEASE == true )
   return;
#endif


	char c_buffer[11];	

     if ( p_char_before ) {
		   HAL_UART_Transmit( &huart1, p_char_before, strlen(p_char_before), MAX_DELAY ); 	      //Send_String_befor( p_char_before, strlen(p_char_before) );
	 }

	fnConvert_32bitHex_To_Decimal_Ascii_String ( u32_data, (uint8_t*)c_buffer);
	HAL_UART_Transmit( &huart1, c_buffer, 10, MAX_DELAY ); //fnDEBUG_PORT_Send_String( c_buffer, 10);

     if ( p_char_after ) {
  	   HAL_UART_Transmit( &huart1, p_char_after, strlen(p_char_after), MAX_DELAY );         //send_String_after( p_char_after, strlen(p_char_after) );
     }
	return;
		
}


void fnDEBUG_8bit_Hex ( const char * p_char_before, uint8_t u8_data, const char * p_char_after ) {

#if ( PRODUCTION_RELEASE == true )
   return;
#endif


	char c_buffer[3];

   if ( p_char_before ) {
	   HAL_UART_Transmit( &huart1, p_char_before, strlen(p_char_before), MAX_DELAY );
   }

    c_buffer[0] = fnConvert_4bitHex_To_Ascii( (u8_data & 0xf0) >> 4 );
    c_buffer[1] = fnConvert_4bitHex_To_Ascii( u8_data & 0x0f );
    c_buffer[2] = '\0';
	HAL_UART_Transmit( &huart1, c_buffer, 3, MAX_DELAY );

   if ( p_char_after ) {
	   HAL_UART_Transmit( &huart1, p_char_after, strlen(p_char_after), MAX_DELAY );
   }

   return;
}

void fnDEBUG_32bit_Hex ( const char * p_char_before, uint32_t u32_data, const char * p_char_after ) {

#if ( PRODUCTION_RELEASE == true )
   return;
#endif


   char c_buffer[9];

   if ( p_char_before ) {
	   HAL_UART_Transmit( &huart1, p_char_before, strlen(p_char_before), MAX_DELAY );
   }

    for (int i=0; i < (sizeof(uint32_t) * 2); i++ ){
       c_buffer[i] = fnConvert_4bitHex_To_Ascii( (u32_data & (0xf0000000 >> (i *4))) >> (32 - 4*(i+1)) );
    }
    c_buffer[sizeof(c_buffer)-1] = '\0';
	HAL_UART_Transmit( &huart1, c_buffer, sizeof(c_buffer), MAX_DELAY );

   if ( p_char_after ) {
	   HAL_UART_Transmit( &huart1, p_char_after, strlen(p_char_after), MAX_DELAY );
   }

   return;
}

/*************************************************************************************************/
/*    INTERNAL FUNCTIONS                                                                         */
/*************************************************************************************************/


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
