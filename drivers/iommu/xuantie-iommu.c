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

#include <linux/init.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/iommu.h>
#include <linux/dma-iommu.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/of_platform.h>
#include <linux/module.h>

#include "xuantie-iommu.h"

u32 rsid_split = 0;
struct xt_iommu_device *xt_iommu;
static struct iommu_ops xt_iommu_ops;

static inline struct xt_iommu_domain *to_xt_iommu_domain(struct iommu_domain *domain)
{
	return container_of(domain, struct xt_iommu_domain, domain);
}

static phys_addr_t xt_iommu_iova_to_phys(struct iommu_domain *domain,
					 dma_addr_t iova)
{
	u64 *page_table1, *page_table2;
	u32 vpn2_idx, vpn1_idx, vpn0_idx;
	phys_addr_t pa = 0;
	struct xt_iommu_domain *xt_domain = to_xt_iommu_domain(domain);
	struct xt_iommu_td *td = &xt_domain->td;
	u64 *page_table0 = td->stage1.s1ppn_va;

	vpn2_idx = (iova >> PGDIR_SHIFT) & PAGE_TABLE_LEVEL_MASK;
	vpn1_idx = (iova >> PMD_SHIFT) & PAGE_TABLE_LEVEL_MASK;
	vpn0_idx = (iova >> PAGE_SHIFT) & PAGE_TABLE_LEVEL_MASK;

	/* get pagetable virtual address and return pa */
	page_table1 = phys_to_virt((*(page_table0 + vpn2_idx) >> PTE_PPN_SHIFT) << PAGE_SHIFT);
	page_table2 = phys_to_virt((*(page_table1 + vpn1_idx) >> PTE_PPN_SHIFT) << PAGE_SHIFT);
	pa = ((page_table2[vpn0_idx] >> PTE_PPN_SHIFT) << PAGE_SHIFT) | (iova & 0xfff);

	return pa;
}

static xt_iommu_iopte xt_iommu_prot_to_pte(int prot)
{
	xt_iommu_iopte pte = PTE_V;

	if (!(prot & IOMMU_WRITE) && (prot & IOMMU_READ))
		pte |= PTE_R;
	if (prot & IOMMU_WRITE)
		pte |= PTE_W | PTE_R;

	return pte;
}

static int xt_iommu_map(struct iommu_domain *domain, unsigned long iova,
			phys_addr_t paddr, size_t size, int iommu_prot, gfp_t gfp)
{
	u64 *page_table1, *page_table2;
	dma_addr_t page_table1_dma, page_table2_dma;
	u32 vpn2_idx, vpn1_idx, vpn0_idx, i;
	u32 loop = size / PAGE_SIZE;
	struct xt_iommu_domain *xt_domain = to_xt_iommu_domain(domain);
	struct xt_iommu_td *td = &xt_domain->td;
	u64 *page_table0 = td->stage1.s1ppn_va;
	xt_iommu_iopte prot;

	prot = xt_iommu_prot_to_pte(iommu_prot);

	if (size % PAGE_SIZE) {
		printk("error: size is not PAGE_SIZE aligned\n");
		return -EINVAL;
	}

	for (i = 0; i < loop; ++i) {
		vpn2_idx = (iova >> PGDIR_SHIFT) & PAGE_TABLE_LEVEL_MASK;
		vpn1_idx = (iova >> PMD_SHIFT) & PAGE_TABLE_LEVEL_MASK;
		vpn0_idx = (iova >> PAGE_SHIFT) & PAGE_TABLE_LEVEL_MASK;
		if (*(page_table0 + vpn2_idx) == 0) {
			page_table1 = dmam_alloc_coherent(xt_domain->iommu->dev, PAGE_SIZE,
							  &page_table1_dma, GFP_KERNEL);
			page_table2 = dmam_alloc_coherent(xt_domain->iommu->dev, PAGE_SIZE,
							  &page_table2_dma, GFP_KERNEL);
			page_table0[vpn2_idx] = (page_table1_dma >> PAGE_SHIFT <<
						 PTE_PPN_SHIFT) | PTE_V;
			page_table1[vpn1_idx] = (page_table2_dma >> PAGE_SHIFT <<
						 PTE_PPN_SHIFT) | PTE_V;
		} else {
			page_table1 = phys_to_virt((*(page_table0 + vpn2_idx)
						    >> PTE_PPN_SHIFT) << PAGE_SHIFT);
			if (page_table1[vpn1_idx] == 0) {
				page_table2 = dmam_alloc_coherent(xt_domain->iommu->dev,
								  PAGE_SIZE,
								  &page_table2_dma,
								  GFP_KERNEL);
				page_table1[vpn1_idx] = (page_table2_dma >> PAGE_SHIFT <<
							 PTE_PPN_SHIFT) | PTE_V;
			} else {
				page_table2 = phys_to_virt((page_table1[vpn1_idx] >>
							    PTE_PPN_SHIFT) << PAGE_SHIFT);
			}
		}
		page_table2[vpn0_idx]= ((paddr >> PAGE_SHIFT) << PTE_PPN_SHIFT) | prot;
		iova += PAGE_SIZE;
		paddr += PAGE_SIZE;
	}

	return 0;
}

