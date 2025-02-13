/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AVRTOS_DEFAULT_CONFIG_H
#define _AVRTOS_DEFAULT_CONFIG_H

#include <avr/io.h>

#include "sys.h"

//
// Enable adaptations for RUST support
//
// 0: RUST support is disabled
// 1: RUST support is enabled
//
#ifndef CONFIG_RUST
#define CONFIG_RUST 0
#endif

//
// Indicates whether the kernel is compiled for the Arduino framework.
//
// 0: The kernel is not compiled for the Arduino framework.
// 1: The kernel is compiled for the Arduino framework.
//
// This option is set to 1 if the ARDUINO macro is detected.
//
#if defined(ARDUINO)
#define Z_CONFIG_ARDUINO_FRAMEWORK 1
#else
#define Z_CONFIG_ARDUINO_FRAMEWORK 0
#endif

//
// Indicates whether the kernel is compiled using the PlatformIO environment.
//
// 0: The kernel is not compiled using the PlatformIO environment.
// 1: The kernel is compiled using the PlatformIO environment.
//
// This option is set to 1 if the PLATFORMIO macro is detected.
//
#if defined(PLATFORMIO)
#define Z_CONFIG_PLATFORMIO_IDE 1
#else
#define Z_CONFIG_PLATFORMIO_IDE 0
#endif

// RUST support is not compatible with Arduino framework or PlatformIO IDE
#if CONFIG_RUST && (CONFIG_ARDUINO_FRAMEWORK || CONFIG_PLATFORMIO_IDE)
#error "RUST support is not compatible with Arduino framework or PlatformIO IDE"
#endif

/*
 * Include Arduino-specific configuration if the Arduino framework is used
 * without PlatformIO IDE (e.g., Arduino IDE).
 *
 * This configuration is adapted for beginners.
 */
#if Z_CONFIG_ARDUINO_FRAMEWORK && !Z_CONFIG_PLATFORMIO_IDE
#include "avrtos_arduinoide_conf.h"
#endif

#if CONFIG_RUST
#include "avrtos_rust_conf.h"
#endif

//
// This file contains all the default configuration options for the kernel.
//
// You may need to clear the build directory if you modify this configuration file.
//
// Some constants defined here may be used in assembly files, so avoid using the
// "unsigned" flag ("u") at the end of numbers.
// e.g., prefer: CONFIG_THREAD_MAIN_STACK_SIZE 0x100
// over CONFIG_THREAD_MAIN_STACK_SIZE 0x100u
//

//
// Indicates whether the AVRTOS linker script was provided, allowing kernel sections to be
// used to reference and initialize kernel objects during RTOS initialization.
//
// Note: If disabled, all kernel objects must be initialized manually.
// - K_THREAD_DEFINE() is disabled and must be replaced by
//   K_THREAD_DEFINE_STATIC() and initialized manually using
//   k_thread_static_create_and_schedule().
// - The following macros are still usable but require manual initialization:
//   K_TIMER_DEFINE(), K_MEM_SLAB_DEFINE().
// - Disables functions: k_dump_stack_canaries(), k_thread_dump_all().
//
// Note: The Arduino framework with Arduino IDE requires this option to be disabled,
// as it is not possible to provide a custom linker script.
//
// 0: The AVRTOS linker script is not provided. Kernel sections are not used. Kernel
//    objects must be initialized manually.
// 1: The AVRTOS linker script is provided. Kernel sections are used. Kernel objects
//    are initialized automatically.
//
// Additional notes:
//
// If CONFIG_AVRTOS_LINKER_SCRIPT is explicitly set by the user, its value is used.
// Otherwise, the presence of the linker script is inferred. This is the purpose of
// specific configuration files like "avrtos_arduinoide_conf.h":
// - If using the Arduino framework with the Arduino IDE, the AVRTOS linker will not be
// used, so assume that sections are not available.
// - If using the Arduino framework with PlatformIO IDE, the AVRTOS linker can and should
// be used, so assume that sections are available.
//
#ifndef CONFIG_AVRTOS_LINKER_SCRIPT
#define CONFIG_AVRTOS_LINKER_SCRIPT 1
#endif

//
// Defines the main thread type (cooperative/preemptive) and priority.
//
// 0: The main thread is preemptive.
// 1: The main thread is cooperative.
//
#ifndef CONFIG_THREAD_MAIN_COOPERATIVE
#define CONFIG_THREAD_MAIN_COOPERATIVE 1
#endif

