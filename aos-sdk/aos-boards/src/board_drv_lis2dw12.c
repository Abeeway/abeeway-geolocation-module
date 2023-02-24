/*
 * \file board_drv_lis2dw12_drv.c
 *
 * \brief Accelerometer lis2dw12 (ST) driver
 *
 * Usage
 * -----
 * The configuration should provides the motion sensitivity and the shock detection threshold (if expected).
 * The 3 accelerometer modes are supported. However, the shock detection can be used only with the high_sensi
 * configuration.
 * The mode is configured via the motion_sensitivity parameter.
 *  - Value range [1-30]: Mode High sensitivity, low power
 *  - Value 100: Regular backward compatibility.
 *  - Value range [101..199] => Backward compatibility with sensitivity (as before)
 *
 * Embedded functions
 * - The sleep/wakup capability is used for motion detection
 * - Single tap detection is used for shock detection
 *
 * Mode high sensitivity
 * - The motion_sensitivity and the shock detection threshold parameters are expressed in step of 0.063g
 * - ODR: 12.5Hz in sleep, configurable in wake. A high ODR generates a high sampling rate, which is required for shock detection.
 * - LP mode: Low power mode 1 (12 bits resolution).
 * - FIFO: Used (required for getting the highest acceleration vector triggering the shock).
 * - Full scale: configurable.
 * - Filter bandwidth: ODR/2. Data not filtered. Motion detection: not filtered.
 * - Low noise filter: No
 * - Consumption. 1.7 uA in sleep, depends on ODR in wake
 * - User offset: Used (for wakeup detection).
 * - User offset weight: 15.6 mg
 *
 *
 *
 */
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <string.h>

#include "FreeRTOS.h"
#include "timers.h"
#include "aos_i2c.h"
#include "aos_log.h"
#include "board_drv_accelero.h"

/*
 * ***********************************************
 * Driver low level access (I2C) and interrupts
 * ***********************************************
 */
// I2C MEMS Chip address
#define LIS2DW12_I2C_BUS_ADDR1		0x18
#define LIS2DW12_I2C_BUS_ADDR2		0x19

// Nb milliseconds in 1 second
#define ONE_SECOND					1000

// Max polling in wake state. 10 seconds
#define LIS2DW12_WAKE_POLLING_MAX_TIMEOUT	10000

// Debug LIS. Disable for release
#define DEBUG_LIS2DW12				0

#if DEBUG_LIS2DW12
#define LIS_STATUS(...)	do { aos_log_status(aos_log_module_accelero, true, __VA_ARGS__);  } while (0)
#define LIS_TRACE(...)	do { aos_log_msg(aos_log_module_accelero, aos_log_level_debug, true, __VA_ARGS__);  } while (0)
#else
#define LIS_STATUS(...)
#define LIS_TRACE(...)
#endif


// Registers address
#define REG_ADDR_OUT_T_L					0x0D
#define REG_ADDR_OUT_T_H					0x0E
#define REG_ADDR_WHO_AM_I					0x0F
#define REG_ADDR_CTRL1						0x20
#define REG_ADDR_CTRL2						0x21
#define REG_ADDR_CTRL3						0x22
#define REG_ADDR_CTRL4_INT1_PAD_CTRL		0x23
#define REG_ADDR_CTRL5_INT2_PAD_CTRL		0x24
#define REG_ADDR_CTRL6						0x25
#define REG_ADDR_OUT_T						0x26
#define REG_ADDR_STATUS						0x27
#define REG_ADDR_OUT_X_L					0x28
#define REG_ADDR_OUT_X_H					0x29
#define REG_ADDR_OUT_Y_L					0x2A
#define REG_ADDR_OUT_Y_H					0x2B
#define REG_ADDR_OUT_Z_L					0x2C
#define REG_ADDR_OUT_Z_H					0x2D
#define REG_ADDR_FIFO_CTRL					0x2E
#define REG_ADDR_FIFO_SAMPLES				0x2F
#define REG_ADDR_TAP_THS_X					0x30
#define REG_ADDR_TAP_THS_Y					0x31
#define REG_ADDR_TAP_THS_Z					0x32
#define REG_ADDR_INT_DUR					0x33
#define REG_ADDR_WAKE_UP_THS				0x34
#define REG_ADDR_WAKE_UP_DUR				0x35
#define REG_ADDR_FREE_FALL					0x36
#define REG_ADDR_STATUS_DUP					0x37
#define REG_ADDR_WAKE_UP_SRC				0x38
#define REG_ADDR_TAP_SRC					0x39
#define REG_ADDR_SIXD_SRC					0x3A
#define REG_ADDR_ALL_INT_SRC				0x3B
#define REG_ADDR_X_OFS_USR					0x3C
#define REG_ADDR_Y_OFS_USR					0x3D
#define REG_ADDR_Z_OFS_USR					0x3E
#define REG_ADDR_CTRL7						0x3F

#define REG_FIRST_ADDR						REG_ADDR_OUT_T_L
#define REG_LAST_ADDR						REG_ADDR_CTRL7


// Bit fields interrupts
#define REG_BIT_ALL_INT_SRC_SLEEP_CHANGE_IA 		(1<<5)
#define REG_BIT_ALL_INT_SRC_6D_IA 					(1<<4)
#define REG_BIT_ALL_INT_SRC_DOUBLE_TAP				(1<<3)
#define REG_BIT_ALL_INT_SRC_SINGLE_TAP				(1<<2)
#define REG_BIT_ALL_INT_SRC_WU_IA					(1<<1)


#define REG_BIT_ALL_INT_SRC_FF_IA					(1<<0)

