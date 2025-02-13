/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timer.h"

#include <avrtos/assert.h>

#define DRIVERS_TIMERS_API                                                               \
	((CONFIG_DRIVERS_TIMER0_API) || (CONFIG_DRIVERS_TIMER1_API) ||                       \
	 (CONFIG_DRIVERS_TIMER2_API) || (CONFIG_DRIVERS_TIMER3_API) ||                       \
	 (CONFIG_DRIVERS_TIMER4_API) || (CONFIG_DRIVERS_TIMER5_API))

#define K_MODULE K_MODULE_DRIVERS_TIMERS

/* not __always_inline version of timer_get_index */
static int8_t _get_index(void *dev)
{
	return timer_get_index(dev);
}

static bool _is_8bits(uint8_t tim_idx)
{
	return TIMER_INDEX_IS_8BITS(tim_idx);
}

static bool _is_16bits(uint8_t tim_idx)
{
	return TIMER_INDEX_IS_16BITS(tim_idx);
}

void ll_timer8_deinit(TIMER8_Device *dev, uint8_t tim_idx)
{
	ARG_UNUSED(tim_idx);

	dev->TCCRnA = 0u;
	dev->TCCRnB = 0u;
}

void ll_timer8_init(TIMER8_Device *dev,
					uint8_t tim_idx,
					const struct timer_config *config)
{
	/* Set timer mode WGMn0:3
	 * bit 0 for timer 0 is ignored */
	const uint8_t mode = config->mode >> 1u;

	dev->TCCRnA = (mode & 0x03u) << WGMn0;
	dev->TCCRnB = (mode >> 2u) << WGMn2;

	switch (config->mode) {
	case TIMER_MODE_NORMAL:
		dev->TCNTn = config->counter;
		break;
	case TIMER_MODE_CTC:
		dev->OCRnA = config->counter & 0xFFU;
	default:
		break;
	}

	ll_timer_set_enable_int_mask(tim_idx, config->timsk);

	/* timer starts counting from here  (prescaler != 0) */
	dev->TCCRnB = (config->prescaler << CSn0);
}

int8_t timer8_init(TIMER8_Device *dev, const struct timer_config *config)
{
	if (config == NULL) {
		return -EINVAL;
	}

	int8_t tim_idx = _get_index(dev);
	if (tim_idx < 0) {
		return tim_idx;
	}

	if (_is_8bits(tim_idx) == false) {
		return -EINVAL;
	}

	if ((tim_idx == 2) && (config->prescaler > TIMER_PRESCALER_1024)) {
		return -EINVAL;
	}

	/* timer starts counting when prescaler is different than 0 */
	ll_timer8_init(dev, tim_idx, config);

	return 0;
}

void ll_timer16_deinit(TIMER16_Device *dev, uint8_t tim_idx)
{
	ARG_UNUSED(tim_idx);
	
	dev->TCCRnA = 0u;
	dev->TCCRnB = 0u;
	dev->TCCRnC = 0u;
}

void ll_timer16_init(TIMER16_Device *dev,
					 uint8_t tim_idx,
					 const struct timer_config *config)
{
	/* Set timer mode WGMn0:3 */
	dev->TCCRnA = (config->mode & 0x03u) << WGMn0;
	dev->TCCRnB = ((config->mode >> 2u) & 0x03u) << WGMn2;

	switch (config->mode) {
	case TIMER_MODE_NORMAL:
		dev->TCNTn = config->counter;
		break;
	case TIMER_MODE_CTC:
		/* read 16.9.2 Clear Timer on Compare Match (CTC) Mode */
		dev->OCRnA = config->counter;
		break;
	case TIMER_MODE_FAST_PWM_8bit:	/* TOP 0x00FF */
	case TIMER_MODE_FAST_PWM_9bit:	/* TOP 0x01FF */
	case TIMER_MODE_FAST_PWM_10bit: /* TOP 0x03FF */
		/* update of OCRnx on BOTTOM */

		break;
	default:
		break;
	}

	ll_timer_set_enable_int_mask(tim_idx, config->timsk);

	/* timer starts counting from here  (prescaler != 0) */
	dev->TCCRnB |= (config->prescaler << CSn0);
}

void ll_timer16_channel_configure(TIMER16_Device *dev,
								  timer_channel_t channel,
								  const struct timer_channel_compare_config *config)
{
	const uint8_t group_shift = (2 * (2 - channel) + 2u);
	const uint8_t reg_val	  = dev->TCCRnA & ~(0x03u << group_shift);
	dev->TCCRnA				  = reg_val | (config->mode << group_shift);
	ll_timer16_write_reg16(&dev->OCRnx[channel], config->value);
}

