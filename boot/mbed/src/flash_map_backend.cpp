/*
 * flash_map_backend.cpp
 *
 *  Created on: Jul 6, 2019
 *      Author: gdbeckstein
 */

#include "flash_map_backend.h"
#include "secondary_bd.h"
#include <sysflash/sysflash.h>
#include <string.h>

#include "platform/mbed_toolchain.h"

#include "BlockDevice.h"
#include "FlashIAPBlockDevice.h"

#if MBED_CONF_MCUBOOT_BOOTLOADER_BUILD

/**
 * Calculating the primary slot start address
 * When built, the bootloader is given a size constraint: target.restrict_size
 *
 * The Mbed build tools automatically create two macros:
 * POST_APPLICATION_ADDR = target.restrict_size
 * POST_APPLICATION_SIZE = end of program memory - target.restrict_size
 *
 * We want to avoid having the bootloader collide with the primary
 * slot header (which is immediately before the primary slot in memory)
 *
 * So the first instruction of the primary application is NOT actually located
 * at the POST_APPLICATION_ADDR address, which instead points to the primary slot
 * header region.
 *
 */
#define PRIMARY_SLOT_START_ADDR POST_APPLICATION_ADDR
#define PRIMARY_APP_START_ADDR  (PRIMARY_SLOT_START_ADDR + MBED_CONF_MCUBOOT_HEADER_SIZE)
#define PRIMARY_SLOT_SIZE       (POST_APPLICATION_SIZE - MBED_CONF_MCUBOOT_SCRATCH_SIZE)

#else

/**
 * In the case of the main application build, the user sets up two configuration
 * parameters, target.mbed_app_start and target.mbed_app_size
 *
 * Based on these, Mbed's build tools define two build macros:
 * APPLICATION_ADDR = target.mbed_app_start
 * APPLICATION_SIZE = target.mbed_app_size
 *
 * Since the primary slot BlockDevice must start at the header info region,
 * we must subtract MBED_CONF_MCUBOOT_HEADER_SIZE from the APPLICATION_ADDR
 * macro and similarly add the header size to the APPLICATION_SIZE macro
 *
 */
#define PRIMARY_SLOT_START_ADDR (APPLICATION_ADDR - MBED_CONF_MCUBOOT_HEADER_SIZE)
#define PRIMARY_APP_START_ADDR  APPLICATION_ADDR
#define PRIMARY_SLOT_SIZE       (APPLICATION_SIZE + MBED_CONF_MCUBOOT_HEADER_SIZE)

#endif

#if MBED_CONF_MCUBOOT_BOOTLOADER_BUILD

/**
 * Calculating the scratch start address
 *
 * During bootloader builds, the aforementioned POST_APPLICATION_SIZE macro
 * does not take into account any other partitions that may divide the primary
 * slot space.
 *
 * So the scratch start address can be calculated as below
 *
 */
#define SCRATCH_START_ADDR (PRIMARY_SLOT_START_ADDR + PRIMARY_SLOT_SIZE)
#else
// Size of application during non-bootloader build takes into account scratch space
#define SCRATCH_START_ADDR (APPLICATION_ADDR + APPLICATION_SIZE)
#endif

#if MBED_CONF_MCUBOOT_DEBUG_ADDRESSES
#define XSTR(x) STR(x)
#define STR(x) #x
#pragma message "The value of APPLICATION_ADDR: " XSTR(APPLICATION_ADDR)
#pragma message "The value of APPLICATION_SIZE: " XSTR(APPLICATION_SIZE)
#pragma message "The value of SCRATCH_START_ADDR: " XSTR(SCRATCH_START_ADDR)
#endif

MBED_WEAK mbed::BlockDevice* get_secondary_bd(void) {
    return mbed::BlockDevice::get_default_instance();
}

/** Application defined secondary block device */
mbed::BlockDevice* mcuboot_secondary_bd = get_secondary_bd();

/** Internal application block device */
static FlashIAPBlockDevice mcuboot_primary_bd(PRIMARY_SLOT_START_ADDR, PRIMARY_SLOT_SIZE);

/** Scratch space is at the end of internal flash, after the main application */
static FlashIAPBlockDevice mcuboot_scratch_bd(SCRATCH_START_ADDR, MBED_CONF_MCUBOOT_SCRATCH_SIZE);

static mbed::BlockDevice* flash_map_bd[3] = {
		(mbed::BlockDevice*) &mcuboot_primary_bd,		/** Primary (loadable) image area */
		mcuboot_secondary_bd,							/** Secondary (update candidate) image area */
		(mbed::BlockDevice*) &mcuboot_scratch_bd			/** Scratch space for swapping images */
};

static struct flash_area flash_areas[3];
static uint8_t open_count[3];

int initialize_flash_areas(void) {
    for(int i = 0; i < 3; i++) {
        flash_map_bd[i]->init();
    }
    return 0;
}