// Bit field status
#define REG_BIT_STATUS_FIFO_THS						(1<<7)
#define REG_BIT_STATUS_WU_IA						(1<<6)
#define REG_BIT_STATUS_SLEEP_STATE					(1<<5)
#define REG_BIT_STATUS_DOUBLE_TAP					(1<<4)
#define REG_BIT_STATUS_SINGLE_TAP					(1<<3)
#define REG_BIT_STATUS_6D_IA						(1<<2)
#define REG_BIT_STATUS_FF_IA						(1<<1)
#define REG_BIT_STATUS_DRDY							(1<<0)

// Bit field WAKE_UP_THS
#define REG_BIT_WAKE_UP_THS_TAP_DETECT				(1<<7)
#define REG_BIT_WAKE_UP_THS_SLEEP_ON				(1<<6)
#define REG_WAKE_UP_DUR_SHIFT						5

// Bit field TAP_THZ_Z
#define REG_BIT_TAP_X_EN							(1<<7)
#define REG_BIT_TAP_Y_EN							(1<<6)
#define REG_BIT_TAP_Z_EN							(1<<5)


// Data QN format conversion based on the scale and with data on 14 bits
#define SCALE_2G_TO_QN_FORMAT	14
#define SCALE_4G_TO_QN_FORMAT	13
#define SCALE_8G_TO_QN_FORMAT	12
#define SCALE_16G_TO_QN_FORMAT	11

// Consumption in nano Ampere
#define LIS2DW12_CONSO_SLEEP_UA		2000		// In sleep. ODR: 12.5 Hz. Sleep used only with high sensi


/*
 * FIFO configuration
 */
#define LIS2DW12_FIFO_NB_SAMPLES		32		// Max number of samples in the FIFO
#define LIS2DW12_FIFO_SAMPLE_SIZE		6		// Size of one sample
#define FIFO_THRESHOLD					30		//Threshold (don't care)
#define FIFO_CTRL_VAL					(0b11000000 | FIFO_THRESHOLD) // FIFO control: mode: continuous
#define CTRL5_ENABLE_FTH				0b00000010	// Enable FIFO threshold interrupt
#define CTRL4_ENABLE_FTH				0b00000010	// Enable FIFO threshold interrupt


// General context
typedef struct {
	aos_i2c_handle_t i2c_hdl;
	uint8_t i2c_address;
	uint64_t debounce_shock_time;		// Time (in ticks) used to debounce the shock
	bool init_done;						// True if init done
	bool opening;						// True if the opening process is in progress
	bool process_shock_pending;			// True if the shock process should be done
	int16_t ofs_x;						// Last offset for x
	int16_t ofs_y;						// Last offset for y
	int16_t ofs_z;						// Last offset for z;
	board_accelero_fs_type_t	fs;		// Full scale used
	board_accelero_odr_type_t	odr;	// ODR used
	uint32_t wake_time;					// Actual wake time in ms
	uint32_t polling_timeout;			// Polling timeout in ms
	board_accelero_init_info_t init_info;	// Initialization information
	board_accelero_notif_type_t notif;
	TimerHandle_t timer_hdl;			// Timer for shock data acquisition
	StaticTimer_t timer_local_data;		// Timer internal data
	UBaseType_t timer_count;			// Timer count
	board_accelero_user_callback_t user_cb; // User callback
	void* user_arg;						// User argument
	uint8_t fifo[LIS2DW12_FIFO_NB_SAMPLES*LIS2DW12_FIFO_SAMPLE_SIZE];	// Area to retrieve all samples as a bulk
} lis2dw12_ctx_t;

static lis2dw12_ctx_t _lis2dw12_ctx = {0};

/*
 * General/common definitions
 */
// Boot time
#define LIS2DW12_BOOT_TIME_MS			(1)

// Sensitivity configured modes
#define LIS2DW12_MAX_SENSI_CFG 		30

// Maximum/minimum ODR frequency
#define MAX_ODR_FREQUENCY				200
#define MIN_ODR_FREQUENCY				12.5

// Debounce time for shock detection
#define LIS2DW12_SHOCK_DEBOUNCE_TIME	2000

// Number of samples after the shock trigger
#define	NB_SHOCK_SAMPLES				20


// Max threshold value: 63. (Full scale 4g: step 4g/64 = 0.063g, Full scale 2g: step 2/64 = 0,03125g)
#define MAX_THRESHOLD_VALUE 			0b00111111

// Control 2. Do a reset
#define CRTL2_RESET_VAL					0b01000000

// Control 2. Enable IF_ADDR_INC and BDU
#define CTRL2_VAL_COMMON				0b00001100


// CTRL6. Full scale = 16g
#define CTRL6_VAL_FS_16G				0b00110000
// CTRL6. Full scale = 8g
#define CTRL6_VAL_FS_8G					0b00100000
// CTRL6. Full scale = 4g
#define CTRL6_VAL_FS_4G					0b00010000
// CTRL6. Full scale = 2g
#define CTRL6_VAL_FS_2G					0b00000000
// CTRL6. Low noise enabled
#define CTRL6_VAL_LOW_NOISE				0b00000100
// Locate the Full scalbe bits in CTRL6
#define CTRL6_FS_SHIFT					4

// BW filter
#define CTRL6_BW_FILT_ODR_OVER_20		0b11000000
#define CTRL6_BW_FILT_ODR_OVER_10		0b10000000
#define CTRL6_BW_FILT_ODR_OVER_4		0b01000000
#define CTRL6_BW_FILT_ODR_OVER_2		0b00000000

// CTRL7. Enable interrupt map int2 on int1, interrupts enable. NO_INT: Disable interrupts
#define CTRL7_VAL_DEFAULT				0b01100000
#define CTRL7_USR_OFF_ON_WU				(1<<3)
#define CTRL7_USR_OFF_W					(1<<2)
#define CTRL7_HP_REF_MODE				(1<<1)

