/*******************************************************************************
 * Copyright (c) 2023 BlackBerry Limited. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software.   Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit [http://licensing.qnx.com] or email licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at [http://licensing.qnx.com/license-guide/]
 * for other information.
 *******************************************************************************/
#ifndef _BASE_H_
#define _BASE_H_
#include <iostream>
#include <vector>
#include <vsomeip/vsomeip.hpp>

typedef enum {
    TYPE_DATA_SPEED,
    TYPE_DATA_RPM,
    TYPE_DATA_FUEL,
    TYPE_DATA_TEMP,
    TYPE_DATA_LI,
    TYPE_DATA_RI,
    TYPE_DATA_UN
} method_t;

typedef struct s_vehicle_data {
    char type;
    int message;
} s_vehicle_data_t;

#define SERVICE_ID 0x1234
#define INSTANCE_ID 0x5678
#define EVENTGROUP_ID_SUBSCRIBE 0x4465
#define EVENTGROUP_ID_PUBLISH 0x5544

#define EVENT_ID_SUBSCRIBE 0x1001 // 订阅车控端的事件ID
#define EVENT_ID_PUBLISH 0x2001   // 发布仪表盘的事件ID

void on_message(const std::shared_ptr<vsomeip::message> &msg);
void testVsomeip();

#endif /* _HELLO_H_ */
