#include "public/base.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <atomic>

std::shared_ptr<vsomeip::application> app;
std::mutex mutex;
std::condition_variable condition;
std::queue<s_vehicle_data_t> data_queue; // 用于存储订阅的数据
std::atomic<bool> has_new_message = false; // 标志位，表示是否有新的订阅消息

void send_data(const s_vehicle_data_t &data) {
    std::shared_ptr<vsomeip::payload> its_payload = vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> its_payload_data;

    // 构造发布的消息格式
    its_payload_data.push_back(static_cast<vsomeip::byte_t>(data.type));
    its_payload_data.push_back(static_cast<vsomeip::byte_t>((data.message >> 8) & 0xFF));
    its_payload_data.push_back(static_cast<vsomeip::byte_t>(data.message & 0xFF));

    its_payload->set_data(its_payload_data);
    app->notify(SERVICE_ID_PUBLISH, INSTANCE_ID_PUBLISH, EVENT_ID_PUBLISH, its_payload);

    std::cout << "Published message: type=" << static_cast<int>(data.type)
              << ", message=" << data.message << std::endl;
}

void on_message(const std::shared_ptr<vsomeip::message> &msg) {
    std::shared_ptr<vsomeip::payload> its_payload = msg->get_payload();
    vsomeip::length_t l = its_payload->get_length();

    if (l < 3) { // 确保 payload 至少有 3 个字节
        std::cerr << "Received payload is too short!" << std::endl;
        return;
    }

    // 解析车控端的数据
    const vsomeip::byte_t* data_ptr = its_payload->get_data();
    s_vehicle_data_t received_data;
    received_data.type = static_cast<char>(data_ptr[0]); // 第 1 字节为 type
    received_data.message = (static_cast<int>(data_ptr[1]) << 8) | static_cast<int>(data_ptr[2]); // 高低字节组合

    std::cout << "Received message from vehicle control: type=" << (int)received_data.type
              << ", value=" << received_data.message << std::endl;

    // 将数据存入队列
    {
        std::lock_guard<std::mutex> lock(mutex);
        data_queue.push(received_data);
        has_new_message = true;
    }
    condition.notify_one(); // 通知发布线程
}

void testVsomeip() {
    app = vsomeip::runtime::get()->create_application("TestApp");

    if (!app->init()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return;
    }

    // 请求服务
    app->request_service(SERVICE_ID_SUBSCRIBE, INSTANCE_ID_SUBSCRIBE);

    // 服务可用性回调
    app->register_availability_handler(SERVICE_ID_SUBSCRIBE, INSTANCE_ID_SUBSCRIBE,
        [&](vsomeip::service_t, vsomeip::instance_t, bool is_available) {
            if (is_available) {
                std::set<vsomeip::eventgroup_t> eventgroups;
                eventgroups.insert(EVENTGROUP_ID_SUBSCRIBE);

                // 请求订阅车控端的事件
                app->request_event(SERVICE_ID_SUBSCRIBE, INSTANCE_ID_SUBSCRIBE, EVENT_ID_SUBSCRIBE, eventgroups, vsomeip::event_type_e::ET_FIELD);

                // 订阅事件组
                app->subscribe(SERVICE_ID_SUBSCRIBE, INSTANCE_ID_SUBSCRIBE, EVENTGROUP_ID_SUBSCRIBE);

                std::cout << "Subscribed to vehicle control eventgroup" << std::endl;
            }
        });

    // 注册消息处理 用于处理接收到的车控端消息
    app->register_message_handler(SERVICE_ID_SUBSCRIBE, INSTANCE_ID_SUBSCRIBE, EVENT_ID_SUBSCRIBE, on_message);



    // 提供服务
    app->offer_service(SERVICE_ID_PUBLISH, INSTANCE_ID_PUBLISH);

    // 提供发布的事件
    std::set<vsomeip::eventgroup_t> eventgroups;
    eventgroups.insert(EVENTGROUP_ID_PUBLISH);
    app->offer_event(SERVICE_ID_PUBLISH, INSTANCE_ID_PUBLISH, EVENT_ID_PUBLISH, eventgroups, vsomeip::event_type_e::ET_FIELD);

    // 启动发布线程
    std::thread publisher_thread([]() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [] { return has_new_message.load(); }); // 等待订阅消息

            while (!data_queue.empty()) {
                s_vehicle_data_t data = data_queue.front();
                data_queue.pop();

                // 转发数据到仪表盘
                send_data(data);
            }

            has_new_message = false; // 重置标志位
        }
    });

    // 启动应用
    app->start();
    publisher_thread.join();
}