// ODR frequency vs configured value (board_accelero_odr_type_t)
#define ODR_FREQUENCY(odr_type) ((odr_type >= board_accelero_odr_type_25HZ) ? (1 <<(odr_type-1)) * 25:12)

/*
 * Shock configuration
 */
// Interrupts INT1. None by default (motion). With_TAP for shock detection
#define CTRL4_VAL_DEFAULT				0b00000000
#define CTRL4_VAL_WITH_TAP				0b01000000

// Max tap threshold
#define MAX_TAP_THRESHOLD_VALUE 		0b00011111

/*
 * Max quiet time to debounce
 * max shock duration time since the over-threshold peak should be resumed before the end of this time
 */
#define INT_DUR_DEFAULT					0b00001111

/*
 * High sensitivity configuration
 */
// Minimum ODR value
#define CTRL1_VAL_ODR_12_5					0x02

// CTRL3 value
#define CTRL3_VAL_HIGH_SENSI			0b00010010 // Latched interrupts. Single data conversion triggered by I2C


// Interrupts INT2. SLEEP_STATE interrupt
#define CTRL5_VAL_HIGH_SENSI			0b01000000


static const board_accelero_reg_item_t _lis2dw12_high_sensi_setup[] = {
		{REG_ADDR_FIFO_CTRL, FIFO_CTRL_VAL},
		{REG_ADDR_CTRL2, CTRL2_VAL_COMMON},
		{REG_ADDR_CTRL3, CTRL3_VAL_HIGH_SENSI},
		{REG_ADDR_CTRL4_INT1_PAD_CTRL, 0}, // Disable interruption at start time
		{REG_ADDR_CTRL5_INT2_PAD_CTRL, 0}, // Disable interruption at start time
		{0, 0}
};


/*
 * ***********************************************
 * Basic functions and IT management
 * ***********************************************
 */

// Consumption in high_sensi_mode based on ODR in nano-ampere.
// Values used here have been measured using the Joulescope. They differ from the LIS application
// note (table 7). But this table is given for a voltage of 1.8V and we power the chip with 3.3V.
// This could explain the difference.
static uint32_t _high_sensi_consumption[] = {
		[board_accelero_odr_type_12_5HZ] =   2000,
		[board_accelero_odr_type_25HZ]   =   3100,
		[board_accelero_odr_type_50HZ]   =   5400,
		[board_accelero_odr_type_100HZ]  =   9600,
		[board_accelero_odr_type_200HZ]  =   16300,
};

static int16_t _lis2dw12_fix16_to_mg(fix16_t f16)
{
	return fix16_to_int(fix16_mul(f16, FIX16_THOUSAND));
}

fix16_t _lis2dw12_vector_magnitude(board_accelero_fs_type_t fs, fix16_vector_t* v)
{
	if (fs == board_accelero_fs_type_16G) {
		fix16_vector_t v2;
		fix16_t mag;
		// For 16G the fix16 value will overload. So divide the vector by 2
		v2.x = fix16_shift(v->x, 1);
		v2.y = fix16_shift(v->y, 1);
		v2.z = fix16_shift(v->z, 1);
		mag = fix16_vector_magnitude(&v2);
		// Multiply the result by 2
		return fix16_shift(mag, -1);
	}
	return fix16_vector_magnitude(v);
}

static bool _lis2dw12_register_set(uint8_t addr, uint8_t value)
{
	if (!_lis2dw12_ctx.i2c_hdl) {
		// Not initialized.
		return false;
	}
	return aos_i2c_master()->write(_lis2dw12_ctx.i2c_hdl, _lis2dw12_ctx.i2c_address, addr, &value, 1)?false:true;
}

static bool _lis2dw12_setup(const board_accelero_reg_item_t* setup)
{
	if ((!_lis2dw12_ctx.i2c_hdl) || (!setup)) {
		// Not initialized.
		return false;
	}

	while (setup->reg) {
		if (!_lis2dw12_register_set(setup->reg, setup->data)) {
			return false;
		}
		setup ++;
	}
	return true;
}

static bool _lis2dw12_register_get(uint8_t addr, uint8_t *ret_value)
{
	if (!_lis2dw12_ctx.i2c_hdl) {
		// Not initialized.
		return false;
	}
	return aos_i2c_master()->read(_lis2dw12_ctx.i2c_hdl, _lis2dw12_ctx.i2c_address, addr, ret_value, 1)?false:true;
}

static float _lis2dw12_calculate_mag_power2_5(fix16_t mag)
{
	fix16_t smag = fix16_sqrt(mag);
	float temp;

	temp = fix16_to_float(mag);

	temp = temp*temp * fix16_to_float(smag);

	return temp;
}

