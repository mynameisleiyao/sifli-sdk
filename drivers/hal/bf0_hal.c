/**
  ******************************************************************************
  * @file   bf0_hal.c
  * @author Sifli software development team
  * @brief   HAL模块驱动。
  *          这是HAL初始化的公共部分
  ******************************************************************************
*/
/**
 *
 * 版权所有 (c) 2019 - 2022，Sifli科技
 *
 * 保留所有权利。
 *
 * 允许在源文件和二进制形式下进行再分发和使用，无论是否经过修改，
 * 但必须满足以下条件：
 *
 * 1. 源文件的再分发必须保留上述版权声明、本条件列表以及以下免责声明。
 *
 * 2. 二进制形式的再分发，除非嵌入到产品中的Sifli集成电路或此类产品的软件更新中，
 *    必须在随分发提供的文档和/或其他材料中重现上述版权声明、本条件列表以及以下免责声明。
 *
 * 3. 未经事先书面许可，不得使用Sifli的名称或其贡献者的名称来认可或推广从本软件衍生的产品。
 *
 * 4. 本软件无论是否经过修改，只能与Sifli集成电路一起使用。
 *
 * 5. 根据本许可以二进制形式提供的任何软件不得进行逆向工程、反编译、修改和/或反汇编。
 *
 * 本软件由SIFLI TECHNOLOGY按"原样"提供，任何明示或暗示的保证，包括但不限于适销性、
 * 非侵权性和适用于特定用途的默示保证，均予以否认。在任何情况下，SIFLI TECHNOLOGY或其贡献者
 * 均不对任何直接、间接、偶然、特殊、示例性或后果性损害（包括但不限于替代商品或服务的采购；
 * 使用、数据或利润的损失；或业务中断）承担责任，无论其产生原因和责任理论如何，
 * 无论是合同责任、严格责任还是侵权行为（包括过失或其他情况），即使已被告知可能发生此类损害，
 * 也不承担责任。
 *
 */
#include "bf0_hal.h"  // 包含bf0_hal头文件
#include "bf0_hal_pinmux.h"  // 包含bf0_hal_pinmux头文件
/** @addtogroup BF0_HAL_Driver
  * @{
  */

/** @defgroup HAL_COMMON Hal公共函数
  * @brief HAL模块驱动
  * @{
  */

#if defined(HAL_MODULE_ENABLED)||defined(_SIFLI_DOXYGEN_)  // 如果定义了HAL_MODULE_ENABLED或_SIFLI_DOXYGEN_

/* 私有类型定义 -----------------------------------------------------------*/
/* 私有定义 ------------------------------------------------------------*/

#if defined(VREFBUF)  // 如果定义了VREFBUF
    #define VREFBUF_TIMEOUT_VALUE     10U   /* 10毫秒（待确认） */
#endif /* VREFBUF */

/* ------------ SYSCFG寄存器在别名区域的位地址 ------------ */
#define SYSCFG_OFFSET             (SYSCFG_BASE - PERIPH_BASE)  // 计算SYSCFG偏移量
/* ---  MEMRMP寄存器 ---*/
/* FB_MODE位的别名字地址 */
#define MEMRMP_OFFSET             SYSCFG_OFFSET  // MEMRMP偏移量
#define FB_MODE_BitNumber         8U  // FB_MODE位编号
#define FB_MODE_BB                (PERIPH_BB_BASE + (MEMRMP_OFFSET * 32U) + (FB_MODE_BitNumber * 4U))  // FB_MODE位带操作地址

/* --- SCSR寄存器 ---*/
/* SRAM2ER位的别名字地址 */
#define SCSR_OFFSET               (SYSCFG_OFFSET + 0x18U)  // SCSR偏移量
#define BRER_BitNumber            0U  // BRER位编号
#define SCSR_SRAM2ER_BB           (PERIPH_BB_BASE + (SCSR_OFFSET * 32U) + (BRER_BitNumber * 4U))  // SCSR_SRAM2ER位带操作地址

/* 私有宏 -------------------------------------------------------------*/
/* 私有变量 ---------------------------------------------------------*/
/* 私有函数原型 ---------------------------------------------------------------*/

/* 导出变量 --------------------------------------------------------*/

