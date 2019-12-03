/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */

/* Date: 2018/12/01 anton.krug@microchip.com */

#ifndef SRC_FRACTAL_CONFIGURATION_H_
#define SRC_FRACTAL_CONFIGURATION_H_

/* What resolution corresponds to what bits:
 *
 * 10 bits => 1024 pixels
 * 9  bits => 512  pixels
 * 8  bits => 256  pixels
 * 7  bits => 128  pixels
 * 6  bits => 64   pixels
 * 5  bits => 32   pixels
 * 4  bits => 16   pixels
 * 3  bits => 8    pixels
 */

#define WIDTH_BITS 7   /* WIDTH  = 1 << WIDTH_BITS  */
#define HEIGHT_BITS 7  /* HEIGHT = 1 << HEIGHT_BITS */

#ifndef ANIMATION_SPEED
#define ANIMATION_SPEED 0.02f /* How large steps are done between the frames */
#endif


#define SCREEN_WIDTH ( 1 << (WIDTH_BITS))
#define SCREEN_HEIGHT ( 1 << (HEIGHT_BITS))

#endif /* SRC_FRACTAL_CONFIGURATION_H_ */
