/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtconfig.h"
#include "drivers/bt_device.h"
#include "stdbool.h"
#include "bts2_app_inc.h"
#include "bf0_ble_common.h"
#include "bf0_ble_err.h"

#include "bt_connection_manager.h"
#include "bt_connection_service.h"
#include "hfp_hf_api.h"
#include "bf0_sibles.h"

#ifdef RT_USING_BT
    #include "bt_rt_device_control.h"
    #include "bt_rt_device_urc.h"
#endif

#ifdef AUDIO_USING_MANAGER
    #include "audio_server.h"
#endif // AUDIO_USING_MANAGER

#define LOG_TAG         "btapp_intf"
//#define DBG_LVL          LOG_LVL_INFO
#include "log.h"


extern bts2_app_stru *bts2g_app_p;

/** @defgroup BT_COMMON  BT device interfaces
  * @{
  */

static bool bt_check_mac_addresses_validity(unsigned char *mac)
{
    U8 i;
    for (i = 0; i < 6; i++)
    {
        if (mac[i] != 0)
        {
            return true;
        }
    }
    return false;
}

void bt_interface_open_bt(void)
{
    bt_open_bt_request();
}

void bt_interface_close_bt(void)
{
    bt_close_bt_request();
}

void bt_interface_start_inquiry(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_start_inquiry(bts2_app_data, BT_DEVCLS_AUDIO, 60, MAX_DISCOV_RESS);
}

int8_t bt_interface_start_inquiry_ex(bt_start_inquiry_ex_t *param)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    return bt_start_inquiry(bts2_app_data, param->dev_cls_mask, param->max_timeout, param->max_rsp);
}

void bt_interface_stop_inquiry(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_stop_inquiry(bts2_app_data);
}

__WEAK void bt_profile_update_connection_state(uint16_t type, uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    bt_interface_bt_event_notify(type, event_id, profile_state, sizeof(bt_notify_profile_state_info_t));
}

__WEAK bt_err_t bt_interface_profile_connect_request(unsigned char *mac, uint8_t profile, uint8_t role)
{
    bt_err_t err = BT_EOK;
    BTS2S_BD_ADDR     bd_addr;

    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);
    if (bd_is_empty(&bd_addr))
    {
        USER_TRACE(">> address invalid\n");
        return BT_ERROR_INPARAM;
    }

    switch (profile)
    {
    case BT_NOTIFY_HFP_PROFILE:
    {
        if (role == BT_NOTIFY_HFP_HF)
        {
#ifdef CFG_HFP_HF
            err = bt_hfp_hf_connect_request(&bd_addr);
#endif
        }
        else if (role == BT_NOTIFY_HFP_AG)
        {
#ifdef CFG_HFP_AG
            err = bt_hfp_ag_connect_request(&bd_addr);
#endif
        }
        break;
    }
#ifdef CFG_AVRCP
    case BT_NOTIFY_AVRCP_PROFILE:
    {
        if (role == BT_NOTIFY_AVRCP_ROLE_CT)
        {
            err = bt_avrcp_controller_connect_request(&bd_addr);
        }
        else if (role == BT_NOTIFY_AVRCP_ROLE_TG)
        {
            err = bt_avrcp_target_connect_request(&bd_addr);
        }
        break;
    }
#endif
#ifdef CFG_AV
    case BT_NOTIFY_A2DP_PROFILE:
    {
        if (role == BT_NOTIFY_A2DP_ROLE_SOURCE)
        {
#ifdef CFG_AV_SRC
            err = bt_a2dp_sink_connect_request(&bd_addr);
#else
            LOG_I("WARRING,not enabled AV source");
#endif
        }
        else if (role == BT_NOTIFY_A2DP_ROLE_SINK)
        {
#ifdef CFG_AV_SNK
            err = bt_a2dp_source_connect_request(&bd_addr);
#else
            LOG_I("WARRING,not enabled AV sink");
#endif
        }
        break;
    }
#endif
#ifdef BT_FINSH_PAN
    case BT_NOTIFY_PAN_PROFILE:
    {
        err = bt_pan_connect_request(&bd_addr);
        break;
    }
#endif
#ifdef CFG_HID
    case BT_NOTIFY_HID_PROFILE:
    {
        bt_hid_connect_requset(&bd_addr);
        break;
    }
#endif
#ifdef CFG_BR_GATT_SRV
    case BT_NOTIFY_BT_GATT_PROFILE:
    {
        bt_gatt_conn_req(&bd_addr);
        break;
    }
#endif
#ifdef CFG_PBAP_CLT
    case BT_NOTIFY_PBAP_PROFILE:
    {
        err = bt_pbap_client_connect_request(&bd_addr, FALSE);
        break;
    }
#endif
    default:
        return BT_ERROR_UNSUPPORTED;
    }
    return err;
}

bt_err_t bt_interface_conn_ext(unsigned char *mac, bt_profile_t ext_profile)
{
    bt_err_t res = BT_EOK;
    uint8_t role = 0;
    uint8_t profile_type = 0;

    switch (ext_profile)
    {
    case BT_PROFILE_HFP:
        role = BT_NOTIFY_HFP_ROLE_HF;
        profile_type = BT_NOTIFY_HFP_PROFILE;
        break;
    case BT_PROFILE_AVRCP:
        role = BT_NOTIFY_AVRCP_ROLE_CT;
        profile_type = BT_NOTIFY_AVRCP_PROFILE;
        break;
    case BT_PROFILE_A2DP:
        role = BT_NOTIFY_A2DP_ROLE_SINK;
        profile_type = BT_NOTIFY_A2DP_PROFILE;
        break;
    case BT_PROFILE_HID:
        role = 0;
        profile_type = BT_NOTIFY_HID_PROFILE;
        break;

    case BT_PROFILE_PAN:
        role = 0;
        profile_type = BT_NOTIFY_PAN_PROFILE;
        break;

    case BT_PROFILE_BT_GATT:
        role = 0;
        profile_type = BT_NOTIFY_BT_GATT_PROFILE;
        break;
    case BT_PROFILE_PBAP:
        role = BT_NOTIFY_PBAP_ROLE_CLIENT;
        profile_type = BT_NOTIFY_PBAP_PROFILE;
        break;

    default:
        res = BT_ERROR_UNSUPPORTED;
        break;
    }

    res = bt_interface_profile_connect_request(mac, profile_type, role);
    return res;
}

