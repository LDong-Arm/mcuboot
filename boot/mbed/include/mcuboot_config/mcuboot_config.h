/*
 * Copyright (c) 2018 Open Source Foundries Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MCUBOOT_CONFIG_H__
#define __MCUBOOT_CONFIG_H__ 

/*
 * Signature types
 *
 * You must choose exactly one signature type.
 */

/* Default to RSA-2048 signatures */
#if !defined(MBED_CONF_MCUBOOT_SIGNATURE_ALGORITHM)
#define MBED_CONF_MCUBOOT_SIGNATURE_ALGORITHM 0
#define MBED_CONF_MCUBOOT_RSA_SIGNATURE_LENGTH 2048
#endif

#if MBED_CONF_MCUBOOT_SIGNATURE_ALGORITHM
#define MCUBOOT_SIGN_EC256
#else
#define MCUBOOT_SIGN_RSA
#define MCUBOOT_SIGN_RSA_LEN MBED_CONF_MCUBOOT_RSA_SIGNATURE_LENGTH
#endif

/*
 * Upgrade mode
 *
 * The default is to support A/B image swapping with rollback.  A
 * simpler code path, which only supports overwriting the
 * existing image with the update image, is also available.
 */

#if !defined(MBED_CONF_MCUBOOT_OVERWRITE_ONLY)
#define MBED_CONF_MCUBOOT_OVERWRITE_ONLY 0
#endif

#if MBED_CONF_MCUBOOT_OVERWRITE_ONLY
#define MCUBOOT_OVERWRITE_ONLY
#endif


#if !defined(MBED_CONF_MCUBOOT_OVERWRITE_ONLY_FAST)
#define MBED_CONF_MCUBOOT_OVERWRITE_ONLY_FAST 0
#endif

#ifdef MCUBOOT_OVERWRITE_ONLY
#if MBED_CONF_MCUBOOT_OVERWRITE_ONLY_FAST
/* Only erase and overwrite those primary slot sectors needed
 * to install the new image, rather than the entire image slot. */
#define MCUBOOT_OVERWRITE_ONLY_FAST
#endif
#endif

/*
 * Cryptographic settings
 *
 * You must choose between mbedTLS and Tinycrypt as source of
 * cryptographic primitives. Other cryptographic settings are also
 * available.
 */

/* Use ARM's mbedTLS cryptographic primitives */
#define MCUBOOT_USE_MBED_TLS

/*
 * Always check the signature of the image in the primary slot before booting,
 * even if no upgrade was performed. This is recommended if the boot
 * time penalty is acceptable.
 */
#if !defined(MBED_CONF_MCUBOOT_VALIDATE_PRIMARY_SLOT)
#define MBED_CONF_MCUBOOT_VALIDATE_PRIMARY_SLOT 1
#endif

#if MBED_CONF_MCUBOOT_VALIDATE_PRIMARY_SLOT
#define MCUBOOT_VALIDATE_PRIMARY_SLOT
#endif

/*
 * Flash abstraction
 */

/*
 * Enabling this option uses newer flash map APIs. This saves RAM and
 * avoids deprecated API usage.
 */
#define MCUBOOT_USE_FLASH_AREA_GET_SECTORS

/* Default maximum number of flash sectors per image slot; change
 * as desirable. */
#if !defined(MBED_CONF_MCUBOOT_MAX_IMG_SECTORS)
#define MCUBOOT_MAX_IMG_SECTORS 128
#else
#define MCUBOOT_MAX_IMG_SECTORS MBED_CONF_MCUBOOT_MAX_IMG_SECTORS
#endif

/*
 * Logging
 */

/*
 * If logging is enabled the following functions must be defined by the
 * platform:
 *
 *    MCUBOOT_LOG_MODULE_REGISTER(domain)
 *      Register a new log module and add the current C file to it.
 *
 *    MCUBOOT_LOG_MODULE_DECLARE(domain)
 *      Add the current C file to an existing log module.
 *
 *    MCUBOOT_LOG_ERR(...)
 *    MCUBOOT_LOG_WRN(...)
 *    MCUBOOT_LOG_INF(...)
 *    MCUBOOT_LOG_DBG(...)
 *
 * The function priority is:
 *
 *    MCUBOOT_LOG_ERR > MCUBOOT_LOG_WRN > MCUBOOT_LOG_INF > MCUBOOT_LOG_DBG
 */
#if !defined(MBED_CONF_MCUBOOT_ENABLE_LOGGING)
#define MBED_CONF_MCUBOOT_ENABLE_LOGGING 1
#endif

#if MBED_CONF_MCUBOOT_ENABLE_LOGGING
#define MCUBOOT_HAVE_LOGGING 1
#else
#define MCUBOOT_HAVE_LOGGING 0
#endif

/*
 * Encrypted Images
 */
#if !defined(MBED_CONF_MCUBOOT_ENABLE_ENCRYPTED_IMAGES)
#define MBED_CONF_MCUBOOT_ENABLE_ENCRYPTED_IMAGES 1
#endif

#if MBED_CONF_MCUBOOT_ENABLE_ENCRYPTED_IMAGES
#define MCUBOOT_ENC_IMAGES
#define MCUBOOT_ENCRYPT_RSA
#endif

/*
 * Assertions
 */

/* Uncomment if your platform has its own mcuboot_config/mcuboot_assert.h.
 * If so, it must provide an ASSERT macro for use by bootutil. Otherwise,
 * "assert" is used. */
// Does not seem to be supported
//#define MCUBOOT_HAVE_ASSERT_H

#endif /* __MCUBOOT_CONFIG_H__ */
