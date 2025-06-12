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
#include "public/hello.h"


#define SERVICE_ID 0x1234
#define INSTANCE_ID 0x5678
#define EVENT_ID 0x8778
#define EVENTGROUP_ID 0x4465

std::shared_ptr<vsomeip::application> app;

void printHello() { 
    std::vector<int> hello;
    for(int i = 0; i < 5; i++){
        hello.push_back(i);
    }
    for(int i = 0; i < 5; i++){
        std::cout << "Hello QNX from C++" << i << std::endl;
    }

    return;
}


void on_message(const std::shared_ptr<vsomeip::message>& msg) {
    std::cout << "Received message [" 
              << std::hex << msg->get_service() << "."
              << msg->get_instance() << "."
              << msg->get_method() << "]\n";
              
    std::string payload(reinterpret_cast<const char*>(msg->get_payload()->get_data()),
                       msg->get_payload()->get_length());
    std::cout << "Payload: " << payload << std::endl;
}

void testVsomeip() {
    app = vsomeip::runtime::get()->create_application("TestApp");
    if (!app->init()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return;
    }

    // 服务可用性回调
    app->register_availability_handler(SERVICE_ID, INSTANCE_ID,
        [&](vsomeip::service_t, vsomeip::instance_t, bool is_available) {
            std::cout << "Service [" << std::hex << SERVICE_ID << "." << INSTANCE_ID
                      << "] is " << (is_available ? "available" : "NOT available") << std::endl;
        std::set<vsomeip::eventgroup_t> eventgroups;
            eventgroups.insert(EVENTGROUP_ID);
            if (is_available) {
                // 在服务端提供事件后，必须显式请求该事件
                app->request_event(
                    SERVICE_ID, 
                    INSTANCE_ID, 
                    EVENT_ID, 
                    eventgroups, 
                    vsomeip::event_type_e::ET_FIELD
                );
                
                // 订阅事件组
                app->subscribe(SERVICE_ID, INSTANCE_ID, EVENTGROUP_ID);
                
                std::cout << "Requested event and subscribed to eventgroup" << std::endl;
            }
        });

    // 注册消息处理 - 必须为事件ID注册
    app->register_message_handler(SERVICE_ID, INSTANCE_ID, EVENT_ID, on_message);
    
    // 请求服务
    app->request_service(SERVICE_ID, INSTANCE_ID);
    
    // 启动应用
    app->start();
    
    // 保持应用运行以接收消息    app->stop();
}