bt_err_t bt_interface_conn_to_source_ext(unsigned char *mac, bt_profile_t ext_profile)
{
    bt_err_t res = BT_EOK;
    uint8_t role = 0;
    uint8_t profile_type = 0;

    switch (ext_profile)
    {
    case BT_PROFILE_HFP:
    {
        role = BT_NOTIFY_HFP_ROLE_AG;
        profile_type = BT_NOTIFY_HFP_PROFILE;
        break;
    }
    case BT_PROFILE_AVRCP:
    {
        role = BT_NOTIFY_AVRCP_ROLE_TG;
        profile_type = BT_NOTIFY_AVRCP_PROFILE;
        break;
    }
    case BT_PROFILE_A2DP:
    {
        role = BT_NOTIFY_A2DP_ROLE_SOURCE;
        profile_type = BT_NOTIFY_A2DP_PROFILE;
        break;
    }
    default:
        return BT_ERROR_INPARAM;;
    }

    res = bt_interface_profile_connect_request(mac, profile_type, role);
    return res;
}

bt_err_t bt_interface_disc_ext(unsigned char *mac, bt_profile_t ext_profile)
{
    BTS2S_BD_ADDR     bd_addr;

    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);

    if (bd_is_empty(&bd_addr))
    {
        USER_TRACE(">> address invalid\n");
        return BT_ERROR_INPARAM;
    }

    USER_TRACE("connect adrr :%x:%x:%x:%x:%x:%x \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    switch (ext_profile)
    {
    case BT_PROFILE_A2DP:
#ifdef CFG_AV_SNK
        bt_avsnk_disc_by_addr(&bd_addr, FALSE);
#endif
        break;

    case BT_PROFILE_AVRCP:
        bt_avrcp_disc_2_dev(&bd_addr);
        break;

    case BT_PROFILE_HID:
#ifdef CFG_HID
        bt_hid_disc_2_dev(&bd_addr);
#endif
        break;

#ifdef CFG_BR_GATT_SRV
    case BT_PROFILE_BT_GATT:
        bt_gatt_disconn_req(&bd_addr);
        break;
#endif

#ifdef CFG_PBAP_CLT
    case BT_PROFILE_PBAP:
        bt_pbap_client_disconnect(&bd_addr);
        break;
#endif
    case BT_PROFILE_HFP:
        bt_hfp_hf_start_disc(&bd_addr);
        break;

#ifdef BT_FINSH_PAN
    case BT_PROFILE_PAN:
        bt_pan_disc(&bd_addr);
        break;
#endif

    default:
        LOG_E("this function is not supported");
        break;
    }
    return BT_EOK;
}

void bt_interface_set_local_name(int len, void *data)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    BTS2S_DEV_NAME  name;

    memset(name, 0, sizeof(BTS2S_DEV_NAME));

    if (len <= MAX_FRIENDLY_NAME_LEN)
    {
        strcpy((char *)name, (const char *)data);
        gap_set_local_name_req(bts2_app_data->phdl, name);

        U8 *eir = bmalloc(2 + len);
        BT_OOM_ASSERT(eir);
        if (eir)
        {
            *eir = 1 + len;
            *(eir + 1) = 9;
            memcpy((void *)(eir + 2), (void *)data, len);
            USER_TRACE("set name %s\n", eir);
            gap_wr_eir_req(0, eir, len + 2);
        }
    }
    else
    {
        USER_TRACE("set name too long\n");
    }
}

void bt_interface_read_eir_data(void)
{
    gap_rd_eir_req();
}

void bt_interface_set_eir_data(U8 fec_required, U8 size_eir_data, U8 *eir_data)
{
    if (size_eir_data <= MAX_EIR_DATA_LENGTH)
    {
        USER_TRACE("set eir data len %d\n", size_eir_data);
        gap_wr_eir_req(fec_required, eir_data, size_eir_data);
    }
    else
    {
        USER_TRACE("eir data too long\n");
    }
}

void bt_interface_wr_inquiry_mode(U8 mode)
{
    gap_wr_inquiry_mode_req(mode);
}

void bt_interface_rd_inquiry_mode(void)
{
    gap_rd_inquiry_mode_req();
}

void bt_interface_rd_extend_feature(unsigned char *mac, uint8_t page_num)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    BTS2S_BD_ADDR bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);

    gap_rd_rmt_ext_featr_req(bts2_app_data->phdl, page_num, bd_addr);
}

int8_t bt_interface_cancel_connect_req(unsigned char *mac)
{
    BTS2S_BD_ADDR bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);

    return bt_cancel_connect_req(&bd_addr);
}

int8_t bt_interface_disconnect_req(unsigned char *mac)
{
    BTS2S_BD_ADDR bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);
    return bt_disconnect_req(&bd_addr);
}

int8_t bt_interface_io_req_res(unsigned char *mac, BTS2E_SC_IO_CAPABILITY io_capability, uint8_t mitm, uint8_t bonding)
{
    BTS2S_BD_ADDR bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);
    sc_io_capability_rsp(&bd_addr, io_capability, mitm, bonding);
    return 0;
}

int8_t bt_interface_user_confirm_res(unsigned char *mac, uint8_t confirm)
{
    BTS2S_BD_ADDR bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);
    sc_user_cfm_rsp(&bd_addr, confirm);
    return 0;
}

void bt_interface_rd_local_name(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    gap_rd_local_name_req(bts2_app_data->phdl);
}

void bt_interface_rd_local_bd_addr(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    gap_rd_local_bd_req(bts2_app_data->phdl);
}

void bt_interface_exit_sniff_mode(unsigned char *mac)
{
    BTS2S_BD_ADDR bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);
    bt_exit_sniff_mode(&bd_addr);
}

void bt_interface_wr_link_policy_setting(unsigned char *mac, uint16_t link_policy_mode)
{
    BTS2S_BD_ADDR bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);
    bt_wr_link_policy(&bd_addr, link_policy_mode);
}

void bt_interface_rd_local_rssi(unsigned char *mac)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    BTS2S_BD_ADDR bd = {0};

    bd.nap = (mac[5] << 8) | mac[4];
    bd.uap = mac[3];
    bd.lap = (mac[2] << 16) | (mac[1] << 8) | mac[0];

    gap_rd_rssi_req(bts2_app_data->phdl, bd);
}

