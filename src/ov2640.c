#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_misc.h>
#include <stm32f4xx_dcmi.h>
#include "sccb.h"
#include "ov2640.h"
#include "systick.h"
#include "ov2640_regs.h"
static const uint8_t default_regs[][2] = {
    { BANK_SEL, BANK_SEL_DSP},
    { 0x2c,    0xff },
    { 0x2e,    0xdf },

    { BANK_SEL, BANK_SEL_SENSOR },
    { 0x3c,    0x32 },
    { CLKRC,   0x01 }, /* Set PCLK divider */
    { COM2,    COM2_OUT_DRIVE_2x }, /* Output drive x2 */
    { REG04,   REG04_SET(0x00)}, /* Mirror/VFLIP/AEC[1:0]*/
    { COM7,    COM7_RES_UXGA | COM7_ZOOM_EN },
    { COM8,    COM8_SET(COM8_BNDF_EN | COM8_AGC_EN | COM8_AEC_EN) },
    { COM9,    COM9_AGC_SET(COM9_AGC_GAIN_8x)},
    { 0x2c,    0x0c },
    { 0x33,    0x78 },
    { 0x3a,    0x33 },
    { 0x3b,    0xfb },
    { 0x3e,    0x00 },
    { 0x43,    0x11 },
    { 0x16,    0x10 },
    { 0x39,    0x02 },
    { 0x35,    0x88 },
    { 0x22,    0x0a },
    { 0x37,    0x40 },
    { 0x23,    0x00 },
    { ARCOM2,  0xa0 },
    { 0x06,    0x02 },
    { 0x06,    0x88 },
    { 0x07,    0xc0 },
    { 0x0d,    0xb7 },
    { 0x0e,    0x01 },
    { 0x4c,    0x00 },
    { 0x4a,    0x81 },
    { 0x21,    0x99 },
    /* AGC/AEC operating region */
    { AEW,     0x40 },
    { AEB,     0x38 },
    /* AGC/AEC fast mode operating region */
    { VV,      VV_AGC_TH_SET(0x08, 0x02) },
    { 0x5c,    0x00 },
    { 0x63,    0x00 },
    { FLL,     0x00 }, //TODO
    { FLH,     0x00 },
    /* Set banding filter */
    { COM3,    COM3_BAND_SET(COM3_BAND_AUTO) },
    { REG5D,   0x55 },
    { REG5E,   0x7d },
    { REG5F,   0x7d },
    { REG60,   0x55 },
    { HISTO_LOW,   0x70 },
    { HISTO_HIGH,  0x80 },
    { 0x7c,    0x05 },
    { 0x20,    0x80 },
    { 0x28,    0x30 },
    { 0x6c,    0x00 },
    { 0x6d,    0x80 },
    { 0x6e,    0x00 },
    { 0x70,    0x02 },
    { 0x71,    0x94 },
    { 0x73,    0xc1 },
    { 0x3d,    0x34 },
    { 0x5a,    0x57 },
    { BD50,    0xbb },
    { BD60,    0x9c },

    { BANK_SEL, BANK_SEL_DSP },
    { 0xe5,    0x7f },
    { MC_BIST, MC_BIST_RESET | MC_BIST_BOOT_ROM_SEL },
    { 0x41,    0x24 },
    { RESET,   RESET_JPEG | RESET_DVP },
    { 0x76,    0xff },
    { 0x33,    0xa0 },
    { 0x42,    0x20 },
    { 0x43,    0x18 },
    { 0x4c,    0x00 },
    { CTRL3,   CTRL3_BPC_EN | CTRL3_WPC_EN | 0x10 },
    { 0x88,    0x3f },
    { 0xd7,    0x03 },
    { 0xd9,    0x10 },
    { R_DVP_SP, R_DVP_SP_AUTO_MODE},
    { 0xc8,    0x08 },
    { 0xc9,    0x80 },
    { BPADDR,  0x00 },
    { BPDATA,  0x00 },
    { BPADDR,  0x03 },
    { BPDATA,  0x48 },
    { BPDATA,  0x48 },
    { BPADDR,  0x08 },
    { BPDATA,  0x20 },
    { BPDATA,  0x10 },
    { BPDATA,  0x0e },
    { 0x90,    0x00 },
    { 0x91,    0x0e },
    { 0x91,    0x1a },
    { 0x91,    0x31 },
    { 0x91,    0x5a },
    { 0x91,    0x69 },
    { 0x91,    0x75 },
    { 0x91,    0x7e },
    { 0x91,    0x88 },
    { 0x91,    0x8f },
    { 0x91,    0x96 },
    { 0x91,    0xa3 },
    { 0x91,    0xaf },
    { 0x91,    0xc4 },
    { 0x91,    0xd7 },
    { 0x91,    0xe8 },
    { 0x91,    0x20 },
    { 0x92,    0x00 },
    { 0x93,    0x06 },
    { 0x93,    0xe3 },
    { 0x93,    0x03 },
    { 0x93,    0x03 },
    { 0x93,    0x00 },
    { 0x93,    0x02 },
    { 0x93,    0x00 },
    { 0x93,    0x00 },
    { 0x93,    0x00 },
    { 0x93,    0x00 },
    { 0x93,    0x00 },
    { 0x93,    0x00 },
    { 0x93,    0x00 },
    { 0x96,    0x00 },
    { 0x97,    0x08 },
    { 0x97,    0x19 },
    { 0x97,    0x02 },
    { 0x97,    0x0c },
    { 0x97,    0x24 },
    { 0x97,    0x30 },
    { 0x97,    0x28 },
    { 0x97,    0x26 },
    { 0x97,    0x02 },
    { 0x97,    0x98 },
    { 0x97,    0x80 },
    { 0x97,    0x00 },
    { 0x97,    0x00 },
    { 0xa4,    0x00 },
    { 0xa8,    0x00 },
    { 0xc5,    0x11 },
    { 0xc6,    0x51 },
    { 0xbf,    0x80 },
    { 0xc7,    0x10 },
    { 0xb6,    0x66 },
    { 0xb8,    0xA5 },
    { 0xb7,    0x64 },
    { 0xb9,    0x7C },
    { 0xb3,    0xaf },
    { 0xb4,    0x97 },
    { 0xb5,    0xFF },
    { 0xb0,    0xC5 },
    { 0xb1,    0x94 },
    { 0xb2,    0x0f },
    { 0xc4,    0x5c },
    { 0xa6,    0x00 },
    { 0xa7,    0x20 },
    { 0xa7,    0xd8 },
    { 0xa7,    0x1b },
    { 0xa7,    0x31 },
    { 0xa7,    0x00 },
    { 0xa7,    0x18 },
    { 0xa7,    0x20 },
    { 0xa7,    0xd8 },
    { 0xa7,    0x19 },
    { 0xa7,    0x31 },
    { 0xa7,    0x00 },
    { 0xa7,    0x18 },
    { 0xa7,    0x20 },
    { 0xa7,    0xd8 },
    { 0xa7,    0x19 },
    { 0xa7,    0x31 },
    { 0xa7,    0x00 },
    { 0xa7,    0x18 },
    { 0x7f,    0x00 },
    { 0xe5,    0x1f },
    { 0xe1,    0x77 },
    { 0xdd,    0x7f },
    /* Enable AEC/AEC_SEL/YUV/YUV422/RGB */
    { CTRL0,  CTRL0_YUV422 | CTRL0_YUV_EN | CTRL0_RGB_EN },

    /* Set DSP input image size to 1600x1200 and offset to 0.
       The sensor output image can be scaled with OUTW/OUTH */
    { BANK_SEL, BANK_SEL_DSP },
    { RESET,   RESET_DVP },
    { HSIZE8,  (1600>>3)}, /* Image Horizontal Size HSIZE[10:3] */
    { VSIZE8,  (1200>>3)}, /* Image Horizontal Size VSIZE[10:3] */
    { SIZEL,   0x00 },     /* {HSIZE[11], HSIZE[2:0], VSIZE[2:0]} */

    { XOFFL,   0x00 }, /* OFFSET_X[7:0] */
    { YOFFL,   0x00 }, /* OFFSET_Y[7:0] */
    { HSIZE,   ((1600>>2)&0xFF) }, /* H_SIZE[7:0] real/4 */
    { VSIZE,   ((1200>>2)&0xFF) }, /* V_SIZE[7:0] real/4 */
    /* V_SIZE[8]/OFFSET_Y[10:8]/H_SIZE[8]/OFFSET_X[10:8] */
    { VHYX,    ((1600>>3)&0x80) | ((1200>>7)&0x08) },
    { TEST,    0x00 }, /* H_SIZE[9] */

    { CTRL2,   CTRL2_DCW_EN | CTRL2_SDE_EN |
               CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN },
    { CTRLI,   CTRLI_LP_DP | 0x1B }, /* H_DIVIDER/V_DIVIDER */
    { R_BYPASS, R_BYPASS_USE_DSP },
    {0x00,  0x00}
};

