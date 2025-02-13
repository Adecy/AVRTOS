/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AVRTOS_KERNEL_TYPES_H
#define _AVRTOS_KERNEL_TYPES_H

#include <stddef.h>

#include "dstruct/dlist.h"
#include "defines.h"
#include "dstruct/tqueue.h"

/**
 * @brief Thread entry point function type.
 *
 * The `k_thread_entry_t` type is a function pointer type representing a thread
 * entry point function. A thread entry point function is the function that serves
 * as the starting point for the execution of a thread. It takes a single
 * `void*` parameter, which is typically used to pass the thread's context or
 * other initial data when the thread begins execution.
 *
 * @param context A pointer to the context data for the thread.
 */
typedef void (*k_thread_entry_t)(void *);

/**
 * @brief Structure representing a thread.
 *
 * The `struct k_thread` structure represents a thread within the kernel. It contains
 * various fields that define the state and properties of the thread, including:
 * - The stack pointer (`sp`)
 * - The thread's flags, which indicate the thread's current state or behavior.
 * - A union of structures for managing the thread within different kernel queues
 * (runqueue, events queue, or pending queues for various kernel objects like mutexes,
 * semaphores, etc.).
 * - A `swap_data` field, which stores data returned by kernel APIs when the thread is
 *   unpended.
 * - A stack structure that defines the stack's end address and size.
 * - A symbol character that can be used to represent the thread in debugging or
 * visualization tools.
 *
 * This structure is designed to be lightweight, with a minimal size of 16 bytes in its
 * basic form.
 */

struct k_thread {
	void *sp; ///< Stack pointer, must be the first member of the structure !!

	uint8_t flags; ///< Thread flags indicating the state or attributes of the thread.

	union {
		struct dnode
			runqueue; ///< Node for the runqueue (used when the thread is runnable).
		struct titem
			event; ///< Node for the events queue (used for delayed or timed events).
	} tie; ///< Union that holds the queue information, ensuring the thread is in only one
		   ///< queue at a time.

	union {
		struct dnode wany;	 ///< Node for waiting on a generic object.
		struct dnode wmutex; ///< Node for waiting on a mutex.
		struct dnode wsem;	 ///< Node for waiting on a semaphore.
		struct dnode wsig;	 ///< Node for waiting on a signal.
		struct dnode wfifo;	 ///< Node for waiting on a FIFO item.
		struct dnode wmsgq;	 ///< Node for waiting on a message queue item.
		struct dnode wflags; ///< Node for waiting on flags.
	}; ///< Union that allows the thread to be pending on different types of kernel
	   ///< objects.

	void *swap_data; ///< Data returned by kernel APIs when the thread is unpended.

	struct {
		void *end;	 ///< End of the stack memory.
		size_t size; ///< Size of the stack.
	} stack;		 ///< Stack information for the thread.

	char symbol; ///< A single character symbol representing the thread, reserved symbols:
				 ///< 'M' for main, 'I' for idle.

#if CONFIG_KERNEL_REENTRANCY
	/**
	 * @brief Depth of nested calls to k_sched_lock().
	 */
	uint8_t sched_lock_cnt;
#endif /* CONFIG_KERNEL_REENTRANCY */
};

/**
 * @brief Define the size of the kernel ticks counter.
 */
typedef char z_ticks_t[CONFIG_KERNEL_TICKS_COUNTER_SIZE];

typedef struct __packed z_kernel {
	/**
	 * @brief Pointer to the thread currently holding the CPU (context set).
	 *
	 * @note There are cases where *runqueue* and *current* do not refer to the same
	 * thread. This is because the next thread to be scheduled is already set, but the
	 * context hasn't been switched yet.
	 *
	 * @note *current* member must not be moved within the structure.
	 */
	struct k_thread *current;

#if CONFIG_KERNEL_TICKS_COUNTER
	/**
	 * @brief Ticks counter for the kernel.
	 *
	 * This array represents the ticks counter used by the kernel to keep track of
	 * time. The counter is implemented as an array of bytes with a specific size
	 * determined by `CONFIG_KERNEL_TICKS_COUNTER_SIZE`.
	 *
	 * @note *ticks* member must not be moved within the structure.
	 */
	z_ticks_t ticks;
#endif

#if Z_KERNEL_TIME_SLICE_MULTIPLE_TICKS
	/**
	 * @brief Number of ticks remaining in the current time slice.
	 *
	 * This variable represents the number of ticks remaining in the current time
	 * slice. A time slice is a predetermined time duration allocated to each thread
	 * for execution before being preempted and giving control to another thread.
	 *
	 * The variable `sched_ticks_remaining` is initialized with the value
	 * specified in `Z_KERNEL_TIME_SLICE_TICKS`, which indicates the length of
	 * the time slice in ticks. As the thread runs and consumes ticks, the value of
	 * this variable decreases. When it reaches zero, the thread may be preempted,
	 * and control is given to another thread.
	 */
	uint8_t sched_ticks_remaining;
#endif

	/**
	 * @brief Number of threads (excluding IDLE) in the runqueue(s).
	 *
	 * The IDLE thread is not included in this count.
	 *
	 * This variable represents the count of threads currently in the runqueue(s),
	 * indicating the number of threads that are ready to run. It is initialized to
	 * 1 during startup, indicating that only the main thread is running initially.
	 *
	 * The possible values and their meanings are as follows:
	 * - 0: Indicates that only the IDLE thread is running.
	 * - 1: Indicates that a single thread is running.
	 * - n > 1: Indicates that multiple threads are running.
	 */
	uint8_t ready_count;

	/**
	 * @brief Pointer to the currently running thread in the runqueue.
	 *
	 * This variable represents a pointer to the thread (node) in the runqueue,
	 * which is currently running. The runqueue is implemented as a doubly-linked
	 * list, where each node represents a thread. During startup, this variable is
	 * initialized with the pointer to the node corresponding to the main thread.
	 *
	 * The runqueue is structured as a circular doubly-linked list, where each node
	 * contains a pointer to the next and previous nodes. The illustration below
	 * demonstrates the relationship between the nodes in the runqueue:
	 *
	 *         A     B     C
	 *     -> n --> n --> n --   (n for next, p for previous)
	 *     -- p <-- p <-- p <-
	 *             /\
	 *  ___________|
	 *  run_queue points to the thread currently running, here B
	 *
	 * By maintaining a pointer to the currently running thread (node) in run_queue, it
	 * becomes convenient to execute the next thread by simply setting run_queue to
	 * run_queue->next.
	 */
	struct dnode *run_queue;

	/**
	 * @brief Pointer to the head of the timeouts queue.
	 *
	 * This variable represents a pointer to the head of the timeouts queue. The
	 * timeouts queue is a linked list that holds the pending threads in the system.
	 * Each thread in the list corresponds to a timeout event and is represented by
	 * a `struct titem`.
	 */
	struct titem *timeouts_queue;

#if CONFIG_KERNEL_ASSERT
	/**
	 * @brief Flag indicating execution context.
	 *
	 * This flag is used to indicate whether the kernel code is currently being
	 * executed. Its purpose is to detect if a user handler is being executed from a
	 * kernel context, such as within kernel timer or kernel event handlers.
	 *
	 * Initially set to 0, indicating that the
	 * kernel code is not being executed. When the kernel code is entered, this flag
	 * is set to 1.
	 */
	uint8_t kernel_mode;
#endif
} z_kernel_t;

