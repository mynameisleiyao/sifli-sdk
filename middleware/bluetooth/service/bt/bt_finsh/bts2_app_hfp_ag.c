/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bts2_global.h"
#ifdef CFG_HFP_AG

#include "bts2_app_inc.h"
#include "hfp_ag_api.h"

#define LOG_TAG         "btapp_ag"
#include "log.h"


#ifdef AUDIO_USING_MANAGER
    #include "hfp_audio_api.h"
#endif

#ifdef BT_USING_AG
    #include "bt_rt_device.h"
#endif

#ifndef BT_USING_AG
    U8 g_flag_auto_answer_call = 1;
#endif

/*******************************************add for ag func test start**********************************/
static hfp_phone_call_info_t g_remote_calls_info ;
hfp_cind_status_t g_cind_states;

void bt_hfp_ag_app_call_status_change(U8 mux_id, char *phone_num, uint8_t phone_len, uint8_t active, uint8_t callsetup_state, uint8_t call_dir)
{
    phone_call_status_t call_status;

    if (0 == active && 0 == callsetup_state)
    {
        call_status = 0xff;
    }

    if (1 == active && 0 == callsetup_state)
    {
        call_status = PHONE_CALL_ACTIVE;
    }

    if (1 == callsetup_state)
    {
        call_status = PHONE_CALL_INCOMING;

    }
    else if (2 == callsetup_state)
    {
        call_status = PHONE_CALL_DAILING;
    }
    else if (3 == callsetup_state)
    {
        call_status = PHONE_CALL_ALERTING;
    }

    g_cind_states.call = active;
    g_cind_states.callsetup = callsetup_state;

    if (0xff == call_status)
    {
        bmemset(&g_remote_calls_info, 0x00, sizeof(hfp_phone_call_info_t));
    }
    else
    {
        if (g_remote_calls_info.call_idx == 0x00)
        {
            g_remote_calls_info.phone_info.type = PHONE_NUMBER_TYPE_UNKNOWN;
            bmemcpy(&g_remote_calls_info.phone_info.phone_number, phone_num, phone_len);
        }

        g_remote_calls_info.call_idx = 1;
        g_remote_calls_info.call_dir = call_dir;
        g_remote_calls_info.call_status = call_status;
        g_remote_calls_info.call_mode = 0;
        g_remote_calls_info.call_mtpty = 0;
    }

    HFP_CALL_INFO_T call_info;
    call_info.num_active = active;
    call_info.num_held = 0;
    call_info.callsetup_state = callsetup_state;
    bmemcpy(&call_info.phone_number, phone_num, phone_len);
    call_info.phone_type = 0x81;
    call_info.phone_len = phone_len;
    bt_hfp_ag_call_state_update_listener(mux_id, &call_info);
}

void bt_hfp_ag_app_cind_status_change(uint8_t type, uint8_t val)
{
    switch (type)
    {
    case HFP_AG_CIND_SERVICE_TYPE:
    {
        g_cind_states.service_status = val;
        break;
    }
    case HFP_AG_CIND_CALL_TYPE:
    {
        g_cind_states.call = val;
        break;
    }
    case HFP_AG_CIND_CALLSETUP_TYPE:
    {
        g_cind_states.callsetup = val;
        break;
    }
    case HFP_AG_CIND_BATT_TYPE:
    {
        g_cind_states.batt_level = val;
        break;
    }
    case HFP_AG_CIND_SIGNAL_TYPE:
    {
        g_cind_states.signal = val;
        break;
    }
    case HFP_AG_CIND_ROAM_TYPE:
    {
        g_cind_states.roam_status = val;
        break;
    }
    case HFP_AG_CIND_CALLHELD_TYPE:
    {
        g_cind_states.callheld = val;
        break;
    }
    }
}

hfp_phone_call_info_t *bt_hfp_ag_app_get_remote_call_info()
{
    return &g_remote_calls_info;
}
/*******************************************add for ag func test end**********************************/

/*******************************************static func**********************************/
static void bt_hfp_ag_app_profile_service_update(bts2_hfp_ag_inst_data *ag_data, bts2_ag_st new_state)
{
    ag_data->old_st = ag_data->st;
    ag_data->st = new_state;
    USER_TRACE("hfp ag profile service new_date:%d, old_state:%d", ag_data->st, ag_data->old_st);
    //to update
}

