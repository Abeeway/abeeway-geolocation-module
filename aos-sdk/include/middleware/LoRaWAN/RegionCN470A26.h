/*!
 * \file  RegionCN470A26.h
 *
 * \brief Specific implementations of Channel plan type A, 26MHz.
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
 * \defgroup  REGIONCN470
 *
 * \{
 */
#ifndef __REGION_CN470_A26_H__
#define __REGION_CN470_A26_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "region/Region.h"

/*!
 * The maximum number of channels.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_CHANNELS_MASK_SIZE        3

/*!
 * The number of entries in the join accept list.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_JOIN_ACCEPT_LIST_SIZE     3

/*!
 * The number of channels available for the beacon.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_BEACON_NB_CHANNELS        1

/*!
 * The number of channels available for the ping slots.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_PING_SLOT_NB_CHANNELS     1

/*!
 * The first RX channel, downstream group 1.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_FIRST_RX_CHANNEL          490100000

/*!
 * The last RX channel, downstream group 1.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_LAST_RX_CHANNEL           494700000

/*!
 * The frequency stepwidth between RX channels,
 * downstream group 1.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_STEPWIDTH_RX_CHANNEL      200000

/*!
 * The first TX channel, upstream group 1.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_FIRST_TX_CHANNEL          470300000

/*!
 * The last TX channel, upstream group 1.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_LAST_TX_CHANNEL           479700000

/*!
 * The frequency stepwidth between RX channels,
 * upstream group 1.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_STEPWIDTH_TX_CHANNEL      200000

/*!
 * The default frequency for RX window 2
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_RX_WND_2_FREQ             492500000

/*!
 * The default frequency for beacon.
 * Channel plan type A, 26MHz.
 */
#define CN470_A26_BEACON_FREQ               494900000

/*!
 * \brief Calculation of the beacon frequency.
 *
 * \param [IN] channel The Beacon channel number.
 *
 * \param [IN] joinChannelIndex The join channel index.
 *
 * \param [IN] isPingSlot Set to true, if its a ping slot.
 *
 * \retval Returns the beacon frequency.
 */
uint32_t RegionCN470A26GetDownlinkFrequency( uint8_t channel, uint8_t joinChannelIndex, bool isPingSlot );

/*!
 * \brief Performs the update of the channelsMask based on the input parameters
 *        for the Channel plan type A, 26MHz.
 *
 * \param [IN] joinChannelIndex The join channel index.
 *
 * \retval Returns the offset for the given join channel.
 */
uint8_t RegionCN470A26GetBeaconChannelOffset( uint8_t joinChannelIndex );

/*!
 * \brief Performs the update of the channelsMask based on the input parameters
 *        for the Channel plan type A, 26MHz.
 *
 * \param [IN] channelsMask A pointer to the channels mask.
 *
 * \param [IN] chMaskCntl The value of the chMaskCntl field of the LinkAdrReq.
 *
 * \param [IN] chanMask The value of the chanMask field of the LinkAdrReq.
 *
 * \param [IN] channels A pointer to the available channels.
 *
 * \retval Status of the operation. Return 0x07 if the channels mask is valid.
 */
uint8_t RegionCN470A26LinkAdrChMaskUpdate( uint16_t* channelsMask, uint8_t chMaskCntl,
                                           uint16_t chanMask, ChannelParams_t* channels );

/*!
 * \brief Verifies if the frequency provided is valid
 *        for the Channel plan type A, 26MHz.
 *
 * \param [IN] frequency The frequency to verify.
 *
 * \retval Returns true, if the frequency is valid.
 */
bool RegionCN470A26VerifyRfFreq( uint32_t frequency );

/*!
 * \brief Initializes all channels, datarates, frequencies and bands
 *        for the Channel plan type A, 26MHz.
 *
 * \param [IN] channels A pointer to the available channels.
 */
void RegionCN470A26InitializeChannels( ChannelParams_t* channels );

/*!
 * \brief Initializes the channels default mask
 *        for the Channel plan type A, 26MHz.
 *
 * \param [IN] channelsDefaultMask A pointer to the channels default mask.
 */
void RegionCN470A26InitializeChannelsMask( uint16_t* channelsDefaultMask );

/*!
 * \brief Computes the frequency for the RX1 window
 *        for the Channel plan type A, 26MHz.
 *
 * \param [IN] channel The channel utilized currently.
 *
 * \retval Returns the frequency which shall be used.
 */
uint32_t RegionCN470A26GetRx1Frequency( uint8_t channel );

/*!
 * \brief Computes the frequency for the RX2 window
 *        for the Channel plan type A, 26MHz.
 *
 * \param [IN] joinChannelIndex The join channel index.
  *
 * \param [IN] isOtaaDevice Set to true, if the device is an OTAA device.
 *
 * \retval Returns the frequency which shall be used.
 */
uint32_t RegionCN470A26GetRx2Frequency( uint8_t joinChannelIndex, bool isOtaaDevice );

/*! \} defgroup REGIONCN470 */

#ifdef __cplusplus
}
#endif

#endif // __REGION_CN470_A26_H__
