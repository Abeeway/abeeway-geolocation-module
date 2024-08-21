/*!
 * \file aos-sdk-version.h
 *
 * \brief AOS SDK Semantic versioning
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

#pragma once

#define AOS_SDK_VERSION_MAJOR 1		//!< MAJOR version - update when you make incompatible API changes
#define AOS_SDK_VERSION_MINOR 2		//!< MINOR version - update when you add functionality in a backwards compatible manner
#define AOS_SDK_VERSION_PATCH 1		//!< PATCH version - 0: Main release. [1..127]: Release with bug fixes. [128..192]: Eloads. [192 ..255] Candidates

#define AOS_SDK_VERSION_NUMBER (AOS_SDK_VERSION_MAJOR<<16 | AOS_SDK_VERSION_MINOR << 8 | AOS_SDK_VERSION_PATCH)		//!< Version
