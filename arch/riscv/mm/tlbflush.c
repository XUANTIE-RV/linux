// SPDX-License-Identifier: GPL-2.0

#include <linux/of.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/sched.h>
#include <asm/sbi.h>
#include <asm/mmu_context.h>

static u32 xuantie_flush_tlb_range_flag = 0;

static inline void local_flush_tlb_all_asid(unsigned long asid)
{
	__asm__ __volatile__ ("sfence.vma x0, %0"
			:
			: "r" (asid)
			: "memory");
}

static inline void local_flush_tlb_page_asid(unsigned long addr,
		unsigned long asid)
{
	__asm__ __volatile__ ("sfence.vma %0, %1"
			:
			: "r" (addr), "r" (asid)
			: "memory");
}

static inline void local_flush_tlb_range(unsigned long start,
		unsigned long size, unsigned long stride)
{
	if (size <= stride)
		local_flush_tlb_page(start);
	else
		local_flush_tlb_all();
}

static inline void local_flush_tlb_range_asid(unsigned long start,
		unsigned long size, unsigned long stride, unsigned long asid)
{
	if (size <= stride)
		local_flush_tlb_page_asid(start, asid);
	else
		local_flush_tlb_all_asid(asid);
}

static void __ipi_flush_tlb_all(void *info)
{
	local_flush_tlb_all();
}

void flush_tlb_all(void)
{
	if (xuantie_flush_tlb_range_flag) {
		__asm__ __volatile__ ("sfence.vma" : : : "memory");
	} else {
		if (riscv_use_ipi_for_rfence())
			on_each_cpu(__ipi_flush_tlb_all, NULL, 1);
		else
			sbi_remote_sfence_vma(NULL, 0, -1);
	}
}

struct flush_tlb_range_data {
	unsigned long asid;
	unsigned long start;
	unsigned long size;
	unsigned long stride;
};

static void __ipi_flush_tlb_range_asid(void *info)
{
	struct flush_tlb_range_data *d = info;

	local_flush_tlb_range_asid(d->start, d->size, d->stride, d->asid);
}

static void __ipi_flush_tlb_range(void *info)
{
	struct flush_tlb_range_data *d = info;

	local_flush_tlb_range(d->start, d->size, d->stride);
}

static void __flush_tlb_range(struct mm_struct *mm, unsigned long start,
			      unsigned long size, unsigned long stride)
{
	struct flush_tlb_range_data ftd;
	struct cpumask *cmask = mm_cpumask(mm);
	unsigned int cpuid;
	bool broadcast;

	if (cpumask_empty(cmask))
		return;

	cpuid = get_cpu();
	/* check if the tlbflush needs to be sent to other CPUs */
	broadcast = cpumask_any_but(cmask, cpuid) < nr_cpu_ids;
	if (static_branch_unlikely(&use_asid_allocator)) {
		unsigned long asid = atomic_long_read(&mm->context.id) & asid_mask;

		if (broadcast) {
			if (riscv_use_ipi_for_rfence()) {
				ftd.asid = asid;
				ftd.start = start;
				ftd.size = size;
				ftd.stride = stride;
				on_each_cpu_mask(cmask,
						 __ipi_flush_tlb_range_asid,
						 &ftd, 1);
			} else
				sbi_remote_sfence_vma_asid(cmask,
							   start, size, asid);
		} else {
			local_flush_tlb_range_asid(start, size, stride, asid);
		}
	} else {
		if (broadcast) {
			if (riscv_use_ipi_for_rfence()) {
				ftd.asid = 0;
				ftd.start = start;
				ftd.size = size;
				ftd.stride = stride;
				on_each_cpu_mask(cmask,
						 __ipi_flush_tlb_range,
						 &ftd, 1);
			} else
				sbi_remote_sfence_vma(cmask, start, size);
		} else {
			local_flush_tlb_range(start, size, stride);
		}
	}

	put_cpu();
}

void flush_tlb_mm(struct mm_struct *mm)
{
	if (xuantie_flush_tlb_range_flag) {
		unsigned long asid = atomic_long_read(&mm->context.id) & asid_mask;
		__asm__ __volatile__ ("sfence.vma zero, %0"
					:
					: "r"(asid)
					: "memory");
	} else {
		__flush_tlb_range(mm, 0, -1, PAGE_SIZE);
	}
}

void flush_tlb_page(struct vm_area_struct *vma, unsigned long addr)
{
	if (xuantie_flush_tlb_range_flag) {
		unsigned long asid = atomic_long_read(&vma->vm_mm->context.id) & asid_mask;
		addr &= PAGE_MASK;
		__asm__ __volatile__ ("sfence.vma %0, %1"
					:
					: "r"(addr), "r"(asid)
					: "memory");
	} else {
		__flush_tlb_range(vma->vm_mm, addr, PAGE_SIZE, PAGE_SIZE);
	}
}

void flush_tlb_range(struct vm_area_struct *vma, unsigned long start,
		     unsigned long end)
{
	if (xuantie_flush_tlb_range_flag) {
		unsigned long asid = atomic_long_read(&vma->vm_mm->context.id) & asid_mask;
		start &= PAGE_MASK;
		end   += PAGE_SIZE - 1;
		end   &= PAGE_MASK;
		while (start < end) {
			__asm__ __volatile__ ("sfence.vma %0, %1"
						:
						: "r"(start), "r"(asid)
						: "memory");
			start += PAGE_SIZE;
		}
	} else {
		__flush_tlb_range(vma->vm_mm, start, end - start, PAGE_SIZE);
	}
}
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
void flush_pmd_tlb_range(struct vm_area_struct *vma, unsigned long start,
			unsigned long end)
{
	__flush_tlb_range(vma->vm_mm, start, end - start, PMD_SIZE);
}
#endif

static int __init xuantie_flush_tlb_init(void)
{
	struct device_node *cpu = of_find_node_by_path("/cpus");
	if (cpu) {
		of_property_read_u32(cpu, "flush-tlb-range", &xuantie_flush_tlb_range_flag);
		of_node_put(cpu);
	}

	return 0;
}
arch_initcall(xuantie_flush_tlb_init);