static void bt_hfp_ag_reg_res_hdl(bts2_app_stru *bts2_app_data)
{
    BTS2S_AG_CFM *msg;
    msg = (BTS2S_AG_CFM *)bts2_app_data->recv_msg;
    switch (msg->res)
    {
    case HFP_AG_SUCC:
    {
        bt_hfp_ag_app_profile_service_update(&bts2_app_data->hfp_ag_inst, HFP_AG_APP_OPEN);
        break;
    }
    case HFP_AG_BUSYING:
    {
        //notify waiting
        break;
    }
    default:
    {
        bts2_hfp_ag_inst_data *ag_data = &bts2_app_data->hfp_ag_inst;
        bt_hfp_ag_app_profile_service_update(ag_data, ag_data->old_st);
        break;
    }
    }

}

static void bt_hfp_ag_dereg_res_hdl(bts2_app_stru *bts2_app_data)
{
    BTS2S_AG_CFM *msg;
    msg = (BTS2S_AG_CFM *)bts2_app_data->recv_msg;
    switch (msg->res)
    {
    case HFP_AG_SUCC:
    {
        bt_hfp_ag_app_profile_service_update(&bts2_app_data->hfp_ag_inst, HFP_AG_APP_INIT);
        break;
    }
    case HFP_AG_BUSYING:
    {
        //notify waiting
        break;
    }
    default:
    {
        bts2_hfp_ag_inst_data *ag_data = &bts2_app_data->hfp_ag_inst;
        bt_hfp_ag_app_profile_service_update(ag_data, ag_data->old_st);
        break;
    }
    }
}

static void bt_hfp_ag_app_device_state_changed(bts2_app_stru *bts2_app_data, BTS2S_AG_CONN_RES *con_msg)
{
    bts2_hfp_ag_inst_data *p_data = &bts2_app_data->hfp_ag_inst;

    USER_TRACE("hfp_ag device state: %d res: %d type %d mux_id:%d", con_msg->device_state, con_msg->res, con_msg->type, con_msg->mux_id);

    switch (p_data->st)
    {
    case HFP_AG_APP_INIT:
    case HFP_AG_APP_OPENING:
    case HFP_AG_APP_OPEN:
    {
        p_data->pre_profile_state = p_data->profile_state;
        p_data->profile_state = con_msg->device_state;

        if (p_data->profile_state == p_data->pre_profile_state)
        {
            return;
        }

        if (con_msg->device_state == HFP_DEVICE_CONNECTED)
        {
            bt_notify_profile_state_info_t profile_state;
            bt_addr_convert(&con_msg->bd, profile_state.mac.addr);
            profile_state.profile_type = BT_NOTIFY_HFP_AG;
            profile_state.res = BTS2_SUCC;
            profile_state.profile_channel = con_msg->mux_id;
            bt_profile_update_connection_state(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_PROFILE_CONNECTED, &profile_state);
#ifndef BT_USING_AG
            if (g_flag_auto_answer_call)
            {
                hfp_phone_call_info_t *call_info = bt_hfp_ag_app_get_remote_call_info();

                if (g_cind_states.call || g_cind_states.callsetup)
                {
                    bt_hfp_ag_app_call_status_change(con_msg->mux_id, (char *)&call_info->phone_info.phone_number, strlen(call_info->phone_info.phone_number) + 1,
                                                     g_cind_states.call, g_cind_states.callsetup, call_info->call_dir);
                }
            }
#endif
        }
        else if (con_msg->device_state == HFP_DEVICE_DISCONNECTED)
        {
            bt_notify_profile_state_info_t profile_state;
            bt_addr_convert(&con_msg->bd, profile_state.mac.addr);
            profile_state.profile_type = BT_NOTIFY_HFP_AG;
            profile_state.res = con_msg->res;
            profile_state.profile_channel = con_msg->mux_id;
            bt_profile_update_connection_state(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_PROFILE_DISCONNECTED,&profile_state);
#ifdef AUDIO_USING_MANAGER
            BTS2S_HF_AUDIO_INFO msg;
            hfp_ag_audio_opt(&msg, 0);
#endif // AUDIO_USING_MANAGER
        }

        break;
    }
    case HFP_AG_APP_CLOSING:
    {
        if (con_msg->device_state == HFP_DEVICE_DISCONNECTED)
        {
            hfp_ag_deregister();
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

//static void bt_hfp_ag_app_onAudioStateChanged(hfp_device_state_t state)

static void bt_hfp_ag_app_vr_state_changed(U8 mux_id)
{
}

static void bt_hfp_ag_app_answercall(U8 mux_id)
{
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_ANSWER_CALL_REQ, &mux_id, 1);
#ifndef BT_USING_AG
    if (g_flag_auto_answer_call)
    {
        hfp_phone_call_info_t *call_info = bt_hfp_ag_app_get_remote_call_info();
        bt_hfp_ag_app_call_status_change(mux_id, (char *)&call_info->phone_info.phone_number, strlen(call_info->phone_info.phone_number) + 1,
                                         1, 0, call_info->call_dir);
    }
#endif
}

static void bt_hfp_ag_app_hangupcall(U8 mux_id)
{
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_HANGUP_CALL_REQ,  &mux_id, 1);
#ifndef BT_USING_AG
    hfp_phone_call_info_t *call_info = bt_hfp_ag_app_get_remote_call_info();
    bt_hfp_ag_app_call_status_change(mux_id, (char *)&call_info->phone_info.phone_number, strlen(call_info->phone_info.phone_number) + 1,
                                     0, 0, call_info->call_dir);
#endif

}

static void bt_hfp_ag_app_vol_changed(U8 mux_id, U16 val)
{
    bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + 1);
    data_info->profile_channel = mux_id;
    data_info->payload_len = 1;
    bmemcpy(data_info->payload, &val, 1);
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_VOLUME_CHANGE, data_info, sizeof(bt_notify_ag_at_arg_t) + 1);
    bfree(data_info);
}

