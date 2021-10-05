#include "mutex.h"

#include <avrtos/kernel.h>
#include <avrtos/debug.h>

#include <util/atomic.h>

void k_mutex_init(struct k_mutex* mutex)
{
    mutex->lock = 0xFFu;
    mutex->owner = NULL;
    dlist_init(&mutex->waitqueue);
}

uint8_t k_mutex_lock(struct k_mutex* mutex, k_timeout_t timeout)
{
    uint8_t lock;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        lock = _k_mutex_lock(mutex);
        if (lock != 0) {
            lock = _k_pend_current(&mutex->waitqueue, timeout);
        }

        if (lock == 0) {
            __K_DBG_MUTEX_LOCKED(_current);    // }
            mutex->owner = _current;
        }
    }
    return lock;
}

void k_mutex_unlock(struct k_mutex* mutex)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        /* we check if the current thread actually owns the mutex */
        if (mutex->owner != _current) {
            return;
        }

        __K_DBG_MUTEX_UNLOCKED(_current); // {

        /* there is a new owner, we don't need to unlock the mutex
         * The mutex owner is changed when returning to the
         * k_mutex_lock function.
         */
        if (_k_unpend_first_thread(&mutex->waitqueue, NULL) != 0) {
            /* no new owner, we need to unlock 
             * the mutex and remove the owner
             */
            _k_mutex_unlock(mutex);
            mutex->owner = NULL;
        }
    }
}