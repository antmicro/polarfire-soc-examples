/*
 * Date:   6 Mar 2019
 * Author: anton.krug@microchip.com
 */

#ifndef MUSTEIN_GPU_MUSTEIN_GPU_COMMON_H_
#define MUSTEIN_GPU_MUSTEIN_GPU_COMMON_H_

#include <stdint.h>

// Comment out the following line below to debug the driver. Usually when using
// debug configuration most of the GPU get optimized
#define MUSTEIN_OPTIMIZE_DEBUG_CONFIGURATION


#ifdef MUSTEIN_OPTIMIZE_DEBUG_CONFIGURATION
#define MUSTEIN_INLINE __attribute__((optimize("O3"))) __attribute__((always_inline)) inline
#define MUSTEIN_OPTIMIZE __attribute__((optimize("O3")))
#else
#define MUSTEIN_INLINE
#define MUSTEIN_OPTIMIZE
#endif


// Trying to detect 32-bit or 64-bit CPU
#if UINTPTR_MAX == 0xffffffff
#define MUSTEIN_CPU_32
#define MUSTEIN_NATIVE_UINT uint32_t
#elif UINTPTR_MAX == 0xffffffffffffffff
#define MUSTEIN_CPU_64
#define MUSTEIN_NATIVE_UINT uint64_t
#else
#define MUSTEIN_CPU_UNKNOWN
#define MUSTEIN_NATIVE_UINT int
#endif


#define MUSTEIN_DEFAULT_CONTROL_BIT_OFFSET 23
#define MUSTEIN_COLORS_BITS 4
#define MUSTEIN_PACKING_BITS 4


// By default two 4-bit parameters are packed together
typedef struct {
    uint8_t colors:MUSTEIN_COLORS_BITS;
    uint8_t packing:MUSTEIN_PACKING_BITS;
}MusteinControllerParameters;


typedef struct {
    MUSTEIN_NATIVE_UINT width;
    MUSTEIN_NATIVE_UINT height;
    MusteinControllerParameters parameters;
} MusteinController;


typedef enum {
    MUSTEIN_COLOR_LOW  = 0,
    MUSTEIN_COLOR_HIGH = 1,
    MUSTEIN_COLOR_TRUE = 2
} MusteinColors;


typedef enum {
    MUSTEIN_PACKING_SINGLE_PIXEL_PER_WRITE = 0,
    MUSTEIN_PACKING_FULLY_32bit            = 1,
    MUSTEIN_PACKING_FULLY_64bit            = 2
} MusteinPixelPacking;


#endif /* MUSTEIN_GPU_MUSTEIN_GPU_COMMON_H_ */