#if defined(__CC_ARM) || defined(__CLANG_ARM)  // 如果是ARM编译器或CLANG_ARM编译器
#elif defined(__GNUC__)  // 如果是GNU编译器
    /* 解决arm-none-eabi-gcc xpack版本产生的大二进制文件问题
    * 参见 https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/discussions/23
    */
    __WEAK int __aeabi_unwind_cpp_pr0(void) { return 0; }  // 弱定义__aeabi_unwind_cpp_pr0函数
    __WEAK int __aeabi_unwind_cpp_pr1(void) { return 0; }  // 弱定义__aeabi_unwind_cpp_pr1函数
    __WEAK int __aeabi_unwind_cpp_pr2(void) { return 0; }  // 弱定义__aeabi_unwind_cpp_pr2函数
#endif

/** @defgroup HAL_Exported_Variables HAL导出变量
  * @{
  */
__IO uint32_t uwTick;  // 输入输出型uint32_t变量uwTick，用于计时
/**
  * @} HAL_Exported_Variables
  */

/* 导出函数 --------------------------------------------------------*/

/** @defgroup HAL_Exported_Functions HAL导出函数
  * @{
  */

/** @defgroup HAL_Exported_Functions_Group1 初始化和反初始化函数
 *  @brief    初始化和反初始化函数
 *
@verbatim
 ===============================================================================
              ##### 初始化和反初始化函数 #####
 ===============================================================================
    [..] 本节提供的函数允许：
      (+) 初始化Flash接口、NVIC分配和初始时基时钟配置。
      (+) 反初始化HAL的公共部分。
      (+) 配置时基源以获得1毫秒时基，并具有专用的滴答中断优先级。
        (++) 系统滴答定时器默认用作时基源，但用户最终可以实现自己的时基源
             （例如通用定时器或其他时间源），请记住时基持续时间应保持1毫秒，
             因为PPP_TIMEOUT_VALUEs是以毫秒为基础定义和处理的。
        (++) 时基配置函数（HAL_InitTick()）在程序开始时由HAL_Init()自动调用，
             或在任何时钟配置时由HAL_RCC_ClockConfig()调用。
        (++) 时基源配置为以固定时间间隔生成中断。如果从外设ISR进程调用HAL_Delay()，
             必须注意滴答中断线的优先级必须高于外设中断（数值上更低）。否则调用者ISR进程将被阻塞。
       (++) 影响时基配置的函数被声明为__weak，以便在用户文件中有其他实现时可以重写。
@endverbatim
  * @{
  */

/**
  * @brief  配置Flash预取、指令和数据缓存、时基源、NVIC以及任何所需的全局低级硬件，
  *         通过调用HAL_PreInit()弱函数，该函数可在用户文件中选择性定义
  *
  * @note   HAL_Init()函数在复位后的程序开始时、时钟配置之前被调用。
  *
  * @note   在默认实现中，系统定时器（Systick）用作时基源。
  *         Systick配置基于MSI时钟，因为MSI是系统复位后使用的时钟，
  *         NVIC配置设置为优先级组4。完成后，时基滴答开始递增：
  *         滴答变量计数器在SysTick_Handler()中断处理程序中每1毫秒递增一次。
  *
  * @retval HAL状态
  */
