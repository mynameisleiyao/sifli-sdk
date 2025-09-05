/**
  ******************************************************************************
  * @file   bf0_hal_adc.c
  * @author Sifli software development team
  * @brief   This file provides firmware functions to manage the following
  *          functionalities of the Analog to Digital Convertor (ADC)
  ******************************************************************************
*/
// 以上是文件头部注释，说明该文件名为bf0_hal_adc.c，由Sifli软件开发团队开发，
// 作用是提供固件函数来管理模数转换器（ADC）的相关功能

/**
 *
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
// 以上是版权和许可信息，版权归2019-2022年Sifli Technology所有，
// 详细规定了该软件在源文件和二进制形式下的再分发和使用条件，
// 包括必须保留版权声明、仅能与Sifli集成电路一起使用、禁止反向工程等，
// 同时声明软件按"现状"提供，不承担相关担保责任等。

#include "bf0_hal.h"
// 包含头文件bf0_hal.h

/** @addtogroup BF0_HAL_Driver
  * @{
  */
// 向BF0_HAL_Driver模块组添加内容，{表示开始

/** @defgroup ADC Analog Digital Converter
  * @brief ADC HAL module driver
  * @{
  */
// 定义ADC模块组，说明是ADC的HAL模块驱动，{表示开始

#ifdef HAL_ADC_MODULE_ENABLED
// 如果HAL_ADC_MODULE_ENABLED宏被定义，则执行以下内容


/* Private typedef -----------------------------------------------------------*/
// 私有类型定义部分

/* Private define ------------------------------------------------------------*/
/** @defgroup ADC_Private_Constants ADC Private Constants
  * @{
  */


/**
    * @}
    */
// ADC_Private_Constants模块组结束

/* Private macro -------------------------------------------------------------*/
// 私有宏定义部分

/* Private variables ---------------------------------------------------------*/
// 私有变量定义部分

/* Private function prototypes -----------------------------------------------*/
/** @defgroup ADC_Private_Functions ADC Private Functions
  * @{
  */


static void ADC_DMAConvCplt(DMA_HandleTypeDef *hdma);
// 声明静态私有函数ADC_DMAConvCplt，参数为DMA句柄指针
static void ADC_DMAError(DMA_HandleTypeDef *hdma);
// 声明静态私有函数ADC_DMAError，参数为DMA句柄指针

/**
    * @}
    */
// ADC_Private_Functions模块组结束

/* Exported functions ---------------------------------------------------------*/
// 导出函数部分

/** @defgroup ADC_Exported_Functions ADC Exported Functions
  * @{
  */
// 定义ADC_Exported_Functions模块组，{表示开始

/** @defgroup ADC_Exported_Functions_Group1 Initialization/de-initialization functions
 *  @brief    Initialization and Configuration functions
 *
@verbatim
 ===============================================================================
              ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize and configure the ADC.
      (+) De-initialize the ADC
@endverbatim
  * @{
  */
// 定义ADC_Exported_Functions_Group1模块组，属于初始化和反初始化函数组，
// 简要说明是初始化和配置函数，verbatim块内详细说明该部分函数的功能：初始化和配置ADC、反初始化ADC，
// {表示开始

/**
  * @brief  Initializes the ADC peripheral and regular group according to
  *         parameters specified in structure "ADC_InitTypeDef".
  * @note   As prerequisite, ADC clock must be configured at RCC top level
  *         depending on both possible clock sources: APB clock of HSI clock.
  * @note   Possibility to update parameters on the fly:
  *         The setting of these parameters is conditioned to ADC state.
  *         For parameters constraints, see comments of structure
  *         "ADC_InitTypeDef".
  * @note   This function configures the ADC within 2 scopes: scope of entire
  *         ADC and scope of regular group. For parameters details, see comments
  *         of structure "ADC_InitTypeDef".
  * @param  hadc ADC handle
  * @retval HAL status
  */
// 函数说明：根据"ADC_InitTypeDef"结构体中指定的参数初始化ADC外设和常规组，
// 注意事项包括ADC时钟必须在RCC顶层配置、可动态更新参数（取决于ADC状态）、配置范围为整个ADC和常规组，
// 参数为ADC句柄hadc，返回值为HAL状态

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef *hadc)
// 定义函数HAL_ADC_Init，使用__HAL_ROM_USED修饰，返回值类型为HAL_StatusTypeDef，参数为ADC_HandleTypeDef类型指针hadc
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;
    // 定义HAL状态变量tmp_hal_status并初始化为HAL_OK
    uint32_t i;
    // 定义无符号32位整数i

    /* Check ADC handle */
    // 检查ADC句柄
    if (hadc == NULL)
    // 如果hadc为NULL
    {
        return HAL_ERROR;
        // 返回HAL_ERROR
    }

    /* Actions performed only if ADC is coming from state reset:                */
    /* - ADC voltage regulator enable                                           */
    // 仅当ADC处于复位状态时执行以下操作：使能ADC电压调节器
    if (hadc->State == HAL_ADC_STATE_RESET)
    // 如果hadc的State成员等于HAL_ADC_STATE_RESET
    {
        /* Initialize ADC error code */
        // 初始化ADC错误代码
        ADC_CLEAR_ERRORCODE(hadc);
        // 调用宏ADC_CLEAR_ERRORCODE清除hadc的错误代码

        /* Allocate lock resource and initialize it */
        // 分配锁资源并初始化
        hadc->Lock = HAL_UNLOCKED;
        // 将hadc的Lock成员设置为HAL_UNLOCKED（未锁定）
    }

    /* Initial ADC Msp */
    // 初始化ADC的Msp（MCU特定外设初始化）
    HAL_ADC_MspInit(hadc);
    // 调用HAL_ADC_MspInit函数，参数为hadc

    /* Configuration of ADC parameters if previous preliminary actions are      */
    /* correctly completed.                                                     */
    /* and if there is no conversion on going on regular group (ADC can be      */
    /* enabled anyway, in case of call of this function to update a parameter   */
    /* on the fly).                                                             */
    // 如果之前的预备操作正确完成，且常规组没有正在进行的转换（无论如何都可以使能ADC，以便在动态更新参数时调用此函数），则配置ADC参数
    if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL) &&
            (tmp_hal_status == HAL_OK))
    // 如果hadc的State成员中HAL_ADC_STATE_ERROR_INTERNAL位未设置，且tmp_hal_status为HAL_OK
    {
        /* Set ADC state */
        // 设置ADC状态
        ADC_STATE_CLR_SET(hadc->State,
                          HAL_ADC_STATE_REG_BUSY,
                          HAL_ADC_STATE_BUSY_INTERNAL);
        // 调用宏ADC_STATE_CLR_SET，清除hadc->State中的HAL_ADC_STATE_REG_BUSY位，设置HAL_ADC_STATE_BUSY_INTERNAL位

        /* TODO , Init ADC configure register for analog */
        // 待办事项，初始化ADC模拟配置寄存器
        //hadc->Instance->ADC_CFG_REG1 = ;
        // 注释掉的代码，用于设置ADC_CFG_REG1寄存器

        /* Set single channel */
        // 设置单通道
        hadc->Instance->ADC_CFG_REG1 |= GPADC_ADC_CFG_REG1_ANAU_GPADC_SE;
        // 向hadc->Instance的ADC_CFG_REG1寄存器写入GPADC_ADC_CFG_REG1_ANAU_GPADC_SE，即设置为单端模式

        i = hadc->Instance->ADC_CTRL_REG;
        // 将hadc->Instance的ADC_CTRL_REG寄存器值赋给i
        // disable all triger mode by default
        // 默认禁用所有触发模式
        i &= (~(GPADC_ADC_CTRL_REG_GPIO_TRIG_EN | GPADC_ADC_CTRL_REG_TIMER_TRIG_EN | GPADC_ADC_CTRL_REG_INIT_TIME));
        // 清除i中GPADC_ADC_CTRL_REG_GPIO_TRIG_EN、GPADC_ADC_CTRL_REG_TIMER_TRIG_EN、GPADC_ADC_CTRL_REG_INIT_TIME对应的位
        // set init time to max
        // 将初始化时间设置为最大
#if (GPADC_CALIB_FLOW_VERSION == 1)
        // 如果GPADC_CALIB_FLOW_VERSION宏等于1
        i |= GPADC_ADC_CTRL_REG_INIT_TIME;
        // 向i中设置GPADC_ADC_CTRL_REG_INIT_TIME位
#else
        // 否则
        i |= (8 << GPADC_ADC_CTRL_REG_INIT_TIME_Pos);
        // 将8左移GPADC_ADC_CTRL_REG_INIT_TIME_Pos位后的值写入i
#endif
        hadc->Instance->ADC_CTRL_REG = i;
        // 将i的值赋给hadc->Instance的ADC_CTRL_REG寄存器
#if (GPADC_CALIB_FLOW_VERSION != 1)
        // 如果GPADC_CALIB_FLOW_VERSION宏不等于1
        __HAL_ADC_SET_SAMPLE_WIDTH(hadc, hadc->Init.sample_width);
        // 调用宏__HAL_ADC_SET_SAMPLE_WIDTH设置ADC的采样宽度，参数为hadc和hadc->Init的sample_width成员
        __HAL_ADC_SET_CONV_WIDTH(hadc, hadc->Init.conv_width);
        // 调用宏__HAL_ADC_SET_CONV_WIDTH设置ADC的转换宽度，参数为hadc和hadc->Init的conv_width成员
        __HAL_ADC_SET_DATA_DELAY(hadc, hadc->Init.data_samp_delay);
        // 调用宏__HAL_ADC_SET_DATA_DELAY设置ADC的数据延迟，参数为hadc和hadc->Init的data_samp_delay成员
        // TODO: Add for Range
        // 待办事项，添加范围相关设置
#else
        // 否则（即GPADC_CALIB_FLOW_VERSION等于1）
        /* Set ADC clock, clk = 12/(div+1) */
        // 设置ADC时钟，时钟频率为12/(div+1)
        __HAL_ADC_SET_CLOCK_DIV(hadc, hadc->Init.clk_div);
        // 调用宏__HAL_ADC_SET_CLOCK_DIV设置ADC的时钟分频，参数为hadc和hadc->Init的clk_div成员
        // set range
        // 设置范围
        if (hadc->Init.atten3)  //enable 3X mode
        // 如果hadc->Init的atten3成员为真（使能3倍模式）
            hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_ATTN3X);
            // 向hadc->Instance的ADC_CFG_REG1寄存器写入GPADC_ADC_CFG_REG1_ANAU_GPADC_ATTN3X，使能3倍衰减模式
        else //disable 3X mode
        // 否则（禁用3倍模式）
            hadc->Instance->ADC_CFG_REG1 &= (~GPADC_ADC_CFG_REG1_ANAU_GPADC_ATTN3X);
            // 清除hadc->Instance的ADC_CFG_REG1寄存器中GPADC_ADC_CFG_REG1_ANAU_GPADC_ATTN3X对应的位