int flash_area_open(uint8_t id, const struct flash_area** fapp) {

	*fapp = &flash_areas[id];

	// Flash area is open
	if(open_count[id]++) {
		return 0;
	}

	struct flash_area* fap = (struct flash_area*)*fapp;

	mbed::BlockDevice* bd = flash_map_bd[id];

	fap->fa_id = id;
	fap->fa_device_id = 0;

	// Only populate the offset if it's internal
	switch(id) {
	case PRIMARY_ID:
	    fap->fa_off = PRIMARY_SLOT_START_ADDR;
		break;
	case SECONDARY_ID:
		fap->fa_off = 0;
		break;
	case SCRATCH_ID:
		fap->fa_off = SCRATCH_START_ADDR;
		break;
	default:
		return -1;
	}

	fap->fa_size = (uint32_t) bd->size();

	return bd->init();
}

void flash_area_close(const struct flash_area* fap) {
	// Delete the flash area struct
	if(--open_count[fap->fa_id] == 0) {
		mbed::BlockDevice* bd = flash_map_bd[fap->fa_id];
		bd->deinit();
	}
}

int flash_area_read(const struct flash_area* fap, uint32_t off, void* dst,
		uint32_t len) {
	mbed::BlockDevice* bd = flash_map_bd[fap->fa_id];

	// If the read is invalid...
	if(!bd->is_valid_read(off, len)) {
		// Read back the minimum supported by the bd and copy over
		uint8_t* buf = new uint8_t[bd->get_read_size()]();
		int ret_val = bd->read(buf, off, bd->get_read_size());
		memcpy(dst, (const void*) buf, len);
		delete[] buf;
		return ret_val;
	}
	else {
		return bd->read(dst, off, len);
	}
}

int flash_area_write(const struct flash_area* fap, uint32_t off, const void* src,
		uint32_t len) {
	mbed::BlockDevice* bd = flash_map_bd[fap->fa_id];
	return bd->program(src, off, len);
}

int flash_area_erase(const struct flash_area* fap, uint32_t off, uint32_t len) {
	mbed::BlockDevice* bd = flash_map_bd[fap->fa_id];
	return bd->erase(off, len);
}

uint8_t flash_area_align(const struct flash_area* fap) {
	mbed::BlockDevice* bd = flash_map_bd[fap->fa_id];
	return bd->get_program_size();
}

uint8_t flash_area_erased_val(const struct flash_area* fap) {
	mbed::BlockDevice* bd = flash_map_bd[fap->fa_id];
	return bd->get_erase_value();
}

int flash_area_read_is_empty(const struct flash_area* fap, uint32_t off,
		void* dst, uint32_t len) {

	if(flash_area_read(fap, off, dst, len)) {
		return -1; // A problem occurred during read operation
	}

	// Check buffer to see if all bytes are equal to erased value
	uint8_t erased_value = flash_area_erased_val(fap);
	for(unsigned int i = 0; i < len; i++) {
		uint8_t val = ((uint8_t*) dst)[i];
		if(val != erased_value) {
			return 0;
		}
	}

	// All bytes were erased
	return 1;

}

int flash_area_get_sectors(int fa_id, uint32_t* count,
		struct flash_sector* sectors) {
	mbed::BlockDevice* bd = flash_map_bd[fa_id];

	// Loop through sectors and collect information on them
	bd_addr_t offset = 0;
	*count = 0;
	while(bd->is_valid_read(offset, bd->get_read_size())
	&& *count < MBED_CONF_MCUBOOT_MAX_IMG_SECTORS) {

		sectors[*count].fs_off = offset;
		bd_size_t erase_size = bd->get_erase_size(offset);
		sectors[*count].fs_size = erase_size;

		offset += erase_size;
		*count += 1;
	}

	return 0;
}

int flash_area_id_from_image_slot(int slot) {
	return slot;
}

int flash_area_id_to_image_slot(int area_id) {
	return area_id;
}

/**
 * TODO The below were added so mcuboot would build -- they have NOT
 * been tested with Multi-image DFU enabled
 */

/*
 * This depends on the mappings defined in sysflash.h.
 * MCUBoot uses continuous numbering for the primary slot, the secondary slot,
 * and the scratch while zephyr might number it differently.
 */
int flash_area_id_from_multi_image_slot(int image_index, int slot)
{
    //FIXME
    return slot;
//    switch (slot) {
//    case 0: return FLASH_AREA_IMAGE_PRIMARY(image_index);
//#if !defined(MBED_CONF_MCUBOOT_SINGLE_IMAGE_DFU)
//    case 1: return FLASH_AREA_IMAGE_SECONDARY(image_index);
//#if !defined(MBED_CONF_MCUBOOT_BOOT_SWAP_MOVE)
//    case 2: return FLASH_AREA_IMAGE_SCRATCH;
//#endif
//#endif
//    }
//
//    return -EINVAL; /* flash_area_open will fail on that */
}

int flash_area_id_to_multi_image_slot(int image_index, int area_id)
{
    return 0;
//    if (area_id == FLASH_AREA_IMAGE_PRIMARY(image_index)) {
//        return 0;
//    }
//#if !defined(MBED_CONF_MCUBOOT_SINGLE_IMAGE_DFU)
//    if (area_id == FLASH_AREA_IMAGE_SECONDARY(image_index)) {
//        return 1;
//    }
//#endif
//
//    //BOOT_LOG_ERR("invalid flash area ID");
//    return -1;
}