static void bt_hfp_ag_app_onDialCall(U8 mux_id, char *number)
{
    bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + strlen(number));
    data_info->profile_channel = mux_id;
    data_info->payload_len = strlen(number);
    bmemcpy(data_info->payload, number, strlen(number));
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_MAKE_CALL_REQ, data_info, sizeof(bt_notify_ag_at_arg_t) + strlen(number));
    bfree(data_info);
#ifndef BT_USING_AG
    if (g_flag_auto_answer_call)
    {
        hfp_ag_at_cmd_result(mux_id, BTS2_SUCC);
        uint8_t phone_len = strlen(number);
        number[phone_len - 1] = '\0';
        bt_hfp_ag_app_call_status_change(mux_id, number, phone_len, 0, 2, PHONE_CALL_DIR_OUTGOING);
        bt_hfp_ag_app_call_status_change(mux_id, number, phone_len, 0, 3, PHONE_CALL_DIR_OUTGOING);
    }
    else
    {
        hfp_ag_at_cmd_result(mux_id, BTS2_FAILED);
    }
#endif
}

static void bt_hfp_ag_app_onSendDtmf(U8 mux_id, char *number)
{
    bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + strlen(number));
    data_info->profile_channel = mux_id;
    data_info->payload_len = strlen(number);
    bmemcpy(data_info->payload, number, strlen(number));
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_RECV_DTMF_KEY, data_info, sizeof(bt_notify_ag_at_arg_t) + strlen(number));
    bfree(data_info);
}

static void bt_hfp_ag_app_onNoiseReductionEnable(U8 mux_id)
{
}

static void bt_hfp_ag_app_onWBS(U8 mux_id)
{
}

static void bt_hfp_ag_app_onAtChld(U8 mux_id)
{
}

static void bt_hfp_ag_app_onAtCnum(U8 mux_id)
{
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_GET_LOCAL_PHONE_INFO_REQ, &mux_id, 1);
#ifndef BT_USING_AG
    hfp_phone_number_t local_phone_num;
    char *str = "19396395979";
    bmemcpy(&local_phone_num.phone_number, str, strlen(str) + 1);
    local_phone_num.type = 0x81;
    bt_hfp_ag_cnum_response(mux_id, &local_phone_num);
