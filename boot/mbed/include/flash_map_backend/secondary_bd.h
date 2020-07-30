/*
 * secondary_bd.h
 *
 *  Created on: Jul 30, 2020
 *      Author: gdbeckstein
 */

#ifndef MCUBOOT_BOOT_MBED_INCLUDE_FLASH_MAP_BACKEND_SECONDARY_BD_H_
#define MCUBOOT_BOOT_MBED_INCLUDE_FLASH_MAP_BACKEND_SECONDARY_BD_H_

#include "BlockDevice.h"

/**
 * This is implemented as a weak function and may be redefined
 * by the application. The default case is to return the
 * BlockDevice object returned by BlockDevice::get_default_instance();
 *
 * @retval secondary_bd Secondary BlockDevice where update candidates are stored
 */
mbed::BlockDevice* get_secondary_bd(void);

#endif /* MCUBOOT_BOOT_MBED_INCLUDE_FLASH_MAP_BACKEND_SECONDARY_BD_H_ */