#endif


        /* Disable all slot at initial */
        // 初始时禁用所有通道槽
        for (i = 0; i < 8; i++)
        // i从0到7循环
            HAL_ADC_EnableSlot(hadc, i, 0);
            // 调用HAL_ADC_EnableSlot函数，禁用第i个通道槽

        // set LDOCORE and LDOREF enable
        // 设置LDOCORE和LDOREF使能
#if (GPADC_CALIB_FLOW_VERSION != 1)
        // 如果GPADC_CALIB_FLOW_VERSION宏不等于1
        // TODO:
        // 待办事项
        uint32_t value;
        // 定义无符号32位整数value
        value = hadc->Instance->ADC_CFG_REG1;
        // 将hadc->Instance的ADC_CFG_REG1寄存器值赋给value
        // register default setting from hardware sample code
        // 来自硬件示例代码的寄存器默认设置
#if (GPADC_CALIB_FLOW_VERSION == 3)
        // 如果GPADC_CALIB_FLOW_VERSION宏等于3
        value &= ~(GPADC_ADC_CFG_REG1_ANAU_GPADC_VSP | GPADC_ADC_CFG_REG1_ANAU_GPADC_CMM | GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_V18);
        // 清除value中GPADC_ADC_CFG_REG1_ANAU_GPADC_VSP、GPADC_ADC_CFG_REG1_ANAU_GPADC_CMM、GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_V18对应的位

        if (hadc->Init.avdd_v18_en)
        // 如果hadc->Init的avdd_v18_en成员为真
            value |= (0x8 << GPADC_ADC_CFG_REG1_ANAU_GPADC_CMM_Pos) | GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_V18;
            // 向value中写入(0x8左移GPADC_ADC_CFG_REG1_ANAU_GPADC_CMM_Pos位)和GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_V18的或值
        else
        // 否则
#else
        // 否则（GPADC_CALIB_FLOW_VERSION不等于3）
        value &= ~(GPADC_ADC_CFG_REG1_ANAU_GPADC_VSP | GPADC_ADC_CFG_REG1_ANAU_GPADC_CMM);
        // 清除value中GPADC_ADC_CFG_REG1_ANAU_GPADC_VSP和GPADC_ADC_CFG_REG1_ANAU_GPADC_CMM对应的位
#endif
            value |= ((2 << GPADC_ADC_CFG_REG1_ANAU_GPADC_VSP_Pos) | (0x10 << GPADC_ADC_CFG_REG1_ANAU_GPADC_CMM_Pos));
            // 向value中写入(2左移GPADC_ADC_CFG_REG1_ANAU_GPADC_VSP_Pos位)和(0x10左移GPADC_ADC_CFG_REG1_ANAU_GPADC_CMM_Pos位)的或值
        hadc->Instance->ADC_CFG_REG1 = value;
        // 将value的值赋给hadc->Instance的ADC_CFG_REG1寄存器

#if (GPADC_CALIB_FLOW_VERSION == 3)
        // 如果GPADC_CALIB_FLOW_VERSION宏等于3
        hwp_hpsys_cfg->ANAU_CR |= (HPSYS_CFG_ANAU_CR_EN_BG);
        // 向hwp_hpsys_cfg的ANAU_CR寄存器写入HPSYS_CFG_ANAU_CR_EN_BG
#endif /* (GPADC_CALIB_FLOW_VERSION == 3) */

#else   // (GPADC_CALIB_FLOW_VERSION == 1)
        // 否则（GPADC_CALIB_FLOW_VERSION等于1）
        hwp_tsen->BGR |= TSEN_BGR_EN;
        // 向hwp_tsen的BGR寄存器写入TSEN_BGR_EN
        hwp_tsen->ANAU_ANA_TP |= TSEN_ANAU_ANA_TP_ANAU_IARY_EN;
        // 向hwp_tsen的ANAU_ANA_TP寄存器写入TSEN_ANAU_ANA_TP_ANAU_IARY_EN
#endif

    }
    else
    // 如果上述条件不满足
    {
        /* Update ADC state machine to error */
        // 将ADC状态机更新为错误状态
        SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL);
        // 调用宏SET_BIT设置hadc->State中的HAL_ADC_STATE_ERROR_INTERNAL位

        tmp_hal_status = HAL_ERROR;
        // 将tmp_hal_status设置为HAL_ERROR
    }

    /* Return function status */
    // 返回函数状态
    return tmp_hal_status;
    // 返回tmp_hal_status
}


/**
  * @brief  Deinitialize the ADC peripheral registers to their default reset
  *         values, with deinitialization of the ADC MSP.
  * @note   For devices with several ADCs: reset of ADC common registers is done
  *         only if all ADCs sharing the same common group are disabled.
  *         If this is not the case, reset of these common parameters reset is
  *         bypassed without error reporting: it can be the intended behaviour in
  *         case of reset of a single ADC while the other ADCs sharing the same
  *         common group is still running.
  * @param  hadc ADC handle
  * @retval HAL status
  */
