/******************************************************************************

   Copyright (C) 2011-2014 ZheJiang Dahua Technology CO.,LTD.

 ******************************************************************************
  File Name     : I2C_hal.c
  Version       : Initial Draft
  Author        : Zxj
  Created       : 2014.4.29
  Last Modified :
  Description   : I2C register hal
  Function List :
  DHstory       :
  1.Date        : 2014/4/29
    Author      : 11853
    Modification: Create
******************************************************************************/

#include <linux/io.h>

#include "./include/dh_define.h"
#include "i2c_reg.h"
#include "i2c_hal.h"

#if DH7300_V100==DHCHIP
DHC_I2C_DEV_TAB_S dev_tab[DHC_MAX_I2C_DEV] =
{
	{0x010E0000, 7},
	{0x010F0000, 8},
};
#else
DHC_I2C_DEV_TAB_S dev_tab[DHC_MAX_I2C_DEV] =
{
	{0x14060000, 7},
	{0x14061000, 8},
	{0x14062000, 9},
	{0x14063000,10},
	{0x14064000, 6},
	{0x14065000,23},
};
#endif

DH_VOID I2C_HAL_SetReg(DH_U32 value,DH_VOID *base,DH_U32 offset)
{
    __raw_writel((value), base + offset);
}

DH_U32 I2C_HAL_GetReg(DH_VOID *base,int offset)
{
    return __raw_readl(base + offset);
}


