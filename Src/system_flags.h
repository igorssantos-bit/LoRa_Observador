#ifndef _SYSTEM_FLAGS_H
#define _SYSTEM_FLAGS_H


/*************************************************************************************************/
/*    INCLUDES                                                                                   */
/*************************************************************************************************/


#include <stdint.h>
#include <stdbool.h>


/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/


/*************************************************************************************************/
/*    TYPEDEFS                                                                                   */
/*************************************************************************************************/


//TODO: adaptar para o magnetômetro da ST
/* System flags */
typedef union {

   uint16_t u16_all_flags;

   struct {

      uint16_t rtc_tick                :1;

      uint16_t fxos_1_int_data_ready   :1;

      uint16_t fxos_1_int_threshold    :1;

      uint16_t radio_timer_int         :1;

      uint16_t radio_packet_received   :1;

      uint16_t pulse1_flag             :1;

      uint16_t pulse2_flag             :1;

      uint16_t lsm303agr_act           :1;

      uint16_t system_active           :1;

      uint16_t reserved                :7;

   } flag;

} un_system_flags_t;


extern volatile un_system_flags_t un_system_flags;


typedef union {

   uint16_t u16_all_flags;

   struct {

      bool b_downlink_error;
      bool b_downlink_frame_received;
      bool b_daily_update_received;
      bool b_config_frame_received;
      bool b_detection_confirmed;

   } flag;

} st_sigfox_events_t;

extern volatile st_sigfox_events_t st_sigfox_events;


/*************************************************************************************************/
/*    EXTERNAL PROTOTYPES                                                                        */
/*************************************************************************************************/


#endif	/* _SYSTEM_FLAGS_H */


/*************************************************************************************************/
/*    END OF FILE                                                                                */
/*************************************************************************************************/
