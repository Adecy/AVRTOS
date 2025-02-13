/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AVRTOS_RUST_H
#define _AVRTOS_RUST_H

#include "kernel.h"

void k_sleep_1s(void);

uint8_t z_rust_irq_lock(void);

void z_rust_irq_unlock(uint8_t key);

uint16_t z_rust_sys(uint16_t sys);

#endif
