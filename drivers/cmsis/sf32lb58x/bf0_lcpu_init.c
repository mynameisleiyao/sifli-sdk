/**
  ******************************************************************************
  * @file   bf0_lcpu_init.c
  * @author Sifli software development team
  * @brief
 * @{
  ******************************************************************************
*/
/**
 * @attention
 * Copyright (c) 2019 - 2025,  Sifli Technology
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

#include <rtconfig.h>
#include <bf0_hal.h>
#include <string.h>

#if defined (APP_BSP_TEST)
    #define bt_rf_cal()
#else
    #if defined(FPGA)
        #define bt_rf_cal() bt_rf_cal_9364()
    #endif
    extern void bt_rf_cal(void);
#endif

#if defined(LCPU_RUN_SEPERATE_IMG)
    #define lcpu_patch_install()
#else
    extern void lcpu_patch_install();
#endif

#if defined(SOC_BF0_HCPU)

static uint8_t g_lcpu_rf_cal_disable;

__WEAK void adc_resume(void)
{
}
#ifdef LCPU_CONFIG_V2
void lcpu_rom_config_default(void)
{
    uint8_t rev_id = __HAL_SYSCFG_GET_REVID();

#ifdef LXT_DISABLE
    uint8_t is_enable_lxt = 0;
    uint8_t is_lcpu_rccal = 1;
#else // !LXT_DISABLE
    uint8_t is_enable_lxt = 1;
    uint8_t is_lcpu_rccal = 0;
#endif // LXT_DISABLE
    uint32_t wdt_staus = 0xFF;
    uint32_t wdt_time = 10;
    uint16_t wdt_clk = 32768;

    HAL_LCPU_CONFIG_set(HAL_LCPU_CONFIG_XTAL_ENABLED, &is_enable_lxt, 1);
    HAL_LCPU_CONFIG_set(HAL_LCPU_CONFIG_WDT_STATUS, &wdt_staus, 4);
    HAL_LCPU_CONFIG_set(HAL_LCPU_CONFIG_WDT_TIME, &wdt_time, 4);
    HAL_LCPU_CONFIG_set(HAL_LCPU_CONFIG_WDT_CLK_FEQ, &wdt_clk, 2);
    HAL_LCPU_CONFIG_set(HAL_LCPU_CONFIG_BT_RC_CAL_IN_L, &is_lcpu_rccal, 1);

    {
        uint32_t tx_queue = HCPU2LCPU_MB_CH1_BUF_START_ADDR;
        hal_lcpu_bluetooth_rom_config_t config = {0};
        config.bit_valid |= 1 << 10 | 1 << 6;
#ifdef FPGA
        config.is_fpga = 1;
#else
        config.is_fpga = 0;
#endif
        config.default_xtal_enabled = is_enable_lxt;
        //config.sco_cfg = 0;
        HAL_LCPU_CONFIG_set(HAL_LCPU_CONFIG_HCPU_TX_QUEUE, &tx_queue, 4);
        HAL_LCPU_CONFIG_set(HAL_LCPU_CONFIG_BT_CONFIG, &config, sizeof(config));
    }

    {
        hal_lcpu_bluetooth_actmove_config_t act_cfg;
        act_cfg.bit_valid = 1;
        act_cfg.act_mov = 0x10;
        HAL_LCPU_CONFIG_set(HAL_LCPU_CONFIG_BT_ACTMOVE_CONFIG, &act_cfg, LCPU_CONFIG_BT_ACTMOVE_ROM_LENGTH);
    }
}
#endif // LCPU_CONFIG_V2
__WEAK void lcpu_rom_config(void)
{
#ifdef LCPU_CONFIG_V2
    lcpu_rom_config_default();
#endif
}

#if defined(LCPU_RUN_ROM_ONLY)
#define lcpu_img_install()
#else
__WEAK void lcpu_img_install(void)
{
}
#endif

static void lcpu_ble_patch_install()
{
    memset((void *)0x204F0000, 0, 0x2000);
#if !defined(LCPU_RUN_SEPERATE_IMG)
    lcpu_patch_install();
#endif
    if (g_lcpu_rf_cal_disable == 0)
        bt_rf_cal();

    adc_resume();
}

void lcpu_disable_rf_cal(uint8_t is_disable)
{
    g_lcpu_rf_cal_disable = is_disable;
}



uint8_t lcpu_power_on(void)
{
    HAL_HPAON_WakeCore(CORE_ID_LCPU);
    HAL_RCC_Reset_and_Halt_LCPU(0);

    lcpu_rom_config();
    lcpu_img_install();

    HAL_LPAON_ConfigStartAddr((uint32_t *)HCPU_LCPU_CODE_START_ADDR);
    lcpu_ble_patch_install();
    HAL_RCC_ReleaseLCPU();
    return 0;
}

uint8_t lcpu_power_off(void)
{
    HAL_RCC_Reset_and_Halt_LCPU(0);
    return 0;
}

#else
uint8_t lcpu_power_on(void)
{
    return 0;
}
#endif /* SOC_BF0_HCPU */


/** @} */


/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/