static bool _lis2dw12_get_data(fix16_vector_t* v, uint32_t *gadd_index, unsigned max_read, bool shock_detected)
{
#define MEMS_WAIT_MAX_LOOP 	10
#define MIN_MAG_FOR_SHOCK	2000 // Minumum magnitude for shock detection in mg
	uint8_t qn = SCALE_4G_TO_QN_FORMAT;
	uint8_t status_reg;
	uint8_t nb_sample;
	uint8_t wait_loop = MEMS_WAIT_MAX_LOOP;
	uint8_t* sample_ptr;
	fix16_vector_t highest_vector = {0, 0, 0};
	bool good_shock = false;
	fix16_t fmag;
	uint32_t magnitude;
	uint32_t highest_magnitude = 0;
	float fgad_idx = 0;

	while (wait_loop) {
		if (!_lis2dw12_register_get(REG_ADDR_STATUS, &status_reg)) {
			return false;
		}
		if ((status_reg & REG_BIT_STATUS_DRDY)) {
			// Data ready.
			break;
		}
		wait_loop --;
	}

	if (!wait_loop) {
		return false;
	}

	qn = SCALE_2G_TO_QN_FORMAT - _lis2dw12_ctx.fs;

	// Read the FIFO SAMPLE register
	if (!_lis2dw12_register_get(REG_ADDR_FIFO_SAMPLES, &status_reg)) {
		return false;
	}

	// Keep number of samples
	nb_sample = status_reg & 0x3F;
	// Do at least one reading
	if (!nb_sample) {
		nb_sample = 1;
	} else if (nb_sample > max_read) {
		nb_sample = max_read;
	}

	// Read the whole FIFO
	LIS_STATUS("FIFO status: 0x%02x. Reading %d sample(s)\n", status_reg, nb_sample);
	if (aos_i2c_master()->read(_lis2dw12_ctx.i2c_hdl, _lis2dw12_ctx.i2c_address, REG_ADDR_OUT_X_L, _lis2dw12_ctx.fifo, nb_sample * LIS2DW12_FIFO_SAMPLE_SIZE) != 0) {
			LIS_STATUS("Reading failure\n");
			return false;
	}

	// Parse all samples
	sample_ptr = _lis2dw12_ctx.fifo;
	for (uint8_t ii = 0; ii < nb_sample; ii ++, sample_ptr += LIS2DW12_FIFO_SAMPLE_SIZE) {
		v->x = fix16_from_Qn(sample_ptr[0] + (sample_ptr[1] << 8), qn);
		v->y = fix16_from_Qn(sample_ptr[2] + (sample_ptr[3] << 8), qn);
		v->z = fix16_from_Qn(sample_ptr[4] + (sample_ptr[5] << 8), qn);

		fmag = _lis2dw12_vector_magnitude(_lis2dw12_ctx.fs, v);
		magnitude = _lis2dw12_fix16_to_mg(fmag);

		LIS_TRACE("Sample: x:%d, y:%d, z:%d. Mag: %d\n",
				board_accelero_fix16_to_mg(v->x),
				board_accelero_fix16_to_mg(v->y),
				board_accelero_fix16_to_mg(v->z),
				magnitude);
		if ((shock_detected) && (magnitude > MIN_MAG_FOR_SHOCK)) {
			// Sample part of the shock
			fgad_idx += _lis2dw12_calculate_mag_power2_5(fmag);

			if (magnitude > highest_magnitude) {
				memcpy(&highest_vector, v, sizeof(fix16_vector_t));
				good_shock = true;
				highest_magnitude = magnitude;
			}
		}
	}

	// In case of a shock we lookup the vector having the highest modulus.
	// Otherwise we return the latest sample read.
	if (shock_detected) {
		if (!good_shock) {
			LIS_STATUS("Bad shock detected\n");
			return false;
		}
		memcpy(v, &highest_vector, sizeof(fix16_vector_t));
		if (gadd_index) {
			// Weight the index based on ODR. Higher ODRs have a lower weight
			float weight = MAX_ODR_FREQUENCY/((_lis2dw12_ctx.odr + 1)* MIN_ODR_FREQUENCY);
			fgad_idx *= weight;
			*gadd_index = fgad_idx;
			LIS_STATUS("Highest shock: x:%d, y:%d, z:%d. Gadd Index: %d\n",
					board_accelero_fix16_to_mg(v->x),
					board_accelero_fix16_to_mg(v->y),
					board_accelero_fix16_to_mg(v->z),
					*gadd_index);
		}
	}
	return true;
}

static void _lis2dw12_feed_user_ofs(fix16_vector_t* vector, lis2dw12_ctx_t* ctx)
{
#define LIS2DW12_USR_WEIGHT_15_6_MG 		0.0156
#define LIS2DW12_USR_MAX_WEIGHT				69			// 67*0.0156=1076 mg
	int16_t  x, y, z;
	fix16_t step = fix16_from_float(LIS2DW12_USR_WEIGHT_15_6_MG);	// Calculate only once

	// Calculate new user offset
	x= fix16_to_int(fix16_div(vector->x, step));
	y= fix16_to_int(fix16_div(vector->y, step));
	z= fix16_to_int(fix16_div(vector->z, step));

	if ((fix16_abs(x) > LIS2DW12_USR_MAX_WEIGHT) ||
		(fix16_abs(y) > LIS2DW12_USR_MAX_WEIGHT) ||
		(fix16_abs(z) > LIS2DW12_USR_MAX_WEIGHT)) {
		// We don't want to set an acceleration vector other than the gravity as user_offset
		return;
	}

	if (ctx->ofs_x != x) {
		// Update
		_lis2dw12_register_set(REG_ADDR_X_OFS_USR, x);
		ctx->ofs_x = x;
	}
	if (ctx->ofs_y != y) {
		// Update
		_lis2dw12_register_set(REG_ADDR_Y_OFS_USR, y);
		ctx->ofs_y = y;
	}
	if (ctx->ofs_z != z) {
		// Update
		_lis2dw12_register_set(REG_ADDR_Z_OFS_USR, z);
		ctx->ofs_z = z;
	}
}


