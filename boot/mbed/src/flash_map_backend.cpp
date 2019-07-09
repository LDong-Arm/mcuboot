/*
 * flash_map_backend.cpp
 *
 *  Created on: Jul 6, 2019
 *      Author: gdbeckstein
 */

#include "flash_map_backend.h"
#include <sysflash/sysflash.h>

#include "BlockDevice.h"
#include "FlashIAPBlockDevice.h"

#define SCRATCH_START_ADDR	((APPLICATION_ADDR + APPLICATION_SIZE) - MBED_CONF_MCUBOOT_SCRATCH_SIZE)
#define SCRATCH_END_ADDR		(SCRATCH_START_ADDR + MBED_CONF_MCUBOOT_SCRATCH_SIZE)

/** Application defined secondary block device */
extern mbed::BlockDevice* mcuboot_secondary_bd;

/** Internal application block device */
static FlashIAPBlockDevice mcuboot_primary_bd(APPLICATION_ADDR, SCRATCH_START_ADDR);

/** Scratch space is at the end of internal flash, after the main application */
static FlashIAPBlockDevice mcuboot_scratch_bd(SCRATCH_START_ADDR, SCRATCH_END_ADDR);


static mbed::BlockDevice* flash_map_bd[] = {
		NULL,											/** Reserved for bootloader flash area */
		(mbed::BlockDevice*) &mcuboot_primary_bd,		/** Primary (loadable) image area */
		mcuboot_secondary_bd,							/** Secondary (update candidate) image area */
		(mbed::BlockDevice*) &mcuboot_scratch_bd			/** Scratch space for swapping images */
};

int flash_area_open(uint8_t id, const struct flash_area** fapp) {
	// Allocate a new flash area struct and populate it
	struct flash_area* fap = new struct flash_area();
	mbed::BlockDevice* bd = flash_map_bd[id];

	fap->fa_id = id;
	fap->fa_device_id = 0;

	// Only populate the offset if it's internal
	switch(id) {
	case RESERVED_ID:
		return -1;
	case PRIMARY_ID:
		fap->fa_off = APPLICATION_ADDR;
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
	mbed::BlockDevice* bd = flash_map_bd[fap->fa_id];
	bd->deinit();
	delete fap;
}

int flash_area_read(const struct flash_area* fap, uint32_t off, void* dst,
		uint32_t len) {
	mbed::BlockDevice* bd = flash_map_bd[fap->fa_id];
	return bd->read(dst, off, len);
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
	return bd->get_erase_size();
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
	for(int i = 0; i < len; i++) {
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
	while(bd->is_valid_read(offset, 1)
	&& *count < MBED_CONF_MCUBOOT_MAX_IMG_SECTORS) {

		sectors[*count].fs_off = offset;
		bd_size_t erase_size = bd->get_erase_size(offset);
		sectors[*count].fs_size = erase_size;

		offset += erase_size;
		*count++;
	}

	return 0;
}

int flash_area_id_from_image_slot(int slot) {
	return slot;
}

int flash_area_id_to_image_slot(int area_id) {
	return area_id;
}
