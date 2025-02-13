/*
 * Copyright (c) 2024 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AVRTOS_TDQUEUE_H
#define _AVRTOS_TDQUEUE_H

#include "dlist.h"
#include "../defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Scheduling queue data structure
 *
 * - n : number of items in the list
 *
 * tqueue_schedule/tqueue_pop are O(1)
 * tqueue_remove is O(1)
 * tqueue_shift is O(n)
 */

struct tditem {
	union {
		k_delta_t delay_shift;
		k_delta_t abs_delay;
		k_delta_t timeout;
	};
	struct dnode node;
};

typedef struct tditem tditem_t;
typedef struct tditem tdqueue_t;

#define INIT_TDITEM(self, timeout_ms)                                                    \
	{                                                                                    \
		{                                                                                \
			.timeout = timeout_ms,                                                       \
		},                                                                               \
			.node = DITEM_INIT(self.node)                                                \
	}
#define INIT_TDITEM_DEFAULT(name) INIT_TDITEM(name, 0)

#define DEFINE_TDQUEUE(name) dlist_t name = DLIST_INIT(name)
#define DEFINE_TDITEM(name)	 struct tditem name = INIT_TDITEM_DEFAULT(name)

/**
 * @brief Schedule an event by adding it to the list.
 * If the new event has the same timeout as another event
 * already in the list, it will be added after i.
 *
 * Assumptions :
 * - root is not null
 * - item is not null
 * - item->node is is DLL default*,
 * 	 doubly linked list default node is a node with both next and prev pointing to itself.
 *
 * @param root
 * @param item
 */
void z_tdqueue_schedule(dlist_t *tqueue, struct tditem *item);

/**
 * @see z_tqueue_schedule
 *
 * Assumptions :
 *  - root is not null
 *
 * @param root
 * @param item
 * @param timeout
 */
void tdqueue_schedule(dlist_t *tqueue, struct tditem *item, k_delta_t timeout);

/**
 * @brief Shift the queue time of {time_passed}
 *
 * Assumptions :
 *  - root is not null
 *
 * @param root
 * @param time_passed
 */
void tdqueue_shift(dlist_t *tqueue, k_delta_t time_passed);

/**
 * @brief Pop an item from the time queue.
 *
 * Note: this function doesn't alter/reset the item->tie nor the item->next members.
 *
 * Assumptions:
 * - root is not null
 *
 * @param root
 * @return struct tditem*
 */
struct tditem *tdqueue_pop(dlist_t *tqueue);

/**
 * @brief
 *
 * @param root
 * @return struct tditem*
 */
struct tditem *tdqueue_pop_reschedule(dlist_t *tqueue, k_delta_t timeout);

/**
 * @brief Remove an item from the time queue.
 * Item->next attribute is set to default if found.
 *
 * Assumptions :
 *  - root is not null
 *  - item is not null
 *  - item is in root tqueue
 *
 * @param root
 * @param item Item to remove if exists in the time queue.
 */
void tdqueue_remove(dlist_t *tqueue, struct tditem *item);

#ifdef __cplusplus
}
#endif

#endif