/*
 * SPDX-FileCopyrightText: 2022-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>
#include <string.h>
#include "drv_bt.h"
//#include "utf8_unicode.h"
#include "bt_rt_device.h"
#include "bts2_global.h"
#include "bts2_app_inc.h"

#define DBG_TAG               "bt_rt_device.urc_avrcp"
//#define DBG_LVL               DBG_INFO
#include <log.h>


void urc_func_bt_avrcp_open_complete_sifli(void)
{
    bt_notify_t args;
    args.event = BT_EVENT_AVRCP_OPEN_COMPLETE;
    args.args = RT_NULL;
    if (bt_sifli_check_bt_event(BT_SET_AVRCP_OPEN_EVENT))
    {
        LOG_I("URC BT avrcp open complete ind %x", bt_sifli_get_bt_event());
        bt_sifli_reset_bt_event(BT_SET_AVRCP_OPEN_EVENT);
        rt_bt_event_notify(&args);
    }
    return;
}

void urc_func_bt_avrcp_close_complete_sifli(void)
{
    bt_notify_t args;
    args.event = BT_EVENT_AVRCP_CLOSE_COMPLETE;
    args.args = RT_NULL;
    if (bt_sifli_check_bt_event(BT_SET_AVRCP_CLOSE_EVENT))
    {
        LOG_I("URC BT avrcp close complete ind %x", bt_sifli_get_bt_event());
        bt_sifli_reset_bt_event(BT_SET_AVRCP_CLOSE_EVENT);
        rt_bt_event_notify(&args);
    }
    return;
}

void urc_func_bt_avrcp_playback_status_sifli(uint8_t play_status)
{
    bt_notify_t args;
    bt_media_play_status_t ind = {0};
    ind.status = play_status; // solution 0x00:playing ;0x01:paused
    //ind.conn_idx = bt_cm_find_conn_index_by_addr(info->peer_addr);
    args.event = BT_EVENT_MUSIC_PLAY_STATUS_CHANGED;
    args.args = &ind;

    rt_bt_event_notify(&args);
    LOG_I("URC BT avrcp playback status ind");
}

void urc_func_bt_avrcp_volume_change_rigister_sifli(void)
{
    bt_notify_t args;
    args.event = BT_EVENT_AVRCP_VOLUME_CHANGE_RIGISTER;
    args.args = RT_NULL;

    rt_bt_event_notify(&args);
    LOG_I("URC BT avrcp volume change rigister ind");
}

void urc_func_bt_avrcp_track_change_sifli(uint8_t track_change)
{
    bt_notify_t args;
    args.event = BT_EVENT_MUSIC_TRACK_CHANGED;
    args.args = &track_change;// solution 0x00:playing ;0x01:paused

    rt_bt_event_notify(&args);
    LOG_I("URC BT avrcp track change ind");
}

void urc_func_bt_avrcp_mp3_detail_sifli(bt_mp3_detail_info_t *detail)
{
    bt_notify_t args;
    args.event = BT_EVENT_MP3_DETAIL_INFO;
    args.args = detail;
    rt_bt_event_notify(&args);

    LOG_I("URC BT mp3 detail ind");
}

void urc_func_bt_avrcp_song_progress_sifli(uint32_t progress)
{
    bt_notify_t args;
    args.event = BT_EVENT_SONG_PLAY_PROGRESS;
    args.args = &progress;//unit :ms. example:0x92be->37566ms

    rt_bt_event_notify(&args);
    LOG_I("URC BT mp3 progress ind");
}

void urc_func_bt_avrcp_absolute_volume_sifli(uint8_t volume)
{
    bt_notify_t args;
    bt_volume_set_t vol = {0};
    uint8_t max_vol = 15;
#ifdef AUDIO_USING_MANAGER
    max_vol = audio_server_get_max_volume();
#endif // AUDIO_USING_MANAGER
    uint8_t local_vol = bt_interface_avrcp_abs_vol_2_local_vol(volume, max_vol);
    vol.mode = BT_VOLUME_MEDIA;
    vol.volume.media_volume = local_vol;
    args.event = BT_EVENT_VOL_CHANGED;
    args.args = &vol;

    rt_bt_event_notify(&args);
    LOG_I("URC BT avrcp ab-volue ind:%d local %d", volume, local_vol);
}

int bt_sifli_notify_avrcp_event_hdl(uint16_t event_id, uint8_t *data, uint16_t data_len)
{
    switch (event_id)
    {
    case BT_NOTIFY_AVRCP_CLOSE_COMPLETE:
    {
        urc_func_bt_avrcp_close_complete_sifli();
        break;
    }
    case BT_NOTIFY_AVRCP_OPEN_COMPLETE:
    {
        urc_func_bt_avrcp_open_complete_sifli();
        break;
    }
    case BT_NOTIFY_AVRCP_PROFILE_CONNECTED:
    {
        bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
        urc_func_profile_conn_sifli(profile_info->mac.addr, BT_PROFILE_AVRCP);
        break;
    }
    case BT_NOTIFY_AVRCP_PROFILE_DISCONNECTED:
    {
        bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
        urc_func_profile_disc_sifli(profile_info->mac.addr, BT_PROFILE_AVRCP, profile_info->res);
        break;
    }
    case BT_NOTIFY_AVRCP_MUSIC_DETAIL_INFO:
    {
        urc_func_bt_avrcp_mp3_detail_sifli((bt_mp3_detail_info_t *) data);
        break;
    }
    case BT_NOTIFY_AVRCP_VOLUME_CHANGED_REGISTER:
    {
        urc_func_bt_avrcp_volume_change_rigister_sifli();
        break;
    }
    case BT_NOTIFY_AVRCP_ABSOLUTE_VOLUME:
    {
        urc_func_bt_avrcp_absolute_volume_sifli(data[0]);
        break;
    }
    case BT_NOTIFY_AVRCP_PLAY_STATUS:
    {
        urc_func_bt_avrcp_playback_status_sifli(data[0]);
        break;
    }
    case BT_NOTIFY_AVRCP_SONG_PROGREAS_STATUS:
    {
        uint32_t *progress = (uint32_t *) data;
        urc_func_bt_avrcp_playback_status_sifli(*progress);
        break;
    }
    case BT_NOTIFY_AVRCP_TRACK_CHANGE_STATUS:
    {
        urc_func_bt_avrcp_track_change_sifli(data[0]);
        break;
    }
    default:
        return -1;
    }

    return 0;
}


