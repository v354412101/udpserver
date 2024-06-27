#include "UDPServer.h"

#include <chrono>

#include "spdlog/spdlog.h"

UDPServer::UDPServer(const Config& config) : config_(config),
                                             udp_data_q_(10),
                                             udp_imu_q_(10) {
}

bool UDPServer::init() {
    bool ret = create_data_socket();
    if(!ret) {
        return false;
    }
    ret = create_cmd_socket();
    if(!ret) {
        return false;
    }
    ret = create_imu_socket();
    if(!ret) {
        return false;
    }

    return true;
}

bool UDPServer::receive(UDPData& udpdata) {
    return udp_data_q_.pop(udpdata);
}

bool UDPServer::receive(ImuData& udp_imu) {
    return udp_imu_q_.pop(udp_imu);
}

bool UDPServer::send(const UDPCmd& udp_cmd) {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    if(pre_send_data_time_ == 0) {
        pre_send_data_time_ = now;
    }
    if(now - pre_send_data_time_ > config_.udp_cmd_send_timeout) {
        spdlog::warn("udp cmd send timeout:{}ms", now - pre_send_data_time_);
    }
    pre_send_data_time_ = now;

    int len = ::send(udp_cmd_sock_fd_, udp_cmd.get(), sizeof(udp_cmd_t), 0);
    if(len != sizeof(udp_cmd_t)) {
        // 当对端ip或端口不存在或不可达时，本地tcpdump抓包会收到一个ICMP包，Destination unreachable(Port unreachable)
        spdlog::error("send error: {}", strerror(errno));
    }
}

void UDPServer::loop() {
    std::uint64_t pre_revice_data_time = 0;
    std::uint64_t pre_revice_imu_time = 0;
    while (true) {
        FD_ZERO(&read_fds_);
        FD_SET(udp_data_sock_fd_, &read_fds_);
        FD_SET(udp_imu_sock_fd_, &read_fds_);

        struct timeval tv;
        tv.tv_sec = config_.udp_timeout / 1000;
        tv.tv_usec = 0;

        int ret = ::select(max_fd_ + 1, &read_fds_, nullptr, nullptr, &tv);
        if (ret <= 0) {
            spdlog::warn("select ret={}", ret);
            continue;
        }

        if (FD_ISSET(udp_data_sock_fd_, &read_fds_)) {
            struct sockaddr_in c_addr;
            socklen_t addr_len = sizeof(c_addr);
            std::memset(&c_addr, 0, sizeof(c_addr));
            UDPData udp_data = std::make_shared<udp_data_t>();
            std::memset(udp_data.get(), sizeof(udp_data_t), 0);
            int recv_len = recvfrom(udp_data_sock_fd_, udp_data.get(), sizeof(udp_data_t), 0, (struct sockaddr*)&c_addr, &addr_len);
            if(recv_len == sizeof(udp_data_t)) {
                auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                if(pre_revice_data_time == 0) {
                    pre_revice_data_time = now;
                }
                if(now - pre_revice_data_time > config_.udp_data_revice_timeout) {
                    spdlog::warn("udp data recvfrom timeout:{}ms", now - pre_revice_data_time);
                }
                pre_revice_data_time = now;

                udp_data_q_.push(udp_data);
            } else {
                spdlog::error("udp data recvfrom error:{}, recv_len={} src ip:{},port:{}",
                               strerror(errno), recv_len, inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
            }
        }
        if (FD_ISSET(udp_imu_sock_fd_, &read_fds_)) {
            struct sockaddr_in c_addr;
            socklen_t addr_len = sizeof(c_addr);
            std::memset(&c_addr, 0, sizeof(c_addr));
            ImuData udp_imu = std::make_shared<IMUDATA>();
            std::memset(udp_imu.get(), sizeof(IMUDATA), 0);
            int recv_len = recvfrom(udp_imu_sock_fd_, udp_imu->data, sizeof(udp_imu->data), 0, (struct sockaddr*)&c_addr, &addr_len);
            if(recv_len > 0) {
                auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                if(pre_revice_imu_time == 0) {
                    pre_revice_imu_time = now;
                }
                if(now - pre_revice_imu_time > config_.udp_imu_revice_timeout) {
                    spdlog::warn("udp imu recvfrom timeout:{}ms", now - pre_revice_imu_time);
                }
                pre_revice_imu_time = now;

                udp_imu->data_len = recv_len;
                udp_imu_q_.push(udp_imu);
            } else {
                spdlog::error("udp cmd recv error:{}, recv_len={} src ip:{},port:{}",
                                strerror(errno), recv_len, inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
            }
        }
    }
}

bool UDPServer::create_data_socket() {
    udp_data_sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_data_sock_fd_ < 0) {
        spdlog::error("udp_data_sock_fd_ create failed");
        close(udp_data_sock_fd_);
        return false;
    }

    if(udp_data_sock_fd_ > max_fd_) {
        max_fd_ = udp_data_sock_fd_;
    }

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(config_.udp_data_port);

    if (bind(udp_data_sock_fd_, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        spdlog::error("udp_data_sock_fd_ bind failed");
        close(udp_data_sock_fd_);
        return false;
    }

    return true;
}
bool UDPServer::create_cmd_socket() {
    udp_cmd_sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_cmd_sock_fd_ < 0) {
        spdlog::error("udp_cmd_sock_fd_ create failed");
        return false;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(config_.udp_cmd_des_ip_addr.c_str());
    serv_addr.sin_port = htons(config_.udp_cmd_des_port);

    if (connect(udp_cmd_sock_fd_, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        spdlog::error("udp_cmd_sock_fd_ connect failed");
        close(udp_cmd_sock_fd_);
        return false;
    }

    fcntl(udp_cmd_sock_fd_, F_SETFL, fcntl(udp_cmd_sock_fd_, F_GETFL, 0) | O_NONBLOCK);
    return true;
}

bool UDPServer::create_imu_socket() {
    udp_imu_sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_imu_sock_fd_ < 0) {
        spdlog::error("udp_imu_sock_fd_ create failed");
        return false;
    }

    if(udp_imu_sock_fd_ > max_fd_) {
        max_fd_ = udp_imu_sock_fd_;
    }

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(config_.udp_imu_port);

    if (bind(udp_imu_sock_fd_, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        spdlog::error("udp_imu_sock_fd_ bind failed");
        close(udp_imu_sock_fd_);
        return false;
    }
    return true;
}