static size_t xt_iommu_unmap(struct iommu_domain *domain, unsigned long iova,
			     size_t size, struct iommu_iotlb_gather *gather)
{
	u64 *page_table1, *page_table2;
	u32 vpn2_idx, vpn1_idx, vpn0_idx, i;
	u32 loop = size / PAGE_SIZE;
	struct xt_iommu_domain *xt_domain = to_xt_iommu_domain(domain);
	struct xt_iommu_td *td = &xt_domain->td;
	u64 *page_table0 = td->stage1.s1ppn_va;

	if (size % PAGE_SIZE) {
		printk("error: size is not PAGE_SIZE aligned\n");
		return -EINVAL;
	}

	for (i = 0; i < loop; ++i) {
		vpn2_idx = (iova >> PGDIR_SHIFT) & PAGE_TABLE_LEVEL_MASK;
		vpn1_idx = (iova >> PMD_SHIFT) & PAGE_TABLE_LEVEL_MASK;
		vpn0_idx = (iova >> PAGE_SHIFT) & PAGE_TABLE_LEVEL_MASK;

		/* get pagetable virtual address and invalid */
		page_table1 = phys_to_virt((*(page_table0 + vpn2_idx) >>
					    PTE_PPN_SHIFT) << PAGE_SHIFT);
		page_table2 = phys_to_virt((page_table1[vpn1_idx] >>
					    PTE_PPN_SHIFT) << PAGE_SHIFT);

		page_table2[vpn0_idx] = 0;
		iova += PAGE_SIZE;
	}

	return size;
}

static u64 *xt_iommu_get_dtep_for_rsid(struct xt_iommu_device *iommu, u32 rsid)
{
	u64 *dtep;
	struct xt_iommu_devtab_cfg *cfg = &iommu->devtab_cfg;

	if (iommu->features & FEAT_2_LVL_DEVTAB) {
		struct xt_iommu_devtab_l1_desc *l1_desc;
		int idx;

		idx = rsid >> rsid_split;
		l1_desc = &cfg->l1_desc[idx];

		idx = rsid & ((1 << rsid_split) -1);
		dtep = &l1_desc->l2ptr[idx];
	} else {
		/* linear lookup */
		dtep = &cfg->devtab[rsid * DTE_DWORDS];
	}

	return dtep;
}

static void xt_iommu_write_devtab_ent(struct xt_iommu_master *master, u32 rsid, u64 *dst)
{
	u64 val = 0;
	struct xt_iommu_td *td;
	struct xt_iommu_domain *xt_domain;

	if (!master) {
		/* by pass */
		dst[0] = 0;
		return;
	}

	xt_domain = master->domain;
	if (!xt_domain) {
		/* detach dev */
		dst[0] = 0;
		return;
	}

	td = &xt_domain->td;
	val |= DTE_STAGE1 | DTE_VALID;
	val |= td->tdcfg.tdtab_dma & DTE_TD_ADDR_MASK;
	dst[0] = val;
}

static void xt_iommu_install_dte_for_dev(struct xt_iommu_master *master)
{
	struct xt_iommu_device *iommu = xt_iommu;
	u64 *dtep = xt_iommu_get_dtep_for_rsid(iommu, master->rsid);

	xt_iommu_write_devtab_ent(master, master->rsid, dtep);
}

static void xt_iommu_detach_device(struct iommu_domain *domain,
				   struct device *dev)
{
	unsigned long flags;
	struct xt_iommu_master *master;
	struct xt_iommu_domain *xt_iommu_domain = to_xt_iommu_domain(domain);

	master = dev_iommu_priv_get(dev);
	master->domain = NULL;
	/* clear DTE */
	xt_iommu_install_dte_for_dev(master);