U8 bt_interface_get_current_scan_mode(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    return bts2_app_data->scan_mode;
}

U8 bt_interface_set_scan_mode(BOOL inquiry_scan, BOOL page_scan)
{
    U8 target_scan_mode = 0;
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    if (inquiry_scan)
    {
        target_scan_mode |= 0x01;
    }

    if (page_scan)
    {
        target_scan_mode |= 0x02;
    }

    if ((target_scan_mode != bts2_app_data->scan_mode) ||
            (bts2_app_data->scan_mode_fsm && (target_scan_mode !=  bts2_app_data->target_scan_mode)))
    {
        gap_wr_scan_enb_req(bts2_task_get_app_task_id(), inquiry_scan, page_scan);
    }
    bts2_app_data->target_scan_mode = target_scan_mode;
    bts2_app_data->scan_mode_fsm = BTS_SCAN_MODE_SETTING;
    return 0;
}

uint8_t bt_addr_convert(BTS2S_BD_ADDR *src_addr, uint8_t *addr)
{
    if (src_addr == NULL || addr == NULL)
        return 0;

    addr[0] = src_addr->lap & 0xFF;
    addr[1] = (src_addr->lap >> 8) & 0xFF;
    addr[2] = (src_addr->lap >> 16) & 0xFF;
    addr[3] = src_addr->uap & 0xFF;
    addr[4] = src_addr->nap & 0xFF;
    addr[5] = (src_addr->nap >> 8) & 0xFF;

    return 1;
}

BTS2S_BD_ADDR *bt_interface_this_connect_addr(unsigned char *mac)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    BTS2S_BD_ADDR temp = {0xffffff, 0xff, 0xffff};

    if (mac && bt_check_mac_addresses_validity(mac))
    {
        USER_TRACE(">> connect mac:%x:%x:%x:%x:%x:%x \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        bts2_app_data->dev_idx = 0;
        bt_addr_convert_to_bts((bd_addr_t *)mac, &bts2_app_data->bd_list[bts2_app_data->dev_idx]);
        return &bts2_app_data->bd_list[bts2_app_data->dev_idx];
    }
    else
    {
        if (0 != memcmp(&(bts2_app_data->bd_list[bts2_app_data->dev_idx]), &temp, sizeof(BTS2S_BD_ADDR)))
        {
            USER_TRACE(">> bd_list address\n");
            return &(bts2_app_data->bd_list[bts2_app_data->dev_idx]);
        }
        else if (0 != memcmp(&(bts2_app_data->last_conn_bd), &temp, sizeof(BTS2S_BD_ADDR)))
        {
            USER_TRACE(">> last_conn_bd address\n");
            return &(bts2_app_data->last_conn_bd);
        }
        else
        {
            USER_TRACE(">> address invalid\n");
        }
    }
    return NULL;
}

void bt_interface_acl_accept_role_set(U8 role) //0；master 1:slave
{
    // bt_acl_accept_role_set(role);
}

void bt_interface_set_linkpolicy(U16 lp_in, U16 lp_out)//bit0:roleswitch   bit2:sniff
{
    bt_acl_set_default_link_policy(lp_in, lp_out);
}

void bt_interface_set_sniff_enable(BOOL enable)
{
    hcia_set_sniff_mode_enable(enable);
}

/// @}  BT_COMMON


/** @defgroup BT_A2DP_SRV  A2DP profile interfaces
  * @{
  */

#ifdef CFG_AV
void bt_interface_open_avsink(void)
{
#ifdef CFG_AV_SNK
    bt_av_snk_open();
#endif
}

bt_err_t bt_interface_close_avsink(void)
{
    U8  connExist;
    bt_err_t ret = BT_EOK;

    //check the conn of av
    connExist = bt_av_conn_check();

    USER_TRACE("a2dp conn exist %d\n", connExist);

    if (TRUE == connExist)
    {
        ret = BT_ERROR_STATE;
        return ret;
    }

#ifdef CFG_AV_SNK
    bt_av_snk_close();
#endif
    return ret;
}

void bt_interface_set_a2dp_bqb_test(U8 value)
{
    bt_av_hdl_set_bqb_test(value);
}

void bt_interface_set_audio_device(U8 device_type)
{
    LOG_D("bt_interface_set_audio_device:%d\n", device_type);

#ifdef AUDIO_USING_MANAGER
    switch (device_type)
    {
    case AUDIO_DEVICE_SPEAKER:
        audio_server_select_private_audio_device(AUDIO_TYPE_LOCAL_MUSIC, AUDIO_DEVICE_SPEAKER);
        break;
    case AUDIO_DEVICE_A2DP_SINK:
        audio_server_select_private_audio_device(AUDIO_TYPE_LOCAL_MUSIC, AUDIO_DEVICE_A2DP_SINK);
        break;

    default:
        LOG_E("error,device type error!!!!\n");
        break;
    }
#endif
}

U8 bt_interface_get_current_audio_device(void)
{
#ifdef AUDIO_USING_MANAGER
    return get_server_current_device();
#else
    return 0;
#endif
}

U8 bt_interface_get_receive_a2dp_start(void)
{
    return bt_av_get_receive_a2dp_start();
}

void bt_interface_release_a2dp(void)
{
    bt_av_release_stream(0);
}

void bt_interface_unregister_av_snk_sdp(void)
{
#ifdef CFG_AV_SNK
    bt_av_unregister_sdp(AV_AUDIO_SNK);
#endif
}

void bt_interface_register_av_snk_sdp(void)
{
#ifdef CFG_AV_SNK
    bt_av_register_sdp(AV_AUDIO_SNK);
#endif
}

#ifdef CFG_AV_SRC
U8 bt_interface_get_a2dp_stream_state(void)
{
    return bt_av_get_a2dp_stream_state();
}
#endif
#endif

/// @}  BT_A2DP_SRV


/** @defgroup BT_AVRCP_SRV  AVRCP profile interfaces
  * @{
  */

#ifdef CFG_AVRCP
void bt_interface_open_avrcp(void)
{
    bt_avrcp_open();
}

void bt_interface_close_avrcp(void)
{
    bt_avrcp_close();
}