static const uint8_t yuv422_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { IMAGE_MODE, IMAGE_MODE_YUV422 },
        { 0xD7,     0x01 },
        { 0x33,     0xa0 },
        { 0xe1,     0x67 },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t rgb565_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { IMAGE_MODE, IMAGE_MODE_RGB565 },
        { 0xd7,     0x03 },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t br_regs[NUM_BR_LEVELS + 1][5] = {
    { BPADDR, BPDATA, BPADDR, BPDATA, BPDATA },
    { 0x00, 0x04, 0x09, 0x00, 0x00 }, /* -2 */
    { 0x00, 0x04, 0x09, 0x10, 0x00 }, /* -1 */
    { 0x00, 0x04, 0x09, 0x20, 0x00 }, /*  0 */
    { 0x00, 0x04, 0x09, 0x30, 0x00 }, /* +1 */
    { 0x00, 0x04, 0x09, 0x40, 0x00 }, /* +2 */
};

static int reset()
{
    int i=0;
    const uint8_t (*regs)[2];

    /* Reset all registers */
    SCCB_Write(BANK_SEL, BANK_SEL_SENSOR);
    SCCB_Write(COM7, COM7_SRST);

    /* delay n ms */
    systick_sleep(10);

    i = 0;
    regs = default_regs;
    /* Write initial regsiters */
    while (regs[i][0]) {
        SCCB_Write(regs[i][0], regs[i][1]);
        i++;
    }

    return 0;
}

