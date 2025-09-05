/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BF0_SIBLES_UTIL_H_
#define _BF0_SIBLES_UTIL_H_


/*
* INCLUDE FILES
****************************************************************************************
*/
#include <board.h>
#include <string.h>
#include "bf0_ble_common.h"
#include "os_adaptor.h"

/*
* ENUMERATION
****************************************************************************************
*/

#ifndef __ARRAY_EMPTY
    #define __ARRAY_EMPTY
#endif

///UART header: command message type
#define HCI_CMD_MSG_TYPE                            0x01

///UART header: ACL data message type
#define HCI_ACL_MSG_TYPE                            0x02

///UART header: event message type
#define HCI_EVT_MSG_TYPE                            0x04

#define AHI_TYPE                                    0x05

/// Message Identifier. The number of messages is limited to 0xFFFF.
/// The message ID is divided in two parts:
/// bits[15~8]: task index (no more than 255 tasks support)
/// bits[7~0]: message index(no more than 255 messages per task)
/*@TRACE*/
typedef uint16_t sifli_msg_id_t;

/// Task Identifier. Composed by the task type and the task index.
typedef uint16_t sifli_task_id_t;

/// Message structure.
typedef __PACKED_STRUCT
{
    uint8_t            type;       ///< AHI type, the value will be 5
    sifli_msg_id_t     id;         ///< Message id.
    sifli_task_id_t    dest_id;    ///< Destination kernel identifier.
    sifli_task_id_t    src_id;     ///< Source kernel identifier.
    uint16_t           param_len;  ///< Parameter embedded struct length.
    uint32_t           param[__ARRAY_EMPTY];   ///< Parameter embedded struct. Must be word-aligned.
} sibles_msg_header_t;

typedef struct
{
    sifli_msg_id_t     id;         ///< Message id.
    sifli_task_id_t    dest_id;    ///< Destination kernel identifier.
    sifli_task_id_t    src_id;     ///< Source kernel identifier.
    uint16_t           param_len;  ///< Parameter embedded struct length.
    uint32_t           param[__ARRAY_EMPTY];   ///< Parameter embedded struct. Must be word-aligned.
} sibles_msg_para_t;


/*
* FUNCTION DECLARATIONS
****************************************************************************************
*/


#if defined(SOC_SF32LB55X) && defined(SOC_BF0_HCPU)
/**
 ****************************************************************************************
 * @brief Convert a parameter pointer to a message pointer
 *
 * @param[in]  param_ptr Pointer to the parameter member of a ke_msg
 *                       Usually retrieved by a sifli_msg_alloc()
 *
 * @return The pointer to the ke_msg
 ****************************************************************************************
 */
__INLINE sibles_msg_para_t *sifli_param2msg(void const *param_ptr)
{
    return (sibles_msg_para_t *)(((uint8_t *)param_ptr) - offsetof(sibles_msg_para_t, param));
}

/**
 ****************************************************************************************
 * @brief Convert a message pointer to a parameter pointer
 *
 * @param[in]  msg Pointer to the sibles_msg_para_t.
 *
 * @return The pointer to the param member
 ****************************************************************************************
 */
__INLINE void *sifli_msg2param(sibles_msg_para_t const *msg)
{
    return (void *)(((uint8_t *) msg) + offsetof(sibles_msg_para_t, param));
}

#else // defined(SOC_SF32LB55X) && defined(SOC_BF0_HCPU)

/**
 ****************************************************************************************
 * @brief Convert a parameter pointer to a message pointer
 *
 * @param[in]  param_ptr Pointer to the parameter member of a ke_msg
 *                       Usually retrieved by a sifli_msg_alloc()
 *
 * @return The pointer to the ke_msg
 ****************************************************************************************
 */
__INLINE sibles_msg_para_t *sifli_param2msg(void const *param_ptr)
{
    return (sibles_msg_para_t *)(((uint8_t *)param_ptr) - offsetof(sibles_msg_para_t, param));
}


#endif //BLUETOOTH_STACK

uint8_t *sifli_get_mbox_buffer(void);

void *sifli_msg_alloc(sifli_msg_id_t const id, sifli_task_id_t const dest_id,
                      sifli_task_id_t const src_id, uint16_t const param_len);


void sifli_msg_send(void const *param_ptr);


void sibles_attm_convert_to128(uint8_t *uuid128, uint8_t *uuid, uint8_t uuid_len);

void sibles_covert_to_raw_data(void *src, void *dest, uint8_t data_len);

sifli_task_id_t sifli_get_stack_id(void);


#ifdef BLUETOOTH

#if  !defined(BSP_USING_PC_SIMULATOR) && defined(BF0_LCPU) && defined(SOC_SF32LB55X)
#define sifli_msg_alloc  ble_stack_msg_alloc
#define sifli_msg_send  ble_stack_msg_send
#elif !defined(SOC_SF32LB55X)
extern void *ke_msg_alloc(sifli_msg_id_t const id, sifli_task_id_t const dest_id,
                          sifli_task_id_t const src_id, uint16_t const param_len);
extern void ahi_int_msg_send(void const *param_ptr);

#define sifli_msg_alloc  ke_msg_alloc
#define sifli_msg_send  ahi_int_msg_send

#endif

int32_t ble_event_process(uint16_t const msgid, void const *param,
                          uint16_t const dest_id, uint16_t const src_id);
#endif

#endif // _BF0_SIBLES_UTIL_H_


