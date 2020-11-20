#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/reset.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "i2c_drv.h"
#include "i2c_reg.h"
#include "i2c_hal.h"

#include "./include/dh_debug.h"

extern DHC_I2C_DEV_TAB_S dev_tab[DHC_MAX_I2C_DEV];
static DH_U32 u32I2cIdIndex;

static irqreturn_t dhc_i2c_irq(int irq, void *dev_id)
{
    I2C_DRV_IrqHandle(dev_id);

    return IRQ_HANDLED;
}


int dhc_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    int ret;
    struct dhc_i2c_dev *dev = i2c_get_adapdata(adap);
    I2C_DRV_GetLock(adap->nr);
    ret = I2C_DRV_Xfer(dev,msgs, num);
    I2C_DRV_FreeLock(adap->nr);
    return ret;
}

static unsigned int dhc_i2c_func(struct i2c_adapter *adap)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}


static struct i2c_algorithm dhc_i2c_algorithm = {
    .master_xfer    = dhc_i2c_xfer,
    .functionality  = dhc_i2c_func,
};


DH_S32 I2C_Write_8bit(DH_U32 u32CtlIndex,DH_U32 u32DevAddr, DH_U32 u32RegAddr,
                          DH_U32 u32Value)
{
    DH_S32 ret;
    I2C_DRV_GetLock(u32CtlIndex);
    ret = I2C_DRV_Write8bit(u32CtlIndex, u32DevAddr,  u32RegAddr, u32Value);
    I2C_DRV_FreeLock(u32CtlIndex);
    return ret;
}
EXPORT_SYMBOL(I2C_Write_8bit);

DH_S32 I2C_Read_8bit(DH_U32 u32CtlIndex,DH_U32 u32DevAddr, DH_U32 u32RegAddr,
                       DH_U8 *buf)
{
    DH_S32 ret;
    I2C_DRV_GetLock(u32CtlIndex);
    ret = I2C_DRV_Read8bit(u32CtlIndex, u32DevAddr,  u32RegAddr, buf);
    I2C_DRV_FreeLock(u32CtlIndex);
    return ret;
}
EXPORT_SYMBOL(I2C_Read_8bit);

DH_S32 I2C_Write_16bit(DH_U32 u32CtlIndex,DH_U32 u32DevAddr, DH_U32 u32RegAddr,
                          DH_U32 u32Value)
{
    DH_S32 ret;
    I2C_DRV_GetLock(u32CtlIndex);
    ret = I2C_DRV_Write16bit(u32CtlIndex, u32DevAddr,  u32RegAddr, u32Value);
    I2C_DRV_FreeLock(u32CtlIndex);
    return ret;

}
EXPORT_SYMBOL(I2C_Write_16bit);

DH_S32 I2C_Read_16bit(DH_U32 u32CtlIndex,DH_U32 u32DevAddr, DH_U32 u32RegAddr,
                       DH_U16 *buf)
{
    DH_S32 ret;
    I2C_DRV_GetLock(u32CtlIndex);
    ret = I2C_DRV_Read16bit(u32CtlIndex, u32DevAddr,  u32RegAddr, buf);
    I2C_DRV_FreeLock(u32CtlIndex);
    return ret;
}
EXPORT_SYMBOL(I2C_Read_16bit);


DH_S32 I2C_Read(DH_U32 u32CtlIndex,DH_U32 u32DevAddr, DH_U32 u32RegAddr,
						   DH_U32 u32RegByte, DH_U32 *buf,  DH_U32 u32ValByte)
{
	DH_S32 ret;
	I2C_DRV_GetLock(u32CtlIndex);
	ret = I2C_DRV_Read( u32CtlIndex, u32DevAddr,  u32RegAddr,
							    u32RegByte,buf,	u32ValByte);
	I2C_DRV_FreeLock(u32CtlIndex);
	return ret;
}
EXPORT_SYMBOL(I2C_Read);

DH_S32 I2C_Write(DH_U32 u32CtlIndex,DH_U32 u32DevAddr, DH_U32 u32RegAddr,
					    DH_U32 u32RegByte, DH_U32 u32Value,   DH_U32 u32ValByte)
{
	DH_S32 ret;
	I2C_DRV_GetLock(u32CtlIndex);
	ret = I2C_DRV_Write( u32CtlIndex, u32DevAddr,  u32RegAddr,
					     u32RegByte,  u32Value,  u32ValByte);
	I2C_DRV_FreeLock(u32CtlIndex);
	return ret;
}
EXPORT_SYMBOL(I2C_Write);


DH_S32 I2C_Group_Send_Get_lock(DH_U32 u32CtlIndex)
{
    DH_S32 ret;
    if(u32CtlIndex > (DHC_MAX_I2C_DEV - 1))
    {
        printk("get iic %d index > max index %d\n",u32CtlIndex,(DHC_MAX_I2C_DEV - 1));
        return -1;
    }

    ret = I2C_DRV_GetLock(u32CtlIndex);
    return ret;
}
EXPORT_SYMBOL(I2C_Group_Send_Get_lock);

