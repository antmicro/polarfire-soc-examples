/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */

/* Date: 2018/12/01 anton.krug@microchip.com */

#include "drivers/mustein_gpu/mustein_gpu.h"

#include "fractal_engine.h"
#include "fractal_configuration.h"
#include "common_macros.h"

/* http://www.malinc.se/m/JuliaSets.php
 * http://usefuljs.net/fractals/docs/julia_mandelbrot.html
 * http://www.karlsims.com/julia.html
 * http://www.cs.bham.ac.uk/~pxt/FY-CS/mandelbrot_julia.pdf
 */

/* Color table used for the fractals */
const uint32_t colors[64] =
{
        0x522e1f00, 0x47282100, 0x3d222400, 0x331c2700,
        0x29172a00, 0x25152f00, 0x21143400, 0x1d123900,
        0x19113f00, 0x17114500, 0x16124c00, 0x15135200,
        0x14145900, 0x13145f00, 0x12156600, 0x11166d00,
        0x10177400, 0x13207d00, 0x16298700, 0x19329000,
        0x1c3c9a00, 0x1f45a300, 0x224fad00, 0x2558b700,
        0x2862c100, 0x306ccd00, 0x3877d900, 0x4082e500,
        0x498df100, 0x609ff400, 0x77b1f800, 0x8ec3fb00,
        0xa6d5ff00, 0xb5defd00, 0xc4e8fb00, 0xd3f2f900,
        0xe3fcf800, 0xe6fbed00, 0xeafae300, 0xedf9d900,
        0xf1f9cf00, 0xf2f1b700, 0xf4e99f00, 0xf6e18700,
        0xf8d96f00, 0xf9d55b00, 0xfbd14700, 0xfdcd3300,
        0xffca2000, 0xf6bb1c00, 0xedad1800, 0xe49e1400,
        0xdc901000, 0xcf851000, 0xc27b1000, 0xb5711000,
        0xa9671000, 0xa1622000, 0x995d3000, 0x91584000,
        0x8a545000, 0x67613c00, 0x456f2800, 0x227c1400
};

uint32_t FORCE_INLINE FORCE_O3 renderFractalPixel(
		float    x,        float    y,
		float    seedReal, float    seedComplex,
		float    gamma,    uint32_t maxIter)
{

    const bool isJulia = true; /* Can switch between Julia and Mandelbrot */

    float u  = 0.0f;
    float v  = 0.0f;
    if (isJulia)
    {
        u = x;
        v = y;
    }
    float u2 = u * u;
    float v2 = v * v;
    uint32_t iter;  /* Counting how many iterations were executed */

    if (isJulia)
    {
        for (iter = 0 ; iter < maxIter && ( u2+v2 < 4.0f); iter++)
        {
            v  = u * v;
            v  = v + v   + seedComplex; /* Addition v+v instead of 2*v */
            u  = u2 - v2 + seedReal;
            u2 = u * u;
            v2 = v * v;
        }
    }
    else {
        for (iter = 0 ; iter < maxIter && ( u2+v2 < 4.0f); iter++)
        {
            v  = u * v;
            v  = v + v    + y;
            u  = u2 - v2  + x;
            u2 = u * u;
            v2 = v * v;
        }
    }

    if (iter >= maxIter)
    {
    	/* Render as black color if iterated for too long */
        return 0x0;
    }
    else
    {
    	/* Render as color from the color lookup table */
        return colors[(uint8_t)(iter * gamma)];
    }
}


void renderFractal(FractalView *item, uint32_t *buffer)
{
    /* Calculate boundaries of the fractal */
    const float xmin  = item->lookAtX - (item->width  / 2);
    const float ymin  = item->lookAtY - (item->height / 2);
    const float stepX = item->width  / SCREEN_WIDTH;
    const float stepY = item->height / SCREEN_HEIGHT;

    /* Max iterations will affect the "exposure", more iterations will resolve
     * even the "black" portions of the fractal but cost more cycles to compute
     */
    const uint32_t maxIter = (uint32_t)((float)NELEMS(colors) / item->gamma);

    for (uint32_t cursorY = 0; cursorY < SCREEN_HEIGHT; cursorY++)
    {
        float y = ymin + cursorY * stepY;

        for (uint32_t cursorX = 0; cursorX < SCREEN_WIDTH; cursorX++)
        {
            float x = xmin + cursorX * stepX;

            /* Render the fractal for each pixel in the buffer */
            uint32_t result = renderFractalPixel(
                    x,              y,
                    item->seedReal, item->seedComplex,
                    item->gamma,    maxIter);

            buffer[cursorX | (cursorY << WIDTH_BITS)] = result;
        }
    }
}


