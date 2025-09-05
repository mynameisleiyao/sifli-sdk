/**
  ******************************************************************************
  * @file   drv_dbg.c
  * @author Sifli software development team
  * @brief Debug functions for BSP driver
  * @{
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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef _WIN32
    #include <windows.h>
#endif

#include <drv_log.h>
#include <drv_common.h>
#include "rthw.h"
#ifndef _WIN32
    #include "bf0_hal_pinmux.h"
    #include "bf0_pin_const.h"
#endif
#include "bf0_hal_rf_cw.h"

#ifdef BSP_USING_BUSMON

static BUSMON_HandleTypeDef BusmonHandle;
static PTC_HandleTypeDef    PtcHandle;
static void(*busmon_callback)(void);

#ifdef SOC_BF0_HCPU
    void PTC1_IRQHandler(void)
#else
    void PTC2_IRQHandler(void)
#endif
{
    rt_interrupt_enter();
    HAL_PTC_IRQHandler(&PtcHandle);
    rt_interrupt_leave();
    if (busmon_callback)
        (*busmon_callback)();
}

void dbg_busmon_reg_callback(void(*callback)(void))
{
    busmon_callback = callback;
}



void dbg_busmon(HAL_BUSMON_FuncTypeDef func, uint32_t address, uint32_t address_end, uint8_t ishcpu, uint32_t count, uint8_t access)
{
    /*##-1- Initialize Bus Monitor #######################################*/
    BusmonHandle.Instance = ishcpu ? hwp_busmon1 : hwp_busmon2; // HCPU using busmon 1
    BusmonHandle.Init.Channel = HAL_BUSMON_CHN_UNASSIGN;        // Use Channel 5 available for OPSRAM
    BusmonHandle.Init.Flags = access;                           // read and/or write access
    BusmonHandle.Init.SelFunc = func;                           // Monitor for OPSRAM
    BusmonHandle.Init.Max = address_end;                        // Address range max
    BusmonHandle.Init.Min = address;                            // Address range min
    BusmonHandle.Init.count = count;                            // Count to generate interrupt
    HAL_RCC_EnableModule(ishcpu ? RCC_MOD_BUSMON1 : RCC_MOD_BUSMON2);

    if (HAL_BUSMON_Init(&BusmonHandle) != HAL_OK)               // Initialize Busmon
    {
        /* Initialization Error */
        RT_ASSERT(RT_FALSE);
    }

    rt_kprintf("Channel=%d, func=%d, count=%d, address=0x%08X\n", BusmonHandle.Init.Channel, func, count, address);

    PtcHandle.Instance = ishcpu ? hwp_ptc1 : hwp_ptc2;
    PtcHandle.Init.Channel = 0;                                 // Use PTC Channel 0
    PtcHandle.Init.Address = (uint32_t) & (hwp_busmon1->CCR);   // Bus monitor clear register
    PtcHandle.Init.data = (1 << BusmonHandle.Init.Channel);     // data to handle with value in Address.
    PtcHandle.Init.Operation = PTC_OP_OR;                       // Or and write back
    PtcHandle.Init.Sel = PTC_HCPU_BUSMON1_OF1 + BusmonHandle.Init.Channel;   // Busmon OF channel.
#ifdef SOC_BF0_HCPU
    NVIC_EnableIRQ(PTC1_IRQn);
    HAL_RCC_EnableModule(RCC_MOD_PTC1);
#else
    NVIC_EnableIRQ(PTC2_IRQn);
    HAL_RCC_EnableModule(RCC_MOD_PTC2);
#endif
    if (HAL_PTC_Init(&PtcHandle) != HAL_OK)                     // Initialize PTC
    {
        /* Initialization Error */
        RT_ASSERT(RT_FALSE);
    }
    HAL_PTC_Enable(&PtcHandle, 1);                              // Enable PTC
    HAL_BUSMON_Enable(&BusmonHandle, 1);                        // Enable busmon
}