//
// Interrupt policy on main thread startup.
//
// 0: Interrupts are disabled.
// 1: Interrupts are enabled.
// 2: Interrupts are enabled, but the scheduler is locked.
//
#ifndef CONFIG_INTERRUPT_POLICY
#define CONFIG_INTERRUPT_POLICY 1
#endif

//
// Indicates whether the main stack location is at RAMEND (0) or allocated in a dedicated
// buffer (1).
//
// 0: The main stack is allocated at RAMEND (size is undetermined).
// 1: The main stack is allocated in a dedicated buffer (size is known at compile time).
//
// IMPORTANT NOTE: If you are using the heap, you must set this to 0, as the standard
// library's malloc uses the stack pointer to check the remaining heap size. i.e., With an
// explicit main stack, a buffer is allocated (for the main thread's stack) before the
// heap. The standard library's malloc() will then consistently fail if the stack pointer
// exceeds the limit (actually on the other side of the heap). The checker expects the
// main stack to be near RAMEND, in the upper memory regions.
//
#ifndef CONFIG_THREAD_EXPLICIT_MAIN_STACK
#define CONFIG_THREAD_EXPLICIT_MAIN_STACK 0
#endif

//
// This configuration option defines the size of the main stack.
// If canaries are enabled, the entire stack will be filled with the canary value.
//
// If CONFIG_THREAD_EXPLICIT_MAIN_STACK is enabled, the main stack is allocated in a
// dedicated buffer in the .data section.
//
// If CONFIG_THREAD_EXPLICIT_MAIN_STACK is enabled, ensure the main stack size does not
// exceed the remaining RAM size.
//
// If the heap grows downward, canaries might be invalidated if the expected stack size is
// too large.
//
#ifndef CONFIG_THREAD_MAIN_STACK_SIZE
#define CONFIG_THREAD_MAIN_STACK_SIZE 0x200
#endif

//
// Indicates whether malloc is expected to be used in the main thread.
//
// This option is a safeguard to ensure you set DEFAULT_THREAD_EXPLICIT_MAIN_STACK=1 when
// using malloc in the main thread.
//
// 0: malloc is not used in the main thread.
// 1: malloc is used in the main thread.
//
#ifndef CONFIG_USE_STDLIB_HEAP_MALLOC_MAIN
#define CONFIG_USE_STDLIB_HEAP_MALLOC_MAIN 0
#endif

//
// Indicates whether malloc is expected to be used in other threads.
//
// 0: malloc is not used in other threads.
// 1: malloc is used in other threads.
//
#ifndef CONFIG_USE_STDLIB_HEAP_MALLOC_THREAD
#define CONFIG_USE_STDLIB_HEAP_MALLOC_THREAD 0
#endif

//
// Default SREG value for other threads on stack creation.
// The main thread's default SREG is always 0.
//
// From the datasheet:
//  Bit 7 6 5 4 3 2 1 0
//  0x3F (0x5F) I T H S V N Z C SREG
//  Read/Write R/W R/W R/W R/W R/W R/W R/W R/W
//  Initial Value 0 0 0 0 0 0 0 0
//
// It is highly recommended to keep interrupts enabled when starting other threads;
// otherwise, interrupts should be re-enabled in the thread code if time-critical
// operations are performed.
//
// 0: Interrupts are disabled when threads start (other than the main thread).
// (1 << SREG_I): Interrupts are enabled when threads start (other than the main thread).
//
#ifndef CONFIG_THREAD_DEFAULT_SREG
#define CONFIG_THREAD_DEFAULT_SREG (1 << SREG_I)
#endif

//
// Allow or disallow thread termination.
// Disabling this option (0) saves up to 2-3 bytes of stack per thread (depending on the
// architecture).
//
// 0: Thread termination is not handled (CPU exception).
// 1: Thread termination is enabled.
// -1: Thread termination is not permitted (kernel fault).
//
#ifndef CONFIG_KERNEL_THREAD_TERMINATION_TYPE
#define CONFIG_KERNEL_THREAD_TERMINATION_TYPE 0
#endif

