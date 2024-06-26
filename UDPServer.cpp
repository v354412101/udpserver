#include "UDPServer.h"
#include "spdlog/spdlog.h"

UDPServer::UDPServer() {
    udp_data_q = std::make_shared<SPSCQueue<UDPData>>(10);
}

UDPServer::~UDPServer() {
    
}

bool UDPServer::init() {
    udp_data_sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_data_sock_fd_ < 0) {
        spdlog::error("socket creation failed");
        return false;
    }

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8080);

    if (bind(udp_data_sock_fd_, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        spdlog::error("bind failed");
        close(udp_data_sock_fd_);
        return false;
    }

    return true;
}

bool UDPServer::revice_data(UDPData& udpdata) {
    return udp_data_q->pop(udpdata);
}

void UDPServer::loop() {
    int ret = -1;
    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(udp_data_sock_fd_, &read_fds);

        ret = ::select(udp_data_sock_fd_ + 1, &read_fds, nullptr, nullptr, nullptr);
        if (ret < 0) {
            spdlog::error("select error");
            continue;
        }

        if (FD_ISSET(udp_data_sock_fd_, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            UDPData udpdata = std::make_shared<udp_data_t>();
            int n = recvfrom(udp_data_sock_fd_, (char*)udpdata.get(), sizeof(udp_data_t), 0, (struct sockaddr*)&client_addr, &addr_len);
            //spdlog::info("recv {} bytes", udpdata->checksum);
            udp_data_q->push(udpdata);
        }
        // if (FD_ISSET(udp_cmd_sock_fd_, &write_fds)) {
        // }
    }
}