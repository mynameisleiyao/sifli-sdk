/**
  ******************************************************************************
  * @file   ft3169.c
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

#include <rtthread.h>
#include "board.h"
#include "ft3169.h"
#include "drv_touch.h"
#include "drv_io.h"

/* Define -------------------------------------------------------------------*/

#define DBG_LEVEL          DBG_ERROR  //  DBG_LOG //
#define LOG_TAG              "drv.ft3169"
#include <drv_log.h>

/*Max point numbers of touch trace*/
#define MAX_POINT_NUM                   2
/*Length of touch information*/
#define MAX_LEN_TOUCH_INFO              (MAX_POINT_NUM * 6 + 2)

#define FT_DEV_ADDR             (0x38)
#define FT_TD_STATUS            (0x02)
#define FT_P1_XH                (0x03)
#define FT_P1_XL                (0x04)
#define FT_P1_YH                (0x05)
#define FT_P1_YL                (0x06)
//#define FT_ID_G_MODE            (0xA4)
#define FT_READ_ID_H            (0xA3)
#define FT_READ_ID_L            (0x9F)



#define FT_MAX_WIDTH                   (410)
#define FT_MAX_HEIGHT                  (494)


typedef enum
{
    CTP_DOWN = 0,
    CTP_UP   = 1,
    CTP_MOVE = 2,
    CTP_RESERVE = 3,
} ctp_pen_state_enum;


// rotate to left with 90, 180, 270
// rotate to left with 360 for mirror
//#define FT_ROTATE_LEFT                 (90)

/* function and value-----------------------------------------------------------*/


static void ft3169_correct_pos(touch_msg_t ppos);
static rt_err_t write_reg(uint8_t reg, rt_uint8_t data);
static rt_err_t read_regs(rt_uint8_t reg, rt_uint8_t len, rt_uint8_t *buf);

static struct rt_i2c_bus_device *ft_bus = NULL;

static struct touch_drivers ft3169_driver;
/*检查定时器*/
static rt_timer_t ft3169_check_timer_handle = NULL;
/*恢复标志位*/
static uint8_t point_report_check_en = 0;
/*100ms计数*/
static uint8_t time_100ms_count = 0;
/*异常恢复标志位*/
static uint8_t abnormal_recovery = 0;

static rt_err_t write_reg(uint8_t reg, rt_uint8_t data)
{
    rt_int8_t res = 0;
    struct rt_i2c_msg msgs;
    rt_uint8_t buf[2] = {reg, data};

    msgs.addr  = FT_DEV_ADDR;    /* slave address */
    msgs.flags = RT_I2C_WR;        /* write flag */
    msgs.buf   = buf;              /* Send data pointer */
    msgs.len   = 2;

    if (rt_i2c_transfer(ft_bus, &msgs, 1) == 1)
    {
        res = RT_EOK;
    }
    else
    {
        res = -RT_ERROR;
    }
    return res;
}

static rt_err_t read_regs(rt_uint8_t reg, rt_uint8_t len, rt_uint8_t *buf)
{
    rt_int8_t res = 0;
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = FT_DEV_ADDR;    /* Slave address */
    msgs[0].flags = RT_I2C_WR;        /* Write flag */
    msgs[0].buf   = &reg;             /* Slave register address */
    msgs[0].len   = 1;                /* Number of bytes sent */

    msgs[1].addr  = FT_DEV_ADDR;    /* Slave address */
    msgs[1].flags = RT_I2C_RD;        /* Read flag */
    msgs[1].buf   = buf;              /* Read data pointer */
    msgs[1].len   = len;              /* Number of bytes read */

    if (rt_i2c_transfer(ft_bus, msgs, 2) == 2)
    {
        res = RT_EOK;
    }
    else
    {
        res = -RT_ERROR;
    }
    return res;
}



static void ft3169_correct_pos(touch_msg_t ppos)
{
    ppos->x = FT_MAX_WIDTH - ppos->x;
    if (ppos->x < 0)
    {
        ppos->x = 0;
    }

    ppos->y = FT_MAX_HEIGHT - ppos->y;
    if (ppos->y < 0)
    {
        ppos->y = 0;
    }
    return;
}


