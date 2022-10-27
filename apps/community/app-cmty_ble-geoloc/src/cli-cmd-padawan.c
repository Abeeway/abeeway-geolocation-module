
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
// #include <stdio.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "srv_cli.h"
#include "app_conf.h"
#include "ble_defs.h"
#include "aos_rf_switch.h"
#include "aos_lpm.h"
#include "aos_ble_core.h"
#include "aos_dis.h"
#include "strnhex.h"

#include "srv_ble_dtm.h"
#include "srv_ble_scan.h"
#include "srv_ble_beaconing.h"
#include "srv_provisioning.h"

static cli_parser_status_t _cmd_padawan_start(void* arg, int argc, char *argv[])
{
	return cli_parser_status_ok;
}

static cli_parser_status_t _cmd_padawan_stop(void* arg, int argc, char *argv[])
{
	return cli_parser_status_ok;
}

static const cli_parser_cmd_t _padawan[] = {
		PARSER_CMD_FUNC("start <time>", "Start the BLE scan and reporting", _cmd_padawan_start, CLI_ACCESS_ALL_LEVELS),
		PARSER_CMD_FUNC("stop", "Stop the scan and reporting", _cmd_padawan_stop, CLI_ACCESS_ALL_LEVELS),
};
// Register main commands against the CLI parser
CLI_COMMAND_TAB_REGISTER(padawan,"Padawan commands", _padawan, CLI_ACCESS_ALL_LEVELS );
