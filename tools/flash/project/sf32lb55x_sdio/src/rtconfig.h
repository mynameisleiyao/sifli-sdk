#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* Sifli Configuration */

#define SOC_SF32LB55X 1

#define CORE "HCPU"
#define CPU "Cortex-M33"

/* RTOS */

#define BSP_USING_NO_OS
#define RT_USING_LIBC
#define BSP_USING_SPI_FLASH 1
#define BSP_ENABLE_QSPI1 1
#define BSP_QSPI1_MODE 0
#define BSP_QSPI1_USING_DMA 1
#define BSP_QSPI1_MEM_SIZE 1

/* Select board peripherals */

#define BSP_USING_BOARD_FPGA_A0

/* Key config */

/* Sifli Middleware */

#define BSP_USING_EMPTY_ASSERT

/* RW Bluetooth */


/* Third party packages */

#define BF0_HCPU


#define rt_hw_interrupt_enable(level)
#define rt_hw_interrupt_disable()  0
#define RT_EOK      0
#define RT_ERROR 1

#endif
