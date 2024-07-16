/*!
 * \file  LoRaMacMessageTypes.h
 *
 * \brief LoRa MAC layer message type definitions
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright Stackforce 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*!
 * \addtogroup LORAMAC
 * \{
 *
 */
#ifndef __LORAMAC_MESSAGE_TYPES_H__
#define __LORAMAC_MESSAGE_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "LoRaMacHeaderTypes.h"

/*!
 * LoRaMac type for Join-request message
 */
typedef struct sLoRaMacMessageJoinRequest
{
    /*!
     * Serialized message buffer
     */
    uint8_t* Buffer;
    /*!
     * Size of serialized message buffer
     */
    uint8_t BufSize;
    /*!
     * MAC header
     */
    LoRaMacHeader_t MHDR;
    /*!
     *  Join EUI
     */
    uint8_t JoinEUI[LORAMAC_JOIN_EUI_FIELD_SIZE];
    /*!
     * Device EUI
     */
    uint8_t DevEUI[LORAMAC_DEV_EUI_FIELD_SIZE];
    /*!
     * Device Nonce
     */
    uint16_t DevNonce;
    /*!
     * Message integrity code (MIC)
     */
    uint32_t MIC;
}LoRaMacMessageJoinRequest_t;

/*!
 * LoRaMac type for rejoin-request type 1 message
 */
typedef struct sLoRaMacMessageReJoinType1
{
    /*!
     * Serialized message buffer
     */
    uint8_t* Buffer;
    /*!
     * Size of serialized message buffer
     */
    uint8_t BufSize;
    /*!
     * MAC header
     */
    LoRaMacHeader_t MHDR;
    /*!
     * Rejoin-request type ( 1 )
     */
    uint8_t ReJoinType;
    /*!
     *  Join EUI
     */
    uint8_t JoinEUI[LORAMAC_JOIN_EUI_FIELD_SIZE];
    /*!
     * Device EUI
     */
    uint8_t DevEUI[LORAMAC_DEV_EUI_FIELD_SIZE];
    /*!
     * ReJoin Type 1 counter
     */
    uint16_t RJcount1;
    /*!
     * Message integrity code (MIC)
     */
    uint32_t MIC;
}LoRaMacMessageReJoinType1_t;

/*!
 * LoRaMac type for rejoin-request type 0 or 2 message
 */
typedef struct sLoRaMacMessageReJoinType0or2
{
    /*!
     * Serialized message buffer
     */
    uint8_t* Buffer;
    /*!
     * Size of serialized message buffer
     */
    uint8_t BufSize;
    /*!
     * MAC header
     */
    LoRaMacHeader_t MHDR;
    /*!
     * Rejoin-request type ( 0 or 2 )
     */
    uint8_t ReJoinType;
    /*!
     * Network ID ( 3 bytes )
     */
    uint8_t NetID[LORAMAC_NET_ID_FIELD_SIZE];
    /*!
     * Device EUI
     */
    uint8_t DevEUI[LORAMAC_DEV_EUI_FIELD_SIZE];
    /*!
     * ReJoin Type 0 and 2 frame counter
     */
    uint16_t RJcount0;
    /*!
     * Message integrity code (MIC)
     */
    uint32_t MIC;
}LoRaMacMessageReJoinType0or2_t;

/*!
 * LoRaMac type for Join-accept message
 */
typedef struct sLoRaMacMessageJoinAccept
{
    /*!
     * Serialized message buffer
     */
    uint8_t* Buffer;
    /*!
     * Size of serialized message buffer
     */
    uint8_t BufSize;
    /*!
     * MAC header
     */
    LoRaMacHeader_t MHDR;
    /*!
     *  Server Nonce ( 3 bytes )
     */
    uint8_t JoinNonce[LORAMAC_JOIN_NONCE_FIELD_SIZE];
    /*!
     * Network ID ( 3 bytes )
     */
    uint8_t NetID[LORAMAC_NET_ID_FIELD_SIZE];
    /*!
     * Device address
     */
    uint32_t DevAddr;
    /*!
     * Device address
     */
    LoRaMacDLSettings_t DLSettings;
    /*!
     * Delay between TX and RX
     */
    uint8_t RxDelay;
    /*!
     * List of channel frequencies (opt.)
     */
    uint8_t CFList[16];
    /*!
     * Message integrity code (MIC)
     */
    uint32_t MIC;
}LoRaMacMessageJoinAccept_t;


/*!
 * LoRaMac type for Data MAC messages
 * (Unconfirmed Data Up, Confirmed Data Up, Unconfirmed Data Down, Confirmed Data Down)
 */
typedef struct sLoRaMacMessageData
{
    /*!
     * Serialized message buffer
     */
    uint8_t* Buffer;
    /*!
     * Size of serialized message buffer
     */
    uint8_t BufSize;
    /*!
     * MAC header
     */
    LoRaMacHeader_t MHDR;
    /*!
     * Frame header (FHDR)
     */
    LoRaMacFrameHeader_t FHDR;
    /*!
     * Port field (opt.)
     */
    uint8_t FPort;
    /*!
     * Frame payload may contain MAC commands or data (opt.)
     */
    uint8_t* FRMPayload;
    /*!
     * Size of frame payload (not included in LoRaMac messages) 
     */
    uint8_t FRMPayloadSize;
    /*!
     * Message integrity code (MIC)
     */
    uint32_t MIC;
}LoRaMacMessageData_t;

/*!
 * LoRaMac message type enumerator
 */
typedef enum eLoRaMacMessageType
{
    /*!
     * Join-request message
     */
    LORAMAC_MSG_TYPE_JOIN_REQUEST,
    /*!
     * Rejoin-request type 1 message
     */
    LORAMAC_MSG_TYPE_RE_JOIN_1,
    /*!
     * Rejoin-request type 1 message
     */
    LORAMAC_MSG_TYPE_RE_JOIN_0_2,
    /*!
     * Join-accept message
     */
    LORAMAC_MSG_TYPE_JOIN_ACCEPT,
    /*!
     * Data MAC messages
     */
    LORAMAC_MSG_TYPE_DATA,
    /*!
     * Undefined message type
     */
    LORAMAC_MSG_TYPE_UNDEF,
}LoRaMacMessageType_t;

/*!
 * LoRaMac general message type
 */
typedef struct sLoRaMacMessage
{
    LoRaMacMessageType_t Type;
    union uMessage
    {
        LoRaMacMessageJoinRequest_t JoinReq;
        LoRaMacMessageReJoinType1_t ReJoin1;
        LoRaMacMessageReJoinType0or2_t ReJoin0or2;
        LoRaMacMessageJoinAccept_t JoinAccept;
        LoRaMacMessageData_t Data;
    }Message;
}LoRaMacMessage_t;

/*! \} addtogroup LORAMAC */

#ifdef __cplusplus
}
#endif

#endif // __LORAMAC_MESSAGE_TYPES_H__