// 函数说明：将ADC外设寄存器反初始化为默认复位值，并反初始化ADC的Msp，
// 注意事项：对于多个ADC的设备，仅当共享同一公共组的所有ADC都禁用时，才复位ADC公共寄存器，否则跳过复位且不报告错误，
// 参数为ADC句柄hadc，返回值为HAL状态

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef *hadc)
// 定义函数HAL_ADC_DeInit，使用__HAL_ROM_USED修饰，返回值类型为HAL_StatusTypeDef，参数为ADC_HandleTypeDef类型指针hadc
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;
    // 定义HAL状态变量tmp_hal_status并初始化为HAL_OK

    /* Check ADC handle */
    // 检查ADC句柄
    if (hadc == NULL)
    // 如果hadc为NULL
    {
        return HAL_ERROR;
        // 返回HAL_ERROR
    }

    /* Set ADC state */
    // 设置ADC状态
    SET_BIT(hadc->State, HAL_ADC_STATE_BUSY_INTERNAL);
    // 调用宏SET_BIT设置hadc->State中的HAL_ADC_STATE_BUSY_INTERNAL位

    /* Disable ADC peripheral if conversions are effectively stopped */
    // 如果转换确实已停止，则禁用ADC外设
    if (tmp_hal_status == HAL_OK)
    // 如果tmp_hal_status为HAL_OK
    {
        // reset LDOCORE and LDOREF enable
        // 复位LDOCORE和LDOREF使能
        //hadc->Instance->ADC_CFG_REG1 &= (~(GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN|GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN));
        // 注释掉的代码，用于清除ADC_CFG_REG1寄存器中LDOCORE和LDOREF使能位

        /* Check if ADC is effectively disabled */
        // 检查ADC是否确实已禁用
        if (tmp_hal_status != HAL_ERROR)
        // 如果tmp_hal_status不是HAL_ERROR
        {
            /* Change ADC state */
            // 更改ADC状态
            hadc->State = HAL_ADC_STATE_READY;
            // 将hadc的State成员设置为HAL_ADC_STATE_READY
        }
    }


    /* Configuration of ADC parameters if previous preliminary actions are      */
    /* correctly completed.                                                     */
    // 如果之前的预备操作正确完成，则配置ADC参数
    if (tmp_hal_status != HAL_ERROR)
    // 如果tmp_hal_status不是HAL_ERROR
    {
        /* Set ADC error code to none */
        // 将ADC错误代码设置为无
        ADC_CLEAR_ERRORCODE(hadc);
        // 调用宏ADC_CLEAR_ERRORCODE清除hadc的错误代码

        /* Set ADC state */
        // 设置ADC状态
        hadc->State = HAL_ADC_STATE_RESET;
        // 将hadc的State成员设置为HAL_ADC_STATE_RESET
    }

    /* Deinitial ADC Msp */
    // 反初始化ADC的Msp
    HAL_ADC_MspDeInit(hadc);
    // 调用HAL_ADC_MspDeInit函数，参数为hadc

    /* Process unlocked */
    // 解锁进程
    __HAL_UNLOCK(hadc);
    // 调用宏__HAL_UNLOCK解锁hadc

    /* Return function status */
    // 返回函数状态
    return tmp_hal_status;
    // 返回tmp_hal_status
}

void HAL_ADC_HwInit(bool cold_boot)
// 定义函数HAL_ADC_HwInit，无返回值，参数为布尔类型cold_boot
{
#if defined(SF32LB55X) || defined(SF32LB56X) || defined(SF32LB58X) || defined(SF32LB52X)
// 如果定义了SF32LB55X、SF32LB56X、SF32LB58X或SF32LB52X中的任意一个宏
#ifdef SOC_BF0_HCPU
// 如果定义了SOC_BF0_HCPU宏
    if (cold_boot)
    // 如果cold_boot为真
    {
        /* init adc by HCPU when cold boot */
        // 冷启动时由HCPU初始化ADC
        hwp_gpadc1->ADC_CTRL_REG &= ~GPADC_ADC_CTRL_REG_TIMER_TRIG_EN;
        // 清除hwp_gpadc1的ADC_CTRL_REG寄存器中GPADC_ADC_CTRL_REG_TIMER_TRIG_EN对应的位
    }
#ifdef SF32LB52X
// 如果定义了SF32LB52X宏
    else
    // 否则（cold_boot为假）
    {
        /* gpadc1 is by HPSYS side on SF32LB52X */
        // 在SF32LB52X上gpadc1位于HPSYS侧
        hwp_gpadc1->ADC_CTRL_REG &= ~GPADC_ADC_CTRL_REG_TIMER_TRIG_EN;
        // 清除hwp_gpadc1的ADC_CTRL_REG寄存器中GPADC_ADC_CTRL_REG_TIMER_TRIG_EN对应的位
    }
#endif /* SF32LB52X */
#else
// 否则（未定义SOC_BF0_HCPU宏）
#ifndef SF32LB52X
// 如果未定义SF32LB52X宏
    if (!cold_boot)
    // 如果cold_boot为假
    {
        /* init adc by LCPU for non-cold boot as gpadc1 is in LPSYS except SF32LB52X,
         */
        // 非冷启动时由LCPU初始化ADC，因为除SF32LB52X外gpadc1位于LPSYS中
        hwp_gpadc1->ADC_CTRL_REG &= ~GPADC_ADC_CTRL_REG_TIMER_TRIG_EN;
        // 清除hwp_gpadc1的ADC_CTRL_REG寄存器中GPADC_ADC_CTRL_REG_TIMER_TRIG_EN对应的位
    }
#endif /* !SF32LB52X */
#endif /* SOC_BF0_HCPU */
#endif /* SF32LB55X || SF32LB56X || SF32LB58X || SF32LB52X */
}

/**
  * @}
  */
// ADC_Exported_Functions_Group1模块组结束

/** @defgroup ADC_Exported_Functions_Group2 IO operation functions
 *  @brief    IO operation functions
 *
@verbatim
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Start conversion of regular group.
      (+) Stop conversion of regular group.
      (+) Poll for conversion complete on regular group.
      (+) Poll for conversion event.
      (+) Get result of regular channel conversion.
      (+) Start conversion of regular group and enable interruptions.
      (+) Stop conversion of regular group and disable interruptions.
      (+) Handle ADC interrupt request
      (+) Start conversion of regular group and enable DMA transfer.
      (+) Stop conversion of regular group and disable ADC DMA transfer.
@endverbatim
  * @{
  */
// 定义ADC_Exported_Functions_Group2模块组，属于IO操作函数组，
// 简要说明是IO操作函数，verbatim块内详细说明该部分函数的功能，包括启动/停止常规组转换、轮询转换完成情况等，
// {表示开始

/**
  * @brief  Prepare ADC setting before start triger.
  * @param  hadc ADC handle
  * @retval HAL status
  */
// 函数说明：在开始触发前准备ADC设置，参数为ADC句柄hadc，返回值为HAL状态

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Prepare(ADC_HandleTypeDef *hadc)
// 定义函数HAL_ADC_Prepare，使用__HAL_ROM_USED修饰，返回值类型为HAL_StatusTypeDef，参数为ADC_HandleTypeDef类型指针hadc
{
    if (hadc == NULL)
    // 如果hadc为NULL
        return HAL_ERROR;
        // 返回HAL_ERROR

#if (GPADC_CALIB_FLOW_VERSION != 1)
// 如果GPADC_CALIB_FLOW_VERSION宏不等于1
#if (GPADC_CALIB_FLOW_VERSION == 3)
// 如果GPADC_CALIB_FLOW_VERSION宏等于3
    // enable analog
    // 使能模拟部分
    hwp_hpsys_cfg->ANAU_CR |= (HPSYS_CFG_ANAU_CR_EN_VBAT_MON);
    // 向hwp_hpsys_cfg的ANAU_CR寄存器写入HPSYS_CFG_ANAU_CR_EN_VBAT_MON
    hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
    // 向hadc->Instance的ADC_CFG_REG1寄存器写入GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN
#else   /* GPADC_CALIB_FLOW_VERSION == 2 */
// 否则（GPADC_CALIB_FLOW_VERSION等于2）
    hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_BG | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
    // 向hadc->Instance的ADC_CFG_REG1寄存器写入GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_BG和GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN的或值
#endif /* GPADC_CALIB_FLOW_VERSION == 3 */
    ADC_SET_UNMUTE(hadc);
    // 调用宏ADC_SET_UNMUTE，参数为hadc

#else   /* (GPADC_CALIB_FLOW_VERSION == 1) */
// 否则（GPADC_CALIB_FLOW_VERSION等于1）
    // set LDOCORE and LDOREF enable
    // 设置LDOCORE和LDOREF使能
    hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
    // 向hadc->Instance的ADC_CFG_REG1寄存器写入GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN和GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN的或值
#endif

    HAL_Delay_us(200);
    // 调用HAL_Delay_us函数，延时200微秒
    hadc->Instance->ADC_CTRL_REG |= (GPADC_ADC_CTRL_REG_FRC_EN_ADC);
    // 向hadc->Instance的ADC_CTRL_REG寄存器写入GPADC_ADC_CTRL_REG_FRC_EN_ADC

    // TODO: force set to single end, if use differential mode later, add new interface or paramter
    // 待办事项：强制设置为单端模式，如果以后使用差分模式，添加新的接口或参数
    __HAL_ADC_SINGLE_END(hadc);
    // 调用宏__HAL_ADC_SINGLE_END，参数为hadc，设置为单端模式

    /* Process locked */
    // 锁定进程
    __HAL_LOCK(hadc);
    // 调用宏__HAL_LOCK锁定hadc

    /* Start conversion if ADC is effectively enabled */
    // 如果ADC确实已使能，则开始转换

    /* Set ADC state                                                        */
    /* - Clear state bitfield related to regular group conversion results   */
    /* - Set state bitfield related to regular operation                    */
    // 设置ADC状态：清除与常规组转换结果相关的状态位，设置与常规操作相关的状态位
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR,
                      HAL_ADC_STATE_REG_BUSY);
    // 调用宏ADC_STATE_CLR_SET，清除hadc->State中的HAL_ADC_STATE_READY、HAL_ADC_STATE_REG_EOC、HAL_ADC_STATE_REG_OVR位，设置HAL_ADC_STATE_REG_BUSY位

    /* Reset ADC all error code fields */
    // 重置ADC所有错误代码字段
    ADC_CLEAR_ERRORCODE(hadc);
    // 调用宏ADC_CLEAR_ERRORCODE清除hadc的错误代码
