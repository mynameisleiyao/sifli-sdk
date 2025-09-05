/*********************************************************************
*            (c) 1995 - 2018 SEGGER Microcontroller GmbH             *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
----------------------------------------------------------------------
File    : FlashPrg.c
Purpose : Implementation of RAMCode template
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <bf0_hal.h>
#include <string.h>
#include <stdio.h>
#include "SdioOS.h"


#define ECC_OFFSET   (8)
#define ECC_SIZE     (8)

typedef enum
{
    JLINK_STAGE_COMPARE,
    JLINK_STAGE_ERASE,
    JLINK_STAGE_PROGRAM,
    JLINK_STAGE_VERIFY
} jlink_stage_t;


#define JLINK_FUNC_INVALID      (0)
#define JLINK_FUNC_ERASE        (1)
#define JLINK_FUNC_PROGRAM      (2)
#define JLINK_FUNC_VERIFY_READ  (3)

#define DEBUG_JLINK         1
#define DEFAULT_TRACE       1


extern void debug_print(char *str);
extern struct FlashDevice const FlashDevice;

SDHCI_HandleTypeDef sd1_handle;

static uint8_t sd_buf[512];

#ifdef _USE_PRODUCTLINE
uint32_t Crc32Table1[256] =
{
    0x0, 0x77073096, 0xee0e612c, 0x990951ba, 0x76dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0xedb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x9b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x1db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x6b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0xf00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x86d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x3b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x4db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0xd6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0xa00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x26d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x5005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0xcb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0xbdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t Crc32Table2[256] =
{
    0x0, 0xf26b8303, 0xe13b70f7, 0x1350f3f4, 0xc79a971f, 0x35f1141c, 0x26a1e7e8, 0xd4ca64eb,
    0x8ad958cf, 0x78b2dbcc, 0x6be22838, 0x9989ab3b, 0x4d43cfd0, 0xbf284cd3, 0xac78bf27, 0x5e133c24,
    0x105ec76f, 0xe235446c, 0xf165b798, 0x30e349b, 0xd7c45070, 0x25afd373, 0x36ff2087, 0xc494a384,
    0x9a879fa0, 0x68ec1ca3, 0x7bbcef57, 0x89d76c54, 0x5d1d08bf, 0xaf768bbc, 0xbc267848, 0x4e4dfb4b,
    0x20bd8ede, 0xd2d60ddd, 0xc186fe29, 0x33ed7d2a, 0xe72719c1, 0x154c9ac2, 0x61c6936, 0xf477ea35,
    0xaa64d611, 0x580f5512, 0x4b5fa6e6, 0xb93425e5, 0x6dfe410e, 0x9f95c20d, 0x8cc531f9, 0x7eaeb2fa,
    0x30e349b1, 0xc288cab2, 0xd1d83946, 0x23b3ba45, 0xf779deae, 0x5125dad, 0x1642ae59, 0xe4292d5a,
    0xba3a117e, 0x4851927d, 0x5b016189, 0xa96ae28a, 0x7da08661, 0x8fcb0562, 0x9c9bf696, 0x6ef07595,
    0x417b1dbc, 0xb3109ebf, 0xa0406d4b, 0x522bee48, 0x86e18aa3, 0x748a09a0, 0x67dafa54, 0x95b17957,
    0xcba24573, 0x39c9c670, 0x2a993584, 0xd8f2b687, 0xc38d26c, 0xfe53516f, 0xed03a29b, 0x1f682198,
    0x5125dad3, 0xa34e59d0, 0xb01eaa24, 0x42752927, 0x96bf4dcc, 0x64d4cecf, 0x77843d3b, 0x85efbe38,
    0xdbfc821c, 0x2997011f, 0x3ac7f2eb, 0xc8ac71e8, 0x1c661503, 0xee0d9600, 0xfd5d65f4, 0xf36e6f7,
    0x61c69362, 0x93ad1061, 0x80fde395, 0x72966096, 0xa65c047d, 0x5437877e, 0x4767748a, 0xb50cf789,
    0xeb1fcbad, 0x197448ae, 0xa24bb5a, 0xf84f3859, 0x2c855cb2, 0xdeeedfb1, 0xcdbe2c45, 0x3fd5af46,
    0x7198540d, 0x83f3d70e, 0x90a324fa, 0x62c8a7f9, 0xb602c312, 0x44694011, 0x5739b3e5, 0xa55230e6,
    0xfb410cc2, 0x92a8fc1, 0x1a7a7c35, 0xe811ff36, 0x3cdb9bdd, 0xceb018de, 0xdde0eb2a, 0x2f8b6829,
    0x82f63b78, 0x709db87b, 0x63cd4b8f, 0x91a6c88c, 0x456cac67, 0xb7072f64, 0xa457dc90, 0x563c5f93,
    0x82f63b7, 0xfa44e0b4, 0xe9141340, 0x1b7f9043, 0xcfb5f4a8, 0x3dde77ab, 0x2e8e845f, 0xdce5075c,
    0x92a8fc17, 0x60c37f14, 0x73938ce0, 0x81f80fe3, 0x55326b08, 0xa759e80b, 0xb4091bff, 0x466298fc,
    0x1871a4d8, 0xea1a27db, 0xf94ad42f, 0xb21572c, 0xdfeb33c7, 0x2d80b0c4, 0x3ed04330, 0xccbbc033,
    0xa24bb5a6, 0x502036a5, 0x4370c551, 0xb11b4652, 0x65d122b9, 0x97baa1ba, 0x84ea524e, 0x7681d14d,
    0x2892ed69, 0xdaf96e6a, 0xc9a99d9e, 0x3bc21e9d, 0xef087a76, 0x1d63f975, 0xe330a81, 0xfc588982,
    0xb21572c9, 0x407ef1ca, 0x532e023e, 0xa145813d, 0x758fe5d6, 0x87e466d5, 0x94b49521, 0x66df1622,
    0x38cc2a06, 0xcaa7a905, 0xd9f75af1, 0x2b9cd9f2, 0xff56bd19, 0xd3d3e1a, 0x1e6dcdee, 0xec064eed,
    0xc38d26c4, 0x31e6a5c7, 0x22b65633, 0xd0ddd530, 0x417b1db, 0xf67c32d8, 0xe52cc12c, 0x1747422f,
    0x49547e0b, 0xbb3ffd08, 0xa86f0efc, 0x5a048dff, 0x8ecee914, 0x7ca56a17, 0x6ff599e3, 0x9d9e1ae0,
    0xd3d3e1ab, 0x21b862a8, 0x32e8915c, 0xc083125f, 0x144976b4, 0xe622f5b7, 0xf5720643, 0x7198540,
    0x590ab964, 0xab613a67, 0xb831c993, 0x4a5a4a90, 0x9e902e7b, 0x6cfbad78, 0x7fab5e8c, 0x8dc0dd8f,
    0xe330a81a, 0x115b2b19, 0x20bd8ed, 0xf0605bee, 0x24aa3f05, 0xd6c1bc06, 0xc5914ff2, 0x37faccf1,
    0x69e9f0d5, 0x9b8273d6, 0x88d28022, 0x7ab90321, 0xae7367ca, 0x5c18e4c9, 0x4f48173d, 0xbd23943e,
    0xf36e6f75, 0x105ec76, 0x12551f82, 0xe03e9c81, 0x34f4f86a, 0xc69f7b69, 0xd5cf889d, 0x27a40b9e,
    0x79b737ba, 0x8bdcb4b9, 0x988c474d, 0x6ae7c44e, 0xbe2da0a5, 0x4c4623a6, 0x5f16d052, 0xad7d5351
};

uint32_t Crc32Table3[256] = {0};
#endif

uint8_t *htoa(uint8_t *p, uint32_t d)
{
    uint8_t *r = p;
    uint8_t ch;
    int i;

    p = r + 6;
    *(r + 8) = '\0';
    for (i = 0; i < 4; i++, p -= 3, d >>= 8)
    {
        ch = d & 0xff;
        (*p) = ch & 0xf0;
        (*p) >>= 4;
        if ((*p) < 10)
            *p += '0';
        else
            *p += ('A' - 10);
        p++;
        (*p) = ch & 0xf;
        if ((*p) < 10)
            *p += '0';
        else
            *p += ('A' - 10);
    }
    return r;
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/


#include "sdmmc_tst_drv.h"
extern uint8_t sdmmc_emmc();
int sd_read_data(uint32_t addr, uint8_t *data, uint32_t len);
extern int sd_write_data(uint32_t addr, uint8_t *data, uint32_t len);

int rt_hw_sd1_init()
{
    int res = 0;
    sd1_handle.Instance = (uint32_t)SDIO1;
    sd1_handle.ErrorCode = 0;
    sd1_handle.Init.flags &= ~(SDHCI_USE_ADMA | SDHCI_USE_SDMA | SDHCI_REQ_USE_DMA);

    //hal_sdhci_reset(&sd1_handle, SDHCI_RESET_ALL);

    res = sdmmc_emmc();
#if (DEBUG_JLINK+DEFAULT_TRACE)
    uint8_t hex[16];
    debug_print("Init : sd1 res -");
    debug_print((char *)htoa(hex, res));
    debug_print("\r\n");
#endif

    return res;
}

int rt_hw_sd2_init()
{
    HAL_StatusTypeDef res = HAL_ERROR;

    return res;
}

static int rt_hw_sd_init(void)
{
    rt_hw_sd1_init();

    rt_hw_sd2_init();

    return 1;
}


void init_clock()
{
    HAL_HPAON_EnableXT48();
    HAL_RCC_HCPU_ClockSelect(RCC_CLK_MOD_UART3, RCC_CLK_USART_HXT48);

    if (PM_COLD_BOOT == SystemPowerOnModeGet())
    {
        HAL_PMU_EnableDLL(1);

        HAL_HPAON_StartGTimer();
        //HAL_PMU_LpCLockSelect(PMU_LPCLK_XT32);

        HAL_RCC_HCPU_SetDiv(1, 1, 5);
        /* Configure in LCPU. */
    }

    //HAL_RCC_HCPU_EnableDLL1(240000000);
    //HAL_RCC_HCPU_ClockSelect(RCC_CLK_MOD_SYS, RCC_SYSCLK_DLL1);
    HAL_RCC_HCPU_ClockSelect(RCC_CLK_MOD_SYS, RCC_SYSCLK_HXT48);

    //HAL_RCC_HCPU_ClockSelect(RCC_CLK_MOD_SDMMC, RCC_CLK_FLASH_DLL2);
    HAL_RCC_HCPU_enable2(HPSYS_RCC_ENR2_SDMMC1, 1);

}


