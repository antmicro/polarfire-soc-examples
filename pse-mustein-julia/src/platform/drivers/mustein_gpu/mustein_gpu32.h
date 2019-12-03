/*
 * Header for 32-bit CPU
 *
 * Date:   6 Mar 2019
 * Author: anton.krug@microchip.com
 */

#ifndef SRC_MUSTEIN_GPU32_H_
#define SRC_MUSTEIN_GPU32_H_

#include "mustein_gpu_common.h"

#ifdef MUSTEIN_CPU_32


void mustein_video_setup(uint32_t base, uint8_t controlBit, uint32_t width,
        uint32_t height, MusteinColors colors, MusteinPixelPacking packing);


/* ------ 8-bit color mode ------ */
void mustein_write_low_color_raw8(uint32_t base, uint32_t offset,
        uint8_t value);


void mustein_write_low_color_rgb(uint32_t base, uint32_t offset,
        uint8_t red, uint8_t green, uint8_t blue);


/* ------ 16-bit color mode ------ */
void mustein_write_high_color_raw16(uint32_t base, uint32_t offset,
        uint16_t value);


void mustein_write_high_color_rgb(uint32_t base, uint32_t offset,
        uint8_t red, uint8_t green, uint8_t blue);


/* ------ 32-bit color mode ------ */
void mustein_write_true_color_raw24(uint32_t base, uint32_t offset,
        uint32_t value);


void mustein_write_true_color_rgb(uint32_t base, uint32_t offset, uint8_t red,
        uint8_t green, uint8_t blue);


/* Write single 32-bit pixel, or packed pair of 16-bit pixels */
void mustein_write_raw32(uint32_t base, uint32_t offset, uint32_t bytecode);


/* ------ Bulk buffer copy ------ */

/* 8-bit color bulk copy (unpacked mode => 1 byte copied, 3 bytes skipped) */
void mustein_write_buffer8(uint32_t base, uint8_t *buffer, uint32_t count);


/* 16-bit color bulk copy (unpacked mode => 2 bytes copied, 2 bytes skipped) */
void mustein_write_buffer16(uint32_t base, uint16_t *buffer, uint32_t count);


/* 32-bit color fully packed bulk copy */
void mustein_write_buffer32(uint32_t base, uint32_t *buffer, uint32_t count);


#endif

#endif /* SRC_MUSTEIN_GPU32_H_ */