HAL_StatusTypeDef HAL_Init(void)
{
    HAL_StatusTypeDef  status = HAL_OK;  // 定义HAL状态变量并初始化为HAL_OK

    /* 配置Flash预取、指令缓存、数据缓存 */
    /* 复位时的默认配置为：                      */
    /* - 预取禁用                                     */
    /* - 指令缓存启用                             */
    /* - 数据缓存启用                                    */
#if (INSTRUCTION_CACHE_ENABLE == 0)  // 如果指令缓存禁用
//   __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();  // 禁用Flash指令缓存（注释掉）
#endif /* INSTRUCTION_CACHE_ENABLE */

#if (DATA_CACHE_ENABLE == 0)  // 如果数据缓存禁用
//   __HAL_FLASH_DATA_CACHE_DISABLE();  // 禁用Flash数据缓存（注释掉）
#endif /* DATA_CACHE_ENABLE */

#if (PREFETCH_ENABLE != 0)  // 如果预取启用
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();  // 启用Flash预取缓冲区
#endif /* PREFETCH_ENABLE */

#if defined(HAL_V2D_GPU_MODULE_ENABLED)  // 如果定义了HAL_V2D_GPU_MODULE_ENABLED
    HAL_RCC_ResetModule(RCC_MOD_GPU);  // 复位GPU模块
#endif

    HAL_PreInit();  // 调用HAL_PreInit函数

#ifdef SOC_BF0_HCPU  // 如果定义了SOC_BF0_HCPU
    if (PM_STANDBY_BOOT != SystemPowerOnModeGet())  // 如果系统上电模式不是PM_STANDBY_BOOT
    {
//TODO: 52x也需要RC48
//#ifndef SF32LB52X
        // 除了待机模式，所有其他启动模式都需要重新校准RC48
        status = HAL_RCC_CalibrateRC48();  // 校准RC48
#ifndef TARMAC
        HAL_ASSERT(HAL_OK == status);  // 断言状态为HAL_OK
#endif
//#endif /* SF32LB52X */
    }
#endif /* SOC_BF0_HCPU */

    HAL_PostMspInit();  // 调用HAL_PostMspInit函数
    HAL_RCC_Init();  // 初始化RCC

#ifdef SOC_BF0_HCPU  // 如果定义了SOC_BF0_HCPU
    if (PM_STANDBY_BOOT != SystemPowerOnModeGet())  // 如果系统上电模式不是PM_STANDBY_BOOT
    {
        HAL_PMU_Init();  // 初始化PMU
    }
#endif /* SOC_BF0_HCPU */

#ifdef HAL_ADC_MODULE_ENABLED  // 如果定义了HAL_ADC_MODULE_ENABLED
    HAL_ADC_HwInit(PM_STANDBY_BOOT != SystemPowerOnModeGet());  // 初始化ADC硬件
#endif /* HAL_ADC_MODULE_ENABLED */

    /* 设置中断组优先级 */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);  // 设置NVIC优先级组为4

#ifndef NONE_HAL_TICK_INIT  // 如果没有定义NONE_HAL_TICK_INIT
    /* 使用SysTick作为时基源并配置1毫秒滴答（复位后的默认时钟是MSI） */
    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)  // 初始化滴答定时器
    {
        status = HAL_ERROR;  // 状态设为HAL_ERROR
    }
    else
#endif
    {
#ifdef SOC_BF0_HCPU  // 如果定义了SOC_BF0_HCPU
        /* 初始化AES_ACC为正常模式 */
        __HAL_SYSCFG_CLEAR_SECURITY();  // 清除SYSCFG安全标志
        HAL_EFUSE_Init();  // 初始化EFUSE
#endif
    }

    /* 返回函数状态 */
    return status;  // 返回状态
}

/**
  * @brief 反初始化HAL的公共部分并停止时基源。
  * @note 此函数是可选的。
  * @retval HAL状态
  */
HAL_StatusTypeDef HAL_DeInit(void)
{

#if 0  // 条件编译，此处代码不执行
    /* 复位所有外设 */
    __HAL_RCC_APB1_FORCE_RESET();  // 强制复位APB1
    __HAL_RCC_APB1_RELEASE_RESET();  // 释放APB1复位

    __HAL_RCC_APB2_FORCE_RESET();  // 强制复位APB2
    __HAL_RCC_APB2_RELEASE_RESET();  // 释放APB2复位

    __HAL_RCC_AHB1_FORCE_RESET();  // 强制复位AHB1
    __HAL_RCC_AHB1_RELEASE_RESET();  // 释放AHB1复位

    __HAL_RCC_AHB2_FORCE_RESET();  // 强制复位AHB2
    __HAL_RCC_AHB2_RELEASE_RESET();  // 释放AHB2复位

    __HAL_RCC_AHB3_FORCE_RESET();  // 强制复位AHB3
    __HAL_RCC_AHB3_RELEASE_RESET();  // 释放AHB3复位
#endif
    /* 反初始化低级硬件 */
    HAL_MspDeInit();  // 调用HAL_MspDeInit函数

    /* 返回函数状态 */
    return HAL_OK;  // 返回HAL_OK
}

/**
  * @brief 在HAL_Init被调用之前，进行板级特定初始化。
  * @retval 无
  */
