/*!
 * \file aos-sdk-version.h
 *
 * \brief AOS SDK Semantic versioning, as per https://semver.org/spec/v2.0.0.html
 *
 *  Created on: Sep 29, 2022
 *      Author: marc
 */

#pragma once

#define AOS_SDK_VERSION_MAJOR 0		// MAJOR version - update when you make incompatible API changes
#define AOS_SDK_VERSION_MINOR 5		// MINOR version - update when you add functionality in a backwards compatible manner
#define AOS_SDK_VERSION_PATCH 0		// PATCH version - update when you make backwards compatible bug fixes
#define AOS_SDK_VERSION_ELOAD 0	// 0 = release, 255 = development, else e-load number.

#define AOS_SDK_VERSION_NUMBER (AOS_SDK_VERSION_MAJOR<<24 | AOS_SDK_VERSION_MINOR << 16 | AOS_SDK_VERSION_PATCH << 8 | AOS_SDK_VERSION_ELOAD)
