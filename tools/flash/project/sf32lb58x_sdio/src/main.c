
/* Includes ------------------------------------------------------------------*/
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "pmic_controller.h"

#define SD1_RESET_PIN       (11)
#define SD1_EN_PIN          (2)


extern void BSP_GPIO_Set(int pin, int val, int is_porta);

UART_HandleTypeDef UartHandle;

static uint16_t flash3_div = 5;
static uint16_t flash4_div = 5;
static uint16_t flash5_div = 2;


static void SystemClock_Config(void);
static void Error_Handler(void);

#if defined(CFG_FACTORY_DEBUG)
extern bool user_pmic_cfg();
extern void user_pin_cfg();
extern bool get_user_sd0_cfg(uint32_t *pAddr, int8_t *pPinIdx, int8_t *pInitIdx);
#endif

void debug_print(char *str)
{
    HAL_UART_Transmit(&UartHandle, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}


uint16_t BSP_GetFlash3DIV(void)
{
    return flash3_div;
}

uint16_t BSP_GetFlash4DIV(void)
{
    return flash4_div;
}

uint16_t BSP_GetFlash5DIV(void)
{
    return flash5_div;
}

void BSP_SetFlash3DIV(uint16_t div)
{
    flash3_div = div;
}

void BSP_SetFlash4DIV(uint16_t div)
{
    flash4_div = div;
}

void BSP_SetFlash5DIV(uint16_t div)
{
    flash5_div = div;
}


void BSP_GPIO_Set(int pin, int val, int is_porta)
{
    GPIO_TypeDef *gpio = (is_porta) ? (GPIO_TypeDef *)hwp_gpio1 : (GPIO_TypeDef *)hwp_gpio2;
    GPIO_InitTypeDef GPIO_InitStruct;

    // set sensor pin to output mode
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);

    // set sensor pin to high == power on sensor board
    HAL_GPIO_WritePin(gpio, pin, (GPIO_PinState)val);
}

