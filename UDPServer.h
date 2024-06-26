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

using UDPCmd = std::shared_ptr<udp_cmd_t>;
using UDPData = std::shared_ptr<udp_data_t>;

class UDPServer {
public:
    UDPServer();
    ~UDPServer();

    bool init();

    bool revice_data(UDPData& udpdata);

    bool send_cmd();

    void loop();
private:
    std::shared_ptr<SPSCQueue<UDPData>> udp_data_q;
    fd_set read_fds;
    fd_set write_fds;
    int udp_data_sock_fd_ = -1;
    int udp_cmd_sock_fd_ = -1;
};

#endif