DH_S32 I2C_Group_Send_Free_lock(DH_U32 u32CtlIndex)
{
    if(u32CtlIndex > (DHC_MAX_I2C_DEV - 1))
    {
        printk("get iic %d index > max index %d\n",u32CtlIndex,(DHC_MAX_I2C_DEV - 1));
        return -1;
    }

    I2C_DRV_FreeLock(u32CtlIndex);

    return 0;
}
EXPORT_SYMBOL(I2C_Group_Send_Free_lock);

DH_S32 I2C_Get_Group_Send_Status(DH_U32 u32CtlIndex)
{
    return I2C_DRV_GetGroupSendStatus( u32CtlIndex );
}
EXPORT_SYMBOL(I2C_Get_Group_Send_Status);

DH_S32 I2C_Set_Group_Send_Status(DH_U32 u32CtlIndex,DH_S32 status)
{
    return I2C_DRV_SetGroupSendStatus( u32CtlIndex, status );
}
EXPORT_SYMBOL(I2C_Set_Group_Send_Status);


DH_S32 I2C_Group_Send(DH_U32 u32CtlIndex,DH_U32 u32DevAddr,
                       DH_U16 *buf,DH_U32 cnt)
{
    DH_S32 ret;
    ret = I2C_DRV_GroupSend( u32CtlIndex, u32DevAddr, buf, cnt);
    return ret;
}
EXPORT_SYMBOL(I2C_Group_Send);



DH_S32 I2C_Get_Speed(DH_VOID)
{
    return I2C_SPEED;
}
EXPORT_SYMBOL(I2C_Get_Speed);


static int dhc_i2c_probe(struct platform_device *pdev)
{
    struct resource *mem,*ioarea;
    int ret,irq;
    struct dhc_i2c_dev *dev;
    struct i2c_adapter *adap;
    struct clk *bus_clk;
    unsigned int freq, bus_hz;

    pdev->id = u32I2cIdIndex;
    u32I2cIdIndex++;
    printk("dhc_i2c_probe\n");

    /* map the registers */
    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (mem == NULL)
    {
        dev_err(&pdev->dev, "no mem resource\n");
        return -EINVAL;
    }

    irq = platform_get_irq(pdev, 0);
    if (irq < 0)
    {
        dev_err(&pdev->dev, "cannot find IRQ\n");
        return irq; /* -ENXIO */
    }

    ioarea = request_mem_region(mem->start, resource_size(mem),pdev->name);
    if (ioarea == NULL)
    {
        dev_err(&pdev->dev, "I2C region already claimed\n");
        return -EBUSY;
    }

    dev = kzalloc(sizeof(struct dhc_i2c_dev), GFP_KERNEL);
    if (!dev)
    {
        ret = -ENOMEM;
        goto err_release_region;
    }

    dev->irq = irq;
    platform_set_drvdata(pdev, dev);

    dev->base = ioremap(mem->start, resource_size(mem));
    if (dev->base == NULL)
    {
        dev_err(&pdev->dev, "failure mapping io resources\n");
        ret = -EBUSY;
        goto err_free_mem;
    }

    /* 在request_irq之前初始化，解决加载i2c驱动时中断触发，调用wake_up函数空指针问题 */
    init_waitqueue_head(&dev->wait);

    ret = request_irq(dev->irq, dhc_i2c_irq, 0,dev_name(&pdev->dev), dev);
    if (ret != 0)
    {
        dev_err(&pdev->dev, "cannot claim IRQ %d\n", dev->irq);
        goto err_iounmap;
    }

    bus_clk = devm_clk_get(&(pdev->dev), NULL);
    if (IS_ERR(bus_clk)) {
        bus_hz = I2C_CLK;
    } else {
        bus_hz = clk_get_rate(bus_clk);
    }
    printk("i2c bus %dHz\n", bus_hz);

    if (device_property_read_u32(&(pdev->dev), "clock-frequency", &freq)) {
        freq = I2C_SPEED;
    } else {
        printk("get freq=%d\n", freq);
    }

    I2C_DRV_Init(dev->base, bus_hz, freq, pdev->id);
    I2C_DRV_InitLock(pdev->id);
    I2C_DRV_InitCmdBuf(pdev->id);
    I2C_DRV_SetGroupSendStatus(pdev->id, STATUS_IDLE );
    adap = &dev->adapter;
    i2c_set_adapdata(adap, dev);
    adap->owner   = THIS_MODULE;
    adap->algo    = &dhc_i2c_algorithm;
    adap->class   = I2C_CLASS_HWMON | I2C_CLASS_SPD;
    adap->dev.of_node = pdev->dev.of_node;
    strlcpy(adap->name, "Synopsys DesignWare I2C adapter", sizeof(adap->name));
    adap->dev.parent = &pdev->dev;
    adap->nr = pdev->id;
    ret = i2c_add_numbered_adapter(adap);
    if (ret < 0)
    {
        dev_err(&pdev->dev, "failed to add bus to i2c core\n");
        goto err_free_irq;
    }

    return 0;

err_free_irq:
    free_irq(dev->irq, dev);
    I2C_DRV_DeInit(dev->base, pdev->id);

err_iounmap:
    iounmap(dev->base);
err_free_mem:
    platform_set_drvdata(pdev, NULL);
    kfree(dev);

err_release_region:
    release_mem_region(mem->start, resource_size(mem));

    return ret;

}


