/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Very simple template engine. No logic, just plain string replacement.
 *
 */


#include <stddef.h>
#include <string.h>

#include "c_template.h"

/* Will replace all instances of keywords with their values inside the given
 * string. Assumes that the string pointer has enough allocated space to grow */
void c_template_apply(char *string, c_template_variable vars[])
{
    /* Go through all key/value entries */
    for (int index = 0; vars[index].key != NULL; ++index)
    {
        c_template_variable var = vars[index];
        char *found;

        /* Keep iterating until no new instances of the keyword can be found */
        while ( NULL != (found = strstr(string, var.key)) )
        {
            int key_len = strlen(var.key);
            int val_len = strlen(var.value);
            int offset  = val_len - key_len;

            if (offset == 0)
            {
                /* When there is no difference between key and the value, then
                 * nothing needs to be moved as the value will perfectly replace
                 * the key */
            }
            else
            {
                /* The original key and the replacement string value have
                 * different lengths and the rest of the document string
                 * needs to shift either to the left or to the right.
                 *
                 * The pointers overlap and the destination could be further
                 * into the memory than the source, use the safe memmove.
                 * Move whole string including the last 0 termination character */
                memmove(found + key_len + offset,
                        found + key_len,
                        strlen(found + key_len) + 1);
            }

            /* Replace the key with the value, but do not copy the \0 char */
            strncpy(found, var.value, strlen(var.value));
        }
    }
}