static rt_err_t read_point(touch_msg_t p_msg)
{
    int8_t ret = -1;
    rt_touch_irq_pin_enable(1);

    {
        uint8_t  point_data[2 + 6 * MAX_POINT_NUM] = {0};
        uint8_t  touch_num = 0;
        uint8_t  pressure = 0x0;
        int input_x = 0;
        int input_y = 0;
        int id = 0;
        uint8_t  eventFlag = 0;
        int i  = 0;

        ret = read_regs(0x01, 2 + 6 * MAX_POINT_NUM, point_data);
        if (ret < 0)
        {
            goto exit_work_func;
        }

        touch_num = point_data[1] & 0x0f;

        LOG_D("%s:touch_num:%d", __func__, touch_num);

        if (touch_num > MAX_POINT_NUM)
        {
            goto exit_work_func;
        }
        else if (touch_num == 0)
        {
            eventFlag = (point_data[2 + i * 6] >> 6) & 0x03;
            id  = point_data[4 + i * 6] >> 4;
            input_x = ((point_data[2 + i * 6] & 0x0F) << 8) + point_data[3 + i * 6];
            input_y = ((point_data[4 + i * 6] & 0x0F) << 8) + point_data[5 + i * 6];

            p_msg->event = TOUCH_EVENT_UP;
            p_msg->x = (uint16_t)input_x;
            p_msg->y = (uint16_t)input_y;

            LOG_D("%s %d :eventFlag:%d, id:%d, x:%d, y:%d", __func__, __LINE__, eventFlag, id, input_x, input_y);
        }

        for (i = 0; i < touch_num; i++)
        {
            eventFlag = (point_data[2 + i * 6] >> 6) & 0x03;
            id  = point_data[4 + i * 6] >> 4;
            input_x = ((point_data[2 + i * 6] & 0x0F) << 8) + point_data[3 + i * 6];
            input_y = ((point_data[4 + i * 6] & 0x0F) << 8) + point_data[5 + i * 6];
            LOG_D("%s %d :eventFlag:%d, id:%d, x:%d, y:%d", __func__, __LINE__, eventFlag, id, input_x, input_y);

            if (1 == abnormal_recovery)
            {
                abnormal_recovery = 0;
                p_msg->event = TOUCH_EVENT_UP;
                p_msg->x = (uint16_t)input_x;
                p_msg->y = (uint16_t)input_y;
                LOG_D("%s %d :ft3169 abnormal", __func__, __LINE__);
            }
            else if ((eventFlag == CTP_DOWN) || (eventFlag == CTP_MOVE))
            {
                p_msg->event = TOUCH_EVENT_DOWN;
                p_msg->x = (uint16_t)input_x;
                p_msg->y = (uint16_t)input_y;
                point_report_check_en = 1;
                time_100ms_count = 0;
            }
            else if (eventFlag == CTP_UP)
            {
                p_msg->event = TOUCH_EVENT_UP;
                p_msg->x = (uint16_t)input_x;
                p_msg->y = (uint16_t)input_y;
                point_report_check_en = 0;
                time_100ms_count = 0;
            }
        }
    }
exit_work_func:
    return RT_EEMPTY;
}


void ft3169_irq_handler(void *arg)
{
    rt_err_t ret = RT_ERROR;

    int value = (int)arg;
    //LOG_D("ft3169 touch_irq_handler\n");

    rt_touch_irq_pin_enable(0);
    ret = rt_sem_release(ft3169_driver.isr_sem);
    RT_ASSERT(RT_EOK == ret);
}

#if 0
/*****************************************************************************
* An interrupt handler, will be called while the timer interrupt trigger , the code based on your platform.
* The program use to check point report.
*****************************************************************************/
static void fts_timer_interrupt_handler(void)
{
    /* point report check */
    if (point_report_check_en == 1)
    {
        time_100ms_count++;
        if (time_100ms_count >= 2)
        {
            point_report_check_en = 0;
            time_100ms_count = 0;
            ft3169_release_all_finger();
        }


    }
    else
    {
        time_100ms_count = 0;
    }
}

/*****************************************************************************
* Initialize timer and set to interrupt mode which period is 100 msec
*****************************************************************************/
static int platform_interrupt_timer_init(void)
{

    /*Initialize timer and set to interrupt mode which period is 100 msec,
    and the corresponding interrupt function is fts_timer_interrupt_handler,
    you should implement it based on your platform*/



    return 0;
}
#endif

/*check abnormal timer ***********************************************************/

static void ft3169_check_timer_handler(void *param)
{
    /* point report check */
    LOG_D("%s %d", __func__, __LINE__);
    if (point_report_check_en == 1)
    {
        time_100ms_count++;
        if (time_100ms_count >= 2)
        {
            point_report_check_en = 0;
            time_100ms_count = 0;
            abnormal_recovery = 1;
            ft3169_irq_handler(NULL);
        }
    }
    else
    {
        time_100ms_count = 0;
        abnormal_recovery = 0;
    }
}

