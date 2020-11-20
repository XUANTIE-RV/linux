/******************************************************************************

   Copyright (C) 2011-2014 ZheJiang Dahua Technology CO.,LTD.

 ******************************************************************************
  File Name     : i2c_hal.h
  Version       : Initial Draft
  Author        : Zxj
  Created       : 2014.4.29
  Last Modified :
  Description   : I2C register define
  Function List :
  DHstory       :
  1.Date        : 2014/4/29
    Author      : 11853
    Modification: Create
******************************************************************************/

#ifndef __I2C_HAL_H__
#define __I2C_HAL_H__


#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif /* __cplusplus */

#include <linux/io.h>

#include "./include/dh_define.h"
#include "./include/dh_common.h"

#define DHC_MAX_I2C_DEV     ( 6 )


typedef struct
{
    DH_U32 devBase;
    DH_U32 devirq;
}DHC_I2C_DEV_TAB_S;

DH_VOID I2C_HAL_SetReg(DH_U32 value,DH_VOID *base,DH_U32 offset);

DH_U32 I2C_HAL_GetReg(DH_VOID *base,int offset);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __I2C_HAL_H__ */