board_accelero_notif_type_t _lis2dw12_process(lis2dw12_ctx_t* ctx, bool process_shock)
{
	uint8_t int_reg;
	uint8_t stat_reg;
	bool res;
	board_accelero_notif_info_t ninfo;
	board_accelero_notif_type_t ntype = board_accelero_notif_type_failure;

	if (!ctx->i2c_hdl) {
		// Not initialized. or bad param
		return ntype;
	}

	// Reading this register acknowledge all interrupts
	if (!_lis2dw12_register_get(REG_ADDR_ALL_INT_SRC, &int_reg)) {
		return ntype;
	}

	if (ctx->opening) {
		// When opening, we are in wake state
		return board_accelero_notif_type_wake;
	}
	if (process_shock) {
		LIS_STATUS("Shock. Reading FIFO\n");
		if (!_lis2dw12_get_data(&ninfo.vector, &ninfo.gadd_index, LIS2DW12_FIFO_NB_SAMPLES, true)) {
			return ntype;
		}
		// Call the user
		ctx->user_cb(board_accelero_notif_type_shock, &ninfo, ctx->user_arg);
		ctx->process_shock_pending = false;
		return board_accelero_notif_type_shock;
	}

	if (ctx->process_shock_pending) {
		// We are waiting for the shock processing
		return ntype;
	}
	// Read status register
	if (!_lis2dw12_register_get(REG_ADDR_STATUS, &stat_reg)) {
		return ntype;
	}

	LIS_STATUS("IT. Reg:0x%02x, Status: 0x%02x\n", int_reg,stat_reg);

	// Process motion vs sleep
	if (int_reg & REG_BIT_ALL_INT_SRC_WU_IA) {
		ntype= board_accelero_notif_type_wake;
	} else {
		if (stat_reg & REG_BIT_STATUS_SLEEP_STATE) {
			ntype = board_accelero_notif_type_sleep;
		} else {
			ntype = board_accelero_notif_type_wake;
		}
	}

	// Process shock interrupt
	if (int_reg & REG_BIT_ALL_INT_SRC_SINGLE_TAP) {
		if (xTaskGetTickCount() - ctx->debounce_shock_time > pdMS_TO_TICKS(LIS2DW12_SHOCK_DEBOUNCE_TIME)) {
			LIS_STATUS("Valid shock detected\n");
			// Shock detected. Start timer for data acquisition
			ctx->debounce_shock_time = xTaskGetTickCount();
			xTimerChangePeriod(ctx->timer_hdl, pdMS_TO_TICKS((ONE_SECOND * NB_SHOCK_SAMPLES) / ODR_FREQUENCY(_lis2dw12_ctx.odr)), 0);
			ctx->process_shock_pending = true;
		} else {
			LIS_STATUS("Shock debounced\n");
		}
		// Indicate we are still in wake
		ntype = board_accelero_notif_type_wake;
	}

	// Read a single sample
	res = _lis2dw12_get_data(&ninfo.vector, NULL, 1, false);

	// Clear GADD index
	ninfo.gadd_index = 0;

	LIS_STATUS("Vector(mg): x:%d, y:%d, z:%d\n",
			board_accelero_fix16_to_mg(ninfo.vector.x),
			board_accelero_fix16_to_mg(ninfo.vector.y),
			board_accelero_fix16_to_mg(ninfo.vector.z));

	if (res) {
		// Feed the user offset
		_lis2dw12_feed_user_ofs(&ninfo.vector, &_lis2dw12_ctx);
	}
	// Call the user
	ctx->user_cb(ntype, &ninfo, ctx->user_arg);
	// Restart the wake polling timer
	if (ntype == board_accelero_notif_type_wake) {
		if (!ctx->process_shock_pending) {
			// Shock is not pending. Timer is free for use
			xTimerChangePeriod(ctx->timer_hdl, pdMS_TO_TICKS(ctx->polling_timeout), 0);
		}
	} else {
		// Stop timer
		xTimerStop(ctx->timer_hdl, 0);
	}
	return ntype;
}

static void _lis2dw12_int1_handler(uint8_t uid, void* user_arg)
{
	_lis2dw12_process(&_lis2dw12_ctx, false);
}


static void _lis2dw12_timeout(TimerHandle_t xExpiredTimer )
{
	_lis2dw12_ctx.opening = false;
	LIS_STATUS("LIS timeout. Shock pending: %s\n", _lis2dw12_ctx.process_shock_pending?"yes":"no");
	// Request the background processing
	if (_lis2dw12_ctx.process_shock_pending) {
		_lis2dw12_process(&_lis2dw12_ctx, true);
		// Restart the polling timer
		xTimerChangePeriod(_lis2dw12_ctx.timer_hdl, pdMS_TO_TICKS(_lis2dw12_ctx.polling_timeout), 0);
		return;
	}

	// Read accelero to update the user offset
	if (_lis2dw12_process(&_lis2dw12_ctx, false) == board_accelero_notif_type_wake) {
		// We are still in wake. Restart the timer
		xTimerChangePeriod(_lis2dw12_ctx.timer_hdl, pdMS_TO_TICKS(_lis2dw12_ctx.polling_timeout), 0);
	}
}

/*
 * ***********************************************
 * Initialization and Configuration
 * ***********************************************
 */
static void _lis2dw12_it_start(uint8_t shock)
{
	uint8_t ctrl4 = shock?CTRL4_VAL_WITH_TAP:0;
	uint8_t ctrl5 = CTRL5_VAL_HIGH_SENSI;

	// Configure interruption here instead of using enable interrupts.
	// That way the chip remains in sleep state before starting.
	_lis2dw12_register_set(REG_ADDR_CTRL4_INT1_PAD_CTRL, ctrl4);
	_lis2dw12_register_set(REG_ADDR_CTRL5_INT2_PAD_CTRL, ctrl5);
}

static void _lis2dw12_it_stop()
{
	// Disable interrupts by clearing them.
	_lis2dw12_register_set(REG_ADDR_CTRL4_INT1_PAD_CTRL, 0);
	_lis2dw12_register_set(REG_ADDR_CTRL5_INT2_PAD_CTRL, 0);
}

