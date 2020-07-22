#ifndef _ASM_RISCV_FENCE_H
#define _ASM_RISCV_FENCE_H

#ifdef CONFIG_SMP
#define RISCV_ACQUIRE_BARRIER		"\tfence r , rw\n"
#define RISCV_RELEASE_BARRIER		"\tfence rw,  w\n"
#else
#define RISCV_ACQUIRE_BARRIER
#define RISCV_RELEASE_BARRIER
#endif

extern int c910_mmu_v1_flag;
#define sync_mmu_v1() \
	if (c910_mmu_v1_flag) asm volatile (".long 0x01b0000b");

#endif	/* _ASM_RISCV_FENCE_H */
