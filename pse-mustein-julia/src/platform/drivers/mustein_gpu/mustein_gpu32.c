/*
 * Driver for 32-bit aligned variant of the Mustein video controller
 * suitable to be used with 32-bit CPUs.
 *
 * Renode Model is located at:
 *   https://github.com/AntonKrug/mustein_gpu_renode_model
 *
 * Date:   6 Mar 2019
 * Author: anton.krug@microchip.com
 */



#include "mustein_gpu32.h"

#ifdef MUSTEIN_CPU_32

void MUSTEIN_INLINE mustein_write_low_color_raw8(uint32_t base, uint32_t offset,
        uint8_t value) {

    *((uint32_t*) base + offset) = value;
}


void MUSTEIN_INLINE mustein_write_low_color_rgb(uint32_t base, uint32_t offset,
        uint8_t red, uint8_t green, uint8_t blue) {

    *((uint64_t*) base + offset) = (blue  >> 6) << 6 |
    		                       (green >> 5) << 3 |
								   (red   >> 5);
}


void MUSTEIN_INLINE mustein_write_high_color_raw16(uint32_t base,
		uint32_t offset, uint16_t value) {

    *((uint32_t*) base + offset) = value;
}


void MUSTEIN_INLINE mustein_write_high_color_rgb(uint32_t base, uint32_t offset,
        uint8_t red, uint8_t green, uint8_t blue) {
    /* https://stackoverflow.com/questions/3578265 */

    *((uint64_t*) base + offset) =  (red   >> 3) << 11 |
    		                        (green >> 2) << 5  |
									(blue  >> 3);
}


/* A 24-bit RGB true color value will be shifted into 32-bit RGBX format */
void MUSTEIN_INLINE mustein_write_true_color_raw24(uint32_t base,
		uint32_t offset, uint32_t value) {

    *((uint32_t*) base + offset) = value << 8;
}


void MUSTEIN_INLINE mustein_write_true_color_rgb(uint32_t base, uint32_t offset,
        uint8_t red, uint8_t green, uint8_t blue) {

    *((uint64_t*) base + offset) = red << 24 | green << 16 | blue << 8;
}


/* Write single 32-bit pixel, or packed pair of 16-bit pixels */
void MUSTEIN_INLINE mustein_write_raw32(uint32_t base, uint32_t offset,
        uint32_t bytecode) {

    *((uint32_t*) base + offset) = bytecode;
}


/* Unpacked copy 1 byte copied, 3 bytes skipped */
void MUSTEIN_OPTIMIZE mustein_write_buffer8(uint32_t base, uint8_t *buffer,
        uint32_t count) {

    uint32_t* pointer = (uint32_t*) base;

    for (uint32_t index = 0; index < count; ++index) {
        *(pointer) = buffer[index];
        pointer++;
    }
}


/* Unpacked copy 2 bytes copied, 2 bytes skipped */
void MUSTEIN_OPTIMIZE mustein_write_buffer16(uint32_t base, uint16_t *buffer,
        uint32_t count) {

    uint32_t* pointer = (uint32_t*) base;

    for (uint32_t index = 0; index < count; ++index) {
        *(pointer) = buffer[index];
        pointer++;
    }
}


/* Packed copy, copies everything without skipping any bytes */
void MUSTEIN_OPTIMIZE mustein_write_buffer32(uint32_t base, uint32_t *buffer,
        uint32_t count) {

    uint32_t* pointer = (uint32_t*) base;

    for (uint32_t index = 0; index < count; ++index) {
        *(pointer) = buffer[index];
        pointer++;
    }
}


void mustein_video_setup(uint32_t base, uint8_t controlBit, uint32_t width,
        uint32_t height, MusteinColors colors, MusteinPixelPacking packing) {

    MusteinController *ctrl  = (MusteinController*) (base | (1 << controlBit));
    ctrl->width              = width;
    ctrl->height             = height;
    ctrl->parameters.colors  = colors;
    ctrl->parameters.packing = packing;
}

#endif