#if (GPADC_CALIB_FLOW_VERSION != 1)
// 如果GPADC_CALIB_FLOW_VERSION宏不等于1
    HAL_Delay_us(200);
    // 调用HAL_Delay_us函数，延时200微秒
#else
// 否则（GPADC_CALIB_FLOW_VERSION等于1）
    HAL_Delay_us(50);
    // 调用HAL_Delay_us函数，延时50微秒
#endif
    /* Process unlocked */
    /* Unlock before starting ADC conversions: in case of potential         */
    /* interruption, to let the process to ADC IRQ Handler.                 */
    // 解锁进程：在开始ADC转换前解锁，以防潜在中断，让进程交给ADC中断处理程序
    __HAL_UNLOCK(hadc);
    // 调用宏__HAL_UNLOCK解锁hadc

    /* Return function status */
    // 返回函数状态
    return HAL_OK;
    // 返回HAL_OK
}

/**
  * @brief  Enables ADC, starts conversion of regular group.
  *         Interruptions enabled in this function: None.
  * @param  hadc ADC handle
  * @retval HAL status
  */
// 函数说明：使能ADC，开始常规组转换，此函数中不使能中断，
// 参数为ADC句柄hadc，返回值为HAL状态

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc)
// 定义函数HAL_ADC_Start，使用__HAL_ROM_USED修饰，返回值类型为HAL_StatusTypeDef，参数为ADC_HandleTypeDef类型指针hadc
{
    HAL_StatusTypeDef res = HAL_OK;
    // 定义HAL状态变量res并初始化为HAL_OK

    /* Perform ADC enable and conversion start if no conversion is on going */
    // 如果没有正在进行的转换，则执行ADC使能和转换开始操作
    res = HAL_ADC_Prepare(hadc);
    // 调用HAL_ADC_Prepare函数，参数为hadc，结果赋给res
    if (res == HAL_OK)
    // 如果res为HAL_OK
        hadc->Instance->ADC_CTRL_REG |= GPADC_ADC_CTRL_REG_ADC_START;
        // 向hadc->Instance的ADC_CTRL_REG寄存器写入GPADC_ADC_CTRL_REG_ADC_START，开始ADC转换

    return res;
    // 返回res
}

/**
  * @brief  Stop ADC conversion of regular group, disable ADC peripheral.
  * @param  hadc ADC handle
  * @retval HAL status.
  */
// 函数说明：停止常规组的ADC转换，禁用ADC外设，
// 参数为ADC句柄hadc，返回值为HAL状态

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc)
// 定义函数HAL_ADC_Stop，使用__HAL_ROM_USED修饰，返回值类型为HAL_StatusTypeDef，参数为ADC_HandleTypeDef类型指针hadc
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;
    // 定义HAL状态变量tmp_hal_status并初始化为HAL_OK

    /* Process locked */
    // 锁定进程
    __HAL_LOCK(hadc);
    // 调用宏__HAL_LOCK锁定hadc

    if (hadc->Init.op_mode != 0)
    // 如果hadc->Init的op_mode成员不等于0
        hadc->Instance->ADC_CTRL_REG |= GPADC_ADC_CTRL_REG_ADC_STOP;
        // 向hadc->Instance的ADC_CTRL_REG寄存器写入GPADC_ADC_CTRL_REG_ADC_STOP，停止ADC转换

    /* Set ADC state */
    // 设置ADC状态
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_REG_BUSY,
                      HAL_ADC_STATE_READY);
    // 调用宏ADC_STATE_CLR_SET，清除hadc->State中的HAL_ADC_STATE_REG_BUSY位，设置HAL_ADC_STATE_READY位

    // reset LDOCORE and LDOREF enable
    // 复位LDOCORE和LDOREF使能
#if (GPADC_CALIB_FLOW_VERSION != 1)
// 如果GPADC_CALIB_FLOW_VERSION宏不等于1
#if (GPADC_CALIB_FLOW_VERSION == 3)
// 如果GPADC_CALIB_FLOW_VERSION宏等于3
    hadc->Instance->ADC_CFG_REG1 &= ~(GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
    // 清除hadc->Instance的ADC_CFG_REG1寄存器中GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN对应的位
    hwp_hpsys_cfg->ANAU_CR &= (~HPSYS_CFG_ANAU_CR_EN_VBAT_MON);
    // 清除hwp_hpsys_cfg的ANAU_CR寄存器中HPSYS_CFG_ANAU_CR_EN_VBAT_MON对应的位
#else   /* (GPADC_CALIB_FLOW_VERSION == 2) */
// 否则（GPADC_CALIB_FLOW_VERSION等于2）
    // TODO
    // 待办事项
    hadc->Instance->ADC_CFG_REG1 &= ~(GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_BG | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
    // 清除hadc->Instance的ADC_CFG_REG1寄存器中GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_BG和GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN对应的位
#endif /* (GPADC_CALIB_FLOW_VERSION == 3) */
    ADC_SET_MUTE(hadc);
    // 调用宏ADC_SET_MUTE，参数为hadc

#else   /* (GPADC_CALIB_FLOW_VERSION == 1) */
// 否则（GPADC_CALIB_FLOW_VERSION等于1）
    hadc->Instance->ADC_CFG_REG1 &= (~(GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN));
    // 清除hadc->Instance的ADC_CFG_REG1寄存器中GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN和GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN对应的位
    hadc->Instance->ADC_CTRL_REG &= (~GPADC_ADC_CTRL_REG_FRC_EN_ADC);
    // 清除hadc->Instance的ADC_CTRL_REG寄存器中GPADC_ADC_CTRL_REG_FRC_EN_ADC对应的位
#endif

    /* Process unlocked */
    // 解锁进程
    __HAL_UNLOCK(hadc);
    // 调用宏__HAL_UNLOCK解锁hadc

    /* Return function status */
    // 返回函数状态
    return tmp_hal_status;
    // 返回tmp_hal_status
}

/**
  * @brief  Wait for regular group conversion to be completed.
  * @param  hadc ADC handle
  * @param  Timeout Timeout value in millisecond.
  * @retval HAL status
  */
// 函数说明：等待常规组转换完成，
// 参数为ADC句柄hadc和毫秒级超时值Timeout，返回值为HAL状态

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *hadc, uint32_t Timeout)
// 定义函数HAL_ADC_PollForConversion，使用__HAL_ROM_USED修饰，返回值类型为HAL_StatusTypeDef，
// 参数为ADC_HandleTypeDef类型指针hadc和无符号32位整数Timeout
{
    uint32_t tickstart;
    // 定义无符号32位整数tickstart

    /* Check ADC handle */
    // 检查ADC句柄
    if (hadc == NULL)
    // 如果hadc为NULL
    {
        return HAL_ERROR;
        // 返回HAL_ERROR
    }

    /* Get tick count */
    // 获取滴答计数
    tickstart = HAL_GetTick();
    // 调用HAL_GetTick函数获取当前滴答数，赋给tickstart

    /* Wait until End of Conversion flag is raised */
    // 等待直到转换结束标志被置位
    while (HAL_IS_BIT_CLR(hadc->Instance->GPADC_IRQ, GPADC_GPADC_IRQ_GPADC_IRSR))
    // 当hadc->Instance的GPADC_IRQ寄存器中GPADC_GPADC_IRQ_GPADC_IRSR位未设置时，循环等待
    {
        /* Check if timeout is disabled (set to infinite wait) */
        // 检查是否禁用超时（设置为无限等待）
        if (Timeout != HAL_MAX_DELAY)
        // 如果Timeout不等于HAL_MAX_DELAY（即不是无限等待）
        {
            if ((Timeout == 0) || ((HAL_GetTick() - tickstart) > Timeout))
            // 如果Timeout为0，或者当前滴答数减去tickstart大于Timeout（超时）
            {
                /* Update ADC state machine to timeout */
                // 将ADC状态机更新为超时状态
                SET_BIT(hadc->State, HAL_ADC_STATE_TIMEOUT);
                // 调用宏SET_BIT设置hadc->State中的HAL_ADC_STATE_TIMEOUT位

                /* Process unlocked */
                // 解锁进程
                __HAL_UNLOCK(hadc);
                // 调用宏__HAL_UNLOCK解锁hadc

                return HAL_TIMEOUT;
                // 返回HAL_TIMEOUT
            }
        }
    }

    /* Clear ISR */
    // 清除中断状态寄存器
    __HAL_ADC_CLEAR_FLAG(hadc, GPADC_GPADC_IRQ_GPADC_ICR);
    // 调用宏__HAL_ADC_CLEAR_FLAG清除hadc的GPADC_GPADC_IRQ_GPADC_ICR标志

    /* Update ADC state machine */
    // 更新ADC状态机
    SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC);
    // 调用宏SET_BIT设置hadc->State中的HAL_ADC_STATE_REG_EOC位

    return HAL_OK;
    // 返回HAL_OK
}

