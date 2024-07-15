/*!
 * \file srv_ble_dtm.h
 *
 * \brief BLE Direct Test Mode header file
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup srv_ble_dtm Device test mode service
 *
 * \addtogroup srv_ble_dtm
 * @{
 */

#define SRV_BLE_DTM_MIN_CHAN_IDX     0			//!< First BLE channel index
#define SRV_BLE_DTM_MAX_CHAN_IDX     39			//!< Last BLE channel index
#define SRV_BLE_DTM_MIN_DATA_LEN     0			//!< Min Length in bytes of payload data in each packet
#define SRV_BLE_DTM_MAX_DATA_LEN     37			//!< Max Length in bytes of payload data in each packet
#define SRV_BLE_DTM_MIN_PAYLOAD_TYPE 0			//!< Min type of packet payload
#define SRV_BLE_DTM_MAX_PAYLOAD_TYPE 7			//!< Max type of packet payload
#define SRV_BLE_DTM_PHY_1M           1			//!< 1M PHY to use for test packet
#define SRV_BLE_DTM_PHY_2M           2			//!< 2M PHY to use for test packet
#define SRV_BLE_DTM_STDR_MODULATION  0			//!< Standard modulation index capability of the transmitter
#define SRV_BLE_DTM_STBL_MODULATION  1			//!< Stable modulation index capability of the transmitter

/*!
 * \enum srv_ble_dtm_test_mode_t
 *
 * \brief test mode state
 */
typedef enum {
	srv_ble_dtm_test_mode_off = 0,		//!< test mode off
	srv_ble_dtm_test_mode_tone,			//!< test mode tone running
	srv_ble_dtm_test_mode_tx,			//!< test mode tx running
	srv_ble_dtm_test_mode_rx			//!< test mode rx running
} srv_ble_dtm_test_mode_t;

/*!
 * \enum srv_ble_dtm_payload_type_t
 *
 * \brief Type of packet payload
 */
typedef enum {
	srv_ble_dtm_payload_type_pseudo_random_bit_seq_9,		//!< Pseudo-Random bit sequence 9
	srv_ble_dtm_payload_type_alternating_bits_11110000,		//!< Pattern of alternating bits '11110000'
	srv_ble_dtm_payload_type_alternating_bits_10101010,		//!< Pattern of alternating bits '10101010'
	srv_ble_dtm_payload_type_pseudo_random_bit_seq_15,		//!< Pseudo-Random bit sequence 15
	srv_ble_dtm_payload_type_all_ones,						//!< Pattern of All '1' bits
	srv_ble_dtm_payload_type_all_zeros,						//!< Pattern of All '0' bits
	srv_ble_dtm_payload_type_alternating_bits_00001111,		//!< Pattern of alternating bits '00001111'
	srv_ble_dtm_payload_type_alternating_bits_0101,			//!< Pattern of alternating bits '0101'
	srv_ble_dtm_payload_type_count,
} srv_ble_dtm_payload_type_t;

/*!
 * \enum srv_ble_dtm_phy_t
 *
 * \brief PHY to use for test packet
 */
typedef enum {
	srv_ble_dtm_phy_le_1m = 1,		//!< Transmitter set to use the LE 1M PHY
	srv_ble_dtm_phy_le_2m,			//!< Transmitter set to use the LE 2M PHY
	srv_ble_dtm_phy_coded_s8,		//!< Transmitter set to use the LE Coded PHY with S=8 data coding
	srv_ble_dtm_phy_coded_s2,		//!< Transmitter set to use the LE Coded PHY with S=2 data coding
	srv_ble_dtm_phy_count,
} srv_ble_dtm_phy_t;

/*!
 * \enum srv_ble_dtm_modulation_index_t
 *
 * \brief Modulation_Index Modulation index capability of the transmitter
 */
typedef enum {
	srv_ble_dtm_modulation_index_standard,		//!< Assume transmitter will have a standard modulation index
	srv_ble_dtm_modulation_index_stable,		//!< Assume transmitter will have a stable modulation index
	srv_ble_dtm_modulation_index_count,
} srv_ble_dtm_modulation_index_t;