	spin_lock_irqsave(&xt_iommu_domain->devices_lock, flags);
	list_del(&master->domain_head);
	spin_unlock_irqrestore(&xt_iommu_domain->devices_lock, flags);
}

static void xt_iommu_free_td_tables(struct xt_iommu_domain *xt_domain)
{
	size_t size;
	size_t td_ents = 1;
	struct xt_iommu_td *td = &xt_domain->td;
	struct xt_iommu_td_cfg *tdcfg = &td->tdcfg;

	size = td_ents * (TRANSDESC_TD_DWORDS << 3);
	dmam_free_coherent(xt_domain->iommu->dev, size,
			   tdcfg->tdtab, tdcfg->tdtab_dma);
	tdcfg->tdtab = NULL;
	tdcfg->tdtab_dma = 0;
}

static int xt_iommu_alloc_td_tables(struct xt_iommu_domain *xt_domain)
{
	size_t size;
	size_t td_ents = 1;
	struct xt_iommu_td *td = &xt_domain->td;
	struct xt_iommu_td_cfg *tdcfg = &td->tdcfg;

	size = td_ents * (TRANSDESC_TD_DWORDS << 3);

	tdcfg->tdtab = dmam_alloc_coherent(xt_domain->iommu->dev, size,
					   &tdcfg->tdtab_dma, GFP_KERNEL);

	return 0;
}

int xt_iommu_write_td(struct xt_iommu_domain *xt_domain, int srsid,
		      struct xt_iommu_stage1 *td)
{
	u64 val = 0;
	u64 *tdtab = xt_domain->td.tdcfg.tdtab;

	if (!tdtab)
		return -ENOMEM;

	val |= (u64)(td->s1mode & STAGE1_MODE_MASK) << STAGE1_MODE_SHIFT;
	val |= (u64)(td->asid & STAGE1_ASID_MASK) << STAGE1_ASID_SHIFT;
	val |= (u64)(td->s1ppn & STAGE1_PPN_MASK) << STAGE1_PPN_SHIFT;
	tdtab[0] = val;
	tdtab[1] = 0; /*No stage2 Configuration */
	tdtab[2] = 0; /*No Device Configuration */

	return 0;
}

static int xt_iommu_domain_td_s1(struct xt_iommu_domain *xt_domain)
{
	int ret;
	u64 *page_table;
	dma_addr_t page_table_dma;
	u32 asid = 0;
	struct xt_iommu_td *td = &xt_domain->td;

	if (!xt_domain->iommu) {
		/* domain free */
		xt_iommu_free_td_tables(xt_domain);
		page_table = td->stage1.s1ppn_va;
		page_table_dma = td->stage1.s1ppn << PAGE_SHIFT;
		dmam_free_coherent(xt_domain->iommu->dev,
				   PTE_SIZE << PGD_BITS,
				   page_table, page_table_dma);
		return 0;
	}

	ret = xt_iommu_alloc_td_tables(xt_domain);
	if (ret)
		return ret;

	page_table = (u64 *)dmam_alloc_coherent(xt_domain->iommu->dev,
						PTE_SIZE << PGD_BITS,
						&page_table_dma, GFP_KERNEL);
	if (!page_table)
		return -1;

	td->stage1.asid	= asid;
	td->stage1.s1ppn = page_table_dma >> PAGE_SHIFT;
	td->stage1.s1ppn_va = page_table;
	td->stage1.s1mode = SV39_MODE;

	ret = xt_iommu_write_td(xt_domain, 0, &td->stage1);

	return 0;
}

static int xt_iommu_attach_device(struct iommu_domain *domain,
				  struct device *dev)
{
	unsigned long flags;
	struct xt_iommu_master *master;
	struct xt_iommu_domain *xt_domain = to_xt_iommu_domain(domain);

	dev_dbg(dev, "Attaching to iommu domain\n");

	master = dev_iommu_priv_get(dev);
	master->domain = xt_domain;

	mutex_lock(&xt_domain->init_mutex);
	/* set DTE */
	xt_iommu_install_dte_for_dev(master);

	spin_lock_irqsave(&xt_domain->devices_lock, flags);
	list_add(&master->domain_head, &xt_domain->devices);
	spin_unlock_irqrestore(&xt_domain->devices_lock, flags);

	mutex_unlock(&xt_domain->init_mutex);

	return 0;
}

