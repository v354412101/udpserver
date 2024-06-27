#include <memory>
#include <thread>

#include "spdlog/spdlog.h"
#include "UDPServer.h"

int main(int argc, char** argv) {
    spdlog::set_pattern("[%m%d %H:%M:%S.%e] [%^%l%$] %v");

    UDPServer::Config config;
    config.udp_data_port = 8888;
    config.udp_data_revice_timeout = 5;
    config.udp_cmd_des_ip_addr = "192.168.11.150";
    config.udp_cmd_des_port = 8080;
    config.udp_cmd_send_timeout = 5;
    config.udp_imu_port = 8840;
    config.udp_imu_revice_timeout = 5;
    config.udp_timeout = 10000;

    auto udpserver = std::make_shared<UDPServer>(config);
    bool ret = udpserver->init();
    if(!ret) {
        spdlog::error("udpserver init failed");
        return -1;
    }

    // std::thread([&]() {
    //     bool ret = true;
    //     //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    //     int32_t pre_checksum = 0;
    //     while(true) {
    //         UDPData udpdata;
    //         ret = udpserver->receive(udpdata);
    //         if (ret) {
    //             if(udpdata != nullptr) {
    //                 //spdlog::info("udpdata: checksum {}", udpdata->checksum);
    //                 if(udpdata->checksum != pre_checksum + 1) {
    //                     spdlog::warn("udpdata->checksum {} != pre_checksum + 1 {}", udpdata->checksum, pre_checksum);
    //                 }
    //                 pre_checksum = udpdata->checksum;
    //             }
    //         }
    //         std::this_thread::sleep_for(std::chrono::milliseconds(1));
    //     }
    // }).detach();


    std::thread([&]() {
        bool ret = true;
        //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        int32_t pre_checksum = 0;
        int seq = 0;
        while(true) {
            UDPCmd udp_cmd = std::make_shared<udp_cmd_t>();
            udp_cmd->checksum = seq++;
            ret = udpserver->send(udp_cmd);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }).detach();

    udpserver->loop();

    return 0;
}
