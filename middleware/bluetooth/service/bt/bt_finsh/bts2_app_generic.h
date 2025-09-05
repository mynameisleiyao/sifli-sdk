/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_GENERIC_H_
#define _BTS2_APP_GENERIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BT_FINSH_PAN
//add for tcpip route
extern BTS2S_ETHER_ADDR   bts2_local_ether_addr;
#endif
// # include "bts2_app_avrcp.h"
// #include "bts2_app_interface_type.h"
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      int8_t.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
int8_t bt_start_inquiry(bts2_app_stru *bts2_app_data, U32 dev_mask_cls, U16 timeout, U8 max_dis_res);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_stop_inquiry(bts2_app_stru *bts2_app_data);

void bt_register_receive_connect_req_handler(BOOL (*cb)(BTS2S_BD_ADDR *p_bd, U24 dev_cls));

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_pincode_indi(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_start_pairing(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_start_unpairing(bts2_app_stru *bts2_app_data);


void bt_io_capability_rsp(BTS2S_BD_ADDR *bd,
                          BTS2E_SC_IO_CAPABILITY io_capability,
                          U8 mitm,
                          U8 bonding);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_init_local_ctrller(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_set_local_name(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_wr_dev_cls(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 * User-implemenation API.
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      class of device.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
uint32_t bt_get_class_of_device(void);

uint8_t bt_is_auto_request_connect(void);
uint8_t bt_open_bt_request(void);
uint8_t bt_close_bt_request(void);
void bt_sc_io_capability_rsp(BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_wr_page_timeout(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_set_connable(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_get_rmt_dev_name(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_get_role_discov(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_get_rmt_version(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_wr_link_supvisn_timeout(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_rd_transmit_power_level(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_get_rssi(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_wr_link_policy(BTS2S_BD_ADDR *bd, uint16_t link_policy_mode);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_rd_clock(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_wr_afh_chnl_assessment_mode(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_set_secu_mode(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_hdl_gap_msg(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/

#ifdef CFG_BQB_SDC_SSA
void bt_hdl_sdp_msg(bts2_app_stru *bts2_app_data);
#endif


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_hdl_sc_msg(bts2_app_stru *bts2_app_data);
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_close_kb_thread(void);

U8  bt_get_oob_data(bts2_app_stru *app_data);
void  bt_get_sele_yesno(bts2_app_stru *app_data);
U8  bt_get_user_psky_value(bts2_app_stru *app_data);
U8  bt_get_passkey_notifi(bts2_app_stru *app_data);
U8  bt_hdl_sc_input(bts2_app_stru *bts2_app_data);
U8  bt_hdl_sc_passkey_notifi(bts2_app_stru *bts2_app_data);
U8  bt_hdl_sc_yesno(bts2_app_stru *bts2_app_data);
U8  bt_hdl_sc_oobdata(bts2_app_stru *bts2_app_data);


#ifdef RT_USING_BT
void bt_sifli_interface_stop_inquiry(void);
void bt_sifli_interface_start_inquiry(void);
#endif

void bt_etner_sniff_mode(BTS2S_BD_ADDR *bd, uint16_t interval, uint16_t attmpt);
void bt_exit_sniff_mode(BTS2S_BD_ADDR *bd);




#ifdef __cplusplus
}
#endif

#endif

