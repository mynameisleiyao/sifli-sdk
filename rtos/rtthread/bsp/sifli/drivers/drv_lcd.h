/**
  ******************************************************************************
  * @file   drv_lcd.h
  * @author Sifli software development team
  ******************************************************************************
*/
/**
 * @attention
 * Copyright (c) 2019 - 2022,  Sifli Technology
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Sifli integrated circuit
 *    in a product or a software update for such product, must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Sifli nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Sifli integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY SIFLI TECHNOLOGY "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SIFLI TECHNOLOGY OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __DRV_LCD_H__
#define __DRV_LCD_H__

#include <rtdevice.h>
#include "bf0_hal.h"

/****************************************    Adapter for LCD driver      **********************************************/
typedef enum
{
    LCD_ROTATE_0_DEGREE = 0,
    LCD_ROTATE_90_DEGREE = 90,   /*Clock wise 90 degrees*/
    LCD_ROTATE_180_DEGREE = 180, /*Clock wise 180 degrees*/
    LCD_ROTATE_270_DEGREE = 270, /*Clock wise 270 degrees*/
} LCD_DrvRotateTypeDef;

typedef struct
{
    void (*Init)(LCDC_HandleTypeDef *hlcdc);
    uint32_t (*ReadID)(LCDC_HandleTypeDef *hlcdc);
    void (*DisplayOn)(LCDC_HandleTypeDef *hlcdc);
    void (*DisplayOff)(LCDC_HandleTypeDef *hlcdc);

    void (*SetRegion)(LCDC_HandleTypeDef *hlcdc, uint16_t, uint16_t, uint16_t, uint16_t);
    void (*WritePixel)(LCDC_HandleTypeDef *hlcdc, uint16_t, uint16_t, const uint8_t *);
    void (*WriteMultiplePixels)(LCDC_HandleTypeDef *hlcdc, const uint8_t *, uint16_t, uint16_t, uint16_t, uint16_t);
    uint32_t (*ReadPixel)(LCDC_HandleTypeDef *hlcdc, uint16_t, uint16_t);

    /*optional operation*/
    void (*SetColorMode)(LCDC_HandleTypeDef *hlcdc, uint16_t);
    void (*SetBrightness)(LCDC_HandleTypeDef *hlcdc, uint8_t);       // in percentage
    void (*IdleModeOn)(LCDC_HandleTypeDef *hlcdc);
    void (*IdleModeOff)(LCDC_HandleTypeDef *hlcdc);
    void (*Rotate)(LCDC_HandleTypeDef *hlcdc, LCD_DrvRotateTypeDef);
    void (*TimeoutDbg)(LCDC_HandleTypeDef *hlcdc);
    void (*TimeoutReset)(LCDC_HandleTypeDef *hlcdc);
    uint32_t (*ESDDetect)(LCDC_HandleTypeDef *hlcdc); //Return 0 if OK
} LCD_DrvOpsDef;


#define LCD_DRV_NAME_MAX_LEN 16

typedef struct
{
    char name[LCD_DRV_NAME_MAX_LEN];
    uint32_t id;
    const LCDC_InitTypeDef *p_init_cfg;
    const LCD_DrvOpsDef *p_ops;
    uint16_t lcd_horizonal_res;
    uint16_t lcd_vertical_res;
    uint16_t ic_max_horizonal_res;
    uint16_t ic_max_vertical_res;
    uint16_t pixel_align;
} lcd_drv_desc_t;

#define LCD_DRIVER_EXPORT(name, id, init_cfg, dev_ops, ic_max_hor_res, ic_max_ver_res, pixel_align)         \
    RT_USED static const lcd_drv_desc_t __lcddriver_##name                       \
    SECTION("LcdDriverDescTab") =                                                \
    {                                                                            \
        #name,        \
        id,           \
        init_cfg,     \
        dev_ops,      \
        LCD_HOR_RES_MAX,       \
        LCD_VER_RES_MAX,       \
        ic_max_hor_res,        \
        ic_max_ver_res,        \
        pixel_align           \
    }
#define LCD_DRIVER_EXPORT2(name, id, init_cfg, dev_ops, pixel_align) LCD_DRIVER_EXPORT(name, id, init_cfg, dev_ops, LCD_HOR_RES_MAX, LCD_VER_RES_MAX, pixel_align)
#define LCD_DRIVER_DELAY_MS(ms) rt_thread_mdelay(ms)  //Use RTOS thread delay
typedef enum
{
    LCD_STATUS_NONE = 0,        /* Not executed lcd open*/
    LCD_STATUS_OPENING,
    LCD_STATUS_NOT_FIND_LCD,
    LCD_STATUS_INITIALIZED,     /*Finded LCD*/
    LCD_STATUS_DISPLAY_ON,
    LCD_STATUS_DISPLAY_OFF_PENDING,
    LCD_STATUS_DISPLAY_OFF,
    LCD_STATUS_DISPLAY_TIMEOUT,
    LCD_STATUS_IDLE_MODE,
} LCD_DrvStatusTypeDef;


