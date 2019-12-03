/*
 * Driver for 64-bit aligned variant of the Mustein video controller
 * suitable to be used with 64bit CPUs
 *
 * Renode Model is located at:
 *   https://github.com/AntonKrug/mustein_gpu_renode_model
 *
 * Date:   6 Mar 2019
 * Author: anton.krug@microchip.com
 */

#include "mustein_gpu64.h"

#ifdef MUSTEIN_CPU_64


void MUSTEIN_INLINE mustein_write_low_color_raw8(uint64_t base, uint64_t offset,
        uint8_t value) {

    *((uint64_t*) base + offset) = value;
}


void MUSTEIN_INLINE mustein_write_low_color_rgb(uint64_t base, uint64_t offset,
        uint8_t red, uint8_t green, uint8_t blue) {

    *((uint64_t*) base + offset) = (blue  >> 6) << 6 |
    		                       (green >> 5) << 3 |
								   (red   >> 5);
}


void MUSTEIN_INLINE mustein_write_high_color_raw16(uint64_t base,
		uint64_t offset, uint16_t value) {

    *((uint64_t*) base + offset) = value;
}


void MUSTEIN_INLINE mustein_write_high_color_rgb(uint64_t base, uint64_t offset,
        uint8_t red, uint8_t green, uint8_t blue) {
    /* https://stackoverflow.com/questions/3578265 */

    *((uint64_t*) base + offset) = (red   >> 3) << 11 |
    		                       (green >> 2) << 5  |
								   (blue  >> 3);
}


/* A 24-bit RGB true color value will be shifted into 32-bit RGBX format */
void MUSTEIN_INLINE mustein_write_true_color_raw24(uint64_t base,
		uint64_t offset, uint32_t value) {

    *((uint64_t*) base + offset) = value << 8;
}


void MUSTEIN_INLINE mustein_write_true_color_rgb(uint64_t base, uint64_t offset,
        uint8_t red, uint8_t green, uint8_t blue) {

    *((uint64_t*) base + offset) = red << 24 | green << 16 | blue << 8;
}


void MUSTEIN_INLINE mustein_write_raw32(uint64_t base, uint64_t offset,
        uint32_t bytecode) {

    *((uint64_t*) base + offset) = bytecode;
}


void MUSTEIN_INLINE mustein_write_raw64(uint64_t base, uint64_t offset,
        uint64_t bytecode) {

    *((uint64_t*) base + offset) = bytecode;
}


/* Unpacked copy 1 byte copied, 7 bytes skipped */
void MUSTEIN_OPTIMIZE mustein_write_buffer8(uint64_t base, uint8_t *buffer,
        uint64_t count) {

    uint64_t* pointer = (uint64_t*) base;

    for (uint64_t index = 0; index < count; ++index) {
        *(pointer) = buffer[index];
        pointer++;
    }
}


/* Unpacked copy 2 bytes copied, 6 bytes skipped */
void MUSTEIN_OPTIMIZE mustein_write_buffer16(uint64_t base, uint16_t *buffer,
        uint64_t count) {

    uint64_t* pointer = (uint64_t*) base;

    for (uint64_t index = 0; index < count; ++index) {
        *(pointer) = buffer[index];
        pointer++;
    }
}


/* Unpacked copy 4 bytes copied, 4 bytes skipped */
void MUSTEIN_OPTIMIZE mustein_write_buffer32(uint64_t base, uint32_t *buffer,
        uint64_t count) {

    uint64_t* pointer = (uint64_t*) base;

    for (uint64_t index = 0; index < count; ++index) {
        *(pointer) = buffer[index];
        pointer++;
    }
}


/* Packed copy, copies everything without skipping any bytes */
void MUSTEIN_OPTIMIZE mustein_write_buffer64_fullypacked(uint64_t base,
        uint64_t *buffer, uint64_t count) {

    uint64_t* pointer = (uint64_t*) base;

    for (uint64_t index = 0; index < count; ++index) {
        *(pointer) = buffer[index];
        pointer++;
    }
}


void mustein_video_setup(uint64_t base, uint8_t controlBit, uint32_t width,
        uint32_t height, MusteinColors colors, MusteinPixelPacking packing) {

    MusteinController *ctrl  = (MusteinController*) (base | (1 << controlBit));
    ctrl->width              = width;
    ctrl->height             = height;
    ctrl->parameters.colors  = colors;
    ctrl->parameters.packing = packing;
}

#endif
