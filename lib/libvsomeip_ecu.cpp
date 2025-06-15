#include "public/base.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

std::shared_ptr<vsomeip::application> app;
std::mutex mutex;
std::condition_variable condition;
std::atomic<bool> has_new_message = false; // 标志位，表示是否有新的订阅消息

void send_data(const s_vehicle_data_t &data) {
    std::shared_ptr<vsomeip::payload> its_payload = vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> its_payload_data;

    // 构造发布的消息格式
    its_payload_data.push_back(static_cast<vsomeip::byte_t>(data.type));
    its_payload_data.push_back(static_cast<vsomeip::byte_t>((data.message >> 8) & 0xFF));
    its_payload_data.push_back(static_cast<vsomeip::byte_t>(data.message & 0xFF));

    its_payload->set_data(its_payload_data);
    app->notify(SERVICE_ID, INSTANCE_ID, EVENT_ID_PUBLISH, its_payload);

    std::cout << "Published message: type=" << data.type
              << ", message=" << data.message << std::endl;
}

void on_message(const std::shared_ptr<vsomeip::message> &msg) {
    std::unique_lock<std::mutex> lock(mutex);

    // 模拟接收到的消息内容
    s_vehicle_data_t received_data;
    received_data.type = TYPE_DATA_SPEED; // 固定类型，例如速度数据
    received_data.message = 100;           // 固定值，例如速度为100

    std::cout << "Received subscribed message: type=" << (int)received_data.type
              << ", value=" << received_data.message << std::endl;

    // 转换为发布的消息格式
    s_vehicle_data_t publish_data;
    publish_data.type = static_cast<char>(received_data.type);
    publish_data.message = static_cast<int>(received_data.message) + 50; // 示例处理逻辑

    // // 设置标志位并通知发布逻辑
    // has_new_message = true;
    // condition.notify_one();
    // send_data(publish_data);

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
            if (is_available) {
                std::set<vsomeip::eventgroup_t> eventgroups;
                eventgroups.insert(EVENT_ID_SUBSCRIBE);

                // 请求订阅的事件
                app->request_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_SUBSCRIBE, eventgroups, vsomeip::event_type_e::ET_FIELD);

                // 订阅事件组
                app->subscribe(SERVICE_ID, INSTANCE_ID, EVENTGROUP_ID_SUBSCRIBE);

                std::cout << "Requested event and subscribed to eventgroup" << std::endl;
            }
        });

    // 注册消息处理 - 订阅的事件
    app->register_message_handler(SERVICE_ID, INSTANCE_ID, EVENT_ID_PUBLISH, on_message);

    // 提供服务
    app->offer_service(SERVICE_ID, INSTANCE_ID);

    // 提供发布的事件
    std::set<vsomeip::eventgroup_t> eventgroups;
    eventgroups.insert(EVENTGROUP_ID_PUBLISH);
    app->offer_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_PUBLISH, eventgroups, vsomeip::event_type_e::ET_FIELD);

    // // 启动发布线程
    // std::thread publisher_thread([]() {
    //     while (true) {
    //         std::unique_lock<std::mutex> lock(mutex);
    //         condition.wait(lock, [] { return has_new_message.load(); }); // 等待订阅消息

    //         // 重置标志位
    //         has_new_message = false;

    //         std::cout << "Publishing message after receiving subscription..." << std::endl;
    //     }
    // });
    // 启动发布线程
    std::thread publisher_thread([]() {
        while (true) {
            // 模拟发布固定消息
            s_vehicle_data_t publish_data;
            publish_data.type = TYPE_DATA_SPEED; // 示例类型
            publish_data.message = 120;         // 示例值

            send_data(publish_data);

            std::this_thread::sleep_for(std::chrono::seconds(1)); // 每秒发布一次
        }
    });

    // 启动应用
    app->start();
    publisher_thread.join();
}