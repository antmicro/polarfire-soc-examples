/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */

/* Date: 2018/12/01 anton.krug@microchip.com */

#ifndef SRC_FRACTAL_ENGINE_H_
#define SRC_FRACTAL_ENGINE_H_

#include <float.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float seedReal;
    float seedComplex;
    float lookAtX;
    float lookAtY;
    float width;
    float height;
    float gamma;
} FractalView;


void renderFractal(FractalView *item, uint32_t *buffer);

void refreshMandelMap();


#endif /* SRC_FRACTAL_ENGINE_H_ */