static struct iommu_domain *xt_iommu_domain_alloc(unsigned type)
{
	struct xt_iommu_domain *xt_domain;

	if (type != IOMMU_DOMAIN_UNMANAGED && type != IOMMU_DOMAIN_DMA)
		return NULL;

	xt_domain = kzalloc(sizeof(*xt_domain), GFP_KERNEL);
	if (!xt_domain)
		return NULL;

	if (type == IOMMU_DOMAIN_DMA &&
	    iommu_get_dma_cookie(&xt_domain->domain)) {
		kfree(xt_domain);
		return NULL;
	}

	/* alloc translation descriptor */
	xt_domain->iommu = xt_iommu;
	xt_iommu_domain_td_s1(xt_domain);

	mutex_init(&xt_domain->init_mutex);
	mutex_init(&xt_domain->pgt_mutex);
	INIT_LIST_HEAD(&xt_domain->devices);
	spin_lock_init(&xt_domain->devices_lock);

	xt_domain->domain.geometry.aperture_start = 0;
	xt_domain->domain.geometry.aperture_end = DMA_BIT_MASK(32);
	xt_domain->domain.geometry.force_aperture = true;

	return &xt_domain->domain;
}

static void xt_iommu_init_bypass_dtes(u64 *devtab, unsigned int nent)
{
	unsigned int i;

	for (i = 0; i < nent; ++i) {
		xt_iommu_write_devtab_ent(NULL, -1, devtab);
		devtab += DTE_DWORDS;
	}
}

static void xt_iommu_write_devtab_l1_desc(u64 *dst, struct xt_iommu_devtab_l1_desc *desc)
{
	u64 val = 0;

	val |= desc->l2ptr_dma & DEVTAB_L1_DESC_L2PTR_MASK;

	*(u64 *)dst = val;
}

static int xt_iommu_init_l2_devtab(struct xt_iommu_device *iommu, u32 rsid)
{
	size_t size;
	void *devtab;
	struct xt_iommu_devtab_cfg *cfg = &iommu->devtab_cfg;
	struct xt_iommu_devtab_l1_desc *desc = &cfg->l1_desc[rsid >> rsid_split];

	if (desc->l2ptr)
		return 0;

	size = 1 << (rsid_split + ilog2(DTE_DWORDS) + 3);
	devtab = &cfg->devtab[(rsid >> rsid_split) * DEVTAB_L1_DESC_DWORDS];

	desc->l2ptr = dmam_alloc_coherent(iommu->dev, size, &desc->l2ptr_dma,
					  GFP_KERNEL);

	if (!desc->l2ptr) {
		printk("failed to allocate l2 device table for RSID\n");
		return -ENOMEM;
	}

	xt_iommu_init_bypass_dtes(desc->l2ptr, 1 << rsid_split);
	xt_iommu_write_devtab_l1_desc(devtab, desc);

	return 0;
}

static struct iommu_device *xt_iommu_probe_device(struct device *dev)
{
	int ret;
	u32 rsid;
	struct xt_iommu_device *iommu;
	struct xt_iommu_master *master;

	iommu = xt_iommu;
	master = kzalloc(sizeof(*master), GFP_KERNEL);
	if (!master)
		return ERR_PTR(-ENOMEM);

	master->dev = dev;
	master->iommu = iommu;
	dev_iommu_priv_set(dev, master);

	if (dev_is_pci(dev)) {
		struct pci_dev *pdev = to_pci_dev(dev);
		master->rsid = pci_dev_id(pdev);
	} /* else { platform device from dts } */

	rsid = master->rsid;
	if (iommu->features & FEAT_2_LVL_DEVTAB) {
		ret = xt_iommu_init_l2_devtab(iommu, rsid);
	}

	return &iommu->iommu;
}

static int xt_iommu_of_xlate(struct device *dev,
			     struct of_phandle_args *args)
{
	return 0;
}

static void xt_iommu_domain_free(struct iommu_domain *domain)
{
	struct xt_iommu_domain *xt_domain = to_xt_iommu_domain(domain);
	iommu_put_dma_cookie(domain);

	/* free translation descriptor */
	xt_domain->iommu = NULL;
	xt_iommu_domain_td_s1(xt_domain);
	xt_domain->domain.geometry.aperture_start = 0;
	xt_domain->domain.geometry.aperture_end   = 0;
	xt_domain->domain.geometry.force_aperture = false;

}

static void xt_iommu_probe_finalize(struct device *dev)
{
	iommu_setup_dma_ops(dev, 1U << 12, (0xFFFFU << 12) - (1U << 12));
}