void dbg_busmon_psram(uint32_t address, uint32_t count, uint8_t access)
{
    BusmonHandle.Instance = hwp_busmon1;                        // PSRAM using busmon 1
    BusmonHandle.Init.Channel = HAL_BUSMON_CHN_UNASSIGN;        // Use Channel 5 available for OPSRAM
    BusmonHandle.Init.Flags = access;                           // read and/or write access
#ifdef SF32LB55X
    BusmonHandle.Init.SelFunc = HAL_BUSMON_OPSRAM;              // Monitor for OPSRAM
#else
    RT_ASSERT(0);//Not support 'HAL_BUSMON_OPSRAM', Fix me!
#endif /* SF32LB55X */
    BusmonHandle.Init.Max = address + 4;                        // Address range max
    BusmonHandle.Init.Min = address;                            // Address range min
    BusmonHandle.Init.count = count;                            // Count to generate interrupt

    HAL_RCC_EnableModule(RCC_MOD_PTC1);
    HAL_RCC_EnableModule(RCC_MOD_BUSMON1);
    if (HAL_BUSMON_Init(&BusmonHandle) != HAL_OK)               // Initialize Busmon
    {
        /* Initialization Error */
        RT_ASSERT(RT_FALSE);
    }

    rt_kprintf("Channel=%d, count=%d, address=0x%08X\n", BusmonHandle.Init.Channel, count, address);

    PtcHandle.Instance = hwp_ptc1;
    PtcHandle.Init.Channel = 0;                                 // Use PTC Channel 0
    PtcHandle.Init.Address = (uint32_t) & (hwp_busmon1->CCR);   // Bus monitor clear register
    PtcHandle.Init.data = (1 << BusmonHandle.Init.Channel);     // data to handle with value in Address.
    PtcHandle.Init.Operation = PTC_OP_OR;                       // Or and write back
    PtcHandle.Init.Sel = PTC_HCPU_BUSMON1_OF1 + BusmonHandle.Init.Channel;   // Busmon OF channel.
    NVIC_EnableIRQ(PTC1_IRQn);
    if (HAL_PTC_Init(&PtcHandle) != HAL_OK)                     // Initialize PTC
    {
        /* Initialization Error */
        RT_ASSERT(RT_FALSE);
    }
    HAL_PTC_Enable(&PtcHandle, 1);                              // Enable PTC
    HAL_BUSMON_Enable(&BusmonHandle, 1);                        // Enable busmon
}

#endif


#ifdef RT_USING_FINSH

#define REG_LOCK_WORD "0000"

uint32_t g_reg_lock;

__WEAK int regop_lock_check(char *passwd, uint32_t len)
{
    int ret = -1;
    if (strncmp((const char *)passwd, REG_LOCK_WORD, sizeof(REG_LOCK_WORD)) == 0)
        ret = 0;
    return ret;
}


static void usage(void)
{
    LOG_E("Usage: regop read <address> <len in word(32-bit)>\n");
    LOG_E("Usage: regop write <address> <data in big-endian>\n");
    LOG_E("Usage: regop write4l <address> <data in little-endian, max 4 bytes>\n");
}


#ifdef BSP_USING_BUSMON
void busmon_cbk()
{
    rt_kprintf("Busmon captured\n");
}
#endif