//
// Compare thread addresses before a thread switch to avoid unnecessary switches to the
// same context.
//
// Note: Inefficient if at least two threads are always ready.
//
// 0: Do not compare thread addresses before switching.
// 1: Compare thread addresses before switching.
//
#ifndef CONFIG_KERNEL_SCHEDULER_COMPARE_THREADS_BEFORE_SWITCH
#define CONFIG_KERNEL_SCHEDULER_COMPARE_THREADS_BEFORE_SWITCH 1
#endif

//
// Enable or disable cooperative threads. This feature allows threads to not be preempted
// by the scheduler when they are ready.
//
// 0: Cooperative threads are disabled.
// 1: Cooperative threads are enabled.
//
#ifndef CONFIG_KERNEL_COOPERATIVE_THREADS
#define CONFIG_KERNEL_COOPERATIVE_THREADS 1
#endif

//
// Sysclock period when precision mode is disabled ("llu" suffix is important).
//
// Reducing this value can cause some precision loss.
//
// The range depends on the hardware timer resolution and prescaler.
//
// Usually, 1 ms is a good value.
//
#ifndef CONFIG_KERNEL_SYSCLOCK_PERIOD_US
#define CONFIG_KERNEL_SYSCLOCK_PERIOD_US 1000llu
#endif

//
// Time slice in microseconds (0 if using SYSCLOCK period).
//
// Interval between two context switches of preemptive threads.
//
// The range depends on CONFIG_KERNEL_SYSCLOCK_PERIOD_US.
// Usually, using the CONFIG_KERNEL_SYSCLOCK_PERIOD_US value is a good choice.
//
#ifndef CONFIG_KERNEL_TIME_SLICE_US
#define CONFIG_KERNEL_TIME_SLICE_US 1000llu
#endif

//
// Select the hardware timer used for the kernel sysclock.
//
// ATmega328P/...
// 0: Timer 0 (8 bits).
// 1: Timer 1 (16 bits).
// 2: Timer 2 (8 bits).
//
// ATmega2560/ATmega328PB/...
// 3: Timer 3 (16 bits).
// 4: Timer 4 (16 bits).
// 5: Timer 5 (16 bits).
//
#ifndef CONFIG_KERNEL_SYSLOCK_HW_TIMER
#define CONFIG_KERNEL_SYSLOCK_HW_TIMER 1
#endif

//
// Use 40 bits for the ticks counter size (instead of 32 bits).
//
// Using a 40-bit counter size allows for a longer period before overflow.
// e.g., with a 1 ms sysclock period, a 40-bit counter allows for a period of
// 34 years before overflow, compared to 49 days with a 32-bit counter.
//
// 0: 32-bit counter size.
// 1: 40-bit counter size.
//
#ifndef CONFIG_CONFIG_KERNEL_TICKS_COUNTER_40BITS
#define CONFIG_CONFIG_KERNEL_TICKS_COUNTER_40BITS 1
#endif

//
// Use 32 bits for delay objects (k_timeout_t / k_ticks_t / k_delta_t).
//
// Disabling this object can save some flash and RAM space.
// - Adding two U32 objects instead of U16 requires twice as much code.
// - There are many wait functions.
// - IMPORTANT: Increasing the sysclock period can help reduce the
//   maximum number needed to store for a delay.
//
// This configuration is unrelated to time functions; it only applies when
//  giving a timeout to a kernel function (such as k_sleep, k_mutex_lock, ...).
//
// 0: Delay objects are 16 bits.
// 1: Delay objects are 32 bits.
//
#ifndef CONFIG_KERNEL_DELAY_OBJECT_U32
#define CONFIG_KERNEL_DELAY_OBJECT_U32 0
#endif

//
// Kernel auto-initialization.
//
// This option might not work if AVRTOS is linked as a library in some environments (e.g.,
// PlatformIO). In this case, you should either call kernel_init() manually or use the
// z_avrtos_init() function manually. Alternatively, link the auto-initialization function
// with the K_KERNEL_LINK_AVRTOS_INIT() macro.
//
// 0: Kernel auto-initialization is disabled.
// 1: Kernel auto-initialization is enabled.
//
#ifndef CONFIG_KERNEL_AUTO_INIT
#define CONFIG_KERNEL_AUTO_INIT 1
#endif

