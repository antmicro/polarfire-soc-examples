/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 */


#ifndef UTILS_C_TEMPLATE_H_
#define UTILS_C_TEMPLATE_H_


typedef struct {
    char *key;
    char *value;
} c_template_variable;


void c_template_apply(char *string, c_template_variable vars[]);


#endif /* UTILS_C_TEMPLATE_H_ */