/**
  * @brief  Enables ADC, starts conversion of regular group with interruption.
  *         Interruptions enabled in this function:
  *          - EOC (end of conversion of regular group) or EOS (end of
  *            sequence of regular group) depending on ADC initialization
  *            parameter "EOCSelection"
  *          - overrun (if available)
  *         Each of these interruptions has its dedicated callback function.
  * @param  hadc ADC handle
  * @retval HAL status
  */
// 函数说明：使能ADC，开始带中断的常规组转换，
// 此函数中使能的中断包括：根据ADC初始化参数"EOCSelection"的常规组转换结束（EOC）或常规组序列结束（EOS）、溢出（如果可用），
// 每个中断都有其专用的回调函数，
// 参数为ADC句柄hadc，返回值为HAL状态

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Start_IT(ADC_HandleTypeDef *hadc)
// 定义函数HAL_ADC_Start_IT，使用__HAL_ROM_USED修饰，返回值类型为HAL_StatusTypeDef，参数为ADC_HandleTypeDef类型指针hadc
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;
    // 定义HAL状态变量tmp_hal_status并初始化为HAL_OK

    /* Check ADC handle */
    // 检查ADC句柄
    if (hadc == NULL)
    // 如果hadc为NULL
    {
        return HAL_ERROR;
        // 返回HAL_ERROR
    }

    __HAL_ADC_ENABLE_IRQ(hadc, GPADC_GPADC_IRQ_GPADC_IMR);
    // 调用宏__HAL_ADC_ENABLE_IRQ使能hadc的GPADC_GPADC_IRQ_GPADC_IMR中断

    __HAL_ADC_START_CONV(hadc);
    // 调用宏__HAL_ADC_START_CONV开始hadc的转换
    /* Return function status */
    // 返回函数状态
    return tmp_hal_status;
    // 返回tmp_hal_status
}


/**
  * @brief  Stop ADC conversion of regular group, disable interruption of
  *         end-of-conversion, disable ADC peripheral.
  * @param  hadc ADC handle
  * @retval HAL status.
  */
// 函数说明：停止常规组的ADC转换，禁用转换结束中断，禁用ADC外设，
// 参数为ADC句柄hadc，返回值为HAL状态

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Stop_IT(ADC_HandleTypeDef *hadc)
// 定义函数HAL_ADC_Stop_IT，使用__HAL_ROM_USED修饰，返回值类型为HAL_StatusTypeDef，参数为ADC_HandleTypeDef类型指针hadc
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;
    // 定义HAL状态变量tmp_hal_status并初始化为HAL_OK

    /* Process locked */
    // 锁定进程
    __HAL_LOCK(hadc);
    // 调用宏__HAL_LOCK锁定hadc

    if (hadc->Init.op_mode != 0)
    // 如果hadc->Init的op_mode成员不等于0
        __HAL_ADC_STOP_CONV(hadc);
        // 调用宏__HAL_ADC_STOP_CONV停止hadc的转换

    __HAL_ADC_DISABLE_IRQ(hadc, GPADC_GPADC_IRQ_GPADC_IMR);
    // 调用宏__HAL_ADC_DISABLE_IRQ禁用hadc的GPADC_GPADC_IRQ_GPADC_IMR中断

    /* Process unlocked */
    // 解锁进程
    __HAL_UNLOCK(hadc);
    // 调用宏__HAL_UNLOCK解锁hadc

    /* Return function status */
    // 返回函数状态
    return tmp_hal_status;
    // 返回tmp_hal_status
}

/**
  * @brief  Enables ADC, starts conversion of regular group and transfers result
  *         through DMA.
  *         Interruptions enabled in this function:
  *          - DMA transfer complete
  *          - DMA half transfer
  *          - overrun
  *         Each of these interruptions has its dedicated callback function.
  * @param  hadc ADC handle
  * @param  pData The destination Buffer address.
  * @param  Length The length of data to be transferred from ADC peripheral to memory.
  * @retval None
  */
__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_DMA_PREPARE(ADC_HandleTypeDef *hadc)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;

    /* Perform ADC enable and conversion start if no conversion is on going */
#if 1
    {
#if (GPADC_CALIB_FLOW_VERSION != 1)
#if (GPADC_CALIB_FLOW_VERSION == 3)
        // TODO
        hwp_hpsys_cfg->ANAU_CR |= (HPSYS_CFG_ANAU_CR_EN_VBAT_MON);
        //hwp_hpsys_cfg->ANAU_CR |= (HPSYS_CFG_ANAU_CR_EN_BG);
        hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#else
        hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_BG | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#endif

#else   //(GPADC_CALIB_FLOW_VERSION == 1)
        // set LDOCORE and LDOREF enable
        hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#endif

        hadc->Instance->ADC_CTRL_REG |= (GPADC_ADC_CTRL_REG_FRC_EN_ADC);
        /* Process locked */
        __HAL_LOCK(hadc);

        /* Start conversion if ADC is effectively enabled */
        if (tmp_hal_status == HAL_OK)
        {
            /* Set ADC state                                                        */
            /* - Clear state bitfield related to regular group conversion results   */
            /* - Set state bitfield related to regular operation                    */
            ADC_STATE_CLR_SET(hadc->State,
                              HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR,
                              HAL_ADC_STATE_REG_BUSY);

            /* Reset ADC all error code fields */
            ADC_CLEAR_ERRORCODE(hadc);
#if (GPADC_CALIB_FLOW_VERSION != 1)
            HAL_Delay_us(200);
#else
            HAL_Delay_us(50);
#endif
            /* Process unlocked */
            /* Unlock before starting ADC conversions: in case of potential         */
            /* interruption, to let the process to ADC IRQ Handler.                 */
            __HAL_UNLOCK(hadc);

        }
    }
