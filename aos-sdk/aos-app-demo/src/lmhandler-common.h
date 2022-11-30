/*
 * lmhandler-common.h
 *
 *  Created on: Sep 27, 2022
 *      Author: marc
 */
#pragma once

#include "LmHandler.h"
#include "srv_provisioning.h"

/*!
 * \brief lmhandler_set_default_callbacks() - set up default callbacks.
 *
 * This function provides a set of default callbacks for the LmHandler.
 * For specific handling, the caller must override the default functions
 * in the LmHandlerCallbacks structure.
 *
 * \param lcp - pointer to an LmHandlerCallbacks_t structure. This struct
 *              is filled in with default callback functions.
 *
 */
void lmhandler_set_default_callbacks(LmHandlerCallbacks_t *lcp);

/*!
 * \brief lmhandler_map_mac_region() - map a provisioning region to a LoRaMAC region.
 *
 * This function maps the srv_provisioning_mac_region_t enumeration value
 * to the corresponding LoRaMacRegion_t enumeration value.
 *
 * \param [IN] prv_region - srv_provisioning_mac_region_t value to convert
 * \param [OUT] mac_region - pointer to a LoRaMacRegion_t value to update.
 *
 * \return The function returns true on success, false otherwise.
 *
 * \note When an error status is returned, the mac_region is not updated.
 *
 */
bool lmhandler_map_mac_region(srv_provisioning_mac_region_t prv_region, LoRaMacRegion_t *mac_region);
