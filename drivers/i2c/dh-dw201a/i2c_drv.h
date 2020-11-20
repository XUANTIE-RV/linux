#ifndef __I2C_DRV_H__
#define __I2C_DRV_H__

#include <linux/i2c.h>  /* i2c */
#include <linux/i2c-dev.h>
#include <linux/spinlock.h>
#include <linux/delay.h>


#include "./include/dh_type.h"
#include "i2c_reg.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif

#define I2C_HARD_GROUP_SEND      1
#define I2C_GROUP_CMD_CNT        (256)

#define DHC_I2C_SPD_STANDARD    (100 * 1000)
#define DHC_I2C_SPD_FAST        (400 * 1000)
#define DHC_I2C_SPD_HIGH        (3400 * 1000)

typedef enum {
    DHC_I2C_SPDMODE_REV = 0,
    DHC_I2C_SPDMODE_STD,
    DHC_I2C_SPDMODE_FAST,
    DHC_I2C_SPDMODE_HIGH,
} DHC_I2C_SPDMODE_E;

typedef enum
{
    STATUS_IDLE      = 0,  /*空闲，说明当前i2c没有处于批量发送状态*/
    STATUS_HANDLING  = 1,  /*资源已经被批量发送接口获取*/
    STATUS_DONE      = 2,  /*批量发送成功*/
    STATUS_ERROR     = 3,  /*批量出错*/
    STATUS_BUTT   = 4,     /*异常状态*/
}DHC_I2C_GROUP_STATUS_E;


typedef struct
{
    DH_S32 devid;
    DH_S32 regAddr;
    DH_S32 nofifo;
    DH_S32 cmdCnt;
    DH_S32 cmdPos;
    DH_U16 cmdBuf[I2C_GROUP_CMD_CNT];
}DHC_I2C_GROUP_CMD_S;

#define DHC_I2C_CMD_BUF_DEP	(I2C_BUF_DEP * 2)
struct dhc_i2c_dev
{
    void __iomem    *base;
    int             irq;

    struct mutex    lock;
    wait_queue_head_t   wait;
    int            complete;
    int             abort;
    struct i2c_adapter  adapter;

    unsigned short cmdBuf[DHC_I2C_CMD_BUF_DEP]; /* cmds for RD and WR */
};

DH_VOID I2C_DRV_IrqHandle(DH_VOID *pvDevID);

DH_S32 I2C_DRV_Xfer(struct dhc_i2c_dev *pstDev, struct i2c_msg *pstMsgs, int s32Num);

DH_S32 I2C_DRV_DeInit(DH_VOID *base, DH_U32 u32CtlIndex);

DH_S32 I2C_DRV_Init(DH_VOID *base, DH_U32 u32BusClk, DH_U32 u32Clk, DH_U32 u32CtlIndex);

DH_S32 I2C_DRV_Write8bit(DH_U32 u32CtlIndex, DH_U32 u32DevAddr, DH_U32 u32RegAddr,
                          DH_U32 u32Value );
DH_S32 I2C_DRV_Read8bit(DH_U32 u32CtlIndex, DH_U32 u32DevAddr, DH_U32 u32RegAddr,
                       DH_U8 *buf);

DH_S32 I2C_DRV_Write16bit(DH_U32 u32CtlIndex, DH_U32 u32DevAddr, DH_U32 u32RegAddr,
                          DH_U32 u32Value );

DH_S32 I2C_DRV_Read16bit(DH_U32 u32CtlIndex, DH_U32 u32DevAddr, DH_U32 u32RegAddr,
                       DH_U16 *buf);

DH_S32 I2C_DRV_Write(DH_U32 u32CtlIndex,DH_U32 u32DevAddr, DH_U32 u32RegAddr,
					    DH_U32 u32RegByte, DH_U32 u32Value,   DH_U32 u32ValByte);

DH_S32 I2C_DRV_Read(DH_U32 u32CtlIndex,DH_U32 u32DevAddr, DH_U32 u32RegAddr,
						   DH_U32 u32RegByte, DH_U32 *buf,  DH_U32 u32ValByte);


DH_S32 I2C_DRV_SetGroupSendStatus(DH_U32 u32CtlIndex,DHC_I2C_GROUP_STATUS_E status);

DHC_I2C_GROUP_STATUS_E I2C_DRV_GetGroupSendStatus(DH_U32 u32CtlIndex);

DH_S32 I2C_DRV_InitLock(DH_U32 u32CtlIndex);

DH_S32 I2C_DRV_GetLock(DH_U32 u32CtlIndex);

DH_S32 I2C_DRV_FreeLock(DH_U32 u32CtlIndex);

DH_S32 I2C_DRV_GroupSend(DH_U32 u32CtlIndex,DH_U32 u32DevAddr,
                       DH_U16 *buf,DH_U32 cnt);

DH_S32 I2C_DRV_InitCmdBuf(DH_U32 u32CtlIndex);

DH_S32 I2C_DRV_WriteFifo(DH_U32 u32CtlIndex,DH_U32 hardgroup);




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

