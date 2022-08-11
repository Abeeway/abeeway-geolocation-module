/*!
 * \file main.c
 *
 * \brief startup of application
 *
 *  * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */


#include "aos_board.h"

#include "srv_cli.h"

#include "srv_provisioning.h"
#include "LmHandler.h"
#include "LmHandlerMsgDisplay.h"
//#include "LoRaMAC.h"
#include "board.h"

#include "lora-init.h"

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
			cli_printf("Retrying join...\n");
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

	switch (appData->Port) {
	case 1: // The application LED can be controlled on port 1 or 2
	case 2:
		cli_printf("Received request to turn the Application LED O%s\n", (appData->Buffer[0] & 0x01) ? "N":"FF");
		aos_board_led_set(aos_board_led_idx_led4, appData->Buffer[0] & 0x01);
		break;
	default:
		break;
	}
}

static void _lm_on_sys_time_update(bool isSynchronized, int32_t timeCorrection)
{
	cli_printf("%s(%d, %lu)\n", isSynchronized, timeCorrection);
}

static void _lm_on_tx_data(LmHandlerTxParams_t *params)
{
	DisplayTxUpdate(params);
}

static uint8_t _lmhandler_buffer[255];

LmHandlerErrorStatus_t lorawan_init()
{

	static LmHandlerParams_t _lmhandler_params = {
			.Region = LORAMAC_REGION_EU868,
			.AdrEnable = true,
			.IsTxConfirmed = LORAMAC_HANDLER_UNCONFIRMED_MSG,
			.TxDatarate = DR_0,
			.PublicNetworkEnable = true,
			.DutyCycleEnabled = true,
			.DataBufferMaxSize = sizeof(_lmhandler_buffer),
			.DataBuffer = _lmhandler_buffer,
	};


	static LmHandlerCallbacks_t lc;

	if (srv_provisioning_data_state() == srv_provisioning_data_state_invalid) {
		cli_printf("Restoring provisioning settings...\n");

		if (srv_provisioning_read() != srv_provisioning_status_success) {
			cli_printf("No provisioning settings found\n");
			return LORAMAC_HANDLER_ERROR;
		}
	}

	srv_provisioning_mac_region_t region;
	if (srv_provisioning_get_lora_mac_region(&region) != srv_provisioning_status_success) {
		cli_printf("Failed to get provisioned region\n");
		return LORAMAC_HANDLER_ERROR;
	}

	switch(region) {
	case srv_provisioning_mac_region_eu868:
		_lmhandler_params.Region = LORAMAC_REGION_EU868;
		break;
	case srv_provisioning_mac_region_ru864:
		_lmhandler_params.Region = LORAMAC_REGION_RU864;
		break;
	case srv_provisioning_mac_region_us915:
		_lmhandler_params.Region = LORAMAC_REGION_US915;
		break;
	case srv_provisioning_mac_region_as923:
		_lmhandler_params.Region = LORAMAC_REGION_AS923;
		break;
	case srv_provisioning_mac_region_au915:
		_lmhandler_params.Region = LORAMAC_REGION_AU915;
		break;
	case srv_provisioning_mac_region_kr920:
		_lmhandler_params.Region = LORAMAC_REGION_KR920;
		break;
	case srv_provisioning_mac_region_in865:
		_lmhandler_params.Region = LORAMAC_REGION_IN865;
		break;
	case srv_provisioning_mac_region_count:
		cli_printf("Unknown provisioning region %u\n", region);
		return LORAMAC_HANDLER_ERROR;
	}

	lc.GetBatteryLevel = BoardGetBatteryLevel;
	lc.GetRandomSeed = BoardGetRandomSeed;
	lc.GetTemperature = NULL;	// BoardGetTemperature;
	lc.OnBeaconStatusChange = _lm_on_beacon_status_change;
	lc.OnClassChange = _lm_on_class_change;
	lc.OnJoinRequest = _lm_on_join_request;
	lc.OnMacMcpsRequest = _lm_on_mac_mcps_request;
	lc.OnMacMlmeRequest = _lm_on_mac_mlme_request;
	lc.OnMacProcess = _lm_on_mac_process;
	lc.OnNetworkParametersChange = _lm_on_network_parameters_change;
	lc.OnNvmDataChange = _lm_on_nvm_data_change;
	lc.OnRxData = _lm_on_rx_data;
	lc.OnSysTimeUpdate = _lm_on_sys_time_update;
	lc.OnTxData = _lm_on_tx_data;

	return LmHandlerInit(&lc, &_lmhandler_params);


}