#endif
}

static void bt_hfp_ag_app_onAtCind(U8 mux_id)
{
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_GET_INDICATOR_STATUS_REQ, &mux_id, 1);
#ifndef BT_USING_AG
    if (g_flag_auto_answer_call)
    {
        hfp_cind_status_t cind_status;
        cind_status.service_status = 1;
        cind_status.call = g_cind_states.call;
        cind_status.callsetup =  g_cind_states.callsetup;
        cind_status.batt_level = 5;
        cind_status.signal = 3;
        cind_status.roam_status = 1;
        cind_status.callheld = g_cind_states.callheld;
        bt_hfp_ag_cind_response(mux_id, &cind_status);
    }
#endif
}

static void bt_hfp_ag_app_onAtCops(U8 mux_id)
{
    char *test_payload = "4G FOR TEST";
    bt_hfp_ag_cops_response(mux_id, test_payload, strlen(test_payload));
}

static void bt_hfp_ag_app_onAtClcc(U8 mux_id)
{
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_GET_ALL_REMT_CALLS_INFO_REQ, &mux_id, 1);
#ifndef BT_USING_AG
    if (g_flag_auto_answer_call)
    {
        hfp_phone_call_info_t *remote_call_info = bt_hfp_ag_app_get_remote_call_info();
        if (remote_call_info->call_idx)
        {
            bt_hfp_ag_clcc_response(mux_id, remote_call_info);
        }
        hfp_ag_at_cmd_result(mux_id, BTS2_SUCC);
    }
#endif
}

static void bt_hfp_ag_app_at_cmd_hdl(BTS2S_AG_AT_CMD_INFO *at_cmd)
{
    USER_TRACE("bt_hfp_ag_app_at_cmd_hdl %d %s", at_cmd->command_id, at_cmd->str);

    switch (at_cmd->command_id)
    {
    case HFP_AG_SPK_EVT:
    case HFP_AG_MIC_EVT:
    {
        bt_hfp_ag_app_vol_changed(at_cmd->mux_id, at_cmd->val);
        break;
    }
    case HFP_AG_AT_A_EVT:
    {
        bt_hfp_ag_app_answercall(at_cmd->mux_id);
        break;
    }
    case HFP_AG_AT_D_EVT:
    {
        bt_hfp_ag_app_onDialCall(at_cmd->mux_id, at_cmd->str);
        break;
    }
    case HFP_AG_AT_CHLD_EVT:
    {
        hfp_ag_at_cmd_result(at_cmd->mux_id, BTS2_SUCC);
        break;
    }
    case HFP_AG_AT_CHUP_EVT:
    {
        bt_hfp_ag_app_hangupcall(at_cmd->mux_id);
        break;
    }
    case HFP_AG_AT_CIND_EVT:
    {
        bt_hfp_ag_app_onAtCind(at_cmd->mux_id);
        break;
    }
    case HFP_AG_AT_VTS_EVT:
    {
        bt_hfp_ag_app_onSendDtmf(at_cmd->mux_id, at_cmd->str);
        break;
    }
    case HFP_AG_AT_BLDN_EVT:
    {
        bt_hfp_ag_app_onDialCall(at_cmd->mux_id, at_cmd->str);
        break;
    }
    case HFP_AG_AT_BVRA_EVT:
    {
        hfp_ag_at_cmd_result(at_cmd->mux_id, BTS2_SUCC);
        break;
    }
    case HFP_AG_AT_NREC_EVT:
    {
        break;
    }
    case HFP_AG_AT_BTRH_EVT:
    {
        break;
    }
    case HFP_AG_AT_CLCC_EVT:
    {
        bt_hfp_ag_app_onAtClcc(at_cmd->mux_id);
        break;
    }
    case HFP_AG_AT_COPS_EVT:
    {
        bt_hfp_ag_app_onAtCops(at_cmd->mux_id);
        hfp_ag_at_cmd_result(at_cmd->mux_id, BTS2_SUCC);
        break;
    }
    case HFP_AG_AT_CNUM_EVT:
    {
        bt_hfp_ag_app_onAtCnum(at_cmd->mux_id);
        break;
    }
    case HFP_AG_AT_UNAT_EVT:
    {
        LOG_I("ag recv at cmd:%s", at_cmd->str); //handle and need send result
        bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + strlen(at_cmd->str));
        data_info->profile_channel = at_cmd->mux_id;
        data_info->payload_len = strlen(at_cmd->str);
        bmemcpy(data_info->payload,  at_cmd->str, strlen(at_cmd->str));
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_AG, BT_NOTIFY_AG_EXTERN_AT_CMD_KEY_REQ, data_info, strlen(at_cmd->str));
        bfree(data_info);
        break;
    }
    case HFP_AG_WBS_EVT:
    {
        bt_hfp_ag_app_onWBS(at_cmd->mux_id);
        break;
    }
    case HFP_AG_BATT_EVT:
    {
        hfp_batt_vaule_t *batt = (hfp_batt_vaule_t *) at_cmd->str;
        // USER_TRACE("recv HFP_AG_BATT_EVT status:%d val :%d",batt->batt_status,batt->batt_val);
        break;
    }
    }

}

