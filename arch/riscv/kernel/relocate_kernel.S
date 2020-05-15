/*
 * kexec for riscv
 *
 * Copyright (C) 2020-2025 Alibaba Group Holding Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kexec.h>
#include <linux/linkage.h>

#include <asm/kexec.h>
#include <asm/page.h>

ENTRY(riscv_relocate_new_kernel)
	/* Start new image. */
	mv	s1, a1
	mv	a0, zero
	mv	a1, a2
	mv	a2, zero
	mv	a3, zero
	jr	s1
ENDPROC(riscv_relocate_new_kernel)

.Lcopy_end:
.org	KEXEC_CONTROL_PAGE_SIZE

.globl riscv_relocate_new_kernel_size
riscv_relocate_new_kernel_size:
	.quad	.Lcopy_end - riscv_relocate_new_kernel