__weak void HAL_PreInit(void)
{
    /* 注意：此函数不应被修改，当需要回调时，HAL_PreInit可在板级特定文件中实现 */
}

/**
  * @brief 在HAL_PreInit之后被调用。
  * @retval 无
  */

__weak void HAL_PostMspInit(void)
{

}


/**
  * @brief 反初始化MSP。
  * @retval 无
  */
__weak void HAL_MspDeInit(void)
{
    /* 注意：此函数不应被修改，当需要回调时，HAL_MspDeInit可在用户文件中实现 */
}

/**
  * @brief 此函数配置时基源：
  *        时基源配置为具有1毫秒时基和专用的滴答中断优先级。
  * @note 此函数在程序开始时由HAL_Init()自动调用，或在任何时钟重新配置时由HAL_RCC_ClockConfig()调用。
  * @note 在默认实现中，SysTick定时器是时基源。它用于以固定时间间隔生成中断。
  *       如果从外设ISR进程调用HAL_Delay()，必须注意SysTick中断的优先级必须高于外设中断（数值上更低）。
  *       否则调用者ISR进程将被阻塞。此函数被声明为__weak，以便在用户文件中有其他实现时可以重写。
  * @param TickPriority  滴答中断优先级。
  * @retval HAL状态
  */
__weak HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    HAL_StatusTypeDef  status = HAL_OK;  // 定义HAL状态变量并初始化为HAL_OK

    /*将SysTick配置为1毫秒时间基础上产生中断*/
    if (HAL_SYSTICK_Config(SystemCoreClock / 1000UL) != 0U)  // 配置SysTick
    {
        status = HAL_ERROR;  // 配置失败，状态设为HAL_ERROR
    }
    else
    {
        /*配置SysTick IRQ优先级 */
        HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 0);  // 设置SysTick中断优先级
    }

    /* 返回函数状态 */
    return status;  // 返回状态
}

/**
  * @} HAL_Exported_Functions_Group1
  */

/** @defgroup HAL_Exported_Functions_Group2 HAL控制函数
 *  @brief    HAL控制函数
 *
@verbatim
 ===============================================================================
                      ##### HAL控制函数 #####
 ===============================================================================
    [..] 本节提供的函数允许：
      (+) 提供毫秒级的滴答值
      (+) 提供毫秒级的阻塞延迟
      (+) 暂停时基源中断
      (+) 恢复时基源中断
      (+) 获取HAL API驱动程序版本
      (+) 获取设备标识符
      (+) 获取设备修订标识符

@endverbatim
  * @{
  */


/**
  * @brief 此函数被调用以递增全局变量"uwTick"，用作应用程序时基。
  * @note 在默认实现中，此变量在SysTick ISR中每1毫秒递增一次。
 * @note 此函数被声明为__weak，以便在用户文件中有其他实现时可以重写。
  * @retval 无
  */
__weak void HAL_IncTick(void)
{
    uwTick++;  // uwTick自增
}

/**
  * @brief 提供毫秒级的滴答值。
  * @note 此函数被声明为__weak，以便在用户文件中有其他实现时可以重写。
  * @retval 滴答值
  */
__weak uint32_t HAL_GetTick(void)
{
    return uwTick;  // 返回uwTick值
}

// 对于CM33，需要5条指令（12个周期），请注意XIP在flash读取时可能需要更多时间。