/*******************************************func API **********************************/
void bt_hfp_ag_msg_hdl(bts2_app_stru *bts2_app_data)
{
    U16 *msg_type;
    bts2_hfp_ag_inst_data *inst_data;
    inst_data = &bts2_app_data->hfp_ag_inst;
    msg_type = (U16 *)bts2_app_data->recv_msg;
    USER_TRACE("hfp ag msg *msg_type %x inst_data->st %d\n", *msg_type, inst_data->st);

    switch (*msg_type)
    {
    case BTS2MU_AG_REG_CFM:
    {
        bt_hfp_ag_reg_res_hdl(bts2_app_data);
        break;
    }
    case BTS2MU_AG_UNREG_CFM:
    {
        bt_hfp_ag_dereg_res_hdl(bts2_app_data);
        break;
    }
    case BTS2MU_AG_DISC_CFM:
    {
        BTS2S_AG_CONN_RES *con_msg;
        con_msg = (BTS2S_AG_CONN_RES *)bts2_app_data->recv_msg;


        bt_hfp_ag_app_device_state_changed(bts2_app_data, con_msg);
        //USER_TRACE("BTS2MU_AG_CONN_STATE state: %d res: %d type %d", con_msg->device_state, con_msg->res, con_msg->type);
        break;
    }
    case BTS2MU_AG_CONN_IND:
    {
        BTS2S_AG_CONN_REQ *con_msg;
        con_msg = (BTS2S_AG_CONN_REQ *)bts2_app_data->recv_msg;
        hfp_ag_connect_ind_res(&con_msg->bd, con_msg->rfcomm_chnl, 1);//acpt connect from remote
        // hfp_ag_connect_ind_res(&con_msg->bd, con_msg->rfcomm_chnl, 0);//reject connect from remote
        break;
    }
    case BTS2MU_AG_CONN_CFM:
    {
        BTS2S_AG_CONN_RES *con_msg;
        con_msg = (BTS2S_AG_CONN_RES *)bts2_app_data->recv_msg;
        bt_hfp_ag_app_device_state_changed(bts2_app_data, con_msg);
        USER_TRACE("BTS2MU_AG_CONN_CFM state: %d res: %d type %d", con_msg->device_state, con_msg->res, con_msg->type);
        break;
    }
    case BTS2MU_AG_DISC_IND:
    {
        BTS2S_AG_CONN_RES *con_msg;
        con_msg = (BTS2S_AG_CONN_RES *)bts2_app_data->recv_msg;
        bt_hfp_ag_app_device_state_changed(bts2_app_data, con_msg);
        USER_TRACE("BTS2MU_AG_DISC_IND state: %d res: %d type %d", con_msg->device_state, con_msg->res, con_msg->type);
        break;
    }
    case BTS2MU_AG_AUDIO_CFM:
    {
        BTS2S_AG_AUDIO_CONN_CFM *recv_msg;
        recv_msg = (BTS2S_AG_AUDIO_CONN_CFM *)bts2_app_data->recv_msg;
        break;
    }
    case BTS2MU_AG_AUDIO_CONN_CFM:
    {
        BTS2S_HF_AUDIO_INFO *msg;
        msg = (BTS2S_HF_AUDIO_INFO *)bts2_app_data->recv_msg;
        if (msg->audio_on)
        {
            USER_TRACE("<< Audio connected\n");
            bt_notify_device_sco_info_t sco_info;
            sco_info.sco_type = BT_NOTIFY_HFP_AG;
            sco_info.sco_res = BTS2_SUCC;
            sco_info.profile_channel = msg->mux_id;
            bt_interface_bt_event_notify(BT_NOTIFY_COMMON, BT_NOTIFY_COMMON_SCO_CONNECTED,
                                         &sco_info, sizeof(bt_notify_device_sco_info_t));
        }
        else
        {
            USER_TRACE("<< Audio disconnected\n");
            bt_notify_device_sco_info_t sco_info;
            sco_info.sco_type = BT_NOTIFY_HFP_AG;
            sco_info.sco_res = BTS2_SUCC;
            sco_info.profile_channel = msg->mux_id;
            bt_interface_bt_event_notify(BT_NOTIFY_COMMON, BT_NOTIFY_COMMON_SCO_DISCONNECTED,
                                         &sco_info, sizeof(bt_notify_device_sco_info_t));
        }

#ifdef AUDIO_USING_MANAGER
        hfp_ag_audio_opt(msg, msg->audio_on);
#endif // AUDIO_USING_MANAGER
        break;
    }
    case BTS2MU_AG_AUDIO_CONN_IND:
    {
        BTS2S_AG_AUDIO_CONN_IND *msg;
        msg = (BTS2S_AG_AUDIO_CONN_IND *)bts2_app_data->recv_msg;
        USER_TRACE("<< BTS2MU_AG_AUDIO_CONN_IND\n");
        if (msg->code_id == 0)
        {
            gap_sync_conn_res(&msg->bd, msg->code_id, 0);
        }
        else
        {
            gap_sync_conn_res(&msg->bd, msg->code_id, 1);
        }
        break;
    }
    case BTS2MU_AG_AUDIO_DISC_CFM:
    {
        BTS2S_HF_AUDIO_INFO *msg;
        msg = (BTS2S_HF_AUDIO_INFO *)bts2_app_data->recv_msg;
        USER_TRACE("<< BTS2MU_AG_AUDIO_DISC_CFM\n");
#ifdef AUDIO_USING_MANAGER
        hfp_ag_audio_opt(msg, msg->audio_on);
#endif // AUDIO_USING_MANAGER
        break;
    }
    case BTS2MU_AG_AUDIO_DISC_IND:
    {
        BTS2S_HF_AUDIO_INFO *msg;
        msg = (BTS2S_HF_AUDIO_INFO *)bts2_app_data->recv_msg;
        USER_TRACE("<< BTS2MU_AG_AUDIO_DISC_IND hdl 0x%2x\n", msg->sco_hdl);
#ifdef AUDIO_USING_MANAGER
        hfp_ag_audio_opt(msg, msg->audio_on);
#endif // AUDIO_USING_MANAGER
        break;
    }
    case BTS2MU_AG_SCO_RENEGOTIATE_IND:
    {
        break;
    }
    case BTS2MU_AG_SCO_RENEGOTIATE_CFM:
    {
        break;
    }
    case BTS2MU_AG_AT_CMD_EVENT:
    {
        BTS2S_AG_AT_CMD_INFO *msg;
        msg = (BTS2S_AG_AT_CMD_INFO *)bts2_app_data->recv_msg;
        bt_hfp_ag_app_at_cmd_hdl(msg);
        break;
    }
    }

}