__ROM_USED void sf_register_access(int argc, char **argv)
{
    int mode = 0;
    if (argc < 3)
    {
        usage();
#ifdef DEBUG_HAL
        dwtIpInit();
#endif
    }
    else
    {
        uint32_t *address = (uint32_t *)atoh(argv[2]);
        uint32_t i;
        if (strcmp(argv[1], "read") == 0)
        {
            if (!g_reg_lock)
            {
                rt_kprintf("Locked!\r\n");
                return;
            }
            uint32_t len = atoh(argv[3]);
            for (i = 0; i < len; i++)
            {
                rt_kprintf("%08X ", *(address + i));
                if (((i + 1) % 4) == 0)
                    rt_kprintf("\n");
            }
            if (len % 4)
                rt_kprintf("\n");
        }
        else if (strcmp(argv[1], "write") == 0)
        {
#define  MAX_WRITE_LEN  16
            if (!g_reg_lock)
            {
                rt_kprintf("Locked!\r\n");
                return;
            }
            static uint32_t data[MAX_WRITE_LEN];
            memset(data, 0, MAX_WRITE_LEN);
            uint32_t len = hex2data(argv[3], (uint8_t *)data, MAX_WRITE_LEN * sizeof(uint32_t));
            if (len > MAX_WRITE_LEN * sizeof(uint32_t))
                len = MAX_WRITE_LEN * sizeof(uint32_t);
            for (i = 0; i < (len + 3) / sizeof(uint32_t); i++)
                *(address + i) = data[i];
        }
        else if (strcmp(argv[1], "write4l") == 0)
        {
            if (!g_reg_lock)
            {
                rt_kprintf("Locked!\r\n");
                return;
            }
            *address = strtoul(argv[3], NULL, 16);
        }
#if defined(RT_DEBUG)
#ifndef _WIN32
        else if (strcmp(argv[1], "debug") == 0)
        {
            char *data = "Sifli";
            HAL_PIN_Set(PAD_PA02, GPIO_A2, PIN_PULLUP, 0);
            HAL_PIN_Set(PAD_PA03, GPIO_A3, PIN_PULLUP, 0);
            HAL_DBG_i2c_pins(GPIO_A2, GPIO_A3);
            HAL_DBG_i2c_print((uint8_t *)data, strlen(data));
        }
#endif
#endif
#ifdef HAL_DEBUG_ENABLED
        else if (strcmp(argv[1], "delay") == 0)
        {
            rt_base_t level = rt_hw_interrupt_disable();
            uint32_t delay, us;
            us = atoi(argv[2]);
            //HAL_RCC_HCPU_ClockSelect(RCC_CLK_MOD_SYS, RCC_SYSCLK_HXT48);
            //HAL_Delay_us(0);
            __disable_irq();
            HAL_DBG_DWT_Reset();
            HAL_Delay_us(us);
            delay = HAL_DBG_DWT_GetCycles();
            __enable_irq();
            rt_kprintf("Cycle=%d, %f\n", delay, (float)delay / us);
        }
#endif
        else if (strcmp(argv[1], "unlock") == 0)
        {
            if (regop_lock_check(argv[2], sizeof(argv[2])) == 0)
            {
                g_reg_lock = 1;
                rt_kprintf("Unlock!\r\n");
            }
        }
#ifdef BSP_USING_BUSMON
        else if (strcmp(argv[1], "busmon") == 0)
        {
            uint32_t address = atoh(argv[3]);
            uint32_t count = 1;

            dbg_busmon_reg_callback(busmon_cbk);
            if (argc > 4)
                count = atoi(argv[4]);
            if (strcmp(argv[2], "r") == 0)
                dbg_busmon_read(address, count);
            else if (strcmp(argv[2], "w") == 0)
                dbg_busmon_write(address, count);
            else if (strcmp(argv[2], "a") == 0)
                dbg_busmon_access(address, count);
            else if (strcmp(argv[2], "pr") == 0)
                dbg_busmon_psram(address, count, BUSMON_OPFLAG_READ);
            else if (strcmp(argv[2], "pw") == 0)
                dbg_busmon_psram(address, count, BUSMON_OPFLAG_WRITE);
            else if (strcmp(argv[2], "pa") == 0)
                dbg_busmon_psram(address, count, BUSMON_OPFLAG_RW);
        }
#endif
        else
            usage();
    }
}
MSH_CMD_EXPORT_ALIAS(sf_register_access, regop, Register read / write);

