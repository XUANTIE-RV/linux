/*
 * CPU reset routines
 *
 * Copyright (C) 2020-2025 Alibaba Group Holding Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/linkage.h>

ENTRY(__cpu_soft_restart)
		fence
		fence.i
		sfence.vma
		mv	s1, a0		//entry
		mv	a0, a1		//arg0
		mv	a1, a2		//arg1
		mv	a2, a3		//arg2
		jr s1
		ebreak
ENDPROC(__cpu_soft_restart)
