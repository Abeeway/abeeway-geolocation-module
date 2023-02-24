/*
 * lmhandler-common.c
 *
 * This module provides a set of default lmhandler callbacks and a function
 * to set up the LmhandlerCallbacks_t structure. It is intended to serve as
 * a sample common default base for various applications.
 *
 *  Created on: Sep 27, 2022
 *      Author: marc
 */

#include <lmhandler-common.h>
#include "board.h"

#include "LmHandler.h"
#include "LmHandlerMsgDisplay.h"

// LM handlers. Display the LmHandler debug messages people are familiar with
// from the github examples. Handled using the stackforce demo code, modified
// to use cli_printf() instead of printf().

static void _lm_on_beacon_status_change(LoRaMacHandlerBeaconParams_t *params)
{
	DisplayBeaconUpdate(params);
}

static void _lm_on_class_change(DeviceClass_t deviceClass)
{
	DisplayClassUpdate(deviceClass);

    // Inform the server as soon as possible that the end-device has switched to ClassB
    LmHandlerAppData_t appData =
    {
        .Buffer = NULL,
        .BufferSize = 0,
        .Port = 0,
    };
	LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG);
}

static void _lm_on_join_request(LmHandlerJoinParams_t *params)
{
	DisplayJoinRequestUpdate(params);

	if (params->Status == LORAMAC_HANDLER_ERROR) {
		// Do not retry for ABP activation, that would blow our stack !
		if (params->CommissioningParams->IsOtaaActivation) {
			// Retry OTAA join
			LmHandlerJoin();
		}
	} else {
		LmHandlerRequestClass(CLASS_A);
	}
}

static void _lm_on_mac_mcps_request(LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxDelay)
{
	DisplayMacMcpsRequestUpdate(status, mcpsReq, nextTxDelay);
}

static void _lm_on_mac_mlme_request(LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxDelay)
{
	DisplayMacMlmeRequestUpdate(status, mlmeReq, nextTxDelay);
}

static void _lm_on_mac_process(void)
{
	// No need for trampoline since mac timers are already decoupled from interrupts.
	LmHandlerProcess();
}

static void _lm_on_network_parameters_change(CommissioningParams_t *params)
{
	DisplayNetworkParametersUpdate(params);
}

static void _lm_on_nvm_data_change(LmHandlerNvmContextStates_t state, uint16_t size)
{
	DisplayNvmDataChange(state, size);
}

static void _lm_on_rx_data(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
	DisplayRxUpdate(appData, params);
}

static void _lm_on_sys_time_update(bool isSynchronized, int32_t timeCorrection)
{
	DisplayTimeUpdate(isSynchronized, timeCorrection);
}

static void _lm_on_tx_data(LmHandlerTxParams_t *params)
{
	DisplayTxUpdate(params);
}

float _lm_get_temperature(void)
{
	// BoardGetTemperature() returns the temperature * 256 as an integer.
	// LmHandler wants it as a float.

	return BoardGetTemperature() / 256.0;
}

void lmhandler_set_default_callbacks(LmHandlerCallbacks_t *cb)
{
	cb->GetBatteryLevel = BoardGetBatteryLevel;
	cb->GetRandomSeed = BoardGetRandomSeed;
	cb->GetTemperature = _lm_get_temperature;
	cb->OnBeaconStatusChange = _lm_on_beacon_status_change;
	cb->OnClassChange = _lm_on_class_change;
	cb->OnJoinRequest = _lm_on_join_request;
	cb->OnMacMcpsRequest = _lm_on_mac_mcps_request;
	cb->OnMacMlmeRequest = _lm_on_mac_mlme_request;
	cb->OnMacProcess = _lm_on_mac_process;
	cb->OnNetworkParametersChange = _lm_on_network_parameters_change;
	cb->OnNvmDataChange = _lm_on_nvm_data_change;
	cb->OnRxData = _lm_on_rx_data;
	cb->OnSysTimeUpdate = _lm_on_sys_time_update;
	cb->OnTxData = _lm_on_tx_data;

}

bool lmhandler_map_mac_region(srv_provisioning_mac_region_t prv_region, LoRaMacRegion_t *mac_region)
{
	static const LoRaMacRegion_t mac_regions[srv_provisioning_mac_region_count] = {
			[srv_provisioning_mac_region_eu868] = LORAMAC_REGION_EU868,
			[srv_provisioning_mac_region_ru864] = LORAMAC_REGION_RU864,
			[srv_provisioning_mac_region_us915] = LORAMAC_REGION_US915,
			[srv_provisioning_mac_region_as923_1] = LORAMAC_REGION_AS923_1,
			[srv_provisioning_mac_region_as923_1_jp] = LORAMAC_REGION_AS923_1_JP,
			[srv_provisioning_mac_region_as923_2] = LORAMAC_REGION_AS923_2,
			[srv_provisioning_mac_region_as923_3] = LORAMAC_REGION_AS923_3,
			[srv_provisioning_mac_region_as923_4] = LORAMAC_REGION_AS923_4,
			[srv_provisioning_mac_region_au915] = LORAMAC_REGION_AU915,
			[srv_provisioning_mac_region_kr920] = LORAMAC_REGION_KR920,
			[srv_provisioning_mac_region_in865] = LORAMAC_REGION_IN865
	};

	if (prv_region < srv_provisioning_mac_region_count) {
		*mac_region = mac_regions[prv_region];
		return true;
	}
	return false;
}
