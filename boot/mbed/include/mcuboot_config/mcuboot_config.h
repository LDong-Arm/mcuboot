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
#define SIGNATURE_TYPE_RSA      0
#define SIGNATURE_TYPE_EC256    1
#define SIGNATURE_TYPE_ED25519  2

/* Default to RSA-2048 signatures */
#if !defined(MBED_CONF_MCUBOOT_SIGNATURE_ALGORITHM)
#define MBED_CONF_MCUBOOT_SIGNATURE_ALGORITHM 0
#define MBED_CONF_MCUBOOT_RSA_SIGNATURE_LENGTH 2048
#endif

#if (MBED_CONF_MCUBOOT_SIGNATURE_ALGORITHM == SIGNATURE_TYPE_RSA)
#define MCUBOOT_SIGN_RSA
#  if (MBED_CONF_MCUBOOT_RSA_SIGNATURE_LENGTH != 2048 && \
       MBED_CONF_MCUBOOT_RSA_SIGNATURE_LENGTH != 3072)
#    error "Invalid RSA key size (must be 2048 or 3072)"
#  else
#    define MCUBOOT_SIGN_RSA_LEN MBED_CONF_MCUBOOT_RSA_SIGNATURE_LENGTH
#  endif
#elif (MBED_CONF_MCUBOOT_SIGNATURE_ALGORITHM == SIGNATURE_TYPE_EC256)
#define MCUBOOT_SIGN_EC256
#elif (MBED_CONF_MCUBOOT_SIGNATURE_ALGORITHM == SIGNATURE_TYPE_ED25519)
#define MCUBOOT_SIGN_ED25519
#endif

#if MBED_CONF_MCUBOOT_USE_MBED_TLS
#define MCUBOOT_USE_MBED_TLS
#elif MBED_CONF_MCUBOOT_USE_TINYCRYPT
#define MCUBOOT_USE_TINYCRYPT
#endif

// TODO - Support for Cryptocell310?
//#if MBED_CONF_CRYPTOCELL310_PRESENT
//#define MCUBOOT_USE_CC310
//#endif

/*
 * Always check the signature of the image in the primary slot before booting,
 * even if no upgrade was performed. This is recommended if the boot
 * time penalty is acceptable.
 */

/*
 * Upgrade mode
 *
 * The default is to support A/B image swapping with rollback.  A
 * simpler code path, which only supports overwriting the
 * existing image with the update image, is also available.
 */

#ifdef MCUBOOT_OVERWRITE_ONLY
#if MBED_CONF_MCUBOOT_OVERWRITE_ONLY_FAST
/* Only erase and overwrite those primary slot sectors needed
 * to install the new image, rather than the entire image slot. */
#define MCUBOOT_OVERWRITE_ONLY_FAST
#endif
#endif

#if MBED_CONF_MCUBOOT_SINGLE_IMAGE_DFU
#define MCUBOOT_SINGLE_IMAGE_DFU 1
#else

#if MBED_CONF_MCUBOOT_BOOT_SWAP_MOVE
#define MCUBOOT_SWAP_USING_MOVE 1
#endif

#if MBED_CONF_MCUBOOT_UPDATEABLE_IMAGE_NUMBER
#define MCUBOOT_IMAGE_NUMBER    MBED_CONF_MCUBOOT_UPDATEABLE_IMAGE_NUMBER
#else
#define MCUBOOT_IMAGE_NUMBER 1
#endif

#if MBED_CONF_MCUBOOT_SWAP_SAVE_ENCTLV
#define MCUBOOT_SWAP_SAVE_ENCTLV 1
#endif

#endif /* MBED_CONF_MCUBOOT_SINGLE_IMAGE_DFU */

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

// MCUBOOT_HAVE_LOGGING is now directly declared through mbed_app.json

/*
 * Encrypted Images
 */
#if MBED_CONF_MCUBOOT_ENCRYPT_RSA
#define MCUBOOT_ENC_IMAGES
#define MCUBOOT_ENCRYPT_RSA
#elif MBED_CONF_MCUBOOT_ENCRYPT_EC256
#define MCUBOOT_ENC_IMAGES
#define MCUBOOT_ENCRYPT_EC256
#elif MBED_CONF_MCUBOOT_ENCRYPT_X25519
#define MCUBOOT_ENC_IMAGES
#define MCUBOOT_ENCRYPT_X25519
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

#define MCUBOOT_WATCHDOG_FEED()         \
    do {                                \
        /* TODO: to be implemented */   \
    } while (0)


#endif /* __MCUBOOT_CONFIG_H__ */