static int set_pixformat(enum sensor_pixformat pixformat)
{
    int i=0;
    const uint8_t (*regs)[2]=NULL;

    /* read pixel format reg */
    switch (pixformat) {
        case PIXFORMAT_RGB565:
            regs = rgb565_regs;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_GRAYSCALE:
            regs = yuv422_regs;
            break;
        case PIXFORMAT_JPEG:
            return -1;
            break;
        default:
            return -1;
    }

    /* Write initial regsiters */
    while (regs[i][0]) {
        SCCB_Write(regs[i][0], regs[i][1]);
        i++;
    }
    return 0;
}

static int set_framesize(enum sensor_framesize framesize)
{
    int ret=0;
    uint16_t w=res_width[framesize];
    uint16_t h=res_height[framesize];

    /* Write output width */
    ret |= SCCB_Write(ZMOW, (w>>2)&0xFF); /* OUTW[7:0] (real/4) */
    ret |= SCCB_Write(ZMOH, (h>>2)&0xFF); /* OUTH[7:0] (real/4) */
    ret |= SCCB_Write(ZMHH, ((h>>8)&0x04)|((w>>10)&0x03)); /* OUTW[9:8] */
    return ret;
}

static int set_framerate(enum sensor_framerate framerate)
{
    return 0;
}

static int set_brightness(uint8_t level)
{
    int i;

    level += (NUM_BR_LEVELS / 2 + 1);
    if (level < 0 || level > NUM_BR_LEVELS) {
        return -1;
    }

    /* Switch to DSP register bank */
    SCCB_Write(BANK_SEL, BANK_SEL_SENSOR);

    /* Write brightness registers */
    for (i=0; i<sizeof(br_regs[0])/sizeof(br_regs[0][0]); i++) {
        SCCB_Write(br_regs[0][i], br_regs[level][i]);
    }
    return 0;
}

static int set_exposure(uint16_t exposure)
{
   return 0;
}

static int set_gainceiling(enum sensor_gainceiling gainceiling)
{
    /* Write gain ceiling register */
    return SCCB_Write(COM9, COM9_AGC_SET(gainceiling));
}

int ov2640_init(struct sensor_dev *sensor)
{
    /* set HSYNC/VSYNC/PCLK polarity */
    sensor->vsync_pol = DCMI_VSPolarity_Low;
    sensor->hsync_pol = DCMI_HSPolarity_Low;
    sensor->pixck_pol = DCMI_PCKPolarity_Rising;

    /* set function pointers */
    sensor->reset = reset;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_framerate = set_framerate;
    sensor->set_brightness= set_brightness;
    sensor->set_exposure  = set_exposure;
    sensor->set_gainceiling = set_gainceiling;
    return 0;
}