/*
    LDR     r0,[sp,#0]  3个周期
    SUBS    r1,r0,#1    1个周期
    CMP     r0,#0       1个周期
    STR     R1,[SP,0]       1个周期
    BNE     6个周期
*/
#define WAIT_US_LOOP_CYCLE 12  // 定义等待微秒循环周期为12
__weak void HAL_Delay_us_(__IO uint32_t us)
{
    //TODO: 用SystemCoreClock替换？
    static uint32_t sysclk_m;  // 静态变量sysclk_m

    if (us == 0 || sysclk_m == 0)  // 如果微秒数为0或sysclk_m为0
    {
        sysclk_m = HAL_RCC_GetHCLKFreq(CORE_ID_DEFAULT) / 1000000;  // 计算系统时钟（MHz）
        if (us == 0)
            return;  // 如果微秒数为0，返回
    }

    if (us > (1 << 24)) // 最大16秒以避免计数溢出 --- 假设时钟最大为256M
        HAL_ASSERT(0);  // 断言失败

    //if (sysclk_m > 48)   // DLL有额外开销，原因待确定。
    //    us -= 1;
    if (us > 0)          // 函数开销额外1微秒
    {
#ifdef DWT  // 如果定义了DWT

        if (0 == HAL_DBG_DWT_IsInit())  // 如果DWT未初始化
        {
            HAL_DBG_DWT_Init();  // 初始化DWT
        }
        uint32_t cnt = sysclk_m * us;  // 计算周期数
        uint32_t start = HAL_DBG_DWT_GetCycles();  // 获取开始周期数
        while ((HAL_DBG_DWT_GetCycles() - start) < cnt)  // 等待达到指定周期数
        {
        }

#else  // 如果未定义DWT
        volatile uint32_t i = sysclk_m * (us - 1) / WAIT_US_LOOP_CYCLE;  // 计算循环次数
        while (i-- > 0); //在ARMCC下需要5条指令
#endif
    }
}

// 使用systick获得更精确的微秒级延迟。
__weak void HAL_Delay_us2_(__IO uint32_t us)
{
    uint32_t reload = SysTick->LOAD;  // 获取SysTick加载值
    uint32_t told, tnow, tcnt = 0;  // 定义变量
    us = us * reload / (1000000 / HAL_TICK_PER_SECOND);  // 计算微秒对应的滴答数
    told = SysTick->VAL;  // 获取当前SysTick值
    while (1)  // 循环
    {
        tnow = SysTick->VAL;  // 获取当前SysTick值
        if (tnow != told)  // 如果当前值与之前值不同
        {
            if (tnow < told)  // 如果当前值小于之前值
            {
                tcnt += told - tnow;  // 累加滴答数
            }
            else  // 否则
            {
                tcnt += reload - tnow + told;  // 累加滴答数
            }
            told = tnow;  // 更新之前值
            if (tcnt >= us)  // 如果累加滴答数达到目标
            {
                break;  // 跳出循环
            }
        }
    }

}

