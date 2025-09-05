/**
  ******************************************************************************
  * @file   os_adaptor_rtthread.h
  * @author Sifli software development team
  * @brief Header file - OS adatpor interface with RTThread OS.
 *
  ******************************************************************************
*/
/*
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

#ifndef __OS_ADAPTOR_RTTHREAD_H
#define __OS_ADAPTOR_RTTHREAD_H

#include "rtconfig.h"


#ifdef BSP_USING_RTTHREAD

#undef __ROM_USED

#include "rtthread.h"
#include "sf_type.h"
#include "os_adaptor_rtthread_internal.h"
#undef OS_WAIT_FORVER
#define OS_WAIT_FORVER                 RT_WAITING_FOREVER

#undef OS_EVENT_FLAG_WAIT_ALL
#define OS_EVENT_FLAG_WAIT_ALL         RT_EVENT_FLAG_AND

#undef OS_EVENT_FLAG_WAIT_ANY
#define OS_EVENT_FLAG_WAIT_ANY         RT_EVENT_FLAG_OR

#undef OS_EVENT_FLAG_CLEAR
#define OS_EVENT_FLAG_CLEAR            RT_EVENT_FLAG_CLEAR

#undef OS_TIMER_FLAG_ONE_SHOT
#define OS_TIMER_FLAG_ONE_SHOT         RT_TIMER_FLAG_ONE_SHOT

#undef OS_TIMER_FLAG_PERIODIC
#define OS_TIMER_FLAG_PERIODIC         RT_TIMER_FLAG_PERIODIC

#undef OS_TIMER_FLAG_SOFT
#define OS_TIMER_FLAG_SOFT         RT_TIMER_FLAG_SOFT_TIMER

#undef OS_ASSERT
#define OS_ASSERT                      RT_ASSERT

// To avoid include rtthread before os_adatpor
#ifndef __ROM_USED
    #ifdef ROM_ENABLED
        #define __ROM_USED RT_WEAK
    #else
        #define __ROM_USED
    #endif
#endif

#define os_enter_critical() rt_enter_critical()

#define os_exit_critical() rt_exit_critical()

#define os_interrupt_disable() rt_hw_interrupt_disable()

#define os_interrupt_enable(mask) rt_hw_interrupt_enable(mask)

#define os_interrupt_enter()        rt_interrupt_enter()
#define os_interrupt_exit()         rt_interrupt_leave()

#undef OS_TIMER_DECLAR
#define OS_TIMER_DECLAR(timer_id) \
    struct rt_timer _##timer_id; \
    rt_timer_t timer_id = &_##timer_id;

#ifdef RT_USING_MUTEX
#undef OS_MUTEX_DECLAR
#define OS_MUTEX_DECLAR(mutex) \
    rt_mutex_t mutex
#endif

#define os_delay(ms) \
    rt_thread_mdelay((int32_t)ms)


#ifdef RT_USING_MESSAGEQUEUE
#define os_message_queue_create(queue, max_count, msg_size, mem_pool, pool_size) \
    queue = os_message_queue_create_int(STRINGIFY(queue), max_count, msg_size, mem_pool, pool_size)


#define os_message_put(queue, data, data_size, timeout) \
    rt_mq_send((rt_mq_t)(((os_handle_t)queue)->handle), data, data_size)


#define os_message_get(queue, data, data_size, timeout) \
    rt_mq_recv((rt_mq_t)(((os_handle_t)queue)->handle), data, data_size, timeout)

#define os_message_delete(queue) \
    os_message_delele_int(queue)
#endif // RT_USING_MESSAGEQUEUE

#ifdef RT_USING_MAILBOX

#define os_mailbox_create(name, size, mem_pool) \
    os_mailbox_create_int(name, size, mem_pool)

#define os_mailbox_put(mailbox, data) \
    rt_mb_send((rt_mailbox_t)(((os_handle_t)mailbox)->handle), data)

#define os_mailbox_get(mailbox, data, timeout) \
    rt_mb_recv((rt_mailbox_t)(((os_handle_t)mailbox)->handle), (rt_uint32_t *)data, (rt_uint32_t)timeout)

#define os_mailbox_delete(mailbox) \
    os_mailbox_delete_int(mailbox)

#endif // RT_USING_MAILBOX


#ifdef RT_USING_EVENT
#define os_event_create(event) \
    event = (os_event_t)rt_event_create(STRINGIFY(event), RT_IPC_FLAG_FIFO)

#define os_event_flags_set(event, flags) \
    rt_event_send(event, flags)

#define os_event_flags_wait(event, flags, option, timeout, recv_evt) \
    rt_event_recv(event, flags, (rt_uint8_t)option, timeout, (rt_uint32_t *)recv_evt)

#define os_event_delete(event) \
    rt_event_delete(event)
#endif // RT_USING_EVENT


#endif // BSP_USING_RTTHREAD



#endif // __OS_ADAPTOR_RTTHREAD_H

/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/
