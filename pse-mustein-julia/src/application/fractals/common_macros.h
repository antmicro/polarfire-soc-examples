/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */

/* Date: 2018/12/01 anton.krug@microchip.com */

#ifndef SRC_COMMON_MACROS_H_
#define SRC_COMMON_MACROS_H_

#include <stdlib.h>

/* https://stackoverflow.com/questions/37538 */
#define NELEMS(x) (sizeof(x) / sizeof((x)[0]))
#define FORCE_INLINE __attribute__((always_inline)) inline
#define FORCE_DEBUG __attribute__((optimize("O0")))
#define FORCE_O3 __attribute__((optimize("O3")))

#endif /* SRC_COMMON_MACROS_H_ */