static bool _lis2dw12_open_device(void)
{
	aos_gpio_config_t gconfig = {0};


	// Turn on the power to the LISDW. This should always remain on, as cutting
	// power will drain a few hundred micro-amps. Instead, the chip needs to be
	// set to power-down mode.
	if (_lis2dw12_ctx.init_info.power_gpio != aos_gpio_id_last) {
		aos_gpio_open(_lis2dw12_ctx.init_info.power_gpio, aos_gpio_mode_output);
		aos_gpio_write(_lis2dw12_ctx.init_info.power_gpio, 1);
	}

	// Init the I2C bus on the fly, as the MEMS is the only device using it.
	_lis2dw12_ctx.i2c_hdl = aos_i2c_master()->open(aos_i2c_bus_id_external);
	if (!_lis2dw12_ctx.i2c_hdl) {
		return false;
	}

	vTaskDelay(pdMS_TO_TICKS(LIS2DW12_BOOT_TIME_MS));	// Allow device to initialize

	// Configure interrupt
	gconfig.mode = aos_gpio_mode_input;
	gconfig.irq_mode = aos_gpio_irq_mode_rising_edge;
	gconfig.irq_prio = aos_gpio_irq_priority_medium;
	gconfig.irq_servicing = aos_gpio_irq_service_type_thread;
	gconfig.irq_handler.sys_cb = _lis2dw12_int1_handler;

	if (aos_gpio_open_ext(_lis2dw12_ctx.init_info.interrupt_gpio, &gconfig) != aos_result_success) {
		if (_lis2dw12_ctx.init_info.power_gpio != aos_gpio_id_last) {
			aos_gpio_open(_lis2dw12_ctx.init_info.power_gpio, aos_gpio_mode_output);
			aos_gpio_write(_lis2dw12_ctx.init_info.power_gpio, 1);
		}
		aos_i2c_master()->close(_lis2dw12_ctx.i2c_hdl);
		_lis2dw12_ctx.i2c_hdl = aos_i2c_handle_invalid;
		return false;
	}
	return true;
}

static bool _lis2dw12_initialize(board_accelero_init_info_t* info)
{

	memset(&_lis2dw12_ctx, 0, sizeof(_lis2dw12_ctx));
	memcpy(&_lis2dw12_ctx.init_info, info, sizeof(_lis2dw12_ctx.init_info));

	// Create the timer for data acquisition:
	// We want x samples after the shock triggered: FIFO feeding frequency in ms: 1000/ODR.
	_lis2dw12_ctx.timer_hdl =  xTimerCreateStatic("LIS2DW12",
			pdMS_TO_TICKS((ONE_SECOND * NB_SHOCK_SAMPLES) / ODR_FREQUENCY(_lis2dw12_ctx.odr)),
			pdFALSE,
			&_lis2dw12_ctx.timer_count,
			_lis2dw12_timeout,
			&_lis2dw12_ctx.timer_local_data);
	xTimerStop(_lis2dw12_ctx.timer_hdl, 0);

	// Mark as properly initialized
	_lis2dw12_ctx.init_done = true;

	return true;
}

static void _lis2dw12_close_device(void)
{
	// Close the interrupt GPIO
	if (!_lis2dw12_ctx.init_done) {
		// Not initialized;
		return;
	}

	aos_gpio_close(_lis2dw12_ctx.init_info.interrupt_gpio);

	// Do not power down the LIS. Instead put in on power off mode;
	_lis2dw12_register_set(REG_ADDR_CTRL1, 0);

	// Stop timer
	xTimerStop(_lis2dw12_ctx.timer_hdl, 0);

	// De-init the I2C bus
	aos_i2c_master()->close(_lis2dw12_ctx.i2c_hdl);
	_lis2dw12_ctx.i2c_hdl = aos_i2c_handle_invalid;
}

static bool _lis2dw12_configure_high_sensitivity(lis2dw12_ctx_t* ctx,
		uint32_t sensitivity,
		uint32_t debounce,
		uint32_t aslp)
{
	uint8_t wakeup_dur = 0;


	if (sensitivity > LIS2DW12_MAX_SENSI_CFG) {
		sensitivity = LIS2DW12_MAX_SENSI_CFG;
	}

	if (!_lis2dw12_setup(_lis2dw12_high_sensi_setup)) {
		return false;
	}

	// Setup FS via CTRL6
	_lis2dw12_register_set(REG_ADDR_CTRL6, ((ctx->fs << CTRL6_FS_SHIFT)| CTRL6_BW_FILT_ODR_OVER_2));

	// Try to adapt the sleep time
	if (aslp) {
		uint32_t val = (aslp * ODR_FREQUENCY(ctx->odr))/1000;	// ODR in Hz
		wakeup_dur = (val/512) + (((val%512) > 256)?1:0);
		if (!wakeup_dur) {
			// Check the best approach between 0 an 1
			val = (16 * 1000) / ODR_FREQUENCY(ctx->odr);
			if (val < aslp) {
				wakeup_dur = 1;
			}
		}
	}

	// Calculate the actual wakeup_ duration in ms
	if (!wakeup_dur) {
		ctx->wake_time = (16 * 1000) / ODR_FREQUENCY(ctx->odr);
	} else {
		ctx->wake_time = (512 * 1000 * wakeup_dur) / ODR_FREQUENCY(ctx->odr);
	}

	// Setup the polling time
	ctx->polling_timeout = ctx->wake_time / 2;
	if (ctx->polling_timeout > LIS2DW12_WAKE_POLLING_MAX_TIMEOUT) {
		ctx->polling_timeout = LIS2DW12_WAKE_POLLING_MAX_TIMEOUT;
	}

	// Debounce: step 1/ODR. restricted to max 3 * 1/ODR
	if (debounce > 3) {
		debounce = 3;
	}
	wakeup_dur |= debounce << REG_WAKE_UP_DUR_SHIFT;

	_lis2dw12_register_set(REG_ADDR_WAKE_UP_DUR, wakeup_dur);

	// Configuration step is 0.063g.
	// Accelero step is FS/64.
	if (ctx->fs == board_accelero_fs_type_2G) {
		// In this case the step is 2/64 = 0.031 mg so multiply the sensitivity by 2
		sensitivity *=2;
	} else {
		sensitivity = sensitivity / (1 << (ctx->fs - 1));
	}

	if (!sensitivity) {
		sensitivity = 1;
	} else if (sensitivity > MAX_THRESHOLD_VALUE) {
		sensitivity = MAX_THRESHOLD_VALUE;
	}
	_lis2dw12_register_set(REG_ADDR_WAKE_UP_THS, REG_BIT_WAKE_UP_THS_SLEEP_ON | sensitivity);

	// Finally configure the CTRL1 register, which starts the MEMS. Select ODR.
	return _lis2dw12_register_set(REG_ADDR_CTRL1, (CTRL1_VAL_ODR_12_5 + ctx->odr) << 4);
}