void bt_hfp_ag_app_init(bts2_app_stru *bts2_app_data)
{
    if (bts2_app_data)
    {
        bts2_app_data->hfp_ag_inst.srv_chnl = 0xff;
        bts2_app_data->hfp_ag_inst.profile_state = HFP_DEVICE_DISCONNECTED;
        bts2_app_data->hfp_ag_inst.pre_profile_state = HFP_DEVICE_DISCONNECTED;
        bts2_app_data->hfp_ag_inst.call_state = HFP_CALL_IDLE;
        bt_hfp_ag_app_profile_service_update(&bts2_app_data->hfp_ag_inst, HFP_AG_APP_INIT);
        bmemset(&g_remote_calls_info, 0x00, sizeof(hfp_phone_call_info_t));
#ifdef AUDIO_USING_MANAGER
        hfp_ag_audio_register();
#endif // AUDIO_USING_MANAGER
    }
}

void bt_hfp_start_profile_service(bts2_app_stru *bts2_app_data)
{
    bts2_hfp_ag_inst_data *ptr;
    ptr = &bts2_app_data->hfp_ag_inst;
    switch (ptr->st)
    {
    case HFP_AG_APP_INIT:
    {
        U32 features = (U32)(HFP_AG_FEAT_ECNR | \
                             HFP_AG_FEAT_INBAND | \
                             HFP_AG_FEAT_REJECT | \
                             HFP_AG_FEAT_ECS | \
                             HFP_AG_FEAT_EXTERR | \
                             HFP_AG_FEAT_CODEC | \
                             HFP_AG_FEAT_ESCO);
        hfp_ag_register(features);
        bt_hfp_ag_app_profile_service_update(ptr, HFP_AG_APP_OPENING);
        break;
    }
    case HFP_AG_APP_OPENING:
    {
        //update to ag is in opening please wait
        break;
    }
    case HFP_AG_APP_OPEN:
    {
        //update to ag have been opened please wait
        break;
    }
    case HFP_AG_APP_CLOSING:
    {
        //update to ag is in closing please open later
        break;
    }
    }

}
void bt_hfp_stop_profile_service(bts2_app_stru *bts2_app_data)
{
    bts2_hfp_ag_inst_data *ptr;
    ptr = &bts2_app_data->hfp_ag_inst;
    switch (ptr->st)
    {
    case HFP_AG_APP_INIT:
    {
        //notify app that ag isnot opened
        break;
    }
    case HFP_AG_APP_OPENING:
    {
        //notify app that ag is in opening please close later
        break;
    }
    case HFP_AG_APP_OPEN:
    {
        if (ptr->profile_state != HFP_DEVICE_DISCONNECTED)
        {
            bt_hfp_ag_disconnect_request(NULL);
        }
        else
        {
            hfp_ag_deregister();
        }

        bt_hfp_ag_app_profile_service_update(ptr, HFP_AG_APP_CLOSING);
        break;
    }
    case HFP_AG_APP_CLOSING:
    {
        //notify APP that ag is in closing please wait
        break;
    }
    }
}