/*********************************************************************
*
*       Init
*
*  Function description
*    Handles the initialization of the flash module.
*
*  Parameters
*    Addr: Flash base address
*    Freq: Clock frequency in Hz
*    Func: Specifies the action followed by Init() (e.g.: 1 - Erase, 2 - Program, 3 - Verify / Read)
*
*  Return value
*    0 O.K.
*    1 Error
*/

__attribute__((used)) int Init(uint32_t Addr, uint32_t Freq, uint32_t Func)
{
    uint32_t sysFreq;

    SCB_DisableICache();
    SCB_DisableDCache();

    HAL_MspInit();

    //sysFreq = HAL_RCC_GetSysCLKFreq(CORE_ID_HCPU);
    //if (sysFreq / 1000000 < 90)
    {
        init_clock();
    }

    rt_hw_sd_init();


#if (DEBUG_JLINK+DEFAULT_TRACE)
    uint8_t hex[16];
    debug_print("Init : Addr-");
    debug_print((char *)htoa(hex, Addr));
    debug_print(" Func-");
    debug_print((char *)htoa(hex, Func));
    debug_print(" sysFreq-");
    debug_print((char *)htoa(hex, sysFreq));
    debug_print("\r\n");
#endif

    return 0;                                  // Finished without Errors
}


