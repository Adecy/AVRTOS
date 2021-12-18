#include "idle.h"

#include <avr/sleep.h>
/*___________________________________________________________________________*/

#if KERNEL_THREAD_IDLE

static void _k_idle_entry(void *context);

K_THREAD_DEFINE(_k_idle, _k_idle_entry, K_TRHEAD_IDLE_STACK_SIZE, K_PREEMPTIVE, NULL, 'I');

/**
 * @brief Idle thread entry function
 * 
 * @param context : ignored for now
 */
static void _k_idle_entry(void *context)
{
        for (;;) {
                /* only an interrupt can wake up the CPU after this instruction */
                sleep_cpu();
        }
}

#endif /* KERNEL_THREAD_IDLE */

extern struct ditem *runqueue;

bool k_is_cpu_idle(void)
{
#if KERNEL_THREAD_IDLE
	return runqueue == &_k_idle.tie.runqueue;
#else 
	return false;
#endif /* KERNEL_THREAD_IDLE */
}

static inline bool _k_runqueue_single(void)
{
        return runqueue->next == runqueue;
}

void k_idle(void)
{
	/* if others thread a ready, yield the CPU */
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if (!_k_runqueue_single()) {
			return k_yield();
		}
	}

	sleep_cpu();
}

/*___________________________________________________________________________*/