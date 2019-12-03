/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 * Code running on U54 hart 1
 */

#include "fractals/common_macros.h"
#include "fractals/fractal_display.h"

#include "inc/common.h"

/* Main function for the HART1(U54_1 processor).
 * Application code running on HART1 is placed here
 */
void u54_1(void)
{
    /* There is no need to sync the harts because the hart's applications
     * do not have any interdependencies */

    juliaMain(0x10100000);

    /* Shouldn't never reach this point */
    while (1)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as gdb hangs when stepping through an empty infinite loop */
       counter = counter + 1;
    }
}
