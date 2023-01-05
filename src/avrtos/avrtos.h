/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AVRTOS_H
#define _AVRTOS_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "common.h"
#include "defines.h"
#include "multithreading.h"
#include "init.h"

#include "dstruct/queue.h"
#include "dstruct/oqueue.h"
#include "dstruct/dlist.h"
#include "dstruct/tqueue.h"
#include "dstruct/debug.h"

#include "atomic.h"
#include "ring.h"
#include "flags.h"
#include "fault.h"
#include "assert.h"
#include "kernel.h"
#include "idle.h"

#include "canaries.h"
#include "stack_sentinel.h"
#include "prng.h"
#include "time.h"

#include "workqueue.h"
#include "mutex.h"
#include "semaphore.h"
#include "timer.h"
#include "event.h"
#include "signal.h"

#include "fifo.h"
#include "mem_slab.h"
#include "msgq.h"

#include "io.h"

#endif