void bt_interface_avrcp_next(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_forward(bts2_app_data);
}

void bt_interface_avrcp_play(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_ply(bts2_app_data);
}

void bt_interface_avrcp_pause(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_pause(bts2_app_data);
}


void bt_interface_avrcp_stop(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_stop(bts2_app_data);
}

void bt_interface_avrcp_volume_up(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_volume_up(bts2_app_data);
}

void bt_interface_avrcp_volume_down(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_volume_down(bts2_app_data);
}

void bt_interface_avrcp_previous(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_backward(bts2_app_data);
}

void bt_interface_avrcp_rewind(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_rewind(bts2_app_data);
}


bt_err_t bt_interface_avrcp_set_absolute_volume_as_ct_role(U8 volume)
{
    bt_err_t ret = BT_ERROR_UNSUPPORTED;

#ifdef CFG_AVRCP
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    ret = bt_avrcp_change_volume(bts2_app_data, volume);
#endif
    return ret;
}


bt_err_t bt_interface_avrcp_set_absolute_volume_as_tg_role(U8 volume)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_err_t ret = BT_EOK;

    if (volume > 127)
        volume = 127;

    ret = bt_avrcp_set_absolute_volume_request(bts2_app_data, volume);
    return ret;
}


U8 bt_interface_avrcp_abs_vol_2_local_vol(U8 abs_vol, U8 local_max_vol)
{
    U8 local_vol;

    if (abs_vol == 0)
    {
        local_vol = 0;
    }
    else if (abs_vol >= 127)
    {
        local_vol = local_max_vol;
    }
    else if ((local_max_vol <= 2) || (local_max_vol > 126))
    {
        // error vol ,just return 0;
        local_vol = 0;
    }
    else
    {
        U16 temp_val = abs_vol * 10;
        // excpet 0 and 127, remain 126 digits should divide to local_max_vol - 1
        U16 temp_step = 1260 / (local_max_vol - 1);
        local_vol = (U8)((temp_val - 1) / temp_step) + 1;
        if (local_vol == local_max_vol)
            local_vol--;
    }

    return local_vol;
}

U8 bt_interface_avrcp_local_vol_2_abs_vol(U8 local_vol, U8 local_max_vol)
{
    U8 abs_vol;
    if (local_vol == 0)
        abs_vol = 0;
    else if (local_vol >= local_max_vol)
        abs_vol = 127;
    else if ((local_max_vol < 3) || (local_max_vol > 126))
        abs_vol = 0;
    else
    {
        // excpet 0 and 127, remain 126 digits should divide to local_max_vol - 1
        U16 temp_step = 1260 / (local_max_vol - 1);
        abs_vol = (U8)(local_vol * temp_step / 10);
    }

    return abs_vol;
}


void bt_interface_avrcp_playback_register_request(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_playback_register_request(bts2_app_data);
}

void bt_interface_playback_pos_register_request(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_playback_pos_register_request(bts2_app_data);
}

void bt_interface_track_change_register_request(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_track_register_request(bts2_app_data);
}

void bt_interface_volume_change_register_request(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_volume_register_request(bts2_app_data);
}

void bt_interface_avrcp_get_element_attributes_request(U8 media_attribute)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_get_element_attributes_request(bts2_app_data, media_attribute);
}

void bt_interface_avrcp_get_play_status_request(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_avrcp_get_play_status_request(bts2_app_data);
}

void bt_interface_set_avrcp_playback_status(U8 playback_status)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
#ifdef CFG_AVRCP
    bts2_app_data->avrcp_inst.playback_status = playback_status;
#endif
}

U8 bt_interface_get_avrcp_playback_status(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
#ifdef CFG_AVRCP
    return bts2_app_data->avrcp_inst.playback_status;
#else
    return 0;
#endif
}


void bt_interface_avrcp_set_can_play(void)
{
#ifdef CFG_AV
    bt_av_set_can_play();
#endif
}

BOOL bt_interface_check_avrcp_role_valid(U8 role)
{
    if ((role == AVRCP_CT) || (role == AVRCP_TG))
        return TRUE;
    else
        return FALSE;
}

// @deprecated - please use bt_interface_set_avrcp_role_ext
bt_err_t bt_interface_set_avrcp_role(BTS2S_BD_ADDR *bd_addr, U8 role)
{
    bt_notify_device_mac_t bd_addr_c;
    bt_addr_convert_to_general(bd_addr, (bd_addr_t *)&bd_addr_c);
    LOG_E("deprecated - please use bt_interface_set_avrcp_role_ext");
    return bt_interface_set_avrcp_role_ext(&bd_addr_c, role);
}

bt_err_t bt_interface_set_avrcp_role_ext(bt_notify_device_mac_t *rmt_addr, U8 role)
{
    bt_err_t ret = BT_EOK;
    BTS2S_BD_ADDR bd_addr;
    bts2_app_stru *bts2_app_data = bts2g_app_p;
#ifdef CFG_AVRCP
    bt_addr_convert_to_bts((bd_addr_t *)rmt_addr, &bd_addr);
    for (U8 i = 0; i < AVRCP_MAX_CONNS; i++)
    {
        if (bd_eq(&bd_addr, &bts2_app_data->avrcp_inst.con[i].rmt_bd) == TRUE)
        {
            if (bt_interface_check_avrcp_role_valid(role))
            {
                bts2_app_data->avrcp_inst.con[i].role = role;
                return ret;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
                return ret;
            }
        }
    }
    return BT_ERROR_INPARAM;
#else
    return BT_ERROR_UNSUPPORTED;
#endif
}
#endif

/// @}  BT_AVRCP_SRV


/** @defgroup BT_HID_SRV  HID profile interfaces
  * @{
  */

#ifdef CFG_HID
void bt_interface_open_hid(void)
{
    bt_hid_open();
}

void bt_interface_close_hid(void)
{
    bt_hid_close();
}

void bt_interface_set_hid_device(U8 is_ios)
{
    bt_hid_set_ios_device(is_ios);
}

void bt_interface_hid_mouse_move(S16 dx, S16 dy)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_mouse_move(bts2_app_data, dx, dy);
}

void bt_interface_hid_mouse_drag_up(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_mouse_drag_page_up(bts2_app_data);
}

