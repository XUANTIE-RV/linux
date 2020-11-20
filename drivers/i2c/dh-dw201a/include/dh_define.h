/******************************************************************************

  Copyright (C), 2014-2020, ZheJiang Dahua Technology CO.,LTD.

 ******************************************************************************
  File Name     : dh_define.h
  Version       : Initial Draft
  Author        : Lv Zhuqing
  Created       : 2014.3.13
  Last Modified :
  Description   : 描述各种宏定义
  Function List :
  DHstory       :
  1.Date        : 2014/3/13
    Author      : 24497
    Modification: Create
******************************************************************************/
#ifndef __DH_DEFINE_H__
#define __DH_DEFINE_H__
    
    
#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif /* __cplusplus */


#define DH7200_V100  0x72000100
#define DH7300_V100  0x73000100
#define DH8000_V100  0x80000100

#ifndef DHCHIP
    #define DHCHIP DH8000_V100
#endif

#if DHCHIP == DH8000_V100
    #define CHIP_NAME    "DH8000"
#else
    #error DHCHIP define may be error
#endif

#define GPIO_BANK_NUM               (6)
#define MAX_GPIO_NUM                (144)
#define MAX_GPIO_NUM_OF_PER_BANK	(128)


#define MDK_VER_PRIX "_MDK_V"

/* AUDIO中断号 */
#define AUDIO_IRQ                   (28)

/* FAAC中断号 */
#define FAAC_IRQ                    (22)

/* AEC中断号 */
#define AEC_IRQ                     (56)

/* CIPHER中断号 */
#define CIPHER_IRQ                  (44)

#define SCALE_IRQ                   (58)


/* pwm中断号 */
#define PWM_IRQ                     (18)

/* gpio中断号 */
#define GPIO0_IRQ                   (15)
#define GPIO1_IRQ                   (19)
#define GPIO2_IRQ                   (6 )
#define GPIO3_IRQ                   (9 )
#define GPIO4_IRQ                   (62)
#define GPIO5_IRQ                   (63)

#define VI_IRQ                      (53)

/*********************VENC 部分规格定义*******************************/
#define VENC_MAX_CHN_NUM            (16)

#define VENC_MAX_WIDTH              (4096)
#define VENC_MAX_HEIGHT             (4096)
#define VENC_MIN_H26X_WIDTH         (128)
#define VENC_MIN_H26X_HEIGHT        (128)
#define VENC_MIN_JPEG_WIDTH         (96)
#define VENC_MIN_JPEG_HEIGHT        (32)

#define VENC_MAX_ROI_NUM            (8)

#define VENC_MIN_USER_DATA_SIZE     (16)
#define VENC_MAX_USER_DATA_SIZE     (2048)

#define AENC_MAX_CHN_NUM            (8)

#define SPI_MAX_NUM                 (8)
#define GPIO_MAX_BANK               (4)


/*********************VDEC 部分规格定义*******************************/
#define VDEC_MAX_CHN_NUM            (16)
#define VDEC_MAX_PP_OUTPUT          (2)

#define VDEC_MAX_WIDTH              (4096)
#define VDEC_MAX_HEIGHT             (2304)
#define VDEC_MIN_WIDTH              (48)
#define VDEC_MIN_HEIGHT             (48)
#define VDEC_PP_MIN_WIDTH           (48)
#define VDEC_PP_MIN_HEIGHT          (48)

/* *********************VD 部分规格定义*******************************/
/*最大支持的动检的通道数*/
#define VD_MAX_CHN_NUM         (1)

/*每个通道最大支持动检的区域*/
#define VD_MAX_DETECT_REGION   (4)

/*********************************VI************************************************/
#define VI_MAX_DEV_NUM              (4)
#define VI_MAX_CHN_NUM_PER_DEV      (4)
#define VI_MAX_CHN_NUM              (8)
#define VI_MAX_DATA_LINE           (16)
#define VI_MIN_DEC_WIDTH            (360)
#define VI_MIN_DEC_HEIGHT           (240)
#define VI_MAX_DEC_WIDTH            (4096)
#define VI_MAX_DEC_HEIGHT           (2160)
#define VI_MAX_FRAME_INTERVAL       (16)
#define VI_MIN_FRAME_DEPTH          (1)
#define VI_MAX_FRAME_DEPTH          (100)
#define VI_MIN_INT_THRED            (3)
#define VI_MAX_INT_THRED            (16)
#define VI_MIN_CHN_INT_THRED        (1)
#define VI_MAX_CHN_INT_THRED        (3)

/*********************************VO************************************************/
/* we has two disp device, 0: is high resolution device;1: is standard resolution device */
#define VO_MAX_DEV_NUM              (2)
#define VO_MAX_CHN_NUM              (1)
#define VO_MAX_WIDTH                (3072)
#define VO_MAX_HEIGHT               (1728)

/*********************************mbuf**********************************************/
#define MBUF_MAX_POOLS              (128)
#define MBUF_MAX_COMM_POOLS         (16)
#define MBUF_MAX_PRIV_POOLS         (MBUF_MAX_POOLS - MBUF_MAX_COMM_POOLS)


/* ddr 名字长度 */
//#define MAX_DDR_NAME_LEN         (16)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef __DH_DEFINE_H__ */