static struct iommu_group *xt_iommu_device_group(struct device *dev)
{
	struct iommu_group *group;

	if (dev_is_pci(dev))
		group = pci_device_group(dev);
	else
		group = generic_device_group(dev);

	return group;
}

static struct iommu_ops xt_iommu_ops = {
	.domain_alloc = xt_iommu_domain_alloc,
	.domain_free = xt_iommu_domain_free,
	.attach_dev = xt_iommu_attach_device,
	.detach_dev = xt_iommu_detach_device,
	.map = xt_iommu_map,
	.unmap = xt_iommu_unmap,
	.probe_device = xt_iommu_probe_device,
	.iova_to_phys = xt_iommu_iova_to_phys,
	.probe_finalize = xt_iommu_probe_finalize,
	.device_group = xt_iommu_device_group,
	.pgsize_bitmap = XT_IOMMU_PGSIZE_BITMAP,
	.of_xlate = xt_iommu_of_xlate
};

static irqreturn_t xt_iommu_int_handler(int irq, void *dev)
{
	printk("enter xt_iommu_int_handler \n");

	return IRQ_HANDLED;
}

static int xt_iommu_device_dt_probe(struct platform_device *pdev,
				    struct xt_iommu_device *iommu)
{
	int ret = 0;

	return ret;
}

static int xt_iommu_device_hw_probe(struct xt_iommu_device *iommu)
{
	u64 rsiddiv;
	bool level2devtab;

	rsiddiv = *(u64 *)(iommu->base + RSIDDIV_OFF);
	level2devtab =  !!rsiddiv;
	iommu->features |= level2devtab << FEAT_2_LVL_DEVTAB_SHIFT;
	rsid_split = rsiddiv;

	return 0;
}

static int xt_iommu_init_devtab_linear(struct xt_iommu_device *iommu)
{
	void *devtab;
	size_t size;
	struct xt_iommu_devtab_cfg *cfg = &iommu->devtab_cfg;

	size = (1 << RSIDLEN) * (DTE_DWORDS << 3);
	devtab = dmam_alloc_coherent(iommu->dev, size, &cfg->devtab_dma,
				     GFP_KERNEL);
	if (!devtab) {
		printk("failed to allocate linear device table\n");
		return -ENOMEM;
	}
	cfg->devtab = devtab;
	xt_iommu_init_bypass_dtes(devtab, 1 << RSIDLEN);

	return 0;
}

static int xt_iommu_init_l1_devtab(struct xt_iommu_device *iommu)
{
	unsigned int i;
	struct xt_iommu_devtab_cfg *cfg = &iommu->devtab_cfg;
	size_t size = sizeof(*cfg->l1_desc) * cfg->num_l1_ents;
	u64 *devtab = iommu->devtab_cfg.devtab;

	cfg->l1_desc = devm_kzalloc(iommu->dev, size, GFP_KERNEL);
	if (!cfg->l1_desc) {
		printk("failed to allocate l1 device table desc\n");
		return -ENOMEM;
	}

	for (i = 0; i < cfg->num_l1_ents; ++i) {
		xt_iommu_write_devtab_l1_desc(devtab, &cfg->l1_desc[i]);
		devtab += DEVTAB_L1_DESC_DWORDS;
	}

	return 0;
}

static int xt_iommu_init_devtab_2lvl(struct xt_iommu_device *iommu)
{
	void *devtab;
	u32 size;
	size_t l1size;
	struct xt_iommu_devtab_cfg *cfg = &iommu->devtab_cfg;

	size = RSIDLEN - rsid_split;
	cfg->num_l1_ents = 1 << size;

	l1size = cfg->num_l1_ents * (DEVTAB_L1_DESC_DWORDS << 3);
	devtab = dmam_alloc_coherent(iommu->dev, l1size, &cfg->devtab_dma,
				     GFP_KERNEL);
	if (!devtab) {
		printk("failed to allocate l1 device table\n");
		return -ENOMEM;
	}
	cfg->devtab = devtab;

	return xt_iommu_init_l1_devtab(iommu);
}

static int xt_iommu_init_devtab(struct xt_iommu_device *iommu)
{
	int ret;

	if (iommu->features & FEAT_2_LVL_DEVTAB)
		ret = xt_iommu_init_devtab_2lvl(iommu);
	else
		ret = xt_iommu_init_devtab_linear(iommu);

	if (ret)
		return ret;

	/* for some other settings.... if need */

	return 0;
}

