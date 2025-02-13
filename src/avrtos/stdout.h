/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AVRTOS_STDIO_H_
#define _AVRTOS_STDIO_H_

#include <stdio.h>

#include "kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set up the standard I/O to use USART0.
 *
 * This function configures the standard I/O to communicate via USART0. This allows
 * the function `printf`, to use USART0 for input and output.
 *
 * The configuration option `CONFIG_STDIO_PRINTF_TO_USART` automatically selects the
 * USART to use for standard I/O.
 */
__kernel void k_set_stdio_usart0(void);

#ifdef __cplusplus
}
#endif

#endif /* _AVRTOS_STDIO_H_ */