static void ft3169_check_timer_create(void)
{
    if (ft3169_check_timer_handle == NULL)
    {
        ft3169_check_timer_handle = rt_timer_create("ft3169_check", ft3169_check_timer_handler, 0, rt_tick_from_millisecond(100), RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
        RT_ASSERT(ft3169_check_timer_handle);
    }

}

static rt_err_t ft3169_check_timer_start(void)
{
    rt_err_t ret;
    if (ft3169_check_timer_handle)
    {
        ret = rt_timer_start(ft3169_check_timer_handle);
    }
    return ret;
}

static rt_err_t ft3169_check_timer_stop(void)
{
    rt_err_t ret;
    if (ft3169_check_timer_handle)
    {
        ret = rt_timer_stop(ft3169_check_timer_handle);
    }
    return ret;
}

static rt_err_t ft3169_check_timer_del(void)
{
    rt_err_t ret;
    if (ft3169_check_timer_handle)
    {
        ret = rt_timer_delete(ft3169_check_timer_handle);
        ft3169_check_timer_handle = NULL;
    }
    return ret;
}
/************************************************************/

static rt_err_t init(void)
{
    rt_err_t err;
    struct touch_message msg;

#ifdef RT_USING_PM
    rt_pm_request(PM_SLEEP_MODE_IDLE);
#endif

    BSP_TP_Reset(0);
    rt_thread_mdelay(5);
    BSP_TP_Reset(1);
    rt_thread_mdelay(80);



//    err = write_reg(FT_ID_G_MODE, 1);
//    if (RT_EOK != err)
//    {
//        LOG_E("G_MODE set fail\n");
//        //return RT_FALSE;
//    }

    {
        uint8_t chip_id[2] = { 0 };
        err = read_regs(FT_READ_ID_H, 1, &chip_id[0]);

        if (RT_EOK == err)
        {
            LOG_E("ft3169 id_H=%x", chip_id[0]);
        }

        err = read_regs(FT_READ_ID_L, 1, &chip_id[1]);

        if (RT_EOK == err)
        {
            LOG_E("ft3169 id_L=%x", chip_id[1]);
        }
    }

#ifdef RT_USING_PM
    rt_pm_release(PM_SLEEP_MODE_IDLE);
#endif

    rt_touch_irq_pin_attach(PIN_IRQ_MODE_FALLING, ft3169_irq_handler, NULL);
    rt_touch_irq_pin_enable(1);

    ft3169_check_timer_create();
    ft3169_check_timer_start();

    LOG_D("ft3169 init OK");

    return RT_EOK;

}

static rt_err_t deinit(void)
{
    LOG_D("ft3169 deinit");
    ft3169_check_timer_stop();
    ft3169_check_timer_del();

    rt_touch_irq_pin_enable(0);
    rt_touch_irq_pin_detach();
    return RT_EOK;
}

static rt_bool_t probe(void)
{
    ft_bus = (struct rt_i2c_bus_device *)rt_device_find(TOUCH_DEVICE_NAME);
    if (RT_Device_Class_I2CBUS != ft_bus->parent.type)
    {
        ft_bus = NULL;
    }
    if (ft_bus)
    {
        rt_device_open((rt_device_t)ft_bus, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    }
    else
    {
        LOG_I("bus not find\n");
        return RT_FALSE;
    }


    {
        struct rt_i2c_configuration configuration =
        {
            .mode = 0,
            .addr = 0,
            .timeout = 500,
            .max_hz  = 400000,
        };

        rt_i2c_configure(ft_bus, &configuration);
    }


    LOG_I("ft3169 probe OK");

    return RT_TRUE;
}


static struct touch_ops ops =
{
    read_point,
    init,
    deinit
};



static int rt_ft3169_init(void)
{

    ft3169_driver.probe = probe;
    ft3169_driver.ops = &ops;
    ft3169_driver.user_data = RT_NULL;
    ft3169_driver.isr_sem = rt_sem_create("ft3169", 0, RT_IPC_FLAG_FIFO);

    rt_touch_drivers_register(&ft3169_driver);

    return 0;

}
INIT_COMPONENT_EXPORT(rt_ft3169_init);

//#define FT3168_FUNC_TEST
#ifdef FT3168_FUNC_TEST

int cmd_ft_test(int argc, char *argv[])
{
    touch_data_t post = {0};
    int res, looper;

    if (argc > 1)
    {
        looper = atoi(argv[1]);
    }
    else
    {
        looper = 0x0fffffff;
    }

    if (NULL == ft_bus)
    {
        ft3169_init();
    }
    while (looper != 0)
    {
        res = touch_read(&post);
        if (post.state)
        {
            LOG_I("x = %d, y = %d", post.point.x, post.point.y);
        }

        looper--;
        rt_thread_delay(100);
    }
    return 0;
}

FINSH_FUNCTION_EXPORT_ALIAS(cmd_ft_test, __cmd_ft_test, Test hw ft3169);
#endif  /* ADS7846_FUNC_TEST */

/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/
