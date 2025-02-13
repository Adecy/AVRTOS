/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "init.h"

#include "canaries.h"
#include "debug.h"
#include "kernel.h"
#include "kernel_private.h"
#include "mem_slab.h"
#include "stack_sentinel.h"
#include "stdout.h"
#include "timer.h"

extern void z_init_sysclock(void);
extern void z_init_threads(void);

#if CONFIG_KERNEL_MINICORE_SAVE_RESET_CAUSE
__noinit uint8_t z_mcusr;
__attribute__((naked, used, section(".init0"))) void z_save_mcusr(void)
{
	/*
	 * Read r2 and write it to z_mcusr
	 * Equivalent to: sts z_mcusr, r2
	 */
	asm volatile("sts %0, r2"
				 : "=m"(z_mcusr)
				 : /* no input */
	);
}

/* This code is equivalent to the following assembly code:

.section .noinit
.global z_mcusr
z_mcusr:
	.byte 1

.section .init0
.global z_save_mcusr
z_save_mcusr:
	sts z_mcusr, r2

*/
#endif

void z_avrtos_init(void)
{	
#if CONFIG_KERNEL_CLEAR_WDT_ON_INIT
	/* Page 52 (ATmega328p datasheet) :
	 *	Note: If the Watchdog is accidentally enabled, for example by a
	 *runaway pointer or brown-out condition, the device will be reset and
	 *the Watchdog Timer will stay enabled. If the code is not set up to
	 *handle the Watchdog, this might lead to an eternal loop of time-out
	 *resets. To avoid this situation, the application software should
	 * always clear the Watchdog System Reset Flag (WDRF) and the WDE
	 *control bit in the initialization routine, even if the Watchdog is not
	 *in use.
	 */

	/* If the watchdog caused the reset, clear the flag */
	if (MCUSR & BIT(WDRF)) {
		MCUSR &= ~BIT(WDRF);
		WDTCSR |= (BIT(WDCE) | BIT(WDE)); // Enable changes to WDTCSR
		WDTCSR = 0x00;					  // Disable the Watchdog Timer
	}
#endif /* CONFIG_KERNEL_INIT_CLEAR_WDT */

#if CONFIG_KERNEL_MINICORE_SAVE_RESET_CAUSE
	/* Clear MCUSR for the next reset */
	MCUSR = 0u;
#endif

	/* Initialize debug pins */
	__Z_DBG_GPIO_0_INIT();
	__Z_DBG_GPIO_1_INIT();
	__Z_DBG_GPIO_2_INIT();
	__Z_DBG_GPIO_3_INIT();

#if CONFIG_SERIAL_AUTO_INIT
	serial_init();
#if CONFIG_AVRTOS_BANNER_ENABLE
	serial_print_banner();
#endif /* CONFIG_AVRTOS_BANNER_ENABLE */
#endif /* CONFIG_SERIAL_AUTO_INIT */

#if CONFIG_KERNEL_DEBUG_PREEMPT_UART
	SET_BIT(UCSR0B, BIT(RXCIE0)); // Enable UART receive complete interrupt
#endif

#if CONFIG_STDIO_PRINTF_TO_USART >= 0
	/* Redirect printf output to USART0 */
	k_set_stdio_usart0();
#endif

	/* Initialize the thread system */
	z_init_threads();

#if CONFIG_AVRTOS_LINKER_SCRIPT
	/* Initialize memory slab module */
	z_mem_slab_init_module();
#endif

#if CONFIG_KERNEL_TIMERS && CONFIG_AVRTOS_LINKER_SCRIPT
	/* Initialize timer module */
	z_timer_init_module();
#endif

#if CONFIG_THREAD_CANARIES && CONFIG_AVRTOS_LINKER_SCRIPT
	/* Initialize stack canaries */
	z_init_stacks_canaries();
#endif

#if CONFIG_THREAD_STACK_SENTINEL
	/* Initialize stack sentinels */
	z_init_stacks_sentinel();
#endif

	/* Initialize system clock */
	z_init_sysclock();

#if (CONFIG_INTERRUPT_POLICY == 2) && (CONFIG_THREAD_MAIN_COOPERATIVE == 0)
	/* Lock the scheduler if required by configuration */
	k_sched_lock();
#endif

#if CONFIG_INTERRUPT_POLICY > 0
	/* Enable global interrupts */
	sei();
#endif
}

/**
 * @brief Automatically initialize AVRTOS when using the Arduino framework.
 *
 * This function is called automatically by the Arduino framework to initialize
 * the AVRTOS kernel if the auto-initialization option is enabled (it wiLL be).
 */
#if Z_CONFIG_ARDUINO_FRAMEWORK && CONFIG_KERNEL_AUTO_INIT
void initVariant(void)
{
	z_avrtos_init();
}
#endif /* ARDUINO */
