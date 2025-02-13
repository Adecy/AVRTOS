/*
 * Copyright (c) 2024 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rust_helpers.h"

#if CONFIG_RUST

void k_sleep_1s(void)
{
	k_msleep(1000);
}

uint8_t z_rust_irq_lock(void)
{
	return irq_lock();
}

void z_rust_irq_unlock(uint8_t key)
{
	irq_unlock(key);
}

uint16_t z_rust_sys(uint16_t sys)
{
	switch (sys)
	{
	case 1:
		k_sleep(K_MSEC(1));
		break;

	case 2:
		k_sleep(K_SECONDS(1));
		break;
	
	default:
		break;
	}

	return sys;
}

#endif