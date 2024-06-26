#include <memory>
#include <thread>

#include "spdlog/spdlog.h"
#include "UDPServer.h"

int main(int argc, char** argv) {
    spdlog::set_pattern("[%m%d %H:%M:%S.%e] [%^%l%$] %v");

    std::shared_ptr<UDPServer> udpserver = std::make_shared<UDPServer>();
    bool ret = udpserver->init();
    if(!ret) {
        spdlog::error("udpserver init failed");
        return -1;
    }

    std::thread([&]() {
        bool ret = true;
        //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        int32_t pre_checksum = 0;
        while(true) {
            UDPData udpdata;
            ret = udpserver->revice_data(udpdata);
            if (ret) {
                if(udpdata != nullptr) {
                    //spdlog::info("udpdata: checksum {}", udpdata->checksum);
                    if(udpdata->checksum != pre_checksum + 1) {
                        spdlog::warn("udpdata->checksum {} != pre_checksum + 1 {}", udpdata->checksum, pre_checksum);
                    }
                    pre_checksum = udpdata->checksum;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }).detach();

    udpserver->loop();

    return 0;
}
