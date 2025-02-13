/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stdout.h"

#include "misc/serial.h"

#if CONFIG_STDIO_PRINTF_TO_USART == 0

/**
 * @brief Custom character output function for USART.
 *
 * @param c The character to be transmitted.
 * @param stream The stream to which the character is written (not used).
 * @return 0 on success.
 */
static int uart_putchar(char c, FILE *stream)
{
	(void)stream;
	serial_transmit(c);
	return 0;
}

/**
 * @brief File stream for USART0 output.
 *
 * This file stream is used to direct standard output (`stdout`) to USART0.
 * It is initialized with `uart_putchar` as the output function.
 */
static FILE z_stdout_usart = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

/**
 * @brief Set standard I/O to use USART0.
 *
 * This function configures the standard I/O library to use USART0 for
 * output operations. It sets `stdout` to the `z_stdout_usart` file stream
 * which directs output to USART0.
 */
void k_set_stdio_usart0(void)
{
	stdout = &z_stdout_usart;
}

#elif CONFIG_STDIO_PRINTF_TO_USART > 0

#warning "CONFIG_STDIO_PRINTF_TO_USART > 0 not supported"

#endif /* CONFIG_STDIO_PRINTF_TO_USART */