int bt_hfp_ag_connect_request(BTS2S_BD_ADDR *bd)
{
    hfp_ag_connect(bd, 0);
    return 0;
}

void bt_hfp_ag_disconnect_request(BTS2S_BD_ADDR *bd)
{
    //BTS2S_BD_ADDR temp = {0xffffff, 0xff, 0xffff};
    hfp_ag_disconnect(bd);
}

void bt_hfp_connect_audio(BTS2S_BD_ADDR *bd)
{
    hfp_ag_connect_audio(bd);
}

void bt_hfp_disconnect_audio(BTS2S_BD_ADDR *bd)
{

#ifdef AUDIO_USING_MANAGER
    BTS2S_HF_AUDIO_INFO msg;
    hfp_ag_audio_opt(&msg, 0);
#endif // AUDIO_USING_MANAGER
    hfp_ag_disconnect_audio(bd);
}

void bt_hfp_ag_call_state_update_listener(U8 mux_id,  HFP_CALL_INFO_T *call_info)
{
    hfp_ag_phone_call_status_changed_api(mux_id, call_info);
}

void bt_hfp_ag_remote_calls_res_hdl(U8 mux_id, hfp_remote_calls_info_t *call_info)
{
    //U8 num_call = call_info->num_call;
    if (call_info->num_call)
    {
        for (U8 i = 0; i < call_info->num_call; i++)
        {
            bt_hfp_ag_clcc_response(mux_id, &call_info->calls[i]);
        }
    }

    hfp_ag_at_cmd_result(mux_id, BTS2_SUCC);
}