static int xt_iommu_init_structures(struct xt_iommu_device *iommu)
{
	/* for something init... */

	/* now device table */
	return xt_iommu_init_devtab(iommu);
}

static int xt_iommu_device_enable(struct xt_iommu_device *iommu)
{
	u64 iommucapen, iommuinten = 0 ;

	/* IOMMU base address */
	*(u64 *)(iommu->base + DTBASE_OFF) = (iommu->devtab_cfg.devtab_dma &
					      DEVTAB_BASE_ADDR_MASK);
	/* IOMMU enable bit */
	iommucapen = *(u64 *)(iommu->base + IOMMUCAPEN_OFF);
	iommucapen |= IOMMUCAPEN_E;
	*(u64 *)(iommu->base + IOMMUCAPEN_OFF) = iommucapen;

	/* IOMMU interrupt enable */
	iommuinten |= IOMMUINTEN_E;
	*(u64 *)(iommu->base + IOMMUINTEN_OFF) = iommuinten;

	return 0;
}

static int xt_iommu_device_probe(struct platform_device *pdev)
{
	int irq, ret;
	struct device *dev = &pdev->dev;
	struct resource *res;
	resource_size_t ioaddr;

	printk("Xuantie IOMMU initialization begins.\n");

	xt_iommu = devm_kzalloc(dev, sizeof(*xt_iommu), GFP_KERNEL);
	if (!xt_iommu) {
		printk("failed to allocate xuantie iommu device\n");
		return -ENOMEM;
	}
	xt_iommu->dev = dev;

	/* probe DT */
	if (dev->of_node) {
		xt_iommu_device_dt_probe(pdev, xt_iommu);
	} /* else from .. */

	/* MMIO registers base address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		printk("failed to get resource\n");
		return -EINVAL;
	}
	ioaddr = res->start;
	xt_iommu->base = ioremap(ioaddr, res->end - res->start);

	/* irq init */
	irq =  platform_get_irq_byname_optional(pdev, "xmmu");
	if (irq > 0) {
		xt_iommu->irq = irq;
		ret = devm_request_irq(&pdev->dev, irq, xt_iommu_int_handler,
				       0, "xuantie-iommu-irq", NULL);
		if (ret < 0)
			printk("failed to enable xmmu irq\n");
	}

	/* Probe the h/w */
	ret = xt_iommu_device_hw_probe(xt_iommu);
	if (ret)
		return ret;

	/* Initialise in-memory data structures */
	ret = xt_iommu_init_structures(xt_iommu);
	if (ret)
		return ret;

	/* sys file */
	ret = iommu_device_sysfs_add(&xt_iommu->iommu, dev, NULL, "xmmu.%pa", &ioaddr);
	if (ret)
		return ret;

	/* iommu ops, fwnode */
	iommu_device_set_ops(&xt_iommu->iommu, &xt_iommu_ops);
	iommu_device_set_fwnode(&xt_iommu->iommu, dev->fwnode);

	/* add current iommu to iommu device list */
	ret = iommu_device_register(&xt_iommu->iommu);
	if (ret)
		return ret;

	/* only supprt pci bus */
	ret = bus_set_iommu(&pci_bus_type, &xt_iommu_ops);
	if (ret)
		return ret;

	/* enable iommu */
	xt_iommu_device_enable(xt_iommu);

	printk("Xuantie IOMMU initialization ends.\n");

	return 0;
}

static const struct of_device_id xt_iommu_of_match[] = {
	{ .compatible = "xuantie-iommu", },
	{ },
};
MODULE_DEVICE_TABLE(of, xt_iommu_of_match);

static void xt_iommu_driver_unregister(struct platform_driver *drv)
{
	return;
}

static struct platform_driver xt_iommu_driver = {
	.driver	= {
		.name			= "xuantie-iommu",
		.of_match_table		= xt_iommu_of_match,
		.suppress_bind_attrs	= true,
	},
	.probe	= xt_iommu_device_probe,
};
module_driver(xt_iommu_driver, platform_driver_register,
	      xt_iommu_driver_unregister);

MODULE_DESCRIPTION("T-Head Xuantie IOMMU API implementations");
MODULE_AUTHOR("maolijie <lijie.mlj@alibaba-inc.com>");
MODULE_ALIAS("platform:xuantie-iommu");
MODULE_LICENSE("GPL v2");
