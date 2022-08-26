/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _DSTRUCT_DEBUG_H
#define _DSTRUCT_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/*___________________________________________________________________________*/

#include <avrtos/misc/uart.h>

/*___________________________________________________________________________*/

#include "queue.h"

//
// Queue
//
void print_queue(struct qitem *root, void (*qitem_printer)(struct qitem *item));

/*___________________________________________________________________________*/

#include "oqueue.h"

//
// OQueue
//
void print_oqueue(struct oqref *oref, void (*qitem_printer)(struct qitem *item));

/*___________________________________________________________________________*/

#include "dlist.h"

//
// DList
//
void print_dlist(struct ditem *dlist, void (*ditem_printer)(struct ditem *item));

/*___________________________________________________________________________*/

#include "tqueue.h"

//
// TQueue
//
void print_tqueue(struct titem *root, void (*titem_printer)(struct titem *item));

/*___________________________________________________________________________*/

#ifdef __cplusplus
}
#endif

#endif