/*********************************************************************
*
*       UnInit
*
*  Function description
*    Handles the de-initialization of the flash module.
*
*  Parameters
*    Func: Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
*
*  Return value
*    0 O.K.
*    1 Error
*/
__attribute__((used)) int UnInit(uint32_t Func)
{
    (void)Func;
    return 0;                                  // Finished without Errors
}


/*********************************************************************
*
*       EraseSector
*
*  Function description
*    Erases one flash sector.
*
*  Parameters
*    SectorAddr: Absolute address of the sector to be erased
*
*  Return value
*    0 O.K.
*    1 Error
*/
__attribute__((used)) int EraseSector(uint32_t SectorAddr)
{
#if DEBUG_JLINK
    uint8_t hex[16];
    debug_print("EraseSector : Addr-");
    debug_print((char *)htoa(hex, SectorAddr));
    debug_print(" Size-");
    debug_print((char *)htoa(hex, PAGE_SIZE));
    debug_print("\r\n");
#endif

    //int res = HAL_QSPIEX_FLASH_ERASE(Addr2Handle(SectorAddr), SectorAddr, PAGE_SIZE);

    return 0;
}


/*********************************************************************
*
*       ProgramPage
*
*  Function description
*    Programs one flash page.
*
*  Parameters
*    DestAddr: Destination address
*    NumBytes: Number of bytes to be programmed (always a multiple of program page size, defined in FlashDev.c)
*    pSrcBuff: Point to the source buffer
*
*  Return value
*    0 O.K.
*    1 Error
*/
__attribute__((used)) int ProgramPage(uint32_t DestAddr, uint32_t NumBytes, uint8_t *pSrcBuff)
{
    int ret;

#if DEBUG_JLINK
    uint8_t hex[16];
    debug_print("ProgramPage :");
    debug_print((char *)htoa(hex, DestAddr));
    debug_print("\r\n");
#endif
    //ret = HAL_QSPIEX_FLASH_WRITE(Addr2Handle(DestAddr), DestAddr, pSrcBuff, NumBytes);

    return (ret < NumBytes) ? -1 : 0;
}


