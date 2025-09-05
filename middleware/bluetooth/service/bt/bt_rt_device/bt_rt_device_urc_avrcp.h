/*
 * SPDX-FileCopyrightText: 2022-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BT_RT_DEVICE_URC_AVRCP_H
#define _BT_RT_DEVICE_URC_AVRCP_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "bts2_bt.h"


extern    void urc_func_bt_avrcp_open_complete_sifli(void);
extern    void urc_func_bt_avrcp_close_complete_sifli(void);
extern    void urc_func_bt_avrcp_playback_status_sifli(uint8_t play_status);
extern    void urc_func_bt_avrcp_volume_change_rigister_sifli(void);
extern    void urc_func_bt_avrcp_track_change_sifli(uint8_t track_change);
extern    void urc_func_bt_avrcp_mp3_detail_sifli(bt_mp3_detail_info_t *detail);
extern    void urc_func_bt_avrcp_song_progress_sifli(uint32_t progress);
extern    void urc_func_bt_avrcp_absolute_volume_sifli(uint8_t volume);
extern    int bt_sifli_notify_avrcp_event_hdl(uint16_t event_id, uint8_t *data, uint16_t data_len);

#endif /* _BT_RT_DEVICE_URC_AVRCP_H */



