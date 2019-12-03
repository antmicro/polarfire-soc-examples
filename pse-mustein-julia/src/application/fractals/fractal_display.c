/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */

/* Date: 2018/12/01 anton.krug@microchip.com */

#include <stdio.h>
#include <float.h>
#include <math.h>

#include "drivers/mustein_gpu/mustein_gpu.h"

#include "fractals/common_macros.h"
#include "fractals/fractal_configuration.h"
#include "fractals/fractal_engine.h"

#define RESCALE_FIELD(SET, INDEX, INDEX_NEXT, PERCENTAGE, FIELD) (rescale((SET)[(INDEX)].FIELD, (SET)[(INDEX_NEXT)].FIELD, (PERCENTAGE)))


/* Coordinate system is similar to Christian Stigen Larsen's approach:
 *   http://tilde.club/~david/m/
 *
 * Some Julia locations are taken from Karl Sims's web-page:
 *   http://www.karlsims.com/julia.html  */


FractalView julias[] =
{
        { -2.0f,   0.00f,    0.0f,   0.0f,   2.90f, 2.90f, 4.0f},
        { -1.5f,   0.00f,    0.0f,   0.0f,   1.20f, 1.20f, 2.0f},
        { -1.5f,   0.00f,    0.0f,   0.0f,   0.10f, 0.10f, 1.0f},
        { -1.5f,   0.00f,    0.0f,   0.0f,   1.20f, 1.20f, 4.0f},
        {-0.79f,   0.15f,    0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        {-0.79f,   0.15f,    0.2f,   0.2f,   0.50f, 0.50f, 2.0f},
        {-0.80f,   0.2f,     0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        {-0.162f,  1.04f,    0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        {-0.162f,  1.04f,    0.0f,   0.0f,   0.30f, 0.30f, 2.0f},
        {-0.162f,  1.05f,    0.0f,   0.0f,   0.30f, 0.30f, 2.0f},
        {-0.162f,  0.90f,    0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        { 0.3f,   -0.01f,    0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        { 0.3f,   -0.01f,    0.41f, -0.25f,  0.50f, 0.50f, 1.0f},
        { 0.369f, -0.12f,    0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        { 0.369f, -0.12f,    0.0f,   0.0f,   0.40f, 0.40f, 1.0f},
        { 0.369f, -0.1f,     0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        { 0.28f,  +0.008f,   0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        { 0.28f,  +0.008f,   0.0f,   0.0f,   0.50f, 0.50f, 1.0f},
        { 0.3f,   +0.01f,    0.0f,   0.0f,   2.00f, 2.00f, 4.0f},
        { 0.26f,   0.0015f,  0.08f,  0.146f, 2.00f, 2.00f, 4.0f},
        { 0.26f,   0.0015f,  0.08f,  0.146f, 0.3f,  0.3f,  1.0f},
        { 0.26f,   0.0017f,  0.08f,  0.146f, 1.5f,  1.5f,  2.0f},
        { 0.50f,   0.15f,    0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        { 0.157f, -1.041f,   0.0f,   0.0f,   3.00f, 3.00f, 4.0f},
        {-0.12f,  -0.77f,    0.0f,   0.0f,   3.00f, 3.00f, 4.0f}
};



float FORCE_INLINE rescale(float old, float new, float percentage)
{
  /* Make sure even with overflowed percentage it will compute correctly */
  return ((new - old) * fminf(1.0f, fmaxf(0.0f, percentage))) + old;
}


void transition(FractalView *old, FractalView *next, float percentage,
		FractalView *ret)
{

    ret->seedReal    = rescale(old->seedReal,    next->seedReal,    percentage);
    ret->seedComplex = rescale(old->seedComplex, next->seedComplex, percentage);
    ret->lookAtX     = rescale(old->lookAtX,     next->lookAtX,     percentage);
    ret->lookAtY     = rescale(old->lookAtY,     next->lookAtY,     percentage);
    ret->width       = rescale(old->width,       next->width,       percentage);
    ret->height      = rescale(old->height,      next->height,      percentage);
    ret->gamma       = rescale(old->gamma,       next->gamma,       percentage);
};


void fractalLoop(uint64_t base, uint32_t *buffer)
{
    static int   i            = 0;
    const  int   iNext        = (i +1) % NELEMS(julias);
           float currentSpeed = ANIMATION_SPEED;

    for (float percentage = 0.0f; percentage <= 1.1f; percentage += currentSpeed)
    {
        /* 0.0f to 1.0f will be transitions
         * Values over 1.0f will render same frame (timing without using timer)
         */

        FractalView current;
        transition(&julias[i], &julias[iNext], percentage, &current);
        renderFractal(&current, buffer);

        if (current.gamma < 3.0f)
        {
        	/* When using low gamma it means that it increases computational
        	 * complexity, so increase the animation speed and do not spend
        	 * as many frames in the very heavy parts of the animation */
            currentSpeed = ANIMATION_SPEED * 3.0f * (1.0f / current.gamma);
        }

        /* Display the rendered buffer */
        mustein_write_buffer32(base, buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
    }
    i = iNext;
}


void juliaMain(uint64_t base)
{
    uint32_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

    mustein_video_setup(base, MUSTEIN_DEFAULT_CONTROL_BIT_OFFSET,
          SCREEN_WIDTH, SCREEN_HEIGHT,
          MUSTEIN_COLOR_TRUE, MUSTEIN_PACKING_SINGLE_PIXEL_PER_WRITE);

    /* Render following fractal series */
    while (1)
    {
        fractalLoop(base, buffer);
    }
}


