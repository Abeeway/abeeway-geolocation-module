#ifndef __SRV_LMH_H__
#define __SRV_LMH_H__

#ifdef __cplusplus
extern "C" {
#endif



#include "LmHandlerMsgDisplay.h"

#define DEF_UPLINK_DR DR_0
#define DEF_UPLINK_PORT 100
#define MIN_UPLINK_PORT 1
#define MAX_UPLINK_PORT 223	// Port values 1-223 are application specific



extern uint8_t srv_lmh_buffer[255];
extern LmHandlerParams_t srv_lmh_params;
extern uint8_t srv_lmh_uplink_port;

typedef enum { lmh_state_closed, lmh_state_opened } lmh_state_t;
extern lmh_state_t srv_lmh_state;


LmHandlerErrorStatus_t srv_lmh_open( void ( *OnRxData )( LmHandlerAppData_t *appData, LmHandlerRxParams_t *params ) );
LmHandlerErrorStatus_t srv_lmh_send(uint8_t *payload_buf, uint8_t payload_len);


#ifdef __cplusplus
}
#endif

#endif // __SRV_LMH_H__
