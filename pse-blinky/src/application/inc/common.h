/*
 * common.h
 *
 *  Created on: Jun 12, 2018
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

typedef struct {
    uint64_t start;
    uint64_t end;
    uint64_t delta;
} time_benchmark_t;

void safe_MSS_UART0_polled_tx_string(const char *string);
void e51(void);
void u54_1(void);
void u54_2(void);
void u54_3(void);
void u54_4(void);

#endif /* COMMON_H_ */