/*********************************************************************
*
*       Verify
*
*  Function description
*    Compares a specified number of bytes of a provided data
*    buffer with the content of the device
*
*  Parameters
*    Addr: Start address in memory which should be compared
*    NumBytes: Number of bytes to be compared
*    pBuff: Pointer to the data to be compared
*
*  Return value
*    == (Addr + NumBytes): O.K.
*    != (Addr + NumBytes): *not* O.K. (ideally the fail address is returned)
*
*/


/*********************************************************************
*
*       BlankCheck
*
*  Function description
*    Checks if a memory region is blank
*
*  Parameters
*    Addr: Blank check start address
*    NumBytes: Number of bytes to be checked
*    BlankData: Pointer to the destination data
*
*  Return value
*    0: O.K., blank
*    1: O.K., *not* blank
*    < 0: Error
*
*/

/*********************************************************************
*
*       EraseChip
*
*  Function description
*    Erases the entire flash
*
*  Return value
*    0: O.K.
*    1: Error
*/

/*********************************************************************
*
*       SEGGER_OPEN_Read
*
*  Function description
*    Reads a specified number of bytes into the provided buffer
*
*  Parameters
*    Addr: Start read address
*    NumBytes: Number of bytes to be read
*    pBuff: Pointer to the destination data
*
*  Return value
*    >= 0: O.K., NumBytes read
*    <  0: Error
*
*/
#ifdef _USE_PRODUCTLINE
__attribute__((used)) int SEGGER_OPEN_Read(uint32_t Addr, uint32_t NumBytes, uint8_t *pDestBuff)
{
    int ret;

#if DEBUG_JLINK
    uint8_t hex[16];
    debug_print("SEGGER_OPEN_Read : Addr-");
    debug_print((char *)htoa(hex, Addr));
    debug_print(" Size-");
    debug_print((char *)htoa(hex, NumBytes));
    debug_print(" DstAddr-");
    debug_print((char *)htoa(hex, (uint32_t)pDestBuff));
    debug_print(" DevBase-");
    debug_print((char *)htoa(hex, (uint32_t)FlashDevice.BaseAddr));
    debug_print("\r\n");
#endif
#if 1
    {
        int retry = 3;
        uint32_t len = NumBytes;
        uint8_t *dst = pDestBuff;
        uint32_t src = Addr - FlashDevice.BaseAddr;

        if ((src & 0x1ff) != 0) // not page aligned
        {
            uint32_t start = src & 0xfffffe00;
            uint32_t offset  = src & 0x1ff;
            uint32_t needcpy = 512 - offset;
            ret = sd_read_data(start, sd_buf, 512);
            memcpy(dst, sd_buf+offset, needcpy);
#if 0
            uint32_t *dumpbuf;
            int i;
            dumpbuf = (uint32_t *)dst;
            debug_print("Not aligned data: 0x");
            for(i=0; i<4; i++)
            {
                debug_print((char *)htoa(hex, *dumpbuf));
                debug_print(" 0x");
                dumpbuf++;
            }
            debug_print("\r\n");
#endif 

            src += needcpy;
            dst += needcpy;
            len -= needcpy;
        }
        while (len >= 512)
        {
            retry = 3;
            while(retry > 0)
            {
                ret = sd_read_data(src, dst, 512);
                HAL_Delay_us(50);
                if(ret > 0)
                {
#if 0
                    uint32_t *dumpbuf;
                    int i;
                    dumpbuf = (uint32_t *)dst;
                    debug_print("data: 0x");
                    for(i=0; i<4; i++)
                    {
                        debug_print((char *)htoa(hex, *dumpbuf));
                        debug_print(" 0x");
                        dumpbuf++;
                    }
                    debug_print("\r\n");
#endif                
                    break;
                }
                retry--;
            }
#if DEBUG_JLINK
            if(retry == 0)
            {
                debug_print("read fail at pos ");
                debug_print((char *)htoa(hex, src));
                debug_print("\r\n");
            }
#endif

            src += 512;
            dst += 512;
            len -= 512;
        }
        if (len > 0)
        {
            ret = sd_read_data(src, sd_buf, 512);
            memcpy(dst, sd_buf, len);
        }
    }
#endif
    return NumBytes;
}
#endif
/*********************************************************************
*
*       SEGGER_OPEN_Program
*
*  Function description
*    Programs a specified number of bytes into the target flash.
*    NumBytes is either FlashDevice.PageSize or a multiple of it.
*
*  Notes
*    (1) This function can rely on that at least FlashDevice.PageSize will be passed
*    (2) This function must be able to handle multiple of FlashDevice.PageSize
*
*  Parameters
*    Addr: Start read address
*    NumBytes: Number of bytes to be read
*    pBuff: Pointer to the destination data
*
*  Return value
*    0 O.K.
*    1 Error
*
*/
#ifdef _USE_PRODUCTLINE
__attribute__((used)) int SEGGER_OPEN_Program(uint32_t DestAddr, uint32_t NumBytes, uint8_t *pSrcBuff)
{
    int ret = -1;
    int verify = -1;
    int cnt = 0;
#if DEBUG_JLINK+DEFAULT_TRACE
    uint8_t hex[16];
#endif
#if DEBUG_JLINK
    debug_print("SEGGER_OPEN_Program : Dst-");
    debug_print((char *)htoa(hex, DestAddr));
    debug_print(" Src-");
    debug_print((char *)htoa(hex, (uint32_t)pSrcBuff));
    debug_print(" Len-");
    debug_print((char *)htoa(hex, (uint32_t)NumBytes));
    debug_print(" DevBase-");
    debug_print((char *)htoa(hex, (uint32_t)FlashDevice.BaseAddr));
    debug_print("\r\n");
#endif
#if 1
    {
        int retry = 3;
        uint32_t len = NumBytes;
        uint8_t *src = pSrcBuff;
        uint32_t dst = DestAddr - FlashDevice.BaseAddr;
        ret = 0;
        cnt = 0;
        while (len >= 512)
        {
            retry = 3;
            while(retry > 0)
            {
                ret = sd_write_data(dst, src, 512);
                HAL_Delay_us(50);
                if(ret > 0)
                    break;
                retry--;
            }
#if DEBUG_JLINK
            if(retry == 0)
            {
                debug_print("write fail at pos ");
                debug_print((char *)htoa(hex, dst));
                debug_print("\r\n");
            }
#endif

            len -= 512;
            dst += 512;
            src += 512;
            cnt += 512;
        }
        if (len > 0)
        {
            //sd_read_data(dst, sd_buf, 512);
            //memcpy(sd_buf, src, len);
            //ret += sd_write_data(dst, sd_buf, 512);
            ret = sd_write_data(dst, src, 512);
            //len -= len;
            dst += len;
            src += len;
            cnt += len;
        }
    }
#endif
    if (cnt >= NumBytes)
    {
        /*
        for (int m = 0; m < NumBytes / 0x1000; m++)
        {

            verify = memcmp((uint8_t *)(pSrcBuff + 0x1000 * m), (uint8_t *)(DestAddr + 0x1000 * m), 0x1000);
            if (verify != 0)
            {
                break;
            }
        }*/
        verify = 0;
    }

#if DEBUG_JLINK
    debug_print(" ret-");
    debug_print((char *)htoa(hex, (uint32_t)ret));
    debug_print(" verify-");
    debug_print((char *)htoa(hex, (uint32_t)verify));
    debug_print("\r\n");
#endif

    return verify;
}
#endif