#endif

    /* Check ADC handle */
    if (hadc == NULL || hadc->DMA_Handle == NULL)
    {
        return HAL_ERROR;
    }

    /* Init DMA configure*/
    //hadc->DMA_Handle->Instance                 = GPADC_DMA_INSTANCE;
    //hadc->DMA_Handle->Init.Request             = GPADC_DMA_REQUEST;
    //hadc->DMA_Handle->Init.Direction = DMA_PERIPH_TO_MEMORY;
    //hadc->DMA_Handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    //hadc->DMA_Handle->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    //hadc->DMA_Handle->Init.PeriphInc           = DMA_PINC_DISABLE;
    //hadc->DMA_Handle->Init.MemInc              = DMA_MINC_ENABLE;
    //hadc->DMA_Handle->Init.Mode                = DMA_NORMAL;
    //hadc->DMA_Handle->Init.Priority            = DMA_PRIORITY_MEDIUM;
    hadc->DMA_Handle->XferCpltCallback = ADC_DMAConvCplt;
    hadc->DMA_Handle->XferErrorCallback = ADC_DMAError;

    tmp_hal_status = HAL_DMA_Init(hadc->DMA_Handle);
    if (tmp_hal_status != HAL_OK)
        return tmp_hal_status;

    return tmp_hal_status;
}
__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef *hadc, uint32_t *pData, uint32_t Length)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;

    /* Perform ADC enable and conversion start if no conversion is on going */
#if 0
    {
#ifndef  SF32LB55X
#ifdef SF32LB52X
        // TODO
        hwp_hpsys_cfg->ANAU_CR |= (HPSYS_CFG_ANAU_CR_EN_VBAT_MON);
        //hwp_hpsys_cfg->ANAU_CR |= (HPSYS_CFG_ANAU_CR_EN_BG);
        hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#else
        hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_BG | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#endif /* SF32LB52X */

#else
        // set LDOCORE and LDOREF enable
        hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#endif

        hadc->Instance->ADC_CTRL_REG |= (GPADC_ADC_CTRL_REG_FRC_EN_ADC);
        /* Process locked */
        __HAL_LOCK(hadc);

        /* Start conversion if ADC is effectively enabled */
        if (tmp_hal_status == HAL_OK)
        {
            /* Set ADC state                                                        */
            /* - Clear state bitfield related to regular group conversion results   */
            /* - Set state bitfield related to regular operation                    */
            ADC_STATE_CLR_SET(hadc->State,
                              HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR,
                              HAL_ADC_STATE_REG_BUSY);

            /* Reset ADC all error code fields */
            ADC_CLEAR_ERRORCODE(hadc);
#ifndef  SF32LB55X
            HAL_Delay_us(200);
#else
            HAL_Delay_us(50);
#endif
            /* Process unlocked */
            /* Unlock before starting ADC conversions: in case of potential         */
            /* interruption, to let the process to ADC IRQ Handler.                 */
            __HAL_UNLOCK(hadc);
            * /

        }
    }
#endif

    /* Check ADC handle */
    if (hadc == NULL || hadc->DMA_Handle == NULL)
    {
        return HAL_ERROR;
    }
#if 0
    /* Init DMA configure*/
    hadc->DMA_Handle->Instance                 = GPADC_DMA_INSTANCE;
    hadc->DMA_Handle->Init.Request             = GPADC_DMA_REQUEST;
    hadc->DMA_Handle->Init.Direction = DMA_PERIPH_TO_MEMORY;
    hadc->DMA_Handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hadc->DMA_Handle->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hadc->DMA_Handle->Init.PeriphInc           = DMA_PINC_DISABLE;
    hadc->DMA_Handle->Init.MemInc              = DMA_MINC_ENABLE;
    hadc->DMA_Handle->Init.Mode                = DMA_NORMAL;
    hadc->DMA_Handle->Init.Priority            = DMA_PRIORITY_MEDIUM;
    hadc->DMA_Handle->XferCpltCallback = ADC_DMAConvCplt;
    hadc->DMA_Handle->XferErrorCallback = ADC_DMAError;


    tmp_hal_status = HAL_DMA_Init(hadc->DMA_Handle);
    if (tmp_hal_status != HAL_OK)
        return tmp_hal_status;
#endif
    ADC_DMA_ENABLE(hadc);
    tmp_hal_status = HAL_DMA_Start(hadc->DMA_Handle, (uint32_t)(&hadc->Instance->ADC_DMA_RDATA), (uint32_t)pData, Length);
    hadc->Instance->ADC_CTRL_REG |= GPADC_ADC_CTRL_REG_ADC_START;

    /* Return function status */
    return tmp_hal_status;
}

/**
  * @brief  Stop ADC conversion of regular group, disable ADC DMA transfer, disable
  *         ADC peripheral.
  *         Each of these interruptions has its dedicated callback function.
  * @param  hadc ADC handle
  * @retval HAL status.
  */
__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Stop_DMA(ADC_HandleTypeDef *hadc)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;

    /* Process locked */
    __HAL_LOCK(hadc);

    ADC_DMA_DISABLE(hadc);

    tmp_hal_status = HAL_DMA_Abort(hadc->DMA_Handle);

    if (hadc->Init.op_mode != 0)
        hadc->Instance->ADC_CTRL_REG |= GPADC_ADC_CTRL_REG_ADC_STOP;

    /* Set ADC state */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_REG_BUSY,
                      HAL_ADC_STATE_READY);

    // reset LDOCORE and LDOREF enable
#if (GPADC_CALIB_FLOW_VERSION != 1)
#if (GPADC_CALIB_FLOW_VERSION == 3)
    hadc->Instance->ADC_CFG_REG1 &= ~(GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
    //hwp_hpsys_cfg->ANAU_CR &= (~HPSYS_CFG_ANAU_CR_EN_BG);
    hwp_hpsys_cfg->ANAU_CR &= (~HPSYS_CFG_ANAU_CR_EN_VBAT_MON);
#else
    // TODO
    hadc->Instance->ADC_CFG_REG1 &= ~(GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_BG | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#endif
#else
    hadc->Instance->ADC_CFG_REG1 &= (~(GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN));
#endif
    hadc->Instance->ADC_CTRL_REG &= (~GPADC_ADC_CTRL_REG_FRC_EN_ADC);


    /* Process unlocked */
    __HAL_UNLOCK(hadc);

    /* Return function status */
    return tmp_hal_status;
}

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_DMA_WAIT_DONE(ADC_HandleTypeDef *hadc, uint32_t timeout)
{
    HAL_StatusTypeDef res = HAL_OK;

    if ((hadc == NULL) || (hadc->DMA_Handle == NULL))
        return HAL_ERROR;

    res = HAL_DMA_PollForTransfer(hadc->DMA_Handle, HAL_DMA_FULL_TRANSFER, timeout);

    ADC_DMA_DISABLE(hadc);

    return res;
}


/**
  * @brief  Get ADC regular group conversion result.
  * @note   Reading register DR automatically clears ADC flag EOC
  *         (ADC group regular end of unitary conversion).
  * @note   This function does not clear ADC flag EOS
  *         (ADC group regular end of sequence conversion).
  *         Occurrence of flag EOS rising:
  *          - If sequencer is composed of 1 rank, flag EOS is equivalent
  *            to flag EOC.
  *          - If sequencer is composed of several ranks, during the scan
  *            sequence flag EOC only is raised, at the end of the scan sequence
  *            both flags EOC and EOS are raised.
  *         To clear this flag, either use function:
  *         in programming model IT: @ref HAL_ADC_IRQHandler(), in programming
  *         model polling: @ref HAL_ADC_PollForConversion()
  *         or @ref __HAL_ADC_CLEAR_FLAG(&hadc, ADC_FLAG_EOS).
  * @param  hadc ADC handle
  * @retval ADC group regular conversion data
  */
__HAL_ROM_USED uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc, uint32_t slot)
{
    uint32_t value, odd;
    __IO uint32_t *p = &(hadc->Instance->ADC_RDATA0);

    /* Check ADC handle */
    if (hadc == NULL || slot >= 8)
    {
        return HAL_ERROR;
    }
    odd = 0;

    if (hadc->Init.en_slot == 1)
    {
        p += slot / 2;
        odd = slot % 2;
    }
    value = *p;

    if (odd)
        value = (value  & GPADC_ADC_RDATA0_SLOT1_RDATA_Msk) >> GPADC_ADC_RDATA0_SLOT1_RDATA_Pos;
    else
        value = value & GPADC_ADC_RDATA0_SLOT0_RDATA_Msk;

    /* Return ADC converted value */
    return value;
}

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Get_All(ADC_HandleTypeDef *hadc, uint32_t *buf)
{
    int i;
    __IO uint32_t *p = &(hadc->Instance->ADC_RDATA0);
    uint32_t *data = (uint32_t *)buf;

    /* Check ADC handle */
    if (hadc == NULL || buf == NULL)
        return HAL_ERROR;

    for (i = 0; i < 8; i++)
    {
        if (i & 1)
        {
            *data = (*p  & GPADC_ADC_RDATA0_SLOT1_RDATA_Msk) >> GPADC_ADC_RDATA0_SLOT1_RDATA_Pos;
            p++;
        }
        else
            *data = *p & GPADC_ADC_RDATA0_SLOT0_RDATA_Msk;
        data++;
    }

    return HAL_OK;
}


