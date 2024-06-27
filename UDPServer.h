#ifndef UDP_SERVER
#define UDP_SERVER

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <memory>

#include "SPSCQueue.h"
#include "payload.h"

typedef struct {
    int data_len;
    unsigned char data[64];
} IMUDATA;

using UDPData = std::shared_ptr<udp_data_t>;
using UDPCmd = std::shared_ptr<udp_cmd_t>;
using ImuData = std::shared_ptr<IMUDATA>;

class UDPServer {
public:

    typedef struct {
        int udp_data_port;
        int udp_data_revice_timeout;
        std::string udp_cmd_des_ip_addr;
        int udp_cmd_des_port;
        int udp_cmd_send_timeout;
        int udp_imu_port;
        int udp_imu_revice_timeout;
        int udp_timeout;
    } Config;

    UDPServer(const Config& config);

    ~UDPServer() = default;

    bool init();

    bool receive(UDPData& udp_data);

    bool receive(ImuData& udp_imu);

    bool send(const UDPCmd& udp_cmd);

    void loop();

private:
    bool create_data_socket();
    bool create_cmd_socket();
    bool create_imu_socket();

    Config config_;

    SPSCQueue<UDPData> udp_data_q_;
    SPSCQueue<ImuData> udp_imu_q_;
    int udp_data_sock_fd_ = -1;
    int udp_cmd_sock_fd_ = -1;
    std::uint64_t pre_send_data_time_ = 0;
    int udp_imu_sock_fd_ = -1;
    int max_fd_ = -1;
    fd_set read_fds_;
};

#endif