/*********************************************************************
*
*       SEGGER_OPEN_Erase
*
*  Function description
*    Erases one or more flash sectors
*
*  Notes
*    (1) This function can rely on that at least one sector will be passed
*    (2) This function must be able to handle multiple sectors at once
*    (3) This function can rely on that only multiple sectors of the same sector
*        size will be passed. (e.g. if the device has two sectors with different
*        sizes, the DLL will call this function two times with NumSectors = 1)
*
*  Parameters
*    SectorAddr: Address of the start sector to be erased
*    SectorIndex: Index of the start sector to be erased
*    NumSectors: Number of sectors to be erased. At least 1 sector is passed.
*
*  Return value
*    0 O.K.
*    1 Error
*
*/
#ifdef _USE_PRODUCTLINE
__attribute__((used)) int SEGGER_OPEN_Erase(uint32_t SectorAddr, uint32_t SectorIndex, uint32_t NumSectors)
{
    int res = 0;

#if DEBUG_JLINK
    uint8_t hex[16];
    debug_print("SEGGER_OPEN_Erase : Addr-");
    debug_print((char *)htoa(hex, SectorAddr));
    debug_print(" Index-");
    debug_print((char *)htoa(hex, SectorIndex));
    debug_print(" Sectors-");
    debug_print((char *)htoa(hex, NumSectors));
    debug_print(" Size-");
    debug_print((char *)htoa(hex, PAGE_SIZE));
#endif

    //res = HAL_QSPIEX_FLASH_ERASE(Addr2Handle(SectorAddr), SectorAddr, NumSectors * SPI_NAND_BLK_SIZE);

__EXIT:

#if DEBUG_JLINK
    debug_print(" res-");
    debug_print((char *)htoa(hex, res));
    debug_print("\r\n");
#endif

    return res;
}
#endif



