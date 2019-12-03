/*
 * Main header to include, it will include 32-bit or 64-bit headers depending
 * what is your CPU.
 *
 * Date:   6 Mar 2019
 * Author: anton.krug@microchip.com
 */

#ifndef MUSTEIN_GPU_MUSTEIN_GPU_H_
#define MUSTEIN_GPU_MUSTEIN_GPU_H_

#include "mustein_gpu_common.h"

#ifdef MUSTEIN_CPU_32
#include "mustein_gpu32.h"
#else
// if it's not MUSTEIN_CPU_32, then assume MUSTEIN_CPU_64
#include "mustein_gpu64.h"
#endif


#endif /* MUSTEIN_GPU_MUSTEIN_GPU_H_ */