//
// Save the reset cause retrieved from the MCUSR register by the MiniCore bootloader.
//
// The MiniCore bootloader saves the MCUSR (reset cause) value in the r2 register
// before jumping to the application. Enabling this option allows copying
// this value before it is overwritten by the kernel.
//
// 0: Reset cause is not saved.
// 1: Reset cause is saved.
//
#ifndef CONFIG_KERNEL_MINICORE_SAVE_RESET_CAUSE
#define CONFIG_KERNEL_MINICORE_SAVE_RESET_CAUSE 0
#endif

//
// Clear the WDT on kernel initialization.
//
//  Page 52 (ATmega328p datasheet):
// 	Note: If the Watchdog is accidentally enabled, for example by a
// runaway pointer or brown-out condition, the device will be reset and
// the Watchdog Timer will remain enabled. If the code is not set up to
// handle the Watchdog, this might lead to an endless loop of time-out
// resets. To avoid this situation, the application software should
//  always clear the Watchdog System Reset Flag (WDRF) and the WDE
// control bit in the initialization routine, even if the Watchdog is not
// in use.
//
//
// 0: WDT is not cleared.
// 1: WDT is cleared.
//
#ifndef CONFIG_KERNEL_CLEAR_WDT_ON_INIT
#define CONFIG_KERNEL_CLEAR_WDT_ON_INIT 0
#endif

//
// Indicates whether the kernel should define an idle thread to enable other threads to
// sleep. If disabled, at least one thread must always be ready; otherwise, a fault will
// be triggered.
//
// 0: Kernel idle thread is disabled.
// 1: Kernel idle thread is enabled.
//
#ifndef CONFIG_KERNEL_THREAD_IDLE
#define CONFIG_KERNEL_THREAD_IDLE 1
#endif

//
// Kernel idle thread additional stack (for interrupt handler stack).
// In some cases, the idle thread stack is not enough to execute heavy interrupt
// handlers. This option allows additional stack space for the idle thread.
//
// The minimum recommended value is 0x50.
//
#ifndef CONFIG_KERNEL_THREAD_IDLE_ADD_STACK
#define CONFIG_KERNEL_THREAD_IDLE_ADD_STACK 0x50
#endif

//
// Indicates whether the idle thread is preemptive or cooperative.
// Note: If preemptive, additional stack is allocated for the thread.
//
// 0: Kernel idle thread is preemptive.
// 1: Kernel idle thread is cooperative.
//
#ifndef CONFIG_THREAD_IDLE_COOPERATIVE
#define CONFIG_THREAD_IDLE_COOPERATIVE 0
#endif

//
// Indicates whether the kernel should define an idle hook to be executed when no thread
// is ready. This hook is executed in the context of the idle thread.
//
// 0: Kernel idle hook is disabled.
// 1: Kernel idle hook is enabled.
//
#ifndef CONFIG_IDLE_HOOK
#define CONFIG_IDLE_HOOK 0
#endif

//
// Enable thread canaries. Thread stacks are filled with a canary symbol
// to detect stack overflows.
//
// There is minimal overhead when initializing the stacks. It is always a good idea to
// enable this for high stack usage.
//
// 0: Thread canaries are disabled.
// 1: Thread canaries are enabled.
//
#ifndef CONFIG_THREAD_CANARIES
#define CONFIG_THREAD_CANARIES 0
#endif

//
// Define the thread canary symbol.
//
// 0xAA or 0x55 are good values.
//
#ifndef CONFIG_THREAD_CANARIES_SYMBOL
#define CONFIG_THREAD_CANARIES_SYMBOL 0xAA
#endif

//
// Indicates whether a sentinel should be added at the end of the thread stack.
// When corrupted, the sentinel will help detect stack overflows.
//
// 0: Thread sentinel is disabled.
// 1: Thread sentinel is enabled.
//
#ifndef CONFIG_THREAD_STACK_SENTINEL
#define CONFIG_THREAD_STACK_SENTINEL 0
#endif

//
// Define thread sentinel size.
//
// 1 is a good value.
//
#ifndef CONFIG_THREAD_STACK_SENTINEL_SIZE
#define CONFIG_THREAD_STACK_SENTINEL_SIZE 1
#endif

//
// Define the thread sentinel symbol.
//
// 0xAA or 0x55 are good values.
//
#ifndef CONFIG_THREAD_STACK_SENTINEL_SYMBOL
#define CONFIG_THREAD_STACK_SENTINEL_SYMBOL 0x55
#endif

