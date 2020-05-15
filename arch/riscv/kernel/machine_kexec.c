/*
 * kexec for riscv
 *
 * Copyright (C) 2020-2025 Alibaba Group Holding Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/kexec.h>
#include <linux/page-flags.h>
#include <linux/smp.h>

#include <asm/cacheflush.h>
#include <asm/mmu.h>
#include <asm/mmu_context.h>
#include <asm/page.h>

#include "cpu-reset.h"

/* Global variables for the riscv_relocate_new_kernel routine. */
extern const unsigned char riscv_relocate_new_kernel[];
extern const unsigned long riscv_relocate_new_kernel_size;

/*
 * kexec_image_info - For debugging output.
 */
#define kexec_image_info(_i) _kexec_image_info(__func__, __LINE__, _i)
static void _kexec_image_info(const char *func, int line,
	const struct kimage *kimage)
{
	unsigned long i;

	pr_debug("%s:%d:\n", func, line);
	pr_debug("  kexec kimage info:\n");
	pr_debug("    type:        %d\n", kimage->type);
	pr_debug("    start:       0x%lx\n", kimage->start);
	pr_debug("    head:        0x%lx\n", kimage->head);
	pr_debug("    nr_segments: %lu\n", kimage->nr_segments);

	for (i = 0; i < kimage->nr_segments; i++) {
		pr_debug("      segment[%lu]: %016lx - %016lx, 0x%lx bytes, %lu pages\n",
			i,
			kimage->segment[i].mem,
			kimage->segment[i].mem + kimage->segment[i].memsz,
			kimage->segment[i].memsz,
			kimage->segment[i].memsz /  PAGE_SIZE);
	}
}

void machine_kexec_cleanup(struct kimage *kimage)
{
	/* Empty routine needed to avoid build errors. */
}

/**
 * machine_kexec_prepare - Prepare for a kexec reboot.
 *
 * Called from the core kexec code when a kernel image is loaded.
 * Forbid loading a kexec kernel if we have no way of hotplugging cpus or cpus
 * are stuck in the kernel. This avoids a panic once we hit machine_kexec().
 */
int machine_kexec_prepare(struct kimage *kimage)
{
	kexec_image_info(kimage);

	return 0;
}

/**
 * machine_kexec - Do the kexec reboot.
 *
 * Called from the core kexec code for a sys_reboot with LINUX_REBOOT_CMD_KEXEC.
 */
void machine_kexec(struct kimage *kimage)
{
	phys_addr_t reboot_code_buffer_phys;
	void *reboot_code_buffer;

	reboot_code_buffer_phys = page_to_phys(kimage->control_code_page);
	reboot_code_buffer = phys_to_virt(reboot_code_buffer_phys);

	pr_debug("%s:%d: control_code_page:        0x%lx\n", __func__, __LINE__,
		(long)kimage->control_code_page);
	pr_debug("%s:%d: reboot_code_buffer_phys:  0x%lx\n", __func__, __LINE__,
		(long)reboot_code_buffer_phys);
	pr_debug("%s:%d: reboot_code_buffer:       0x%lx\n", __func__, __LINE__,
		(long)reboot_code_buffer);
	pr_debug("%s:%d: relocate_new_kernel:      0x%lx\n", __func__, __LINE__,
		(long)riscv_relocate_new_kernel);
	pr_debug("%s:%d: relocate_new_kernel_size: 0x%lx(%lu) bytes\n",
		__func__, __LINE__, riscv_relocate_new_kernel_size,
		riscv_relocate_new_kernel_size);

	/*
	 * Copy riscv_relocate_new_kernel to the reboot_code_buffer for use
	 * after the kernel is shut down.
	 */
	memcpy(reboot_code_buffer, riscv_relocate_new_kernel,
		riscv_relocate_new_kernel_size);

	pr_info("Bye!\n");

	local_irq_disable();

	cpu_soft_restart(reboot_code_buffer_phys, kimage->head,
				kimage->segment[0].mem, kimage->segment[2].mem);

	BUG(); /* Should never get here. */
}

static void machine_kexec_mask_interrupts(void)
{
	unsigned int i;
	struct irq_desc *desc;

	for_each_irq_desc(i, desc) {
		struct irq_chip *chip;
		int ret;

		chip = irq_desc_get_chip(desc);
		if (!chip)
			continue;

		/*
		 * First try to remove the active state. If this
		 * fails, try to EOI the interrupt.
		 */
		ret = irq_set_irqchip_state(i, IRQCHIP_STATE_ACTIVE, false);

		if (ret && irqd_irq_inprogress(&desc->irq_data) &&
		    chip->irq_eoi)
			chip->irq_eoi(&desc->irq_data);

		if (chip->irq_mask)
			chip->irq_mask(&desc->irq_data);

		if (chip->irq_disable && !irqd_irq_disabled(&desc->irq_data))
			chip->irq_disable(&desc->irq_data);
	}
}

/**
 * machine_crash_shutdown - shutdown non-crashing cpus and save registers
 */
extern void crash_smp_send_stop(void);
void machine_crash_shutdown(struct pt_regs *regs)
{
	local_irq_disable();

	/* shutdown non-crashing cpus */
	crash_smp_send_stop();

	/* for crashing cpu */
	crash_save_cpu(regs, smp_processor_id());
	machine_kexec_mask_interrupts();

	pr_info("Starting crashdump kernel...\n");
}
