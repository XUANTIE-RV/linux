// SPDX-License-Identifier: GPL-2.0
/*
 * T-Head Xuantie IOMMU API implementations
 *
 * Copyright (C) 2022 Alibaba Group Holding Limited
 *
 * Author: maolijie <lijie.mlj@alibaba-inc.com>
 *         zhaosiqi <zhaosiqi.zsq@alibaba-inc.com>
 *
 */

#ifndef XUANTIE_IOMMU_H
#define XUANTIE_IOMMU_H

#include <asm/page.h>

/* Xuantie IOMMU RSID length config */
#define CONFIG_XT_IOMMU_RSIDLEN		12

/* SV39 */
#define CONFIG_RISCV64_VA_BITS		39
#define SV39_MODE			8
#define VA_BITS				CONFIG_RISCV64_VA_BITS
#define PAGE_TABLE_LEVEL_MASK		0x1ff
#define PGD_BITS 			(VA_BITS - PGDIR_SHIFT)

/* Page Table Entry */
#define PTE_V				BIT(0)
#define PTE_R 				BIT(1)
#define PTE_W 				BIT(2)
#define PTE_X				BIT(3)
#define PTE_U				BIT(4)
#define PTE_SIZE			8
#define PTE_PPN_SHIFT			10
#define SV39_PTE_PPN_MASK  		GENMASK_ULL(53, 10)

#define PAGE_4K				0x00001000
#define PAGE_2M				0x00200000
#define PAGE_1G				0x40000000
#define XT_IOMMU_PGSIZE_BITMAP		(PAGE_1G | PAGE_2M | PAGE_4K)

/* MMIO register */
#define IOMMUCAP_OFF			0x0
#define RSIDDIV_OFF			0x8
#define ERSIDDIV_OFF  			0x10
#define DTBASE_OFF    			0x18
#define FTVAL_OFF     			0x20
#define IOMMUCAPEN_OFF			0x1000
#define IOMMUINTEN_OFF			0x1008

/* IOMMUCAP bitfield : TBD */
#define IOMMUCAP_S			BIT(0)
#define IOMMUCAP_A			BIT(0)
#define IOMMUCAP_MRF			BIT(0)

/* IOMMUCAPEN bitfile */
#define IOMMUCAPEN_E			BIT(0)
#define IOMMUINTEN_E			BIT(0)
/*
 * Device Table Entry
 * +---------------------+-+-+-+
 * |     TD address      |F|S|V|
 * +---------------------+-+-+-+
 * Bit 63:3  - Translation Descriptor address (always starts on a 8 Byte boundary)
 * Bit    2  - F stand for the First stage
 * Bit    1  - S stand for the Second stage
 * Bit    0  - V 1 if TD address is valid
 */
#define DTE_VALID			BIT(0)
#define DTE_STAGE2			BIT(1)
#define DTE_STAGE1			BIT(2)
#define DTE_TD_ADDR_MASK		GENMASK_ULL(63, 3)
#define DTE_DWORDS			1

/* Device Table */
#define RSIDLEN 			CONFIG_XT_IOMMU_RSIDLEN
#define DEVTAB_L1_DESC_DWORDS 		1
#define DEVTAB_L1_DESC_L2PTR_MASK	GENMASK_ULL(63, 3)
#define DEVTAB_BASE_ADDR_MASK		GENMASK_ULL(63, 3)

/*
 * Translation Descriptor
 * Bit  63:  0 - Stage One Control
 * Bit 127: 64 - Stage Two Control
 * Bit 191:128 - Device Conguration
 *
 * Stage One Control Field in When S2MODE is Zero
 * +--------+------+-------+
 * | S1MODE | ASID | S1PPN |
 * +--------+------+-------+
 * Bit 43: 0 - S1PPN
 * Bit 59:40 - ASID
 * Bit 63:60 - S1MODE
 *
 */
#define TRANSDESC_TD_DWORDS		3

typedef u64 xt_iommu_iopte;

struct xt_iommu_stage1 {
#define STAGE1_PPN_SHIFT	      	0
#define STAGE1_PPN_MASK			0xfffffffffff
	u64				s1ppn;
	u64				*s1ppn_va;
#define STAGE1_ASID_SHIFT              	44
#define STAGE1_ASID_MASK               	0xffff
	u16				asid;
#define STAGE1_MODE_SHIFT              	60
#define STAGE1_MODE_MASK               	0xf
	u16				s1mode;
};

struct xt_iommu_td_cfg {
	u64				*tdtab;
	dma_addr_t			tdtab_dma;
};

struct xt_iommu_td {
	struct xt_iommu_stage1		stage1;
	struct xt_iommu_td_cfg		tdcfg;
};

struct xt_iommu_devtab_l1_desc {
	u64				*l2ptr;
	dma_addr_t			l2ptr_dma;
};

struct xt_iommu_devtab_cfg {
	u64				*devtab;
	dma_addr_t			devtab_dma;
	u32				num_l1_ents;
	struct xt_iommu_devtab_l1_desc 	*l1_desc;
};

/* Xuantie IOMMU instance */
struct xt_iommu_device {
	struct device 			*dev;
	void 				*base;
#define FEAT_2_LVL_DEVTAB_SHIFT		0
#define FEAT_2_LVL_DEVTAB		BIT(0)
	u32 				features;
	int 				num_irq;
	int 				irq;
	struct iommu_device 		iommu;
	struct xt_iommu_devtab_cfg	devtab_cfg;
};

struct xt_iommu_domain {
	struct xt_iommu_device 		*iommu;
	spinlock_t 			devices_lock;	/* lock for iommu's devices list */
	struct mutex			pgt_mutex;	/* Protects page directory tagble */
	struct mutex			init_mutex;	/* Protects xmmu */
	struct list_head 		devices;
	struct iommu_domain 		domain;
	struct xt_iommu_td 		td;
};

struct xt_iommu_master {
	struct device 			*dev;
	struct xt_iommu_device		*iommu;
	struct xt_iommu_domain		*domain;
	struct list_head 		domain_head;
	u32				rsid;
};

#endif
