#include "kernel.h"

#include "idle.h"

#include <util/atomic.h>

#include "debug.h"

/*___________________________________________________________________________*/

#define K_MODULE    K_MODULE_KERNEL

/*___________________________________________________________________________*/

/**
 * @brief Runqueue containing the queue of all ready threads.
 * Should never be NULL.
 */
struct ditem *runqueue = &_k_thread_main.tie.runqueue;

struct titem *events_queue = NULL;

static inline bool _k_runqueue_single(void)
{
        return runqueue->next == runqueue;
}

/*___________________________________________________________________________*/

//
// Kernel Public API
//

void k_sched_lock(void)
{
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
                SET_BIT(_current->flags, K_FLAG_SCHED_LOCKED);
        }

        __K_DBG_SCHED_LOCK(_current);
}

void k_sched_unlock(void)
{
        __K_DBG_SCHED_UNLOCK();

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
                CLR_BIT(_current->flags, K_FLAG_SCHED_LOCKED);
        }
}

bool k_sched_locked(void)
{
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
                return (bool)TEST_BIT(_current->flags, K_FLAG_SCHED_LOCKED);
        }

        __builtin_unreachable();
}

void k_sleep(k_timeout_t timeout)
{
        if (timeout.value != K_NO_WAIT.value) {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                {
                        _k_reschedule(timeout);

                        k_yield();
                }
        }
}

/*___________________________________________________________________________*/

//
// Not tested
//

void k_suspend(void)
{
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
                _k_suspend();
        }
}

void k_resume(struct k_thread *th)
{
        if (th->state == PENDING) {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                {
                        th->state = READY;
                        _k_schedule(&th->tie.runqueue);
                }
        } else {
                /* thread pending, ready of running and then already started */
        }
}

void k_start(struct k_thread *th)
{
        if (th->state == STOPPED) {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                {
                        _k_queue(th);
                }
        }
}

/*___________________________________________________________________________*/

//
// Kernel Private API
//

extern struct k_thread __k_threads_start;
extern struct k_thread __k_threads_end;

uint8_t _k_thread_count = 0;

inline static void swap_endianness(uint16_t *const addr)
{
        *addr = HTONS(*addr);
}

void _k_kernel_init(void)
{
        _k_thread_count = &__k_threads_end - &__k_threads_start;

        /* main thread is the first running (ready or not),
         * and it is already in queue */
        for (uint8_t i = 0; i < _k_thread_count; i++) {
                struct k_thread *const thread = &(&__k_threads_start)[i];

                /* idle thread must not be added to the
                 * runqueue as the main thread is running */
                if (!IS_THREAD_IDLE(thread) &&
                        (thread->state == READY) &&
                        (thread != _current)) {
                        push_back(runqueue, &thread->tie.runqueue);
                }

                /* Swap endianness of addresses in compilation-time built stacks.
                 * We cannot change the endianness of addresses determined by the
                 * linker at compilation time. So we need to do it on system start up
                 */
                if (_current != thread) {
                    /* thread kernel entry function address */
                        swap_endianness(thread->stack.end - 1u);

#if THREAD_ALLOW_RETURN == 1
                        /* thread kernel entry function address */
                        swap_endianness(thread->stack.end - 1u -
                                (6u + _K_ARCH_STACK_SIZE_FIXUP + 2u));
#endif

                        /* thread context address */
                        swap_endianness(thread->stack.end - 1u -
                                (8u + _K_ARCH_STACK_SIZE_FIXUP + 2u));
                }
        }
}

void _k_queue(struct k_thread *const th)
{
        __ASSERT_NOINTERRUPT();

        push_back(runqueue, &th->tie.runqueue);
}

void _k_schedule(struct ditem *const thread_tie)
{
        __ASSERT_NOINTERRUPT();

#if KERNEL_THREAD_IDLE
        if (_k_runqueue_idle()) {
                dlist_ref(thread_tie);
                runqueue = thread_tie;
                return;
        }
#endif

        push_front(runqueue, thread_tie);
}

void _k_schedule_wake_up(struct k_thread *thread, k_timeout_t timeout)
{
        __ASSERT_NOINTERRUPT();

        if (timeout.value != K_FOREVER.value) {
                _current->tie.event.timeout = timeout.value;
                _current->tie.event.next = NULL;
                _tqueue_schedule(&events_queue, &_current->tie.event);
        }
}