void bt_interface_hid_mouse_drag_down(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_mouse_drag_page_down(bts2_app_data);
}

void bt_interface_hid_mouse_once_left_click(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_mouse_left_click(bts2_app_data);
}

void bt_interface_hid_mouse_double_left_click(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_mouse_left_double_click(bts2_app_data);
}

void bt_interface_hid_consumer_take_picture(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_consumer_report_volume_up(bts2_app_data);
}

void bt_interface_hid_consumer_volume_up(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_consumer_report_volume_up(bts2_app_data);
}

void bt_interface_hid_consumer_volume_down(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_consumer_report_volume_down(bts2_app_data);
}

void bt_interface_hid_consumer_play_status_change(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_consumer_report_play_status(bts2_app_data);
}

void bt_interface_hid_consumer_next_track(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_consumer_report_next_track(bts2_app_data);
}

void bt_interface_hid_consumer_prev_track(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_consumer_report_back_track(bts2_app_data);
}

void bt_interface_controller_report_right_arrow(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_controller_report_right_arrow(bts2_app_data);
    bt_hid_controller_report_reset(bts2_app_data);
}

void bt_interface_controller_report_left_arrow(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_controller_report_left_arrow(bts2_app_data);
    bt_hid_controller_report_reset(bts2_app_data);
}

void bt_interface_controller_report_up_arrow(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_controller_report_up_arrow(bts2_app_data);
    bt_hid_controller_report_reset(bts2_app_data);
}

void bt_interface_controller_report_down_arrow(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_hid_controller_report_down_arrow(bts2_app_data);
    bt_hid_controller_report_reset(bts2_app_data);
}

void bt_interface_add_hid_descriptor(U8 *data, U8 len)
{
    bt_hid_add_descriptor(data, len);
}
#endif

/// @}  BT_HID_SRV


/** @defgroup BT_SPP_SRV  SPP profile interfaces
  * @{
  */

#ifdef CFG_SPP_SRV
bt_err_t bt_interface_spp_conn_req_hdl(BOOL accept, bt_notify_device_mac_t *rmt_addr, U8 srv_chl)
{
    bt_err_t ret = BT_EOK;
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    BTS2S_BD_ADDR bd_addr;

    bt_addr_convert_to_bts((bd_addr_t *)rmt_addr, &bd_addr);

    if (accept)
    {
        bt_spp_srv_rfc_conn_accept_hdl(srv_chl, bd_addr);
    }
    else
    {
        bt_spp_srv_rfc_conn_rej_hdl(srv_chl, bd_addr);
    }

    return ret;
}

// @deprecated - please use bt_interface_spp_send_data_ext
bt_err_t bt_interface_spp_send_data(U8 *data, U16 len, BTS2S_BD_ADDR *bd_addr, U8 srv_chl)
{
    bt_notify_device_mac_t bd_addr_c;
    bt_addr_convert_to_general(bd_addr, (bd_addr_t *)&bd_addr_c);
    LOG_E("deprecated - please use bt_interface_spp_send_data_ext");
    return bt_interface_spp_send_data_ext(data, len, &bd_addr_c, srv_chl);
}

bt_err_t bt_interface_spp_send_data_ext(U8 *data, U16 len, bt_notify_device_mac_t *rmt_addr, U8 srv_chl)
{
    bt_err_t ret = BT_EOK;
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    U8 idx = 0xff;
    bts2_spp_srv_inst_data *bts2_spp_srv_inst = NULL;
    bts2_spp_service_list *spp_service_list = NULL;
    BTS2S_BD_ADDR bd_addr;

    bt_addr_convert_to_bts((bd_addr_t *)rmt_addr, &bd_addr);

    if (bt_spp_check_addr_is_connected(bts2_app_data, &bd_addr, &idx))
    {
        bts2_spp_srv_inst = &bts2_app_data->spp_srv_inst[idx];

        if ((bts2_spp_srv_inst->service_list & (1 << srv_chl)) == 0)
        {
            ret = BT_ERROR_DISCONNECTED;
            return ret;
        }

        if (len > bt_spp_get_mtu_size(bts2_app_data, bts2_spp_srv_inst->device_id))
        {
            ret = BT_ERROR_INPARAM;
            return ret;
        }
    }
    else
    {
        ret = BT_ERROR_DISCONNECTED;
        return ret;
    }

    bt_spp_sending_data_by_device_id_and_srv_chnl(bts2_app_data, bts2_spp_srv_inst->device_id, srv_chl, data, len);

    return ret;
}

bt_err_t bt_interface_spp_prepare_send_rand_data(U32 rand_len, bt_notify_device_mac_t *rmt_addr, U8 srv_chl)
{
    bt_err_t ret = BT_EOK;
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    U8 idx = 0xff;
    bts2_spp_srv_inst_data *bts2_spp_srv_inst = NULL;
    bts2_spp_service_list *spp_service_list = NULL;
    BTS2S_BD_ADDR bd_addr;

    bt_addr_convert_to_bts((bd_addr_t *)rmt_addr, &bd_addr);

    if (bt_spp_check_addr_is_connected(bts2_app_data, &bd_addr, &idx))
    {
        bts2_spp_srv_inst = &bts2_app_data->spp_srv_inst[idx];

        if ((bts2_spp_srv_inst->service_list & (1 << srv_chl)) == 0)
        {
            ret = BT_ERROR_DISCONNECTED;
            return ret;
        }

        spp_service_list = bt_spp_get_service_list_by_srv_chl(bts2_spp_srv_inst, srv_chl);
        spp_service_list->is_rand_going = 1;
        spp_service_list->rand_total_len = rand_len;
        spp_service_list->rand_packet_len = spp_service_list->mtu_size;
    }
    else
    {
        ret = BT_ERROR_DISCONNECTED;
        return ret;
    }

    return ret;
}

bt_err_t bt_interface_add_spp_uuid(U8 *uuid, U8 uuid_len, char *srv_name)
{
    bt_err_t ret = BT_EOK;
    LOG_D("bt_interface_add_spp_uuid\n");
    if (uuid == NULL)
    {
        LOG_D("Please pass in a valid uuid\n");
        ret = BT_ERROR_INPARAM;
        return ret;
    }

    if ((uuid_len != 2) && (uuid_len != 4) && (uuid_len != 16))
    {
        LOG_D("Please pass in a valid uuid length\n");
        ret = BT_ERROR_INPARAM;
        return ret;
    }
    ret = (bt_err_t)spp_add_uuid_list_node(uuid, uuid_len, srv_name);
    return ret;
}

