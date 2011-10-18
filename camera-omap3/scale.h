/* =============================================================================
*             Texas Instruments OMAP(TM) Platform Software
*  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
*
*  Use of this software is controlled by the terms and conditions found
*  in the license agreement under which this software has been supplied.
* ============================================================================
* */
/**
* @file amrdecsocket_ti.h
*
* Auto-generated UUID structures for DSP/BIOS Bridge nodes.
* Do not modify.
*
* @path $(CSLPATH)\
*
* @rev 0.1
*/
/* ---------------------------------------------------------------------------
 * */


#ifndef VPPSOCKET_TI_H
#define VPPSOCKET_TI_H

#define ASPECT_RATIO_FLAG_KEEP_ASPECT_RATIO     (1<<0)  // By default is enabled
#define ASPECT_RATIO_FLAG_SHRINK_WHOLE_SRC      (1<<1)
#define ASPECT_RATIO_FLAG_CROP_BY_PHYSICAL      (1<<2)
#define ASPECT_RATIO_FLAG_CROP_BY_SOURCE        (1<<3)
#define ASPECT_RATIO_FLAG_CROP_BY_DESTINATION   (1<<4)

#define ASPECT_RATIO_FLAG_CROP_BY_ALL           (ASPECT_RATIO_FLAG_CROP_BY_PHYSICAL| \
                                                ASPECT_RATIO_FLAG_CROP_BY_SOURCE| \
                                                ASPECT_RATIO_FLAG_CROP_BY_DESTINATION)

struct DSP_UUID VPPNODE_TI_UUID = {
    0xfbb1c6fc, 0x8d9d, 0x4ac3, 0x80, 0x3f, {
    0x99, 0x7b, 0xd6, 0x17, 0xd8, 0xb7
    }
};

typedef enum VGPOP_EInFormat {
    VGPOP_ERGB24_IN,
    VGPOP_ERGB16_IN,
    VGPOP_ERGB12_IN,
    VGPOP_ERGB8_IN,
    VGPOP_ERGB4_IN,
    VGPOP_E422_IN_UY,
    VGPOP_E422_IN_YU,
    VGPOP_E422_IN_UY_WS,
    VGPOP_E422_IN_YU_WS,
    VGPOP_E420_IN,
    VGPOP_EGRAY8_IN,
    VGPOP_EGRAY4_IN,
    VGPOP_EGRAY2_IN,
    VGPOP_EGRAY1_IN
} VGPOP_EInFormat;

/*** Output YUV formats definitions */
typedef enum VGPOP_EYUVOut {
    VGPOP_EYUV_NONE,
    VGPOP_E420_OUT,
    VGPOP_E422_OUT_UY,
    VGPOP_E422_OUT_YU
} VGPOP_EYUVOut;

/*** OutputRGB formats definitions */
typedef enum VGPOP_ERGBOut {
    VGPOP_ERGB_NONE,
    VGPOP_ERGB4_OUT,
    VGPOP_ERGB8_OUT,
    VGPOP_ERGB12_OUT,
    VGPOP_EARGB16_OUT,
    VGPOP_ERGB16_OUT,
    VGPOP_ERGB24_OUT,
    VGPOP_EARGB32_OUT,
    VGPOP_ERGBA32_OUT,
    VGPOP_ERGB32_OUT,
    VGPOP_EGRAY8_OUT,
    VGPOP_EGRAY4_OUT,
    VGPOP_EGRAY2_OUT,
    VGPOP_EGRAY1_OUT
} VGPOP_ERGBOut;


#endif /* MP3DECSOCKET_TI_H */