//
// Monitor thread stack to detect overflows.
//
// 0: Thread stack monitor is disabled.
// 1: Thread stack monitor is enabled.
//
#ifndef CONFIG_THREAD_MONITOR
#define CONFIG_THREAD_MONITOR 0
#endif

//
// Monitor the main thread stack to detect overflows.
//
// * This option is discouraged with CONFIG_THREAD_EXPLICIT_MAIN_STACK=1, except
// if you can predict the main thread's maximum stack usage by defining
// CONFIG_THREAD_MAIN_STACK_SIZE.
//
// 0: Main thread stack monitor is disabled.
// 1: Main thread stack monitor is enabled.
//
#ifndef CONFIG_THREAD_MAIN_MONITOR
#define CONFIG_THREAD_MAIN_MONITOR 0
#endif

//
// Enable the system workqueue.
//
// 0: System workqueue is disabled.
// 1: System workqueue is enabled.
//
#ifndef CONFIG_SYSTEM_WORKQUEUE_ENABLE
#define CONFIG_SYSTEM_WORKQUEUE_ENABLE 0
#endif

//
// Define the system workqueue stack size.
// Workqueue stack size depends on the maximum stack required by the work
// handlers when executed.
//
#ifndef CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE
#define CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE 0x200
#endif

//
// Indicates whether the system workqueue should be executed cooperatively.
//
// 0: System workqueue is preemptive.
// 1: System workqueue is cooperative.
//
#ifndef CONFIG_SYSTEM_WORKQUEUE_COOPERATIVE
#define CONFIG_SYSTEM_WORKQUEUE_COOPERATIVE 0
#endif

//
// Indicates whether support for delayable work items is enabled.
//
// 0: Delayable work items are disabled.
// 1: Delayable work items are enabled.
//
#ifndef CONFIG_WORKQUEUE_DELAYABLE
#define CONFIG_WORKQUEUE_DELAYABLE 0
#endif

//
// Enable kernel assertion tests for debugging purposes.
//
// 0: Kernel assertion is disabled.
// 1: Kernel assertion is enabled.
//
#ifndef CONFIG_KERNEL_ASSERT
#define CONFIG_KERNEL_ASSERT 0
#endif

//
// Enable kernel argument checks.
//
// 0: Kernel argument checks are disabled.
// 1: Kernel argument checks are enabled.
//
#ifndef CONFIG_KERNEL_ARGS_CHECKS
#define CONFIG_KERNEL_ARGS_CHECKS 0
#endif

//
// Enable timer support.
// - This feature requires additional stack space for threads.
//
// 0: Kernel timers are disabled.
// 1: Kernel timers are enabled.
//
#ifndef CONFIG_KERNEL_TIMERS
#define CONFIG_KERNEL_TIMERS 0
#endif

//
// Enable event support.
// - This feature requires additional stack space for threads.
//
// 0: Kernel events are disabled.
// 1: Kernel events are enabled.
//
#ifndef CONFIG_KERNEL_EVENTS
#define CONFIG_KERNEL_EVENTS 0
#endif

//
// Allow scheduling an event with K_NO_WAIT.
// This will cause the event callback to be executed immediately.
//
// 0: Require a non-zero timeout to schedule an event.
// 1: Allow scheduling an event with K_NO_WAIT.
//
#ifndef CONFIG_KERNEL_EVENTS_ALLOW_NO_WAIT
#define CONFIG_KERNEL_EVENTS_ALLOW_NO_WAIT 1
#endif

//
// Automatic initialization of the serial console.
//
#ifndef CONFIG_SERIAL_AUTO_INIT
#define CONFIG_SERIAL_AUTO_INIT 0u
#endif

//
// USART0 (serial console) default baud rate.
//
#ifndef CONFIG_SERIAL_USART_BAUDRATE
#define CONFIG_SERIAL_USART_BAUDRATE 115200u
#endif

//
// Redirect printf function output to a specified USART.
// Disable this option by setting it to -1.
//
// -1: printf is disabled.
// 0: printf is redirected to USART0.
// 1: printf is redirected to USART1, if it exists.
// 2: printf is redirected to USART2, if it exists.
// 3: printf is redirected to USART3, if it exists.
// 4: printf is redirected to USART4, if it exists.
// 5: printf is redirected to USART5, if it exists.
// 6: printf is redirected to USART6, if it exists.
//
#ifndef CONFIG_STDIO_PRINTF_TO_USART
#define CONFIG_STDIO_PRINTF_TO_USART -1
#endif