__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_SetSource(ADC_HandleTypeDef *hadc, HAL_ADC_SRC__T src)
{
    if (hadc == NULL || src > HAL_ADC_SRC_TIMER)
    {
        return HAL_ERROR;
    }

    hadc->Instance->ADC_CTRL_REG |= (src << GPADC_ADC_CTRL_REG_GPIO_TRIG_EN_Pos);

    return HAL_OK;
}

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_SetTimer(ADC_HandleTypeDef *hadc, HAL_ADC_SRC_TIME_T src)
{
    if (hadc == NULL || src >= HAL_ADC_SRC_TIMER_CNT)
    {
        return HAL_ERROR;
    }

    hadc->Instance->ADC_CTRL_REG |= (src << GPADC_ADC_CTRL_REG_TIMER_TRIG_SRC_SEL_Pos);

    return HAL_OK;
}


/**
  * @brief  Handles ADC interrupt request.
  * @param  hadc ADC handle
  * @retval None
  */
__HAL_ROM_USED void HAL_ADC_IRQHandler(ADC_HandleTypeDef *hadc)
{
    /* Clear ISR */
    __HAL_ADC_CLEAR_FLAG(hadc, GPADC_GPADC_IRQ_GPADC_ICR);

    /* Set status */
    SET_BIT(hadc->State, HAL_ADC_START_IRQ_DONE);
}

/**
  * @brief  Conversion complete callback in non blocking mode
  * @param  hadc ADC handle
  * @retval None
  */
__weak void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hadc);

    /* NOTE : This function should not be modified. When the callback is needed,
              function HAL_ADC_ConvCpltCallback must be implemented in the user file.
     */
}


/**
  * @}
  */

/** @defgroup ADC_Exported_Functions_Group3 Peripheral Control functions
 *  @brief    Peripheral Control functions
 *
@verbatim
 ===============================================================================
             ##### Peripheral Control functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Configure channels on regular group
      (+) Configure the analog watchdog

@endverbatim
  * @{
  */

/**
  * @brief  Configures the the selected channel to be linked to the regular
  *         group.
  * @note   In case of usage of internal measurement channels:
  *         VrefInt/Vbat/TempSensor.
  *         Sampling time constraints must be respected (sampling time can be
  *         adjusted in function of ADC clock frequency and sampling time
  *         setting).
  *         Refer to device datasheet for timings values, parameters TS_vrefint,
  *         TS_vbat, TS_temp (values rough order: 5us to 17us).
  *         These internal paths can be be disabled using function
  *         HAL_ADC_DeInit().
  * @note   Possibility to update parameters on the fly:
  *         This function initializes channel into regular group, following
  *         calls to this function can be used to reconfigure some parameters
  *         of structure "ADC_ChannelConfTypeDef" on the fly, without reseting
  *         the ADC.
  *         The setting of these parameters is conditioned to ADC state.
  *         For parameters constraints, see comments of structure
  *         "ADC_ChannelConfTypeDef".
  * @param  hadc ADC handle
  * @param  sConfig Structure of ADC channel for regular group.
  * @retval HAL status
  */
__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc, ADC_ChannelConfTypeDef *sConfig)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;
    uint32_t value = 0;
    __IO uint32_t *p = &(hadc->Instance->ADC_SLOT0_REG);

    /* Check ADC handle */
    if (hadc == NULL || sConfig == NULL || sConfig->Channel >= 8)
    {
        return HAL_ERROR;
    }

    /* Process locked */
    __HAL_LOCK(hadc);

    if (hadc->Init.en_slot == 1)
        p += sConfig->Channel;

    value = *p;

    value &= ~(GPADC_ADC_SLOT0_REG_NCHNL_SEL | GPADC_ADC_SLOT0_REG_PCHNL_SEL | GPADC_ADC_SLOT0_REG_ACC_NUM);
    value |= ((sConfig->nchnl_sel << GPADC_ADC_SLOT0_REG_NCHNL_SEL_Pos) & GPADC_ADC_SLOT0_REG_NCHNL_SEL_Msk)
             | ((sConfig->pchnl_sel << GPADC_ADC_SLOT0_REG_PCHNL_SEL_Pos) & GPADC_ADC_SLOT0_REG_PCHNL_SEL_Msk)
             | ((sConfig->acc_num << GPADC_ADC_SLOT0_REG_ACC_NUM_Pos) & GPADC_ADC_SLOT0_REG_ACC_NUM_Msk);


    if (sConfig->slot_en)
    {
        value |= GPADC_ADC_SLOT0_REG_SLOT_EN;
    }
    else
    {
        value &= ~GPADC_ADC_SLOT0_REG_SLOT_EN;
    }

    *p = value;

    /* Process unlocked */
    __HAL_UNLOCK(hadc);

    /* Return function status */
    return tmp_hal_status;
}

/**
  * @brief Set ADC sampling frequency.
  * @param hadc ADC handle.
  * @param freq frequence want to be set.
  * @retval actual work frequency, 0 if fail.
  */
__HAL_ROM_USED uint32_t HAL_ADC_SetFreq(ADC_HandleTypeDef *hadc, uint32_t freq)
{
    uint32_t fpclk; // = HAL_RCC_GetPCLKFreq(CORE_ID_LCPU, 1);
    uint32_t div; // = fpclk / freq;

    if (freq == 0)
        return 0;

    //extern void rt_kprintf(const char *fmt, ...);
#if (GPADC_CALIB_FLOW_VERSION != 3)
    fpclk = HAL_RCC_GetPCLKFreq(CORE_ID_LCPU, 1);
    div = fpclk / freq;
#else
    fpclk = HAL_RCC_GetPCLKFreq(CORE_ID_HCPU, 1);
    div = fpclk / freq;
#endif
    //for single triger mode, init time will effect adc frequency, init time only work once after strt
    //int init_time = (hadc->Instance->ADC_CTRL_REG & GPADC_ADC_CTRL_REG_INIT_TIME_Msk)>>GPADC_ADC_CTRL_REG_INIT_TIME_Pos;
    //div = fpclk / (freq * (1+init_time));
#if (GPADC_CALIB_FLOW_VERSION != 1)
    uint32_t min_conv, min_samp, data_dly;
    uint32_t max_conv, max_samp;

    max_conv = GPADC_ADC_CTRL_REG2_CONV_WIDTH >> GPADC_ADC_CTRL_REG2_CONV_WIDTH_Pos;
    max_samp = GPADC_ADC_CTRL_REG2_SAMP_WIDTH >> GPADC_ADC_CTRL_REG2_SAMP_WIDTH_Pos;
#if (GPADC_CALIB_FLOW_VERSION == 3)
    uint32_t std_conv, std_samp, std_dly, std_freq;
    std_conv = 76;
    std_samp = 74;
    std_dly = 2;
    std_freq = 36000000; // same to ATE
    std_freq /= 1000;   // to avoid over range for u32
    fpclk /= 1000;     // to avoid over range for u32
    min_conv = (std_conv * fpclk + std_freq / 2) / std_freq - 1;
    data_dly = (std_dly * fpclk + std_freq / 2) / std_freq;
    min_samp = (std_samp * fpclk + std_freq / 2) / std_freq - data_dly - 1;
    if ((min_conv > max_conv) || (min_samp > max_samp)) // over rage
        HAL_ASSERT(0);
#else
    min_conv = 24;
    min_samp = 22;
    data_dly = 2;

    //cnt = min_conv + min_samp + data_dly + 2;
    if ((div <= max_conv * 2) && (div <= max_samp * 2)) // have enough clock
    {
        min_conv = div / 2;
        data_dly = 2;
        min_samp = min_conv - data_dly - 2;
    }
    else // not support
        return 0;
#endif

    //sampel rate too high, not support, set to max freq
    hadc->Init.data_samp_delay = data_dly;
    hadc->Init.conv_width = min_conv;
    hadc->Init.sample_width = min_samp;

    __HAL_ADC_SET_SAMPLE_WIDTH(hadc, hadc->Init.sample_width);
    __HAL_ADC_SET_CONV_WIDTH(hadc, hadc->Init.conv_width);
    __HAL_ADC_SET_DATA_DELAY(hadc, hadc->Init.data_samp_delay);
    //rt_kprintf("src freq %d, div %d, data_samp_delay %d, conv_width %d, sample_width %d\n", fpclk, div, data_dly, min_conv, min_samp);
#else
    uint32_t maxdiv;
    maxdiv = GPADC_ADC_CTRL_REG_ADC_CLK_DIV >> GPADC_ADC_CTRL_REG_ADC_CLK_DIV_Pos;
    maxdiv = maxdiv + 1; // divider + 1

    if (div > maxdiv)
        div = maxdiv;
    if (div < 1)
        div = 1;
    hadc->Init.clk_div = div - 1;
    __HAL_ADC_SET_CLOCK_DIV(hadc, hadc->Init.clk_div);
#endif
    return fpclk / div;
}

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_EnableSlot(ADC_HandleTypeDef *hadc, uint32_t slot, uint8_t en)
{
    __IO uint32_t *p = &(hadc->Instance->ADC_SLOT0_REG);

    /* Check ADC handle */
    if (hadc == NULL || slot >= 8)
    {
        return HAL_ERROR;
    }
    if (hadc->Init.en_slot == 1)
        p += slot;

    // Clear these bit, disable
    //*p &= (~GPADC_ADC_SLOT0_REG_SLOT_EN);
    //*p &= (~GPADC_ADC_SLOT0_REG_ACC_NUM);

    if (en != 0)
    {
        *p |= GPADC_ADC_SLOT0_REG_SLOT_EN;
        //*p |= (3 << GPADC_ADC_SLOT0_REG_ACC_NUM_Pos);
    }
    else
    {
        *p &= (~GPADC_ADC_SLOT0_REG_SLOT_EN);
    }

    return HAL_OK;
}