// @deprecated - please use bt_interface_spp_srv_data_rsp_ext
bt_err_t bt_interface_spp_srv_data_rsp(BTS2S_BD_ADDR *bd_addr, U8 srv_chl)
{
    bt_notify_device_mac_t bd_addr_c;
    bt_addr_convert_to_general(bd_addr, (bd_addr_t *)&bd_addr_c);
    LOG_E("deprecated - please use bt_interface_spp_srv_data_rsp_ext");
    return bt_interface_spp_srv_data_rsp_ext(&bd_addr_c, srv_chl);
}

bt_err_t bt_interface_spp_srv_data_rsp_ext(bt_notify_device_mac_t *rmt_addr, U8 srv_chl)
{

    bt_err_t ret = BT_EOK;
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    U8 idx = 0xff;
    bts2_spp_srv_inst_data *bts2_spp_srv_inst = NULL;
    bts2_spp_service_list *spp_service_list = NULL;
    BTS2S_BD_ADDR bd_addr;

    bt_addr_convert_to_bts((bd_addr_t *)rmt_addr, &bd_addr);

    if (bt_spp_check_addr_is_connected(bts2_app_data, &bd_addr, &idx))
    {
        bts2_spp_srv_inst = &bts2_app_data->spp_srv_inst[idx];

        if ((bts2_spp_srv_inst->service_list & (1 << srv_chl)) == 0)
        {
            ret = BT_ERROR_DISCONNECTED;
            return ret;
        }
    }
    else
    {
        ret = BT_ERROR_DISCONNECTED;
        return ret;
    }

    spp_data_rsp_ext(bts2_spp_srv_inst->device_id, srv_chl);

    return ret;
}

// @deprecated - please use bt_interface_dis_spp_by_addr_and_chl_ext
bt_err_t bt_interface_dis_spp_by_addr_and_chl(BTS2S_BD_ADDR *bd_addr, U8 srv_chl)
{
    bt_notify_device_mac_t bd_addr_c;
    bt_addr_convert_to_general(bd_addr, (bd_addr_t *)&bd_addr_c);
    LOG_E("deprecated - please use bt_interface_dis_spp_by_addr_and_chl_ext");
    return bt_interface_dis_spp_by_addr_and_chl_ext(&bd_addr_c, srv_chl);
}

bt_err_t bt_interface_dis_spp_by_addr_and_chl_ext(bt_notify_device_mac_t *rmt_addr, U8 srv_chl)
{

    bt_err_t ret = BT_EOK;
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    U8 idx = 0xff;
    bts2_spp_srv_inst_data *bts2_spp_srv_inst = NULL;
    bts2_spp_service_list *spp_service_list = NULL;
    BTS2S_BD_ADDR bd_addr;

    bt_addr_convert_to_bts((bd_addr_t *)rmt_addr, &bd_addr);

    if (bt_spp_check_addr_is_connected(bts2_app_data, &bd_addr, &idx))
    {
        bts2_spp_srv_inst = &bts2_app_data->spp_srv_inst[idx];

        if ((bts2_spp_srv_inst->service_list & (1 << srv_chl)) == 0)
        {
            ret = BT_ERROR_DISCONNECTED;
            return ret;
        }
    }
    else
    {
        ret = BT_ERROR_DISCONNECTED;
        return ret;
    }

    bt_spp_disc_req(bts2_app_data, bts2_spp_srv_inst->device_id, srv_chl);

    return ret;
}

U8 bt_interface_spp_get_device_id_by_addr(BTS2S_BD_ADDR *bd_addr)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bts2_spp_srv_inst_data *bts2_spp_srv_inst = NULL;
    U8 idx = 0xff;
    if (bt_spp_check_addr_is_connected(bts2_app_data, bd_addr, &idx))
    {
        bts2_spp_srv_inst = &bts2_app_data->spp_srv_inst[idx];
        return bts2_spp_srv_inst->device_id;
    }
    return 0xff;
}

bt_err_t bt_interface_dis_spp_all(void)
{

    bt_err_t ret = BT_EOK;
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    bt_spp_disc_req_all(bts2_app_data);
    return ret;
}

void bt_interface_dump_all_spp_connection_info(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    bt_spp_dump_all_spp_connect_information(bts2_app_data);
}

#if RT_USING_DFS
bt_err_t bt_interface_spp_srv_send_file(bt_notify_device_mac_t *rmt_addr, U8 srv_chl, char *file_name)
{
    bt_err_t ret = BT_EOK;
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    U8 idx = 0xff;
    bts2_spp_srv_inst_data *bts2_spp_srv_inst = NULL;
    bts2_spp_service_list *spp_service_list = NULL;
    BTS2S_BD_ADDR bd_addr;

    bt_addr_convert_to_bts((bd_addr_t *)rmt_addr, &bd_addr);

    if (bt_spp_check_addr_is_connected(bts2_app_data, &bd_addr, &idx))
    {
        bts2_spp_srv_inst = &bts2_app_data->spp_srv_inst[idx];

        if ((bts2_spp_srv_inst->service_list & (1 << srv_chl)) == 0)
        {
            ret = BT_ERROR_DISCONNECTED;
            return ret;
        }
    }
    else
    {
        ret = BT_ERROR_DISCONNECTED;
        return ret;
    }

    bt_spp_select_file_to_send(bts2_app_data, bts2_spp_srv_inst->device_id, srv_chl, file_name);
    return ret;
}
#endif

#ifdef CFG_SPP_CLT
bt_err_t bt_interface_spp_client_conn_req(bt_notify_device_mac_t *rmt_addr, U8 *uuid, U8 uuid_len)
{
    bt_err_t ret = BT_EOK;
    BTS2S_BD_ADDR bd_addr;

    if (uuid == NULL)
    {
        LOG_D("Please pass in a valid uuid\n");
        ret = BT_ERROR_INPARAM;
        return ret;
    }

    if ((uuid_len != 2) && (uuid_len != 4) && (uuid_len != 16))
    {
        LOG_D("Please pass in a valid uuid length\n");
        ret = BT_ERROR_INPARAM;
        return ret;
    }

    bt_addr_convert_to_bts((bd_addr_t *)rmt_addr, &bd_addr);

    bt_spp_client_start_w4conn(&bd_addr, uuid, uuid_len);

    return ret;
}