//
// Enable the logging subsystem.
//
// 0: Logging subsystem is disabled.
// 1: Logging subsystem is enabled.
//
#ifndef CONFIG_LOGGING_SUBSYSTEM
#define CONFIG_LOGGING_SUBSYSTEM 1
#endif

//
// Enable the uptime counter.
//
// 0: Uptime counter is disabled.
// 1: Uptime counter is enabled.
//
#ifndef CONFIG_KERNEL_UPTIME
#define CONFIG_KERNEL_UPTIME 0
#endif

//
// Enable time API
//
// 0: Time API is disabled
// 1: Time API is enabled
//
#ifndef CONFIG_KERNEL_TIME_API
#define CONFIG_KERNEL_TIME_API 0
#endif

//
// Enable time API with millisecond precision
//
// 0: Time API is using second precision
// 1: Time API is using millisecond precision
//
#ifndef CONFIG_KERNEL_TIME_API_MS_PRECISION
#define CONFIG_KERNEL_TIME_API_MS_PRECISION 0
#endif

//
// Enable atomic API
//
// 0: Atomic API is disabled
// 1: Atomic API is enabled
//
#ifndef CONFIG_KERNEL_ATOMIC_API
#define CONFIG_KERNEL_ATOMIC_API 1
#endif

//
// Use UART0 RX interrupt as preemptive signal
// Note: Reserved for debug purpose
//
// 0: UART0 RX interrupt is not used as preemptive signal
// 1: UART0 RX interrupt is used as preemptive signal
//
#ifndef CONFIG_KERNEL_DEBUG_PREEMPT_UART
#define CONFIG_KERNEL_DEBUG_PREEMPT_UART 0
#endif

//
// Force inlining of kernel functions, reserved for debug purpose
//
// 0: Kernel function inlining is disabled
// 1: Kernel function inlining is enabled
//
#ifndef CONFIG_KERNEL_API_NOINLINE
#define CONFIG_KERNEL_API_NOINLINE 0
#endif

//
// Enable Kernel Debug features
//
// 0: Kernel Debug is disabled
// 1: Kernel Debug is enabled
//
#ifndef CONFIG_KERNEL_SYSCLOCK_DEBUG
#define CONFIG_KERNEL_SYSCLOCK_DEBUG 0
#endif

//
// Enable Kernel Debug in scheduler
//
// 0: Kernel Debug in scheduler is disabled
// 1: Kernel Debug in scheduler is enabled
//
#ifndef CONFIG_KERNEL_SCHEDULER_DEBUG
#define CONFIG_KERNEL_SCHEDULER_DEBUG 0
#endif

//
// Maximum number of file descriptors
//
// 0: No file descriptor
// n: Maximum number of file descriptors
//
#ifndef CONFIG_FD_MAX_COUNT
#define CONFIG_FD_MAX_COUNT 0
#endif

//
// Enable reentrancy support for kernel functions
//
// For each manipulated object, it must be released as many times as it has been
// acquired, otherwise the expected behavior of the "release" function is not
// guaranteed.
//
// This applies to:
// - k_mutex_lock()/k_mutex_unlock(): Enable IRQ lock counter for each mutex to allow
//   k_mutex_lock() to be called recursively.
// - k_sched_lock()/k_sched_unlock(): Enable scheduler lock counter for each thread to
//   allow k_sched_lock() to be called recursively.
//
// 0: Kernel reentrancy support is disabled
// 1: Kernel reentrancy support is enabled
//
#ifndef CONFIG_KERNEL_REENTRANCY
#define CONFIG_KERNEL_REENTRANCY 0
#endif

//
// Enable asynchronous support for USART0
//
// 0: USART0 asynchronous support is disabled
// 1: USART0 asynchronous support is enabled
//
#ifndef CONFIG_DRIVERS_USART0_ASYNC
#define CONFIG_DRIVERS_USART0_ASYNC 0
#endif

//
// Enable asynchronous support for USART1
//
// 0: USART1 asynchronous support is disabled
// 1: USART1 asynchronous support is enabled
//
#ifndef CONFIG_DRIVERS_USART1_ASYNC
#define CONFIG_DRIVERS_USART1_ASYNC 0
#endif