/*************************************AT CMD *************************************/
void bt_hfp_ag_spk_vol_control(U8 mux_id, U8 vol)
{
    if (0 <= vol && vol <= 15)
    {
        hfp_ag_spk_volume_control(mux_id, vol);
    }
    else
    {
        USER_TRACE("AG spk ERROR VOL %d", vol);
    }
}

void bt_hfp_ag_mic_vol_control(U8 mux_id, U8 vol)
{
    if (0 <= vol && vol <= 15)
    {
        hfp_ag_mic_volume_control(mux_id, vol);
    }
    else
    {
        USER_TRACE("AG mic ERROR VOL %d", vol);
    }
}

void bt_hfp_ag_cind_response(U8 mux_id, hfp_cind_status_t *cind_status)
{
    char payload[24];
    U8 payload_len = snprintf(payload, sizeof(payload), "%d,%d,%d,%d,%d,%d,%d",
                              cind_status->service_status,
                              cind_status->call,
                              cind_status->callsetup,
                              cind_status->batt_level,
                              cind_status->signal,
                              cind_status->roam_status,
                              cind_status->callheld);
    hfp_ag_cind_response(mux_id, payload, payload_len);
    hfp_ag_at_cmd_result(mux_id, BTS2_SUCC);
}

void bt_hfp_ag_ind_status_update(U8 mux_id, U8 type, U8 val)
{
    char payload[6];
    U8 payload_len = snprintf(payload, sizeof(payload), "%u,%u", type, val);
    hfp_ag_ind_status_update(mux_id, payload, payload_len);
}

void bt_hfp_ag_brva_response(U8 mux_id, U8 val)
{
    if (0 <= val && val <= 1)
    {
        hfp_ag_brva_response(mux_id, val);
    }
    else
    {
        USER_TRACE("AG brva ERROR VOL %d", val);
    }
}

void bt_hfp_ag_set_inband(U8 mux_id, U8 val)
{
    if (0 <= val && val <= 1)
    {
        hfp_ag_set_inband(mux_id, val);
    }
    else
    {
        USER_TRACE("AG inband ERROR VOL %d", val);
    }
}

void bt_hfp_ag_cnum_response(U8 mux_id, hfp_phone_number_t *local_phone_num)
{
    char payload[40];
    U8 payload_len = snprintf(payload, sizeof(payload), ",\"%s\",%u, ,...", local_phone_num->phone_number, local_phone_num->type);

    hfp_ag_cnum_response(mux_id, payload, payload_len);
    hfp_ag_at_cmd_result(mux_id, BTS2_SUCC);
}

void bt_hfp_ag_set_btrh(U8 mux_id, U8 val)
{
    hfp_ag_set_btrh(mux_id, val);
}

void bt_hfp_ag_clcc_response(U8 mux_id, hfp_phone_call_info_t *call_info)
{
    char payload[100];
    int payload_len = snprintf(payload, sizeof(payload), "%d,%d,%d,%d,%d",  call_info->call_idx, call_info->call_dir,
                               call_info->call_status, call_info->call_mode, call_info->call_mtpty);
    if (call_info->phone_info.type != 0)
    {
        payload_len += snprintf(payload + payload_len, sizeof(payload) - payload_len - 3, ",\"%s\",%u",
                                call_info->phone_info.phone_number, call_info->phone_info.type);
    }

    hfp_ag_clcc_response(mux_id, payload, payload_len);
}

void bt_hfp_ag_cops_response(U8 mux_id, char *payload, U8 payload_len)
{
    hfp_ag_cops_response(mux_id, payload, payload_len);
}

void bt_hfp_ag_set_bcs(U8 mux_id, U8 code_id)
{
    hfp_ag_set_codec_id(mux_id, code_id);
}

void bt_hfp_ag_clip_response(U8 mux_id, hfp_phone_number_t *remote_phone_num)
{
    char payload[40];
    U8 payload_len = snprintf(payload, sizeof(payload), "\"%s\",%u", remote_phone_num->phone_number, remote_phone_num->type);

    hfp_ag_clip_response(mux_id, payload, payload_len);
}

void bt_hfp_ag_at_result_res(U8 mux_id, U8 res)
{
    hfp_ag_at_cmd_result(mux_id, res);
}

#endif



