#ifndef GESTURES_ALOG_H__
#define GESTURES_ALOG_H__

#include "stdint.h"
#include "stdio.h"
//#include "app.h"
#include "math.h"

typedef struct
{
    float  mpss[3];
} rising_hand_data_t;


typedef enum
{
    EVENT_RAISE_HAND,      // ̧������
    EVENT_FAILING_HAND,    // ���ֹ���
} rising_hand_type;

//can be used in 25HZ and 28HZ

//����Ҫ��
/************************************
**����ˮƽ0�ȣ����泯��ƽ�� Z=9.8F
**���洹ֱ90�ȣ��������� X=-9.8F
**������90�ȣ��������� Y=9.8F
*************************************
**/
extern void SmartWear_SportHealth_Gesture_State_Update_Callback(rising_hand_type);
extern void Rising_hand_algorithm_data_handle(rising_hand_data_t sensor_data);
extern uint32_t get_algo_rise_hand_lib_version_major(void);
extern uint32_t get_algo_rise_hand_lib_version_minor(void);
extern uint32_t get_algo_rise_hand_lib_version_patch(void);

#endif