#if defined(SOC_BF0_HCPU)
#if defined(SOC_SF32LB55X)
static void pr_usage(void)
{
    rt_kprintf("Usage: pwr_ctrl setmode <mode: 0:power class 3 - 0dbm, 1:power class 2 - 4dbm, 2:power class 1.5 - 10dbm>\r\n");
}
static void bt_power_command(int argc, char **argv)
{
    if (strcmp(argv[1], "setmode") == 0)
    {
        uint8_t mode = atoi(argv[2]);
        switch (mode)
        {
        case 0:
        {
            // Power class 3
            hwp_ble_rfc->TRF_REG1 &= ~BLE_RF_DIG_TRF_REG1_BRF_PA_PM_LV_Msk;
            hwp_ble_rfc->TRF_REG1 &= ~BLE_RF_DIG_TRF_REG1_BRF_PA_CAS_BP_LV_Msk;

            hwp_ble_rfc->TRF_REG2 &= ~BLE_RF_DIG_TRF_REG2_BRF_PA_UNIT_SEL_LV_Msk;
            hwp_ble_rfc->TRF_REG2 &= ~BLE_RF_DIG_TRF_REG2_BRF_PA_MCAP_LV_Msk;

            hwp_ble_rfc->TRF_REG1 |= 0x01 << BLE_RF_DIG_TRF_REG1_BRF_PA_PM_LV_Pos;
            hwp_ble_rfc->TRF_REG1 |= 0x01 << BLE_RF_DIG_TRF_REG1_BRF_PA_CAS_BP_LV_Pos;

            hwp_ble_rfc->TRF_REG2 |= 0x01 << BLE_RF_DIG_TRF_REG2_BRF_PA_UNIT_SEL_LV_Pos;
            hwp_ble_rfc->TRF_REG2 |= 0x0 << BLE_RF_DIG_TRF_REG2_BRF_PA_MCAP_LV_Pos;

            hwp_ble_phy->TX_PA_CFG &= ~BLE_PHY_TX_PA_CFG_PA_CTRL_TARGET_Msk;
            hwp_ble_phy->TX_PA_CFG |= 0x3E << BLE_PHY_TX_PA_CFG_PA_CTRL_TARGET_Pos;
            break;
        }
        case 1:
        {
            // Power class 2
            hwp_ble_rfc->TRF_REG1 &= ~BLE_RF_DIG_TRF_REG1_BRF_PA_PM_LV_Msk;
            hwp_ble_rfc->TRF_REG1 &= ~BLE_RF_DIG_TRF_REG1_BRF_PA_CAS_BP_LV_Msk;

            hwp_ble_rfc->TRF_REG2 &= ~BLE_RF_DIG_TRF_REG2_BRF_PA_UNIT_SEL_LV_Msk;
            hwp_ble_rfc->TRF_REG2 &= ~BLE_RF_DIG_TRF_REG2_BRF_PA_MCAP_LV_Msk;

            hwp_ble_rfc->TRF_REG1 |= 0x00 << BLE_RF_DIG_TRF_REG1_BRF_PA_PM_LV_Pos;
            hwp_ble_rfc->TRF_REG1 |= 0x01 << BLE_RF_DIG_TRF_REG1_BRF_PA_CAS_BP_LV_Pos;

            hwp_ble_rfc->TRF_REG2 |= 0x1F << BLE_RF_DIG_TRF_REG2_BRF_PA_UNIT_SEL_LV_Pos;
            hwp_ble_rfc->TRF_REG2 |= 0x01 << BLE_RF_DIG_TRF_REG2_BRF_PA_MCAP_LV_Pos;

            hwp_ble_phy->TX_PA_CFG &= ~BLE_PHY_TX_PA_CFG_PA_CTRL_TARGET_Msk;
            hwp_ble_phy->TX_PA_CFG |= 0x3D << BLE_PHY_TX_PA_CFG_PA_CTRL_TARGET_Pos;
            break;
        }
        case 2:
        {
            // Power class 1.5
            hwp_ble_rfc->TRF_REG1 &= ~BLE_RF_DIG_TRF_REG1_BRF_PA_PM_LV_Msk;
            hwp_ble_rfc->TRF_REG1 &= ~BLE_RF_DIG_TRF_REG1_BRF_PA_CAS_BP_LV_Msk;


            hwp_ble_rfc->TRF_REG2 &= ~BLE_RF_DIG_TRF_REG2_BRF_PA_UNIT_SEL_LV_Msk;
            hwp_ble_rfc->TRF_REG2 &= ~BLE_RF_DIG_TRF_REG2_BRF_PA_MCAP_LV_Msk;

            hwp_ble_rfc->TRF_REG1 |= 0x02 << BLE_RF_DIG_TRF_REG1_BRF_PA_PM_LV_Pos;
            hwp_ble_rfc->TRF_REG1 |= 0x00 << BLE_RF_DIG_TRF_REG1_BRF_PA_CAS_BP_LV_Pos;


            hwp_ble_rfc->TRF_REG2 |= 0x1F << BLE_RF_DIG_TRF_REG2_BRF_PA_UNIT_SEL_LV_Pos;
            hwp_ble_rfc->TRF_REG2 |= 0x01 << BLE_RF_DIG_TRF_REG2_BRF_PA_MCAP_LV_Pos;

            hwp_ble_phy->TX_PA_CFG &= ~BLE_PHY_TX_PA_CFG_PA_CTRL_TARGET_Msk;
            hwp_ble_phy->TX_PA_CFG |= 0x3E << BLE_PHY_TX_PA_CFG_PA_CTRL_TARGET_Pos;
            break;
        }
        default:
        {
            rt_kprintf("Unsupported power class mode\r\n");
            break;
        }
        }

    }
    else if (strcmp(argv[1], "settxpwr") == 0)
    {
        int8_t txpwr = atoi(argv[2]);
        rt_kprintf("set txpwr %d", txpwr);

        extern void ble_rf_tx_power_set(int8_t txpwr);
        ble_rf_tx_power_set(txpwr);
    }
    else
        pr_usage();

}
#elif defined(SOC_SF32LB58X) || defined(SOC_SF32LB56X) || defined(SOC_SF32LB52X)
RT_WEAK uint8_t btdm_rf_power_set(uint8_t type, int8_t txpwr)
{
    return 0xFF;
}
static void bt_power_command(int argc, char **argv)
{
    if (strcmp(argv[1], "set_pwr") == 0)
    {
        int8_t txpwr = atoi(argv[3]);
        uint8_t ret = 0xFF;
        if (strcmp(argv[2], "blebr") == 0)
        {
            //blebr_rf_power_set(txpwr);
            ret = btdm_rf_power_set(0, txpwr);

        }
        else if (strcmp(argv[2], "edr") == 0)
        {
            //edr_rf_power_set(txpwr);
            ret = btdm_rf_power_set(1, txpwr);
        }

        rt_kprintf("set txpwr %d, stat %d\r\n", txpwr, ret);
    }
}