int8_t timer16_init(TIMER16_Device *dev, const struct timer_config *config)
{
	if (config == NULL) {
		return -EINVAL;
	}

	int8_t tim_idx = _get_index(dev);
	if (tim_idx < 0) {
		return tim_idx;
	}

	if (_is_16bits(tim_idx) == false) {
		return -EINVAL;
	}

	ll_timer16_init(dev, tim_idx, config);

	return 0;
}

int8_t timer8_deinit(TIMER8_Device *dev)
{
	int8_t tim_idx = _get_index(dev);
	if (tim_idx < 0) {
		return tim_idx;
	}

	ll_timer_clear_enable_int_mask(tim_idx);
	ll_timer_clear_irq_flags(tim_idx);

	dev->TCCRnB = 0U;
	dev->TCCRnA = 0U;

	dev->OCRnA = 0U;
	dev->OCRnB = 0U;
	dev->TCNTn = 0U;

	return 0;
}

int8_t timer16_deinit(TIMER16_Device *dev)
{
	int8_t tim_idx = _get_index(dev);
	if (tim_idx < 0) {
		return tim_idx;
	}

	ll_timer_clear_enable_int_mask(tim_idx);
	ll_timer_clear_irq_flags(tim_idx);

	/* disable interrupts first */
	dev->TCCRnA = 0U;
	dev->TCCRnB = 0U;
	dev->TCCRnC = 0U;

	dev->OCRnA = 0U;
	dev->OCRnB = 0U;
	dev->OCRnC = 0U;
	dev->IRCN  = 0U;
	dev->TCNTn = 0U;

	return 0;
}

static int timer_get_prescaler_value(timer_prescaler_t prescaler)
{
	switch (prescaler) {
	case TIMER_PRESCALER_1:
		return 1;
	case TIMER_PRESCALER_8:
		return 8;
	case TIMER_PRESCALER_64:
		return 64;
	case TIMER_PRESCALER_256:
		return 256;
	case TIMER_PRESCALER_1024:
		return 1024;
	default:
		return -EINVAL;
	}
}

static int timer2_get_presc_value(timer2_prescaler_t prescaler)
{
	switch (prescaler) {
	case TIMER2_PRESCALER_1:
		return 1;
	case TIMER2_PRESCALER_8:
		return 8;
	case TIMER2_PRESCALER_32:
		return 32;
	case TIMER2_PRESCALER_64:
		return 64;
	case TIMER2_PRESCALER_128:
		return 128;
	case TIMER2_PRESCALER_256:
		return 256;
	case TIMER2_PRESCALER_1024:
		return 1024;
	default:
		return -EINVAL;
	}
}

int8_t timer_calc_prescaler(uint8_t timer_index, uint32_t period_us, uint16_t *counter)
{
	const bool is_16bit		   = _is_16bits(timer_index);
	const uint16_t max_counter = is_16bit ? 0xFFFFu : 0xFFu;
	const bool is_timer2	   = timer_index == 2; /* timer2 has more prescaler values */

	/* iterate over prescaler values and find the best match */
	uint8_t prescaler_id = 0;

	uint32_t calc_counter;

	do {
		prescaler_id++; /* fetch next prescaler */
		int prescaler_val = is_timer2 ? timer2_get_presc_value(prescaler_id)
									  : timer_get_prescaler_value(prescaler_id);
		if (prescaler_val < 0) {
			return -ENOTSUP;
		}
		calc_counter = TIMER_CALC_COUNTER_VALUE(period_us, prescaler_val);
	} while (calc_counter > max_counter);

	*counter = (uint16_t)calc_counter;

	return (int8_t)prescaler_id;
}

#if DRIVERS_TIMERS_API

static void *_get_device(uint8_t tim_idx)
{
	return timer_get_device(tim_idx);
}

struct timer_api_ctx {
	timer_callback_t cb;
	void *user_data;

	uint8_t prescaler : 3U;
};

__bss struct timer_api_ctx tim_ctx[TIMERS_COUNT];