//
// Enable asynchronous support for USART2
//
// 0: USART2 asynchronous support is disabled
// 1: USART2 asynchronous support is enabled
//
#ifndef CONFIG_DRIVERS_USART2_ASYNC
#define CONFIG_DRIVERS_USART2_ASYNC 0
#endif

//
// Enable asynchronous support for USART3
//
// 0: USART3 asynchronous support is disabled
// 1: USART3 asynchronous support is enabled
//
#ifndef CONFIG_DRIVERS_USART3_ASYNC
#define CONFIG_DRIVERS_USART3_ASYNC 0
#endif

//
// Enable high level support for timer0
//
// 0: High level support for timer0 is disabled
// 1: High level support for timer0 is enabled
//
#ifndef CONFIG_DRIVERS_TIMER0_API
#define CONFIG_DRIVERS_TIMER0_API 0
#endif

//
// Enable high level support for timer1
//
// 0: High level support for timer1 is disabled
// 1: High level support for timer1 is enabled
//
#ifndef CONFIG_DRIVERS_TIMER1_API
#define CONFIG_DRIVERS_TIMER1_API 0
#endif

//
// Enable high level support for timer2
//
// 0: High level support for timer2 is disabled
// 1: High level support for timer2 is enabled
//
#ifndef CONFIG_DRIVERS_TIMER2_API
#define CONFIG_DRIVERS_TIMER2_API 0
#endif

//
// Enable high level support for timer3
//
// 0: High level support for timer3 is disabled
// 1: High level support for timer3 is enabled
//
#ifndef CONFIG_DRIVERS_TIMER3_API
#define CONFIG_DRIVERS_TIMER3_API 0
#endif

//
// Enable high level support for timer4
//
// 0: High level support for timer4 is disabled
// 1: High level support for timer4 is enabled
//
#ifndef CONFIG_DRIVERS_TIMER4_API
#define CONFIG_DRIVERS_TIMER4_API 0
#endif

//
// Enable high level support for timer5
//
// 0: High level support for timer5 is disabled
// 1: High level support for timer5 is enabled
//
#ifndef CONFIG_DRIVERS_TIMER5_API
#define CONFIG_DRIVERS_TIMER5_API 0
#endif

//
// Debug systick using given GPIO pin of PORTB
//
// 0: Systick debug is disabled
// 1 << n: Systick debug is enabled on given GPIO pin of PORTB
//
// Example: with CONFIG_KERNEL_SYSTICK_GPIOB_DEBUG=0x80,
// systick debug is enabled on GPIO pin 7 of PORTB
//
#ifndef CONFIG_KERNEL_SYSTICK_GPIOB_DEBUG
#define CONFIG_KERNEL_SYSTICK_GPIOB_DEBUG 0
#endif

//
// Enable statistics for kernel
//
// 0: Kernel statistics is disabled
// 1: Kernel statistics is enabled
//
#ifndef CONFIG_KERNEL_STATS
#define CONFIG_KERNEL_STATS 0
#endif

//
// Enable faults verbosity
//
// 0: Kernel faults silently
// 1: Kernel faults with message
// 2: Kernel faults with message and stack dump
//
#ifndef CONFIG_KERNEL_FAULT_VERBOSITY
#define CONFIG_KERNEL_FAULT_VERBOSITY 1
#endif

//
// Enable kernel debug GPIO
//
// 0: Kernel debug GPIO is disabled
// 1: GPIO port A pins 0 to 3
// 2: GPIO port A pins 4 to 7
// 3: GPIO port B pins 0 to 3
// 4: GPIO port B pins 4 to 7
// 5: GPIO port C pins 0 to 3
// 6: GPIO port C pins 4 to 7
// 7: GPIO port D pins 0 to 3
// 8: GPIO port D pins 4 to 7
//
#ifndef CONFIG_KERNEL_DEBUG_GPIO
#define CONFIG_KERNEL_DEBUG_GPIO 0
#endif

//
// Enable kernel debug GPIO for systick
//
// Depends on CONFIG_KERNEL_DEBUG_GPIO
//
// 0: Kernel debug GPIO for systick is disabled
// 1: Kernel debug GPIO for systick is enabled (toggle on systick)
// 2: Kernel debug GPIO for systick is enabled (enter/exit on systick)
#ifndef CONFIG_KERNEL_DEBUG_GPIO_SYSTICK
#define CONFIG_KERNEL_DEBUG_GPIO_SYSTICK 1
#endif