bt_err_t bt_interface_spp_client_sdp_search_req(bt_notify_device_mac_t *rmt_addr, U8 *uuid, U8 uuid_len)
{
    bt_err_t ret = BT_EOK;
    BTS2S_BD_ADDR bd_addr;

    if (uuid == NULL)
    {
        LOG_D("Please pass in a valid uuid\n");
        ret = BT_ERROR_INPARAM;
        return ret;
    }

    if ((uuid_len != 2) && (uuid_len != 4) && (uuid_len != 16))
    {
        LOG_D("Please pass in a valid uuid length\n");
        ret = BT_ERROR_INPARAM;
        return ret;
    }

    bt_addr_convert_to_bts((bd_addr_t *)rmt_addr, &bd_addr);
    bt_spp_client_start_sdp_search(&bd_addr, uuid, uuid_len);

    return ret;
}
#endif
#endif

/// @}  BT_SPP_SRV


/** @defgroup BR_GATT_SRV  GATT over BR profile interfaces
  * @{
  */

#ifdef CFG_BR_GATT_SRV
void bt_interface_bt_gatt_reg(br_att_sdp_data_t *sdp_info)
{
    bt_gatt_create_sdp_record(sdp_info->att_uuid_len, sdp_info->att_uuid,
                              sdp_info->gatt_start_handle, sdp_info->gatt_end_handle);
}

void bt_interface_bt_gatt_unreg(U32 sdp_hdl)
{
    bt_gatt_create_sdp_unreg(sdp_hdl);
}

void bt_interface_bt_gatt_mtu_changed(U16 mtu)
{
    bt_gatt_change_mtu_req(mtu);
}
#endif

/// @}  BR_GATT_SRV


/** @defgroup HFP_AG  HFP_AG profile interfaces
  * @{
  */

#ifdef BT_USING_AG
void bt_interface_phone_state_changed(HFP_CALL_INFO_T *call_info)
{
    bt_hfp_ag_call_state_update_listener(call_info);
}

void bt_interface_local_phone_info_res(uint16_t profile_channel, hfp_phone_number_t *local_phone_num)
{
    bt_hfp_ag_cnum_response((uint8_t)profile_channel, local_phone_num);
}

void bt_interface_remote_call_info_res(uint16_t profile_channel, hfp_remote_calls_info_t *calls_info)
{
    bt_hfp_ag_remote_calls_res_hdl((uint8_t)profile_channel, calls_info);
}

void bt_interface_get_all_indicator_info_res(uint16_t profile_channel, hfp_cind_status_t *cind_status)
{
    bt_hfp_ag_cind_response((uint8_t)profile_channel, cind_status);
}

void bt_interface_indicator_status_changed(uint16_t profile_channel, HFP_IND_INFO_T *ind_info)
{
    bt_hfp_ag_ind_status_update((uint8_t)profile_channel, ind_info->ind_type, ind_info->ind_val);
}

void bt_interface_spk_vol_change_req(uint16_t profile_channel, uint8_t vol)
{
    bt_hfp_ag_spk_vol_control((uint8_t)profile_channel, vol);
}

void bt_interface_mic_vol_change_req(uint16_t profile_channel, uint8_t vol)
{
    bt_hfp_ag_mic_vol_control((uint8_t)profile_channel, vol);
}

void bt_interface_make_call_res(Uuint16_t profile_channel, uint8_t res)
{
    bt_hfp_ag_at_result_res((uint8_t)profile_channel, res);
}

bt_err_t bt_interface_ag_audio_switch(bt_hfp_audio_switch_t *audio)
{
    BTS2S_BD_ADDR *bd_addr;

    // general_addr_convert_to_bt_addr((bd_addr_t *)&audio->peer_addr, &bd_addr);
    bd_addr = bt_interface_this_connect_addr((unsigned char *)audio->peer_addr.addr);
    if (bd_addr == NULL)
    {
        USER_TRACE(">> ag audio switch address invalid\n");
        return BT_ERROR_INPARAM;
    }
    if (audio->type)
        bt_hfp_disconnect_audio(bd_addr);
    else
        bt_hfp_connect_audio(bd_addr);
    return BT_EOK;
}

#endif

/// @}  HFP_AG


/** @defgroup HFP_HF  HFP_HF profile interfaces
  * @{
  */

#ifdef CFG_HFP_HF
bt_err_t bt_interface_hfp_hf_start_connecting(unsigned char *mac)
{
    BTS2S_BD_ADDR     bd_addr;

    if (bt_check_mac_addresses_validity(mac))
    {
        bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);
        bt_hfp_hf_connect_request(&bd_addr);
        return BT_EOK;
    }

    return BT_ERROR_INPARAM;
}

bt_err_t bt_interface_get_ph_num(void)
{
    bt_err_t ret = bt_hfp_hf_at_cnum_send();
    return ret;
}

bt_err_t bt_interface_get_remote_ph_num(void)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_at_clcc_send();
    return ret;
}

bt_err_t bt_interface_get_remote_call_status(void)
{
    bt_err_t ret = BT_EOK;
    hfp_hf_send_at_cind_status_api();
    return ret;
}

bt_err_t bt_interface_hf_out_going_call(int len, void *data)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_make_call_by_number_send((U8 *)data, (U8)len);
    return ret;
}

bt_err_t bt_interface_start_last_num_dial_req_send(void)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_last_num_dial_send();
    return ret;
}

bt_err_t bt_interface_start_hf_answer_req_send(void)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_answer_call_send();
    return ret;
}

bt_err_t bt_interface_handup_call(void)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_hangup_call_send();
    return ret;
}

bt_err_t bt_interface_start_dtmf_req_send(char key)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_at_dtmf_send(key);
    return ret;
}