static void _lis2dw12_configure_shock(uint8_t threshold)
{
	// We use the tap feature
	if (threshold > MAX_TAP_THRESHOLD_VALUE) {
		threshold = MAX_TAP_THRESHOLD_VALUE;
	}
	_lis2dw12_register_set(REG_ADDR_TAP_THS_X, threshold);
	_lis2dw12_register_set(REG_ADDR_TAP_THS_Y, threshold);
	_lis2dw12_register_set(REG_ADDR_TAP_THS_Z, threshold | REG_BIT_TAP_X_EN | REG_BIT_TAP_Y_EN | REG_BIT_TAP_Z_EN);
	_lis2dw12_register_set(REG_ADDR_INT_DUR, INT_DUR_DEFAULT);
}

// Reset the mems, Need to reconfigure after
static void _lis2dw12_reset(void)
{
	// Power on and reset accelerometer
	_lis2dw12_register_set(REG_ADDR_CTRL2, CRTL2_RESET_VAL);
	vTaskDelay(pdMS_TO_TICKS(LIS2DW12_BOOT_TIME_MS));
}

static board_accelero_result_t _lis2dw12_configure(lis2dw12_ctx_t* ctx, board_accelero_config_t* config)
{
	uint8_t sensitivity = config->motion_sensi;
	uint8_t shock_thx;

	ctx->fs = config->fs;
	ctx->odr = config->odr;
	ctx->user_cb = config->callback;
	ctx->user_arg = config->user_arg;
	_lis2dw12_configure_high_sensitivity(ctx, sensitivity,	config->motion_debounce, config->wake_duration);

	// Actual threshold is based on the FS. 0 means no shock detection
	shock_thx = config->shock_threshold;
	if (shock_thx) {
		// Configuration step is 0.063g. So adapt based on the selected FS
		shock_thx /= (1 << config->fs);
	}

	if (config->shock_threshold) {
		// Shock configured. Check again the shock_thx since it could set to null
		if (shock_thx) {
			// If we are running shock detection then configure it
			_lis2dw12_configure_shock(shock_thx);
		} else {
			aos_log_warning(aos_log_module_app, true, "Shock disabled due to either the selected mode or to a too low threshold\n");
		}
	}

	// Enable interrupts
	_lis2dw12_register_set(REG_ADDR_CTRL7, CTRL7_VAL_DEFAULT | CTRL7_USR_OFF_ON_WU | CTRL7_USR_OFF_W);

	// Setup the interrupts
	_lis2dw12_it_start(config->shock_threshold);
	_lis2dw12_ctx.opening = true;

	// Start the timer: We have to wait for at least one wake duration. Add here a margin
	xTimerChangePeriod(_lis2dw12_ctx.timer_hdl, pdMS_TO_TICKS(_lis2dw12_ctx.wake_time + 200), 0);

	return board_accelero_result_success;
}
static bool _lis2dw12_device_exist(aos_i2c_handle_t handle, uint8_t address)
{
	aos_i2c_ioctl_t ioc;

	ioc.request = aos_i2c_ioctl_req_device_exists;
	ioc.value = address;

	return (aos_i2c_master()->ioctl(handle, &ioc) == aos_result_success);
}

/*
 * ***********************************************
 * Driver access
 * ***********************************************
 */
static board_accelero_result_t _lis2dw12_init(board_accelero_init_info_t* info)
{
	bool exist = false;

	if (info->interrupt_gpio == aos_gpio_id_last) {
		return board_accelero_result_other_error;
	}

	if (_lis2dw12_ctx.init_done) {
		return board_accelero_result_success;
	}

	if (!_lis2dw12_initialize(info)) {
		return board_accelero_result_other_error;
	}

	// Open the chip
	if (!_lis2dw12_open_device()) {
		return board_accelero_result_other_error;
	}

	if (_lis2dw12_device_exist(_lis2dw12_ctx.i2c_hdl, LIS2DW12_I2C_BUS_ADDR1)) {
		exist = true;
		_lis2dw12_ctx.i2c_address = LIS2DW12_I2C_BUS_ADDR1;
	} else if (_lis2dw12_device_exist(_lis2dw12_ctx.i2c_hdl, LIS2DW12_I2C_BUS_ADDR2)) {
		exist = true;
		_lis2dw12_ctx.i2c_address = LIS2DW12_I2C_BUS_ADDR2;
	}

	if ((!exist) && (info->power_gpio != aos_gpio_id_last)) {
		aos_gpio_close(info->power_gpio);
	}

	_lis2dw12_close_device();
	return exist?board_accelero_result_success:board_accelero_result_chip_not_found;
}