//
// Enable asynchronous support for SPI
//
// This option defines ISR(SPI_STC_vect) { } interrupt handler, which
// prevents the developer from defining its own.
//
// Synchronous functions can still be used when this option is enabled.
// However, synchronous functions should not be used when an asynchronous
// transfer is in progress.
//
// 0: SPI asynchronous support is disabled
// 1: SPI asynchronous support is enabled
//
#ifndef CONFIG_SPI_ASYNC
#define CONFIG_SPI_ASYNC 0
#endif

//
// Enable AVRTOS banner on startup
//
// This option automatically initializes the serial console with default baudrate
//
// 0: AVRTOS banner is disabled
// 1: AVRTOS banner is enabled
//
#ifndef CONFIG_AVRTOS_BANNER_ENABLE
#define CONFIG_AVRTOS_BANNER_ENABLE 0
#endif

//
// AVRTOS banner string
//
#ifndef CONFIG_AVRTOS_BANNER
#define CONFIG_AVRTOS_BANNER "*** AVRTOS ***\n"
#endif

//
// Enable I2C driver
//
// 0: I2C driver is disabled
// 1: I2C driver is enabled
//
#ifndef CONFIG_I2C_DRIVER_ENABLE
#define CONFIG_I2C_DRIVER_ENABLE 1
#endif

//
// Enable I2C0 device
//
// 0: I2C0 device is disabled
// 1: I2C0 device is enabled
//
#ifndef CONFIG_I2C0_ENABLED
#define CONFIG_I2C0_ENABLED 1
#endif

//
// Enable I2C1 device
//
// 0: I2C1 device is disabled
// 1: I2C1 device is enabled
//
#ifndef CONFIG_I2C1_ENABLED
#define CONFIG_I2C1_ENABLED 0
#endif

//
// Configure I2C driver as interrupt driven
//
// 0: I2C driver is not interrupt driven
// 1: I2C driver is interrupt driven
//
#ifndef CONFIG_I2C_INTERRUPT_DRIVEN
#define CONFIG_I2C_INTERRUPT_DRIVEN 0
#endif

//
// Configure I2C driver frequency
//
// Value in Hz
//
#ifndef CONFIG_I2C_FREQ
#define CONFIG_I2C_FREQ 400000
#endif

//
// Make I2C transactions blocking
//
// 0: I2C transactions are non-blocking
// 1: I2C transactions are blocking
//
#ifndef CONFIG_I2C_BLOCKING
#define CONFIG_I2C_BLOCKING 1
#endif

//
// Configure I2C driver maximum buffer length.
// Maximum buffer length is 2^CONFIG_I2C_MAX_BUF_LEN_BITS
//
// 0: Maximum buffer length is 1 byte
// 1: Maximum buffer length is 2 bytes
// 2: Maximum buffer length is 4 bytes
// 3: Maximum buffer length is 8 bytes
// 4: Maximum buffer length is 16 bytes
// 5: Maximum buffer length is 32 bytes
//
#ifndef CONFIG_I2C_MAX_BUF_LEN_BITS
#define CONFIG_I2C_MAX_BUF_LEN_BITS 3u
#endif

//
// Enable I2C driver last error tracking
//
// 0: I2C driver last error tracking is disabled
// 1: I2C driver last error tracking is enabled
//
#ifndef CONFIG_I2C_LAST_ERROR
#define CONFIG_I2C_LAST_ERROR 1
#endif

//
// Enable I2C driver debug
//
// 0: I2C driver debug is disabled
// 1: I2C driver debug is enabled
//
#ifndef CONFIG_I2C_DEBUG
#define CONFIG_I2C_DEBUG 0
#endif

//
// Define the size of the flags of the flags subsystem (k_flags_* functions)
//
// 1: 8 bits (default)
// 2: 16 bits
// 4: 32 bits
// 8: 64 bits
//
#ifndef CONFIG_KERNEL_FLAGS_SIZE
#define CONFIG_KERNEL_FLAGS_SIZE 1
#endif

//
// Enable support for the MCP2515 CAN controller
//
// 0: MCP2515 driver is disabled
// 1: MCP2515 driver is enabled
//
#ifndef CONFIG_DEVICE_MCP2515
#define CONFIG_DEVICE_MCP2515 0
#endif

#endif