__HAL_ROM_USED HAL_StatusTypeDef HAL_ADC_Set_MultiMode(ADC_HandleTypeDef *hadc, uint8_t multi_mode)
{
    if (hadc == NULL)
        return HAL_ERROR;

    if (multi_mode)
        hadc->Init.en_slot = 1;
    else
        hadc->Init.en_slot = 0;

    return HAL_OK;
}


__HAL_ROM_USED int HAL_ADC_Get_Offset(ADC_HandleTypeDef *hadc)
{
    int offset;

#if (GPADC_CALIB_FLOW_VERSION != 1)
#if (GPADC_CALIB_FLOW_VERSION == 3)
    hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#else
    // TODO
    hadc->Instance->ADC_CFG_REG1 |= (GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_BG | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#endif
#else

    hadc->Instance->ADC_CFG_REG1 |= GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN;

    hadc->Instance->ADC_CFG_REG1 |= GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN;
#endif
    //hadc->Instance->ADC_CFG_REG1 |= GPADC_ADC_CFG_REG1_ANAU_GPADC_SE;
    hadc->Instance->ADC_CFG_REG1 |= GPADC_ADC_CFG_REG1_ANAU_GPADC_MUTE;

    hadc->Instance->ADC_SLOT0_REG &= (~GPADC_ADC_SLOT0_REG_ACC_NUM);
    hadc->Instance->ADC_SLOT0_REG |= (3 << GPADC_ADC_SLOT0_REG_ACC_NUM_Pos);

    HAL_Delay_us(50);
    hadc->Instance->ADC_CTRL_REG |= GPADC_ADC_CTRL_REG_ADC_START;
    while (!(hadc->Instance->GPADC_IRQ & GPADC_GPADC_IRQ_GPADC_IRSR_Msk));

    offset = hadc->Instance->ADC_RDATA0 & GPADC_ADC_RDATA0_SLOT0_RDATA_Msk;

    hadc->Instance->GPADC_IRQ |= GPADC_GPADC_IRQ_GPADC_ICR;


#if (GPADC_CALIB_FLOW_VERSION != 1)
#if (GPADC_CALIB_FLOW_VERSION == 3)
    hadc->Instance->ADC_CFG_REG1 &= ~(GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#else
    // TODO
    hadc->Instance->ADC_CFG_REG1 &= ~(GPADC_ADC_CFG_REG1_ANAU_GPADC_EN_BG | GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN);
#endif
#else
    hadc->Instance->ADC_CFG_REG1 &= ~GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOREF_EN;
    hadc->Instance->ADC_CFG_REG1 &= ~GPADC_ADC_CFG_REG1_ANAU_GPADC_LDOCORE_EN;
#endif
    hadc->Instance->ADC_CFG_REG1 &= ~GPADC_ADC_CFG_REG1_ANAU_GPADC_MUTE;

    return offset - 512;
}

/**
  * @}
  */


/** @defgroup ADC_Exported_Functions_Group4 Peripheral State functions
 *  @brief    Peripheral State functions
 *
@verbatim
 ===============================================================================
            ##### Peripheral State and Errors functions #####
 ===============================================================================
    [..]
    This subsection provides functions to get in run-time the status of the
    peripheral.
      (+) Check the ADC state
      (+) Check the ADC error code

@endverbatim
  * @{
  */

/**
  * @brief  Return the ADC state
  * @note   ADC state machine is managed by bitfields, ADC status must be
  *         compared with states bits.
  *         For example:
  *           " if (HAL_IS_BIT_SET(HAL_ADC_GetState(hadc1), HAL_ADC_STATE_REG_BUSY)) "
  *           " if (HAL_IS_BIT_SET(HAL_ADC_GetState(hadc1), HAL_ADC_STATE_AWD1)    ) "
  * @param  hadc ADC handle
  * @retval HAL state
  */
__HAL_ROM_USED uint32_t HAL_ADC_GetState(ADC_HandleTypeDef *hadc)
{
    /* Return ADC state */
    return hadc->State;
}

/**
  * @brief  Return the ADC error code
  * @param  hadc ADC handle
  * @retval ADC Error Code
  */
__HAL_ROM_USED uint32_t HAL_ADC_GetError(ADC_HandleTypeDef *hadc)
{
    return hadc->ErrorCode;
}

/**
  * @brief  Initialize the ADC MSP.
  * @param  hadc pointer to a ADC_HandleTypeDef structure that contains
  *               the configuration information for ADC module.
  * @retval None
  */
__weak void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hadc);

    /* NOTE : This function should not be modified, when the callback is needed,
              the HAL_ADC_MspInit should be implemented in the user file
     */
}

/**
  * @brief  De-Initialize the ADC MSP.
  * @param  hadc pointer to a ADC_HandleTypeDef structure that contains
  *               the configuration information for ADC module.
  * @retval None
  */
__weak void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hadc);

    /* NOTE : This function should not be modified, when the callback is needed,
              the HAL_ADC_MspDeInit should be implemented in the user file
     */
}

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup ADC_Private_Functions ADC Private Functions
  * @{
  */


/**
  * @brief  DMA transfer complete callback.
  * @param  hdma pointer to DMA handle.
  * @retval None
  */
static void ADC_DMAConvCplt(DMA_HandleTypeDef *hdma)
{
    /* Retrieve ADC handle corresponding to current DMA handle */
    ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    /* Update state machine on conversion status if not in error state */
    if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL | HAL_ADC_STATE_ERROR_DMA))
    {
        /* Set ADC state */
        SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC);

        /* Conversion complete callback */
        HAL_ADC_ConvCpltCallback(hadc);
    }
    else
    {
        /* Call DMA error callback */
        hadc->DMA_Handle->XferErrorCallback(hdma);
    }

}

/**
  * @brief  DMA error callback
  * @param  hdma pointer to DMA handle.
  * @retval None
  */
static void ADC_DMAError(DMA_HandleTypeDef *hdma)
{
    /* Retrieve ADC handle corresponding to current DMA handle */
    ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    /* Set ADC state */
    SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_DMA);

    /* Set ADC error code to DMA error */
    SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_DMA);

}

/**
  * @}
  */

#endif /* HAL_ADC_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/
