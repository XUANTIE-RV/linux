// SPDX-License-Identifier: GPL-2.0

#include <linux/mm.h>
#include <linux/smp.h>

#define XUANTIE
#ifdef  XUANTIE
#include <asm/mmu_context.h>

int c910_mmu_v1_flag = 0;

void flush_tlb_all(void)
{
if (c910_mmu_v1_flag) {
	sync_mmu_v1();
	sync_mmu_v1();
	sync_mmu_v1();
}

	__asm__ __volatile__ ("sfence.vma" : : : "memory");
}

void flush_tlb_mm(struct mm_struct *mm)
{
if (c910_mmu_v1_flag) {
	int newpid = cpu_asid(mm);

	__asm__ __volatile__ ("sfence.vma zero, %0"
				:
				: "r"(newpid)
				: "memory");
} else {
	sync_mmu_v1();
	sync_mmu_v1();
	sync_mmu_v1();
	__asm__ __volatile__ ("sfence.vma" : : : "memory");
}
}

void flush_tlb_page(struct vm_area_struct *vma, unsigned long addr)
{
if (c910_mmu_v1_flag) {
	int newpid = cpu_asid(vma->vm_mm);

	addr &= PAGE_MASK;

	__asm__ __volatile__ ("sfence.vma %0, %1"
				:
				: "r"(addr), "r"(newpid)
				: "memory");

} else {
	addr &= PAGE_MASK;

	sync_mmu_v1();
	sync_mmu_v1();
	sync_mmu_v1();
	__asm__ __volatile__ ("sfence.vma %0"
				:
				: "r"(addr)
				: "memory");
}
}

void flush_tlb_range(struct vm_area_struct *vma, unsigned long start,
			unsigned long end)
{
	unsigned long newpid = cpu_asid(vma->vm_mm);

	start &= PAGE_MASK;
	end   += PAGE_SIZE - 1;
	end   &= PAGE_MASK;

if (c910_mmu_v1_flag) {
	while (start < end) {
		sync_mmu_v1();
		sync_mmu_v1();
		sync_mmu_v1();
		__asm__ __volatile__ ("sfence.vma %0"
					:
					: "r"(start)
					: "memory");
		start += PAGE_SIZE;
	}
} else {
	while (start < end) {
		__asm__ __volatile__ ("sfence.vma %0, %1"
					:
					: "r"(start), "r"(newpid)
					: "memory");
		start += PAGE_SIZE;
	}
}
}

static int __init c910_mmu_v1(char *str)
{
	c910_mmu_v1_flag = 1;
	return 0;
}
early_param("c910_mmu_v1", c910_mmu_v1);

#else
#include <asm/sbi.h>

void flush_tlb_all(void)
{
	sbi_remote_sfence_vma(NULL, 0, -1);
}

static void __sbi_tlb_flush_range(struct cpumask *cmask, unsigned long start,
				  unsigned long size)
{
	struct cpumask hmask;

	riscv_cpuid_to_hartid_mask(cmask, &hmask);
	sbi_remote_sfence_vma(hmask.bits, start, size);
}

void flush_tlb_mm(struct mm_struct *mm)
{
	__sbi_tlb_flush_range(mm_cpumask(mm), 0, -1);
}

void flush_tlb_page(struct vm_area_struct *vma, unsigned long addr)
{
	__sbi_tlb_flush_range(mm_cpumask(vma->vm_mm), addr, PAGE_SIZE);
}

void flush_tlb_range(struct vm_area_struct *vma, unsigned long start,
		     unsigned long end)
{
	__sbi_tlb_flush_range(mm_cpumask(vma->vm_mm), start, end - start);
}
#endif
