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
#include <iostream>
#include <vector>
#include <vsomeip/vsomeip.hpp>


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

// Callback function for vsomeip events
void on_message(const std::shared_ptr<vsomeip::message> &msg) {
    std::cout << "Received vsomeip message!" << std::endl;
}

void testVsomeip() {
    // Create vsomeip application
    auto app = vsomeip::runtime::get()->create_application("TestApp");

    // Initialize the application
    if (!app->init()) {
        std::cerr << "Failed to initialize vsomeip application!" << std::endl;
        return;
    }

    // Register a callback for incoming messages
    app->register_message_handler(0x1234, 0x5678, 0x0001, on_message);

    // Start the vsomeip application
    app->start();

    std::cout << "vsomeip application started successfully!" << std::endl;
}