/* ARCHITECTURE-SPECIFIC TYPES */

/**
 * @brief Structure representing the call-saved registers plus the program counter (PC).
 *
 * The `struct z_callsaved_ctx` structure represents the registers that must be saved
 * during a context switch. It includes:
 * - The status register (`sreg`).
 * - Registers `r7` to `r29`, which are saved as part of the context.
 * - The stack pointer and program counter, depending on the AVR architecture
 *   (with/without 3 byte wide PC).
 *
 * This structure is used to save the context of a thread when it is preempted or
 * voluntarily yields the CPU.
 *
 * For more information, refer to the "Call-Saved Registers" section in the AVR GCC
 * manual: https://gcc.gnu.org/wiki/avr-gcc#Call-Saved_Registers
 */
struct z_callsaved_ctx {
	uint8_t sreg; ///< Status register.

	union {
		uint8_t regs[13]; ///< Array to access all saved registers.
		struct {
			uint8_t r29;
			uint8_t r28;
			uint8_t r17;
			uint8_t r16;
			uint8_t r15;
			uint8_t r14;
			uint8_t r13;
			uint8_t r12;
			uint8_t r11;
			uint8_t r10;
			uint8_t r9;
			uint8_t r8;
			uint8_t r7;
		};
	};

	union {
		uint8_t r6;		   ///< Register r6.
		uint8_t init_sreg; ///< Initial status register value.
	};

	union {
		struct {
			uint8_t r5;
			uint8_t r4;
		};
		k_thread_entry_t thread_entry; ///< Thread entry point function.
	};

	union {
		struct {
			uint8_t r3;
			uint8_t r2;
		};
		void *thread_context; ///< Pointer to the thread's context data.
	};

	/*
	 * DOCUMENTATION
	 * R0, T-Flag:
	 * The temporary register and the T-flag in SREG are also
	 * call-clobbered, but this knowledge is not exposed explicitly to the
	 * compiler (R0 is a fixed register).
	 */

	struct {
#if Z_ARCH_PC_SIZE == 3
		uint8_t pch; ///< High byte of the program counter, if using a 3-byte PC.
#endif				 /* Z_ARCH_PC_SIZE == 3 */
		void *pc;	 ///< Program counter (PC) value.
	};
};

/**
 * @brief Structure representing the call-used registers of a function call.
 *
 * The `struct z_callused_ctx` structure holds the registers that are used during a
 * function call but are not preserved across function calls. These registers must
 * be saved by the caller if their values are needed after a function call.
 *
 * For more information, refer to the "Call-Used Registers" section in the AVR GCC manual:
 * https://gcc.gnu.org/wiki/avr-gcc#Call-Used_Registers
 */
struct z_callused_ctx {
	uint8_t r18;
	uint8_t r19;
	uint8_t r20;
	uint8_t r21;
	uint8_t r22;
	uint8_t r23;
	uint8_t r24;
	uint8_t r25;
	uint8_t r26;
	uint8_t r27;
	uint8_t r30;
	uint8_t r31;
};

/**
 * @brief Structure representing the context saved during an interrupt.
 *
 * The `struct z_intctx` structure represents the registers and program counter (PC)
 * that are saved when an interrupt occurs. This context includes the program counter,
 * status register, and the call-used registers.
 *
 * The structure is used by the ISR to restore the thread's context once the interrupt
 * has been handled.
 */
struct z_intctx {
#if Z_ARCH_PC_SIZE == 3
	uint8_t pch; ///< High byte of the program counter, if using a 3-byte PC.
#endif			 /* Z_ARCH_PC_SIZE == 3 */
	void *pc;	 ///< Program counter (PC) value at the time of the interrupt.

	uint8_t r1;	  ///< Register r1.
	uint8_t r0;	  ///< Register r0.
	uint8_t sreg; ///< Status register.

	struct z_callused_ctx
		callused_reg; ///< Call-used registers saved during the interrupt.
};

#endif /* _AVRTOS_KERNEL_TYPES_H */