#define MAX_US_DELAY    10000  // 定义最大微秒延迟为10000
__weak void HAL_Delay_us(uint32_t us)
{
    uint32_t ticks;  // 定义变量

    if (0 == us)  // 如果微秒数为0
    {
        /* sysclk_m需要初始化，因为在keil驱动中它是一个随机值 */
        HAL_Delay_us_(0);  // 调用HAL_Delay_us_函数
    }

    while (us > 0)  // 当还有剩余微秒数时
    {
        if (us > MAX_US_DELAY)  // 如果剩余微秒数大于最大微秒延迟
        {
            ticks = MAX_US_DELAY;  // 本次延迟为最大微秒延迟
            us -= MAX_US_DELAY;  // 减去已延迟的微秒数
        }
        else  // 否则
        {
            ticks = us;  // 本次延迟为剩余微秒数
            us = 0;  // 剩余微秒数清零
        }
        if ((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == 0) // Systick尚未启用，使用循环
            HAL_Delay_us_(ticks);  // 调用HAL_Delay_us_函数
        else  // 否则
            HAL_Delay_us2_(ticks);  // 调用HAL_Delay_us2_函数
    }
}


/**
  * @brief 此函数基于递增的变量提供最小延迟（以毫秒为单位）。
  * @note 在默认实现中，SysTick定时器是时基源。它用于以固定时间间隔生成中断，其中uwTick递增。
  * @note 此函数被声明为__weak，以便在用户文件中有其他实现时可以重写。
  * @param Delay  指定延迟时间长度，以毫秒为单位。
  * @retval 无
  */
__weak void HAL_Delay(__IO uint32_t Delay)
{
#if 1  // 条件编译，执行此部分
    while (Delay > 0)  // 当延迟数大于0时
    {
        HAL_Delay_us(1000);  // 延迟1000微秒（1毫秒）
        Delay--;  // 延迟数减1
    }
#else  // 否则执行此部分
    uint32_t tickstart = HAL_GetTick();  // 获取开始滴答值
    uint32_t wait = Delay;  // 等待时间

    /* 添加一个周期以保证最小等待时间 */
    if (wait < HAL_MAX_DELAY)  // 如果等待时间小于最大延迟
    {
        wait++;  // 等待时间加1
    }

    while ((HAL_GetTick() - tickstart) < wait)  // 等待达到指定时间
    {
    }
#endif
}

/**
  * @brief 暂停滴答递增。
  * @note 在默认实现中，SysTick定时器是时基源。它用于以固定时间间隔生成中断。
  *       一旦调用HAL_SuspendTick()，SysTick中断将被禁用，因此滴答递增被暂停。
  * @note 此函数被声明为__weak，以便在用户文件中有其他实现时可以重写。
  * @retval 无
  */
__weak void HAL_SuspendTick(void)
{
    /* 禁用SysTick中断 */
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;  // 清除SysTick中断使能位
}

/**
  * @brief 恢复滴答递增。
  * @note 在默认实现中，SysTick定时器是时基源。它用于以固定时间间隔生成中断。
  *       一旦调用HAL_ResumeTick()，SysTick中断将被启用，因此滴答递增被恢复。
  * @note 此函数被声明为__weak，以便在用户文件中有其他实现时可以重写。
  * @retval 无
  */
__weak void HAL_ResumeTick(void)
{
    /* 启用SysTick中断 */
    SysTick->CTRL  |= SysTick_CTRL_TICKINT_Msk;  // 设置SysTick中断使能位
}

__weak void HAL_AssertFailed(char *file, uint32_t line)
{
    volatile uint32_t dummy = 0;  // 定义volatile变量dummy
    while (0 == dummy);  // 无限循环
}

#ifndef SF32LB55X  // 如果没有定义SF32LB55X
#ifdef HAL_WDT_MODULE_ENABLED  // 如果定义了HAL_WDT_MODULE_ENABLED
__weak void WDT_IRQHandler(void)
{

}
#endif

__weak void DBG_Trigger_IRQHandler(void)
{

}

//SF32LB52X在安全模式下使用
#if defined(SF32LB52X) && defined(SOC_BF0_HCPU)  // 如果定义了SF32LB52X和SOC_BF0_HCPU
void PendSv_DBG_Trigger(void)
{
    DBG_Trigger_IRQHandler();  // 调用DBG_Trigger_IRQHandler函数
}
#endif /* SF32LB52X && SOC_BF0_HCPU */

#ifdef HAL_NMI_HANLDER_OVERRIDED  // 如果定义了HAL_NMI_HANLDER_OVERRIDED
    __weak void NMI_Handler(void)
#else  // 否则
    void NMI_Handler(void)
#endif /* HAL_NMI_HANLDER_OVERRIDED */
{
    if (__HAL_SYSCFG_Get_Trigger_Assert_Flag())  // 如果获取到触发断言标志
        DBG_Trigger_IRQHandler();  // 调用DBG_Trigger_IRQHandler函数
#ifdef HAL_WDT_MODULE_ENABLED  // 如果定义了HAL_WDT_MODULE_ENABLED
    else  // 否则
        WDT_IRQHandler();  // 调用WDT_IRQHandler函数
#endif
}

#if defined(SF32LB52X) && defined(SOC_BF0_LCPU)  // 如果定义了SF32LB52X和SOC_BF0_LCPU
__HAL_ROM_USED uint32_t HAL_GetLXTEnabled(void)
{
    uint8_t is_lxt_enabled;  // 定义变量
    uint16_t len = 1;  // 长度为1
    HAL_StatusTypeDef ret = HAL_LCPU_CONFIG_get(HAL_LCPU_CONFIG_XTAL_ENABLED, &is_lxt_enabled, &len);  // 获取LCPU配置

    //默认是32K
    if (ret != HAL_OK)  // 如果获取失败
        is_lxt_enabled = 1;  // 默认LXT使能

    return (uint32_t)is_lxt_enabled;  // 返回LXT使能状态
}
#endif


#endif // SF32LB55X

/**
 * @} HAL_Exported_Functions_Group2
 */

/**
 * @} HAL_Exported_Functions
 */

#endif /* HAL_MODULE_ENABLED */

/**
 * @} HAL_COMMON
 */

/**
 * @} BF0_HAL_Driver
 */

/************************ (C) COPYRIGHT Sifli Technology *******文件结束****/