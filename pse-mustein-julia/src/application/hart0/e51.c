/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */

#include "inc/common.h"

void e51(void)
{
    /* Hart 0 for this demo is not needed and is put it into sleep continuously.
     * There is no need to sync the harts because the hart's applications
     * do not have any interdependencies */

    while (1)
    {
        __asm("wfi");
    }
}