#define __DECL_TIMER_COMPA_ISR(n)                                                        \
	ISR(TIMER##n##_COMPA_vect)                                                           \
	{                                                                                    \
		struct timer_api_ctx *const ctx = &tim_ctx[n];                                   \
		void *const dev					= timer_get_device(n);                           \
		if (ctx->cb != NULL) {                                                           \
			ctx->cb(dev, n, ctx->user_data);                                             \
		}                                                                                \
	}

#if CONFIG_DRIVERS_TIMER0_API && TIMER_INDEX_EXISTS(0)
__STATIC_ASSERT(CONFIG_KERNEL_SYSLOCK_HW_TIMER != 0,
				"Cannot enable drivers API for system timer 0");
__DECL_TIMER_COMPA_ISR(0);
#endif /* CONFIG_DRIVERS_TIMER0_API */

#if CONFIG_DRIVERS_TIMER1_API && TIMER_INDEX_EXISTS(1)
__STATIC_ASSERT(CONFIG_KERNEL_SYSLOCK_HW_TIMER != 1,
				"Cannot enable drivers API for system timer 1");
__DECL_TIMER_COMPA_ISR(1);
#endif /* CONFIG_DRIVERS_TIMER1_API */

#if CONFIG_DRIVERS_TIMER2_API && TIMER_INDEX_EXISTS(2)
__STATIC_ASSERT(CONFIG_KERNEL_SYSLOCK_HW_TIMER != 2,
				"Cannot enable drivers API for system timer 2");
__DECL_TIMER_COMPA_ISR(2);
#endif /* CONFIG_DRIVERS_TIMER2_API */

#if CONFIG_DRIVERS_TIMER3_API && TIMER_INDEX_EXISTS(3)
__STATIC_ASSERT(CONFIG_KERNEL_SYSLOCK_HW_TIMER != 3,
				"Cannot enable drivers API for system timer 3");
__DECL_TIMER_COMPA_ISR(3);
#endif /* CONFIG_DRIVERS_TIMER3_API */

#if CONFIG_DRIVERS_TIMER4_API && TIMER_INDEX_EXISTS(4)
__STATIC_ASSERT(CONFIG_KERNEL_SYSLOCK_HW_TIMER != 4,
				"Cannot enable drivers API for system timer 4");
__DECL_TIMER_COMPA_ISR(4);
#endif /* CONFIG_DRIVERS_TIMER4_API */

#if CONFIG_DRIVERS_TIMER5_API && TIMER_INDEX_EXISTS(5)
__STATIC_ASSERT(CONFIG_KERNEL_SYSLOCK_HW_TIMER != 5,
				"Cannot enable drivers API for system timer 5");
__DECL_TIMER_COMPA_ISR(5);
#endif /* CONFIG_DRIVERS_TIMER5_API */

int8_t timer_init(uint8_t tim_idx,
				  uint32_t period_us,
				  timer_callback_t cb,
				  void *user_data,
				  uint8_t flags)
{
	if ((tim_idx == CONFIG_KERNEL_SYSLOCK_HW_TIMER) || (cb == NULL)) {
		return -EINVAL;
	}

	void *const dev = _get_device(tim_idx);
	if (dev == NULL) {
		return -EINVAL;
	}

	uint16_t counter;

	int8_t prescaler_id = timer_calc_prescaler(tim_idx, period_us, &counter);
	if (prescaler_id < 0) {
		return prescaler_id;
	}

	tim_ctx[tim_idx].cb		   = cb;
	tim_ctx[tim_idx].user_data = user_data;
	tim_ctx[tim_idx].prescaler = prescaler_id;

	const struct timer_config cfg = {
		.mode	   = TIMER_MODE_CTC,
		.prescaler = (flags & TIMER_API_FLAG_AUTOSTART) ? prescaler_id : 0U,
		.counter   = counter,
		.timsk	   = BIT(OCIEnA)};

	const bool is_16bit_timer = _is_16bits(tim_idx);
	if (is_16bit_timer) {
		ll_timer16_init((TIMER16_Device *)dev, tim_idx, &cfg);
	} else {
		ll_timer8_init((TIMER8_Device *)dev, tim_idx, &cfg);
	}

	return 0;
}

void timer_start(uint8_t tim_idx)
{
	__ASSERT_TRUE(TIMER_INDEX_EXISTS(tim_idx));

	void *const dev = _get_device(tim_idx);
	if (dev != NULL) {
		ll_timer_start(dev, timer_get_prescaler(tim_idx));
	}
}

void timer_stop(uint8_t tim_idx)
{
	__ASSERT_TRUE(TIMER_INDEX_EXISTS(tim_idx));

	void *const dev = _get_device(tim_idx);
	if (dev != NULL) {
		ll_timer_stop(dev);
	}
}

uint8_t timer_get_prescaler(uint8_t tim_idx)
{
	__ASSERT_TRUE(TIMER_INDEX_EXISTS(tim_idx));

	return tim_ctx[tim_idx].prescaler;
}

uint32_t timer_get_max_period_us(uint8_t tim_idx)
{
	if (_is_16bits(tim_idx)) {
		return (1024LU * 0x10000LU) / (F_CPU / 1000000LU);
	} else if (_is_8bits(tim_idx)) {
		return (1024LU * 0x100LU) / (F_CPU / 1000000LU);
	} else {
		return 0;
	}
}

#endif /* DRIVERS_TIMERS_API */