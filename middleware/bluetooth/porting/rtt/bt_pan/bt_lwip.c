/*
 * SPDX-FileCopyrightText: 2023-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rthw.h>
#include <rtthread.h>
#include <stdint.h>
//#include "bnep.h"
#include "bts2_app_pan.h"
#include "bt_lwip.h"
#include "bt_prot.h"
#include "bts2_app_demo.h"

#include "rtdef.h"
#include "bts2_bt.h"




#ifdef RT_USING_LWIP
#include <netif/ethernetif.h>
#include <lwip/netifapi.h>
#include "lwip/pbuf.h"
#include "lwip/timeouts.h"
#ifdef LWIP_USING_DHCPD
    #include <dhcp_server.h>
#endif



#define DBG_TAG "BT.lwip"
//#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifndef IPADDR_STRLEN_MAX
    #define IPADDR_STRLEN_MAX    (32)
#endif

struct bt_lwip_prot_des
{
    struct rt_bt_prot prot;
    struct eth_device eth;
    rt_int8_t connected_flag;
};

extern BTS2S_ETHER_ADDR   bts2_local_ether_addr;
extern void lwip_system_uninit(void);
extern void lwip_sys_init();

void rt_bt_lwip_event_handle(struct rt_bt_lwip_pan_dev *bt_dev, int event)
{
    struct bt_lwip_prot_des *lwip_prot = (struct bt_lwip_prot_des *)bt_dev->prot;
    struct eth_device *eth_dev = &lwip_prot->eth;

    switch (event)
    {
    case RT_BT_PROT_EVT_CONNECT:
    {
        LOG_D("event: CONNECT");
        lwip_prot->connected_flag = RT_TRUE;
        sys_timeouts_init();//to restart timer
        netifapi_netif_common(eth_dev->netif, netif_set_link_up, NULL);
#ifdef RT_LWIP_DHCP
        dhcp_start(eth_dev->netif);
#endif
        break;
    }
    case RT_BT_PROT_EVT_DISCONNECT:
    {
        LOG_D("event: DISCONNECT");
        lwip_prot->connected_flag = RT_FALSE;
        netifapi_netif_common(eth_dev->netif, netif_set_link_down, NULL);
#ifdef RT_LWIP_DHCP
        ip_addr_t ip_addr = { 0 };
        dhcp_stop(eth_dev->netif);
        netif_set_addr(eth_dev->netif, &ip_addr, &ip_addr, &ip_addr);
#endif
        lwip_system_uninit(); // to stop lwip timer
        break;
    }
    default :
    {
        LOG_D("event: UNKNOWN");
        break;
    }
    }
}

BTS2S_ETHER_ADDR bt_pan_get_mac_address()
{
    BTS2S_ETHER_ADDR   local_ether_addr;
    local_ether_addr = bts2_local_ether_addr;
    return local_ether_addr;
}

static rt_err_t rt_bt_lwip_protocol_control(rt_device_t device, int cmd, void *args)
{
    struct eth_device *eth_dev = (struct eth_device *)device;
    struct rt_bt_lwip_pan_dev  *bt_dev;
    rt_err_t err = RT_EOK;
    BTS2S_ETHER_ADDR   mac_addr;
    rt_uint8_t *address;

    RT_ASSERT(eth_dev != RT_NULL);

    address = (rt_uint8_t *)args;

    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get MAC address */
        bt_dev = eth_dev->parent.user_data;
        mac_addr = bt_pan_get_mac_address();

        address[0] = (mac_addr.w[0] >> 8) & 0xff;
        address[1] = mac_addr.w[0] & 0xff;
        address[2] = (mac_addr.w[1] >> 8) & 0xff;
        address[3] = mac_addr.w[1] & 0xff;
        address[4] = (mac_addr.w[2] >> 8) & 0xff;
        address[5] = mac_addr.w[2] & 0xff;

        LOG_D("address[0] %x\n", address[0]);
        LOG_D("address[1] %x\n", address[1]);
        LOG_D("address[2] %x\n", address[2]);
        LOG_D("address[3] %x\n", address[3]);
        LOG_D("address[4] %x\n", address[4]);
        LOG_D("address[5] %x\n", address[5]);

        //err = rt_device_control((rt_device_t)wlan, RT_WLAN_CMD_GET_MAC, args);
        break;
    default :
        break;
    }
    return err;
}