static void JLINK_DRV_BSP_PIN_Init(void)
{
    //MODIFY_REG(hwp_qspi1->WDTR, QSPI_WDTR_TIMEOUT_Msk, QSPI_WDTR_TIMEOUT_Msk);
    //MODIFY_REG(hwp_qspi2->WDTR, QSPI_WDTR_TIMEOUT_Msk, QSPI_WDTR_TIMEOUT_Msk);
    //MODIFY_REG(hwp_qspi3->WDTR, QSPI_WDTR_TIMEOUT_Msk, QSPI_WDTR_TIMEOUT_Msk);
    int8_t idx = -1;
    
    get_user_sd0_cfg(NULL, &idx, NULL);
	
	if(idx <= 0)
	{
	    HAL_PIN_Set(PAD_PA09, SD1_CLK, PIN_NOPULL, 1); // SDIO1
	    HAL_PIN_Set(PAD_PA10, SD1_CMD, PIN_PULLUP, 1);
	    HAL_PIN_Set(PAD_PA05, SD1_DIO0, PIN_PULLUP, 1);
	    HAL_PIN_Set(PAD_PA04, SD1_DIO1, PIN_PULLUP, 1);
	    HAL_PIN_Set(PAD_PA01, SD1_DIO2, PIN_PULLUP, 1);
	    HAL_PIN_Set(PAD_PA06, SD1_DIO3, PIN_PULLUP, 1);
	    HAL_PIN_Set(PAD_PA07, SD1_DIO4, PIN_PULLUP, 1);
	    HAL_PIN_Set(PAD_PA03, SD1_DIO5, PIN_PULLUP, 1);
	    HAL_PIN_Set(PAD_PA08, SD1_DIO6, PIN_PULLUP, 1);
	    HAL_PIN_Set(PAD_PA00, SD1_DIO7, PIN_PULLUP, 1);

	    HAL_PIN_Set(PAD_PA02, GPIO_A2, PIN_PULLUP, 1);     // SD1 EN
	    HAL_PIN_Set(PAD_PA12, GPIO_A12, PIN_PULLUP, 1);     //EMMC_3V3_EN for Keep
	    HAL_PIN_Set(PAD_PA13, GPIO_A13, PIN_PULLUP, 1);     //EMMC_1V8_EN for Keep
	    //BSP_GPIO_Set(2, 1, 1);
	    HAL_PIN_Set(PAD_PA11, GPIO_A11, PIN_PULLUP, 1);     // SD1 RESET, need set 0 first?
	    //BSP_GPIO_Set(11, 1, 1);
	}
	else
	{
	    HAL_PIN_Set(PAD_PA39, SD1_CLK, PIN_NOPULL, 1); 
        HAL_PIN_Set(PAD_PA34, SD1_CMD, PIN_PULLUP, 1);
        HAL_PIN_Set(PAD_PA41, SD1_DIO0, PIN_PULLUP, 1);
        HAL_PIN_Set(PAD_PA30, SD1_DIO1, PIN_PULLUP, 1);
        HAL_PIN_Set(PAD_PA36, SD1_DIO2, PIN_PULLUP, 1);
        HAL_PIN_Set(PAD_PA40, SD1_DIO3, PIN_PULLUP, 1);
        HAL_PIN_Set(PAD_PA38, SD1_DIO4, PIN_PULLUP, 1);
        HAL_PIN_Set(PAD_PA37, SD1_DIO5, PIN_PULLUP, 1);
        HAL_PIN_Set(PAD_PA35, SD1_DIO6, PIN_PULLUP, 1);
        HAL_PIN_Set(PAD_PA33, SD1_DIO7, PIN_NOPULL, 1);

        HAL_PIN_Set(PAD_PA80, GPIO_A80, PIN_PULLUP, 1);     // SD1 EN
        //BSP_GPIO_Set(2, 1, 1);
        HAL_PIN_Set(PAD_PA49, GPIO_A49, PIN_PULLUP, 1);     // SD1 RESET, need set 0 first?
        //BSP_GPIO_Set(11, 1, 1);
	}
    HAL_PIN_Set(PAD_PB37, USART4_TXD, PIN_PULLUP, 0);
    HAL_PIN_Set(PAD_PB36, USART4_RXD, PIN_PULLUP, 0);
    HAL_PIN_Set(PAD_PB18, USART5_TXD, PIN_PULLUP, 0);
    HAL_PIN_Set(PAD_PB17, USART5_RXD, PIN_PULLUP, 0);


    BSP_GPIO_Set(SD1_EN_PIN, 1, 1);
    BSP_GPIO_Set(SD1_RESET_PIN, 1, 1);


#if defined(CFG_FACTORY_DEBUG)
    if (user_pmic_cfg() == false)
    {
        BSP_PMIC_Init(-1, -1);
        BSP_PMIC_Control(PMIC_OUT_1V8_LVSW100_1, 1, 1); //LCD_1V8 power
        BSP_PMIC_Control(PMIC_OUT_1V8_LVSW100_2, 1, 1);
        BSP_PMIC_Control(PMIC_OUT_1V8_LVSW100_3, 1, 1);
        BSP_PMIC_Control(PMIC_OUT_1V8_LVSW100_4, 1, 1);
        BSP_PMIC_Control(PMIC_OUT_1V8_LVSW100_5, 1, 1); 	
        BSP_PMIC_Control(PMIC_OUT_LDO33_VOUT, 1, 1);
    }

    user_pin_cfg();
#endif
    
    __HAL_WDT_DISABLE();

    // delay 6ms to wait for flash power stable
    //HAL_Delay_us(0);
    //HAL_Delay_us(50000);
    {
        uint32_t sysFreq = HAL_RCC_GetSysCLKFreq(CORE_ID_HCPU);
        volatile uint32_t lootMs = 20 * (sysFreq / 1000) / 5;

        if (lootMs == 0)
        {
            lootMs = 480000;
        }

        while (lootMs-- > 0);
    }
}


void HAL_MspInit(void)
{
    JLINK_DRV_BSP_PIN_Init();

    /*##-1- Configure the UART peripheral ######################################*/
    /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
    /* UART configured as follows:
        - Word Length = 8 Bits (7 data bit + 1 parity bit) :
       BE CAREFUL : Program 7 data bits + 1 parity bit in PC HyperTerminal
        - Stop Bit    = One Stop bit
        - Parity      = ODD parity
        - BaudRate    = 9600 baud
        - Hardware flow control disabled (RTS and CTS signals) */
    UartHandle.Instance        = USART4;
    UartHandle.Init.BaudRate   = 1000000;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        /* Initialization Error */
        HAL_ASSERT(0);
    }
    //debug_print("Init\r\n");
}


HAL_StatusTypeDef HAL_EFUSE_Init()
{
    return HAL_OK;
}

//HAL_StatusTypeDef HAL_HPAON_StartGTimer(void)
//{
//    return HAL_OK;
//}

/* Private functions ---------------------------------------------------------*/

#ifdef JLINK
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    /* HAL library initialization:
         - Configure the Flash prefetch, instruction and Data caches
         - Systick timer is configured by default as source of time base, but user
           can eventually implement his proper time base source (a general purpose
           timer for example or other time source), keeping in mind that Time base
           duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
           handled in milliseconds basis.
         - Set NVIC Group Priority to 4
         - Low Level Initialization: global MSP (MCU Support Package) initialization
       */
    /* Initialize system resurces, such as MPU, SCB, efuse etc. */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

uint32_t SystemCoreClock = 48000000;

void SystemInit(void)
{
}

pm_power_on_mode_t SystemPowerOnModeGet(void)
{
    return PM_COLD_BOOT;
}

/**
  * @}
  */

/**
  * @}
  */