static int   dhc_i2c_remove(struct platform_device *pdev)
{
    struct dhc_i2c_dev *dev = platform_get_drvdata(pdev);
    struct resource *mem;
    printk("i2c remove\n");
    I2C_DRV_DeInit(dev->base, pdev->id);
    platform_set_drvdata(pdev, NULL);
    i2c_del_adapter(&dev->adapter);

    free_irq(dev->irq, dev);
    iounmap(dev->base);
    kfree(dev);

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    release_mem_region(mem->start, resource_size(mem));

    return 0;
}

static const struct of_device_id dh_i2c_of_match[] = {
	{ .compatible = "dahua,i2c-dw201a"},
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, dh_i2c_of_match);


static struct platform_driver dhc_i2c_driver = {
    .probe  = dhc_i2c_probe,
    .remove = dhc_i2c_remove,
    .driver = {
        .name   = "dhc_i2c",
        .of_match_table  = dh_i2c_of_match,
    },
};

/* platform_del_devices(struct platform_device **devs, int num)
{
    int i;
    for (i = 0; i < num; i++)
    {
        platform_device_unregister(devs[i]);
    }
    return 0;
}*/

//tern struct sensor_handle i2c_group_interface;

static int __init I2C_Init(void)
{
    DH_S32 s32Ret;

	/*c_i2c_resource = kmalloc(DHC_MAX_I2C_DEV * sizeof(struct resource[2]),GFP_KERNEL);
	if(dhc_i2c_resource == NULL)
	{
		printk("kmalloc i2c resource faild\n");
		return DH_FAILURE;
	}
	memset(dhc_i2c_resource,0,DHC_MAX_I2C_DEV * sizeof(struct resource[2]));
	dhc_i2c_devices = kmalloc(DHC_MAX_I2C_DEV * sizeof(struct platform_device),GFP_KERNEL);
	if(dhc_i2c_devices == NULL)
	{
		kfree(dhc_i2c_resource);
		printk("kmalloc i2c devices faild\n");
		return DH_FAILURE;
	}
	memset(dhc_i2c_devices,0,DHC_MAX_I2C_DEV * sizeof(struct platform_device));

	for(i = 0; i < DHC_MAX_I2C_DEV; i++)
	{
		pI2cResource = dhc_i2c_resource + i*2;
		pI2cResource->start = dev_tab[i].devBase;
		pI2cResource->end   = dev_tab[i].devBase + DHC_I2C_SIZE - 1;
		pI2cResource->flags = IORESOURCE_MEM;

		pI2cResource++;
		pI2cResource->start = dev_tab[i].devirq;
		pI2cResource->end   = dev_tab[i].devirq;
		pI2cResource->flags = IORESOURCE_IRQ;


		pI2cDevice = dhc_i2c_devices + i;
		pI2cDevice->name = "dhc_i2c",
		pI2cDevice->id = i,
		pI2cDevice->num_resources = 2,
		pI2cDevice->resource = dhc_i2c_resource + i*2;
		pI2cDevice->dev.release = dev_release;
		dhc_i2c_devices_array[i] = dhc_i2c_devices + i;

	}*/

    /*t = platform_add_devices(dhc_i2c_devices_array, DHC_MAX_I2C_DEV);
    if(ret != DH_SUCCESS)
    {
    	kfree(dhc_i2c_resource);
		kfree(dhc_i2c_devices);
        printk("platform_add_devices is fail\n");
        return DH_FAILURE;
    }*/
    s32Ret = platform_driver_register(&dhc_i2c_driver);
    if(s32Ret < 0)
    {
        return s32Ret;
    }
    /*i2c_group_interface.get_lock = I2C_Group_Send_Get_lock;
    i2c_group_interface.free_lock = I2C_Group_Send_Free_lock;
    i2c_group_interface.get_status = I2C_Get_Group_Send_Status;
    i2c_group_interface.set_status = I2C_Set_Group_Send_Status;
    i2c_group_interface.get_speed = I2C_Get_Speed;
    i2c_group_interface.send = I2C_Group_Send;*/

    return DH_SUCCESS;
}

static void __exit I2C_Exit(void)
{

    platform_driver_unregister(&dhc_i2c_driver);
#if 0
    platform_del_devices(dhc_i2c_devices_array, DHC_MAX_I2C_DEV);
    platform_driver_unregister(&dhc_i2c_driver);
	kfree(dhc_i2c_resource);
	kfree(dhc_i2c_devices);
    i2c_group_interface.get_lock = NULL;
    i2c_group_interface.free_lock = NULL;
    i2c_group_interface.get_status = NULL;
    i2c_group_interface.set_status = NULL;
    i2c_group_interface.get_speed = NULL;
    i2c_group_interface.send = NULL;
#endif
}

module_init(I2C_Init);
module_exit(I2C_Exit);

MODULE_DESCRIPTION("I2C Driver");
MODULE_AUTHOR("ZHONG_ZHAOHUI");
MODULE_LICENSE("GPL");