static board_accelero_result_t _lis2dw12_open(board_accelero_config_t* config)
{
	if (!_lis2dw12_ctx.init_done) {
		// Init not done
		return board_accelero_result_no_init;
	}

	if (_lis2dw12_ctx.i2c_hdl != aos_i2c_handle_invalid) {
		// Already open
		return board_accelero_result_success;
	}

	if (!config->callback) {
		return board_accelero_result_bad_params;
	}

	if (!_lis2dw12_open_device()) {
		return board_accelero_result_other_error;
	}
	return _lis2dw12_configure(&_lis2dw12_ctx, config);
}

static board_accelero_result_t _lis2dw12_close(void)
{
	if (!_lis2dw12_ctx.init_done) {
		// Init not done
		return board_accelero_result_no_init;
	}

	if (_lis2dw12_ctx.i2c_hdl != aos_i2c_handle_invalid) {
		_lis2dw12_it_stop();
		_lis2dw12_close_device();
	}
	return board_accelero_result_success;
}

static board_accelero_result_t _lis2dw12_read_data(fix16_vector_t* vector)
{
	if (_lis2dw12_ctx.i2c_hdl == aos_i2c_handle_invalid) {
		// Not open
		return board_accelero_result_not_open;
	}

	// Get the last sample
	return _lis2dw12_get_data(vector, NULL, LIS2DW12_FIFO_NB_SAMPLES, false)?board_accelero_result_success:board_accelero_result_data_not_ready;
}

static board_accelero_result_t _lis2dw12_get_state(lis2dw12_ctx_t *ctx, board_accelero_state_t *state)
{
	uint8_t reg;

	if (ctx->i2c_hdl == aos_i2c_handle_invalid) {
		return board_accelero_result_not_open;
	}

	if (ctx->opening) {
		*state = board_accelero_state_starting;
	} else {
		// Read the status register
		if (!_lis2dw12_register_get(REG_ADDR_STATUS, &reg)) {
			return board_accelero_result_other_error;
		}
		if (reg & REG_BIT_STATUS_SLEEP_STATE) {
			*state = board_accelero_state_sleep;
		} else {
			*state = board_accelero_state_wake;
		}
	}

	return board_accelero_result_success;
}

static board_accelero_result_t _lis2dw12_get_current(lis2dw12_ctx_t *ctx, uint32_t *current)
{
	board_accelero_state_t state;
	board_accelero_result_t result;

	if (_lis2dw12_ctx.i2c_hdl == aos_i2c_handle_invalid) {
		return board_accelero_result_not_open;
	}

	result = _lis2dw12_get_state(ctx, &state);
	if (result != board_accelero_result_success) {
		return result;
	}
	if (state == board_accelero_state_wake) {
		*current = _high_sensi_consumption[ctx->odr];
	} else {
		*current = LIS2DW12_CONSO_SLEEP_UA;
	}

	return board_accelero_result_success;
}

static board_accelero_result_t _lis2dw12_ioctl(board_accelero_ioctl_t* info)
{
	if (_lis2dw12_ctx.i2c_hdl == aos_i2c_handle_invalid) {
		return board_accelero_result_not_open;
	}

	switch (info->type) {
	case board_accelero_ioctl_type_reconfigure:
		// Command may not may not work since mems_setup cannot be called when active!
		// Reset the mems
		_lis2dw12_reset();
		return _lis2dw12_configure(&_lis2dw12_ctx, info->config);

	case board_accelero_ioctl_type_get_state:
		return _lis2dw12_get_state(&_lis2dw12_ctx, &info->state);

	case board_accelero_ioctl_type_get_current:
		return _lis2dw12_get_current(&_lis2dw12_ctx, &info->current);

	case board_accelero_ioctl_type_get_info:
		info->info.fs = _lis2dw12_ctx.fs;
		info->info.odr = _lis2dw12_ctx.odr;
		info->info.wake_time = _lis2dw12_ctx.wake_time;
		info->info.poll_timeout = _lis2dw12_ctx.polling_timeout;
		return board_accelero_result_success;

	case board_accelero_ioctl_type_read_reg:
		if (info->reg_item->reg < REG_FIRST_ADDR) {
			info->reg_item->reg = REG_FIRST_ADDR;
		}
		if (info->reg_item->reg > REG_LAST_ADDR) {
			return board_accelero_result_bad_params;
		}
		return _lis2dw12_register_get(info->reg_item->reg, &info->reg_item->data)?board_accelero_result_success:board_accelero_result_other_error;

	case board_accelero_ioctl_type_write_reg:
		if ((info->reg_item->reg < REG_FIRST_ADDR) || (info->reg_item->reg > REG_LAST_ADDR)) {
			return board_accelero_result_bad_params;
		}
		return _lis2dw12_register_set(info->reg_item->reg, info->reg_item->data)?board_accelero_result_success:board_accelero_result_other_error;
	}

	return board_accelero_result_bad_params;
}

/*
 * ***********************************************
 * Driver export
 * ***********************************************
 */
const board_accelero_drv_t board_drv_lis2dw12 = {
		_lis2dw12_init,
		_lis2dw12_open,
		_lis2dw12_close,
		_lis2dw12_read_data,
		_lis2dw12_ioctl,
};

/*
 * ***********************************************
 * Helper
 * ***********************************************
 */
int16_t board_accelero_fix16_to_mg(fix16_t f16)
{
	return fix16_to_int(fix16_mul(f16, FIX16_THOUSAND));
}