/*********************************************************************
*
*       SEGGER_OPEN_CalcCRC
*
*  Function description
*    Calculates the CRC over a specified number of bytes
*    Even more optimized version of Verify() as this avoids downloading the compare data into the RAMCode for comparison.
*    Heavily reduces traffic between J-Link software and target and therefore speeds up verification process significantly.
*
*  Parameters
*    CRC       CRC start value
*    Addr      Address where to start calculating CRC from
*    NumBytes  Number of bytes to calculate CRC on
*    Polynom   Polynom to be used for CRC calculation
*
*  Return value
*    CRC
*
*  Notes
*    (1) This function is optional
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not accidentally removed by linker from ELF file.
*/

#ifdef _USE_PRODUCTLINE
void init_CRC32_table(uint32_t Polynom)
{
    for (int i = 0; i < 256; i++)
    {
        uint32_t crcs = i;

        for (int j = 0; j < 8; j++)
        {
            if (crcs & 1)
            {
                crcs = (crcs >> 1) ^ Polynom;
            }
            else
            {
                crcs >>= 1;
            }
        }
        Crc32Table3[i] = crcs;
    }
}

static uint8_t sdio_cache[PAGE_SIZE];

uint32_t SEGGER_OPEN_CalcCRC(uint32_t crcs, uint32_t Addr, uint32_t NumBytes, uint32_t Polynom)
{
    uint8_t   *pCrcData;
    uint32_t  *pCrcTable = NULL;
    uint32_t remain, fill, offset;

#if (DEBUG_JLINK)
    uint8_t hex[16];
    debug_print("SEGGER_OPEN_CalcCRC : crcs-");
    debug_print((char *)htoa(hex, crcs));
    debug_print(" Addr-");
    debug_print((char *)htoa(hex, Addr));
    debug_print(" NumBytes-");
    debug_print((char *)htoa(hex, NumBytes));
    debug_print(" Polynom-");
    debug_print((char *)htoa(hex, Polynom));
    debug_print("\r\n");
#endif


    pCrcData = (uint8_t *)(&sdio_cache[0]);
    remain = NumBytes;
    offset = Addr;
    //fill = remain >= PAGE_SIZE ? PAGE_SIZE : remain;
    //SEGGER_OPEN_Read(Addr, fill, pCrcData);
    //remain -= fill;

    if (Polynom == 0xEDB88320)
    {
        pCrcTable = Crc32Table1;
    }
    else if (Polynom == 0x82F63B78)
    {
        pCrcTable = Crc32Table2;
    }
    else
    {
        init_CRC32_table(Polynom);
        pCrcTable = Crc32Table3;
    }

    //crcs = 0;
    while(remain > 0)
    {
        pCrcData = (uint8_t *)(&sdio_cache[0]);
        fill = remain >= PAGE_SIZE ? PAGE_SIZE : remain;
        SEGGER_OPEN_Read(offset, fill, pCrcData);
        remain -= fill;
        offset+=fill;
        /* Always reprogram all if flashing file system */
        while (fill--)
        {
            crcs = (crcs >> 8) ^ pCrcTable[(crcs & 0xFF) ^ *pCrcData++];
        }
    }

#if (DEBUG_JLINK)
    debug_print(" crc-");
    debug_print((char *)htoa(hex, crcs));
    debug_print("\r\n");
#endif

    return crcs;
}
#endif

/**************************** End of file ***************************/