bt_err_t bt_interface_hf_3way_hold(bt_3way_coded_t cmd, int idx)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    bt_err_t ret = BT_EOK;
    switch (cmd)
    {
    case BT_3WAY_REL_HOLDCALLS://AT+CHLD=0
        bts2_app_data->input_str[0] = '0';
        bts2_app_data->input_str[1] = '\0';
        bts2_app_data->input_str_len = 2;
        break;

    case BT_3WAY_REL_ACTCALLS:
        if (0xff == idx) //AT+CHLD=1
        {
            bts2_app_data->input_str[0] = '1';
            bts2_app_data->input_str[1] = '\0';
            bts2_app_data->input_str_len = 2;
        }
        else//AT+CHLD=1x
        {
            bts2_app_data->input_str[0] = '1';
            bts2_app_data->input_str[1] = idx + '0';
            bts2_app_data->input_str[2] = '\0';
            bts2_app_data->input_str_len = 3;
        }
        break;

    case BT_3WAY_HOL_ACTCALLS_ACP_OTHER:
        if (0xff == idx) //AT+CHLD=2
        {
            bts2_app_data->input_str[0] = '2';
            bts2_app_data->input_str[1] = '\0';
            bts2_app_data->input_str_len = 2;
        }
        else//AT+CHLD=2x
        {
            bts2_app_data->input_str[0] = '2';
            bts2_app_data->input_str[1] = idx + '0';
            bts2_app_data->input_str[2] = '\0';
            bts2_app_data->input_str_len = 3;
        }
        break;

    case BT_3WAY_ADD_HOL_TO_CONV://AT+CHLD=3
        bts2_app_data->input_str[0] = '3';
        bts2_app_data->input_str[1] = '\0';
        bts2_app_data->input_str_len = 2;
        break;

    default:
        return BT_ERROR_STATE;

    }

    ret = bt_hfp_hf_at_chld_send(bts2_app_data->input_str, bts2_app_data->input_str_len);
    return ret;
}

bt_err_t bt_interface_hf_3way_btrh(bt_3way_incom_t cmd)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_at_btrh_cmd_send(cmd);
    return ret;
}

bt_err_t bt_interface_hf_3way_ccwa(unsigned int enable)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_at_ccwa_send((BOOL)enable);
    return ret;
}

bt_err_t bt_interface_voice_recog(U8 flag)
{
    USER_TRACE("set voice reg %d\n", flag);
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_voice_recog_send(flag);
    return ret;
}

bt_err_t bt_interface_audio_switch(U8 type)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_audio_transfer(type);
    return ret;
}

bt_err_t bt_interface_set_speaker_volume(int volume)
{
    USER_TRACE(">> set volume %d\n", volume);
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_update_spk_vol(volume); //the volume set by solution should be 0-15 .
    return ret;
}

//batt_val: 0 ~ 9  are effected.
bt_err_t bt_interface_hf_update_battery(U8 batt_val)
{
    bt_err_t ret = BT_EOK;
    ret = bt_hfp_hf_update_batt_send(batt_val);
    return ret;
}

bt_err_t bt_interface_set_wbs_status(U8 status)
{
    bt_err_t ret = BT_EOK;
    hfp_hf_set_wbs(status);
    return ret;
}
#endif

/// @}  HFP_HF


/** @defgroup BT_PAN_SRV  BT PAN profile interfaces
  * @{
  */

#ifdef BT_FINSH_PAN
// @deprecated - please use bt_interface_update_pan_addr_ext
void bt_interface_update_pan_addr(BTS2S_BD_ADDR *bd_addr)
{
    bt_pan_update_addr(bd_addr);
}

void bt_interface_update_pan_addr_ext(bt_notify_device_mac_t *local_addr)
{
    BTS2S_BD_ADDR bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)local_addr, &bd_addr);
    bt_pan_update_addr(&bd_addr);
}

BOOL bt_interface_check_pan_in_sniff(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    return bts2_app_data->pan_inst_ptr->mode == SNIFF_MODE;
}
#endif

/// @}  BT_PAN_SRV

/*****************************************************notify start********************************************************************/
bt_notify_register_callback_t *notify_callback_list = NULL;

void bt_interface_bt_event_notify(uint16_t type, uint16_t event_id, void *data, uint16_t data_len)
{
    /* on the first pass through the svc db, look for Psms enabling connless reception */
    bt_notify_register_callback_t *p_rec;
    p_rec = notify_callback_list;

    while (p_rec)
    {
        p_rec->function(type, event_id, data, data_len);
        p_rec = p_rec->next;
    }
}

bt_interface_status_t bt_interface_register_bt_event_notify_callback(register_bt_notify_func_t func)
{
    bt_notify_register_callback_t *p_rec;
    bt_notify_register_callback_t **pp_list;
    pp_list = &notify_callback_list;

    while (*pp_list)
    {
        if ((*pp_list)->function == func)
        {
            break;
        }

        pp_list = &((*pp_list)->next);
    }
    if (*pp_list)
    {
        /* existing record found */
        // p_rec = *pp_list;
        return BT_INTERFACE_STATUS_ALREADY_EXIST; //failed
    }
    else
    {
        /* existing record not found, add a new one to the list */
        p_rec = (bt_notify_register_callback_t *)bmalloc(sizeof(bt_notify_register_callback_t));
        bmemset(p_rec, 0x00, (U32)(sizeof(bt_notify_register_callback_t)));
        if (p_rec)
        {
            p_rec->function = func;
            p_rec->next = NULL;
            * pp_list = p_rec;
        }
        else
        {
            return BT_INTERFACE_STATUS_NO_MEMORY; //failed
        }
    }
    return BT_INTERFACE_STATUS_OK;  // success
}

bt_interface_status_t bt_interface_unregister_bt_event_notify_callback(register_bt_notify_func_t func)
{
    bt_notify_register_callback_t *p_rec;
    bt_notify_register_callback_t **pp_list;
    bt_interface_status_t res = BT_INTERFACE_STATUS_NOT_EXIST;
    /* srch for the record (must be a standard in / out record) */
    pp_list = &notify_callback_list;

    while (*pp_list)
    {
        p_rec = *pp_list;
        if (p_rec->function == func)
        {
            /* remove the record from the list */
            * pp_list = p_rec->next;

            /* del the record */
            free(p_rec);
            res = BT_INTERFACE_STATUS_OK;  // success
            break;
        }
        pp_list = &((*pp_list)->next);
    }

    return res;
}
/******************************************************notify end*******************************************************************/