void _k_suspend(void)
{
        __ASSERT_NOINTERRUPT();

        _current->state = PENDING;

#if KERNEL_THREAD_IDLE
        if (_k_runqueue_single()) {
                struct ditem *const tie = &_k_idle.tie.runqueue;
                dlist_ref(tie);
                runqueue = tie;
                return;
        }
#else
        __ASSERT_LEASTONE_RUNNING();
#endif

        pop_ref(&runqueue);
}

void _k_unschedule(struct k_thread *th)
{
        __ASSERT_NOINTERRUPT();

        tqueue_remove(&events_queue, &th->tie.event);
}

struct k_thread *_k_scheduler(void)
{
        __ASSERT_NOINTERRUPT();

        _current->timer_expired = 0;
        if (_current->state == PENDING) {
                /* runqueue is positionned to the 
                 * normally next thread to be executed */
                __K_DBG_SCHED_PENDING();        // ~
        } else {
                /* next thread is positionned at the top of the runqueue */
                ref_requeue(&runqueue);

                __K_DBG_SCHED_REQUEUE();        // >
        }

        _current = CONTAINER_OF(runqueue, struct k_thread, tie.runqueue);

        __K_DBG_SCHED_NEXT(_current);

        return _current;
}

void _k_wake_up(struct k_thread *th)
{
        __ASSERT_NOINTERRUPT();
        __ASSERT_THREAD_STATE(th, PENDING);

        __K_DBG_WAKEUP(th); // @

        th->state = READY;

        _k_unschedule(th);

        _k_schedule(&th->tie.runqueue);
}

void _k_reschedule(k_timeout_t timeout)
{
        __ASSERT_NOINTERRUPT();

        _k_suspend();

        _k_schedule_wake_up(_current, timeout);
}

/*___________________________________________________________________________*/

void _k_system_shift(void)
{
        __ASSERT_NOINTERRUPT();

        tqueue_shift(&events_queue, KERNEL_TIME_SLICE);
        
        struct ditem *ready = (struct ditem *)tqueue_pop(&events_queue);
        if (ready != NULL) {
                __K_DBG_SCHED_EVENT(THREAD_FROM_EVENTQUEUE(ready));  // !

                /* set ready thread expired flag */
                THREAD_FROM_EVENTQUEUE(ready)->timer_expired = 1u;

                _k_schedule(ready);
        }

#if KERNEL_TIMERS
        _k_timers_process();
#endif
}

/*___________________________________________________________________________*/

int8_t _k_pend_current(struct ditem *waitqueue, k_timeout_t timeout)
{
        __ASSERT_NOINTERRUPT();

        int8_t err = -1;
        if (timeout.value != 0) {
                /* queue thread to pending queue of the object */
                dlist_queue(waitqueue, &_current->wany);

                /* schedule thread wake up if timeout is set */
                _k_reschedule(timeout);

                k_yield();

                /* if timer expired, we manually remove the thread from
                 * the pending queue
                 */
                if (_current->timer_expired) {
                        dlist_remove(&_current->wany);
                        err = -ETIMEOUT;
                } else {
                        err = 0;
                }
        }
        return err;
}
    
uint8_t _k_unpend_first_thread(struct ditem *waitqueue, void *swap_data)
{
        __ASSERT_NOINTERRUPT();

        struct ditem *pending_thread = dlist_dequeue(waitqueue);
        if (DITEM_VALID(waitqueue, pending_thread)) {
                struct k_thread *th = THREAD_FROM_WAITQUEUE(pending_thread);

                /* immediate wake up is not more required because
                 * with the swap model, the object is already reserved for the
                 * first pending thread
                 */
                _k_wake_up(th);

                /* set the available object address */
                th->swap_data = swap_data;

#if KERNEL_YIELD_ON_UNPEND
                k_yield();
#endif

                /* we return 0 if a pending thread got the object*/
                return 0;
        }
        /* if no thread is pending on the object, we simply return */
        return -1;
}

/*___________________________________________________________________________*/

void _k_on_thread_return(void)
{
        k_suspend();
}

/*___________________________________________________________________________*/