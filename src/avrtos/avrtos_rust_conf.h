/*
 * Copyright (c) 2023 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AVRTOS_SYS_RUST_DEFAULT_CONFIG_H
#define _AVRTOS_SYS_RUST_DEFAULT_CONFIG_H

/*
 * Adapter configuration in an rust environment
 * This file is included by avrtos_conf.h
 */

/* A linker script is provided by the rust environment */
#ifndef CONFIG_AVRTOS_LINKER_SCRIPT
#define CONFIG_AVRTOS_LINKER_SCRIPT 1
#endif

/* Kernel must be explicitly initialized in rust environment */
#ifndef CONFIG_KERNEL_AUTO_INIT
#define CONFIG_KERNEL_AUTO_INIT 0
#endif

#ifndef CONFIG_KERNEL_DELAY_OBJECT_U32
#define CONFIG_KERNEL_DELAY_OBJECT_U32 1
#endif

#ifndef CONFIG_AVRTOS_BANNER
#define CONFIG_AVRTOS_BANNER "*** AVRTOS RUST ***\n"
#endif

#endif /* _AVRTOS_SYS_RUST_DEFAULT_CONFIG_H */