/********************************************   Extend drv_lcd control commands   ****************************/
typedef struct _graphic_layer_info_t
{
    uint8_t  cmpr_rate;     //<! compression rate
    uint16_t color_format;  //<! color formate of layer see RTGRAPHIC_PIXEL_FORMAT_xxx
    uint8_t *pL8_Tab;      //<! L8 lookup table, must be format in ARGB8888*256 = 1024 bytes.
    LCDC_ColorDef A8_Bg;    //<! A8 background
    int16_t x0;            //<! Layer start coodinate x
    int16_t y0;            //<! Layer start coodinate y
    int16_t x1;            //<!
    int16_t y1;            //<!
    uint16_t total_width;  //<! Original data's width in pixels
    const uint8_t *pixel;
    HAL_LCDC_RotateDef rotate;
} graphic_layer_info_t;

typedef struct
{
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
} LCD_AreaDef;

typedef struct _lcd_flush_info_t
{
    uint8_t  cmpr_rate;     //<! compression rate
    const uint8_t *pixel;
    uint16_t color_format;  //<! color formate of layer see RTGRAPHIC_PIXEL_FORMAT_xxx
    LCD_AreaDef  window;
    LCD_AreaDef  pixel_area;
} lcd_flush_info_t;

inline static void lcd_area_copy(LCD_AreaDef *dest, const LCD_AreaDef *src)
{
    dest->x0 = src->x0;
    dest->y0 = src->y0;
    dest->x1 = src->x1;
    dest->y1 = src->y1;
}

#define SF_GRAPHIC_CTRL_LCDC_FLUSH  2000


/***************************************    add for test purpose    ****************************************/
//LCDC output color format
#define SF_GRAPHIC_CTRL_LCDC_OUT_FORMAT 1001
//The comression rate of LCDC layer buffer
#define SF_GRAPHIC_CTRL_LCDC_LAYER_BUF_CMPR_RATE  1002
//Format of LCDC layer buffer
//#define SF_GRAPHIC_CTRL_LCDC_LAYER_BUF_FORMAT 1000

//#define SF_GRAPHIC_CTRL_LCDC_LAYER_INFO_GET  1003
#define SF_GRAPHIC_CTRL_LCDC_LAYER_INFO_SET  1004

//#define SF_GRAPHIC_CTRL_LCDC_CHECK_RUNNING   1005

#define SF_GRAPHIC_CTRL_LCDC_CP2COMPRESS_FB_AND_SEND_CMPRATE_SET   1006
#define SF_GRAPHIC_CTRL_LCDC_CP2COMPRESS_FB_AND_SEND_CMPRATE_GET   1007

#define SF_GRAPHIC_CTRL_LCD_PRESENT   1008
#define SF_GRAPHIC_CTRL_ASSERT_IF_DRAWTIMEOUT   1009
#define SF_GRAPHIC_CTRL_GET_DRAW_ERR   1010

typedef struct _sf_graphic_lcdc_layer_t
{
    uint8_t  cmpr_rate;     //<! compression rate
    uint16_t color_format;  //<! color formate of layer
    uint32_t *pL8_Tab;      //<! L8 lookup table
    LCDC_ColorDef A8_Bg;    //<! A8 background
} sf_graphic_lcdc_layer_t;



#ifdef LCDC_SUPPORTED_COMPRESSED_LAYER
    #if LCD_HOR_RES_MAX <= LCDC_SUPPORTED_COMPRESSED_LAYER_MAX_WIDTH
        #define DRV_LCD_COMPRESSED_BUF_AVALIABLE
    #endif
#endif /* LCDC_SUPPORTED_COMPRESSED_LAYER */

/*
    Use 'DPI_AUX mode' if LCD horizontal pixels are more than 'LCDC_DPI_MAX_WIDTH'
*/
#if (LCDC_DPI_MAX_WIDTH >= LCD_HOR_RES_MAX)
    #define AUTO_SELECTED_DPI_INTFACE LCDC_INTF_DPI
#else
    #define AUTO_SELECTED_DPI_INTFACE LCDC_INTF_DPI_AUX
#endif /* (LCDC_DPI_MAX_WIDTH >= LCD_HOR_RES_MAX) */


#endif





/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/