#else
static void bt_power_command(int argc, char **argv)
{
    rt_kprintf("Not support\r\n");
}
#endif

MSH_CMD_EXPORT_ALIAS(bt_power_command, pwr_ctrl, BT TX power adjust)
#endif // SOC_BF0_HCPU


static void assert_command(int argc, char **argv)
{
    RT_ASSERT(0);
}


MSH_CMD_EXPORT_ALIAS(assert_command, assert, Force assert);



#if defined(SOC_SF32LB58X) || defined(SOC_SF32LB56X)
static void switch_jlink_to_lcpu(int argc, char **argv)
{
    uint8_t is_to_lcpu = atoi(argv[1]);
    if (is_to_lcpu)
        hwp_lpsys_aon->PMR |= LPSYS_AON_PMR_FORCE_LCPU;
    else
        hwp_lpsys_aon->PMR &= ~LPSYS_AON_PMR_FORCE_LCPU_Msk;
}
MSH_CMD_EXPORT_ALIAS(switch_jlink_to_lcpu, swd_t_l, Force switch jlink to LCPU);
#endif // defined(SOC_SF32LB58X) || defined(SOC_SF32LB56X)

#if defined(SF32LB52X)
extern void msh_set_console(const char *name);
static void dbguart_to_jlink(int argc, char **argv)
{
    uint8_t to_jlink = atoi(argv[1]);

    uint32_t *pa18_pinmux = (uint32_t *)0x5000307c; //PAD_PA18
    uint32_t *pa19_pinmux = (uint32_t *)0x50003080; //PAD_PA19

    rt_kprintf("app_setting_dlvp_set_jlink_uart_switch(PA18=%x,PA19=%x) \r\n", *pa18_pinmux, *pa19_pinmux);

    if (to_jlink) // ON
    {
        rt_kprintf("Switch to Jlink \r\n");
        //HAL_PIN_Set(PAD_PA18, SWDIO, PIN_PULLDOWN, 1);
        //HAL_PIN_Set(PAD_PA19, SWCLK, PIN_PULLDOWN, 1);
        //HAL_PIN_SetMode(PAD_PA18,1,PIN_DIGITAL_IO_PULLDOWN);
        //HAL_PIN_SetMode(PAD_PA19,1,PIN_DIGITAL_IO_PULLDOWN);

        *pa18_pinmux = 0x2D2;
        *pa19_pinmux = 0x2D2;

#ifdef FINSH_USING_MSH
        msh_set_console("segger");
#endif /* FINSH_USING_MSH */
    }
    else //off
    {
        rt_kprintf("Switch to DBG UART \r\n");
        //HAL_PIN_Set(PAD_PA18, USART1_RXD, PIN_PULLUP, 1);
        //HAL_PIN_Set(PAD_PA19, USART1_TXD, PIN_PULLUP, 1);
        *pa18_pinmux = 0x2F4;
        *pa19_pinmux = 0x2B4;

#ifdef FINSH_USING_MSH
        msh_set_console("uart1");
#endif /* FINSH_USING_MSH */
    }

}
MSH_CMD_EXPORT_ALIAS(dbguart_to_jlink, dbguart2jlink, Switch debug uart to jlink);
#endif /* SF32LB52X */

#endif // RT_USING_FINSH



/// @} drv_dbg
/// @} bsp_driver
/// @} file

/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/