static rt_err_t rt_bt_lwip_protocol_recv(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len)
{
    struct eth_device *eth_dev = &((struct bt_lwip_prot_des *)bt_dev->prot)->eth;
    struct pbuf *p = RT_NULL;

    // LOG_D("F:%s L:%d run", __FUNCTION__, __LINE__);

    if (eth_dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    int count = 0;

    while (p == RT_NULL)
    {
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        if (p != RT_NULL)
            break;

        p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
        if (p != RT_NULL)
            break;

        LOG_D("F:%s L:%d wait for pbuf_alloc!", __FUNCTION__, __LINE__);
        rt_thread_delay(1);
        count++;

        //wait for 10ms or give up!!
        if (count >= 10)
        {
            LOG_W("F:%s L:%d pbuf allocate fail!!!", __FUNCTION__, __LINE__);
            return -RT_ENOMEM;
        }
    }

    /*copy data dat -> pbuf*/
    pbuf_take(p, buff, len);
    if ((eth_dev->netif->input(p, eth_dev->netif)) != ERR_OK)
    {
        LOG_D("F:%s L:%d IP input error", __FUNCTION__, __LINE__);
        pbuf_free(p);
        p = RT_NULL;
    }

    // LOG_D("F:%s L:%d netif iput success! len:%d", __FUNCTION__, __LINE__, len);
    return RT_EOK;

}

static rt_err_t rt_bt_lwip_protocol_send(rt_device_t device, struct pbuf *p)
{
    struct rt_bt_lwip_pan_dev *bt_dev = ((struct eth_device *)device)->parent.user_data;

    //LOG_D("F:%s L:%d run", __FUNCTION__, __LINE__);

    rt_uint8_t *frame;

    /* sending data directly */
    if (p->len == p->tot_len)
    {
        // rt_kprintf("enter rt_bt_lwip_protocol_send total ,total len %d\n",p->tot_len);
        frame = (rt_uint8_t *)p->payload;
        rt_bt_prot_send_data(bt_dev, frame, p->tot_len);
        // LOG_D("F:%s L:%d run len:%d", __FUNCTION__, __LINE__, p->tot_len);
        return RT_EOK;
    }

    frame = rt_malloc(p->tot_len);
    if (frame == RT_NULL)
    {
        LOG_E("F:%s L:%d malloc out_buf fail\n", __FUNCTION__, __LINE__);
        return -RT_ENOMEM;
    }
    /*copy pbuf -> data dat*/
    pbuf_copy_partial(p, frame, p->tot_len, 0);
    /* send data */
    //rt_kprintf("enter rt_bt_lwip_protocol_send fragment ,total len %d\n",p->tot_len);
    rt_bt_prot_send_data(bt_dev, frame, p->tot_len);
    // LOG_D("F:%s L:%d run len:%d", __FUNCTION__, __LINE__, p->tot_len);
    rt_free(frame);
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops bt_lwip_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    rt_bt_lwip_protocol_control
};
#endif

static struct rt_bt_prot *rt_bt_lwip_protocol_register(struct rt_bt_prot *prot, struct rt_bt_lwip_pan_dev *bt_dev)
{
    struct eth_device *eth = RT_NULL;
    char eth_name[4], timer_name[16];
    rt_device_t device = RT_NULL;
    static struct bt_lwip_prot_des *lwip_prot = NULL;
    lwip_sys_init();// to init lwip and lwip timer
    LOG_I("lwip:enter register_eth %d  %d\n", bt_dev, prot);

    if (bt_dev == RT_NULL || prot == RT_NULL)
        return RT_NULL;

    /* find ETH device name */
    eth_name[0] = 'b';
    eth_name[1] = '0';
    eth_name[2] = '\0';
    device = rt_device_find(eth_name);

    if (device == NULL)
    {
        RT_ASSERT(lwip_prot == NULL);
        lwip_prot = rt_malloc(sizeof(struct bt_lwip_prot_des));

        if (lwip_prot == RT_NULL)
        {
            //shutdown bnep connection????
            LOG_E("F:%s L:%d malloc mem failed", __FUNCTION__, __LINE__);
            return RT_NULL;
        }

        rt_memset(lwip_prot, 0, sizeof(struct bt_lwip_prot_des));
        eth = &lwip_prot->eth;

#ifdef RT_USING_DEVICE_OPS
        eth->parent.ops        = &bt_lwip_ops;
#else
        eth->parent.init       = RT_NULL;
        eth->parent.open       = RT_NULL;
        eth->parent.close      = RT_NULL;
        eth->parent.read       = RT_NULL;
        eth->parent.write      = RT_NULL;
        eth->parent.control    = rt_bt_lwip_protocol_control;
#endif

        eth->parent.user_data  = bt_dev;
        eth->eth_rx     = RT_NULL;
        eth->eth_tx     = rt_bt_lwip_protocol_send;

        /* register ETH device */
        if (eth_device_init(eth, eth_name) != RT_EOK)
        {
            LOG_E("eth device init failed");
            rt_free(lwip_prot);
            lwip_prot = RT_NULL;
            return RT_NULL;
        }
        rt_memcpy(&lwip_prot->prot, prot, sizeof(struct rt_bt_prot));
        netif_set_up(eth->netif);
    }

    lwip_system_uninit(); // To stop timer,and Reduce power consumption
    LOG_I("eth device init ok name:%s", eth_name);
    return &lwip_prot->prot;
}

static void rt_bt_lwip_protocol_unregister(struct rt_bt_prot *prot, struct rt_bt_lwip_pan_dev *bt_dev)
{
    struct eth_device *eth_dev = &((struct bt_lwip_prot_des *)bt_dev->prot)->eth;
    LOG_I("Unregister device:%p\n", eth_dev);
    if (eth_dev && eth_dev->netif)
    {
        dhcp_release(eth_dev->netif);
        dhcp_stop(eth_dev->netif);
        dhcp_cleanup(eth_dev->netif);
        netif_set_link_down(eth_dev->netif);
    }

    //to do
}

static struct rt_bt_prot_ops ops =
{
    rt_bt_lwip_protocol_recv,
    rt_bt_lwip_protocol_register,
    rt_bt_lwip_protocol_unregister
};


int rt_bt_lwip_init(void)
{
    static struct rt_bt_prot prot;
    rt_bt_prot_event_t event;

    rt_memset(&prot, 0, sizeof(prot));
    rt_strncpy(&prot.name[0], RT_BT_PROT_LWIP, RT_BT_PROT_NAME_LEN);
    prot.ops = &ops;

    if (rt_bt_prot_regisetr(&prot) != RT_EOK)
    {
        LOG_E("F:%s L:%d protocol regisetr failed", __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

INIT_PREV_EXPORT(rt_bt_lwip_init);

#endif