/*!
 * \struct srv_ble_dtm_param_t
 *
 * \brief BLE DTM context structure
 *
 * \note
 * channel_idx: channel index for tone test, TX_Frequency N = (F - 2402) / 2
 *        Frequency Range : 2402 MHz to 2480 MHz
 *        Values:
 *        - 0 ... 39
 * data_length: Length in bytes of payload data in each packet.
 *        Values:
 *        - 0 ... 37
 * packet_payload: Payload Type.
 *        Values:
 *        - 0x00: Pseudo-Random bit sequence 9
 *        - 0x01: Pattern of alternating bits '11110000'
 *        - 0x02: Pattern of alternating bits '10101010'
 *        - 0x03: Pseudo-Random bit sequence 15
 *        - 0x04: Pattern of All '1' bits
 *        - 0x05: Pattern of All '0' bits
 *        - 0x06: Pattern of alternating bits '00001111'
 *        - 0x07: Pattern of alternating bits '0101'
 * phy: PHY to use for test packet
 *        Values:
 *        - 0x00: Reserved for future use
 *        - 0x01: Transmitter set to use the LE 1M PHY
 *        - 0x02: Transmitter set to use the LE 2M PHY
 *        - 0x03: Transmitter set to use the LE Coded PHY with S=8 data coding
 *        - 0x04: Transmitter set to use the LE Coded PHY with S=2 data coding
 * modulation_idx: Modulation index capability of the transmitter
 *        Values:
 *        - 0x00: Assume transmitter will have a standard modulation index
 *        - 0x01: Assume transmitter will have a stable modulation index
 * test_mode: Actual test mode
 */
typedef struct {
    uint8_t channel_idx;							//!< channel index for tone test, TX_Frequency N = (F - 2402) / 2
    uint8_t data_length;							//!< Length in bytes of payload data in each packet
    srv_ble_dtm_payload_type_t packet_payload;		//!< Type of packet payload
    srv_ble_dtm_phy_t phy;							//!< PHY to use for test packet
    srv_ble_dtm_modulation_index_t modulation_idx; 	//!< Modulation index capability of the transmitter
    srv_ble_dtm_test_mode_t test_mode;				//!< Actual test mode
} srv_ble_dtm_param_t;

typedef uint32_t srv_ble_test_count_result_t;	//!< Total number of transmitted/received packets during TX/RX test

/*!
 * \brief Init test context
 *
 * \return ongoing test result
 *
 */
void srv_ble_dtm_init(void);

/*!
 * \brief Get Tx or Tx test mode results
 *
 * \return  Total number of transmitted/received packets during TX/RX test
 *
 */
srv_ble_test_count_result_t srv_ble_dtm_get_result(void);

/*!
 * \brief Set test parameters
 *
 * \param param test parameters
 *
 */
void srv_ble_dtm_set_params(srv_ble_dtm_param_t* param);

/*!
 * \brief Start carrier transmission
 *
 * \return Result of the operation
 */
bool srv_ble_dtm_tone_start(void);

/*!
 * \brief Start TX test mode
 * This command is used to start a test where the DUT transmit test reference
 * packets at a fixed interval.
 *
 * \return Result of the operation
 *
 * \note, The function expects the gpio opened before calling
 */
bool srv_ble_dtm_tx_start(void);

/*!
 * \brief Start RX test mode
 * This command is used to start a test where the DUT receives test reference
 * packets at a fixed interval. The tester generates the test reference packets.
 * see Bluetooth Specification v5.0 [Vol 6] Part B, Section 7.8.50
 *
 * \return Result of the operation
 */
bool srv_ble_dtm_rx_start(void);

/*!
 * \brief Return DTM status
 * This function return the running test or off
 *
 * \return Return actual test mode
 */
srv_ble_dtm_test_mode_t srv_ble_dtm_get_test_mode(void);

/*!
 * \brief Stop running test
 *
 * \return Result of the operation
 */
bool srv_ble_dtm_test_stop(void);

/*! @}*/
#ifdef __cplusplus
}
#endif
