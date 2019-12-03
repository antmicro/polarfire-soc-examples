/*
 * Header for 64-bit CPU
 *
 * Date:   6 Mar 2019
 * Author: anton.krug@microchip.com
 */

#ifndef SRC_MUSTEIN_GPU64_H_
#define SRC_MUSTEIN_GPU64_H_

#include "mustein_gpu_common.h"

#ifdef MUSTEIN_CPU_64


void mustein_video_setup(uint64_t base, uint8_t controlBit, uint32_t width,
        uint32_t height, MusteinColors colors, MusteinPixelPacking packing);


/* ------ 8-bit color mode ------ */
void mustein_write_low_color_raw8(uint64_t base, uint64_t offset,
        uint8_t value);


void mustein_write_low_color_rgb(uint64_t base, uint64_t offset,
        uint8_t red, uint8_t green, uint8_t blue);


/* ------ 16-bit color mode ------ */
void mustein_write_high_color_raw16(uint64_t base, uint64_t offset,
        uint16_t value);


void mustein_write_high_color_rgb(uint64_t base, uint64_t offset,
        uint8_t red, uint8_t green, uint8_t blue);


/* ------ 32-bit color mode ------ */
void mustein_write_true_color_raw24(uint64_t base, uint64_t offset,
        uint32_t value);


void mustein_write_true_color_rgb(uint64_t base, uint64_t offset, uint8_t red,
        uint8_t green, uint8_t blue);


/* Write single 32-bit pixel, or packed pair of 16-bit pixels */
void mustein_write_raw32(uint64_t base, uint64_t offset, uint32_t bytecode);


/* Single fully packed write (can work with any color mode) */
void mustein_write_raw64(uint64_t base, uint64_t offset, uint64_t bytecode);


/* ------ Bulk buffer copy ------ */

/* 8-bit color bulk copy (unpacked mode => 1 byte copied, 7 bytes skipped) */
void mustein_write_buffer8(uint64_t base, uint8_t *buffer, uint64_t count);


/* 16-bit color bulk copy (unpacked mode => 2 bytes copied, 6 bytes skipped) */
void mustein_write_buffer16(uint64_t base, uint16_t *buffer, uint64_t count);


/* 32-bit color bulk copy (unpacked mode => 4 bytes copied, 4 bytes skipped) */
void mustein_write_buffer32(uint64_t base, uint32_t *buffer, uint64_t count);


/* fully packed bulk copy */
void mustein_write_buffer64_fullypacked(uint64_t base, uint64_t *buffer,
        uint64_t count);


#endif

#endif /* SRC_MUSTEIN_GPU64_H_ */
