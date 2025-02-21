#include <cstring>
#include <cstdarg>
#include <climits>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "wreath/can/can.hpp"

namespace Wreath{
namespace CAN{

//---------------------------------------------------------------------------------------------------------

int create_socket(int protocol){
    return socket(PF_CAN, SOCK_RAW, protocol);
}
int close_socket(int socket){
    return close(socket);
}
int bind_socket(int socket, const char* can_id, sockaddr_can* out_addr){
    sockaddr_can addr{};
    ifreq if_req;

    std::strcpy(if_req.ifr_name, can_id);
    ioctl(socket, SIOCGIFINDEX, &if_req);
    addr.can_ifindex = if_req.ifr_ifindex;
    addr.can_family = AF_CAN;

    if (out_addr) *out_addr = addr;
    return bind(socket, (sockaddr*)&addr, sizeof(addr));
}

//---------------------------------------------------------------------------------------------------------

ssize_t read_bus(int socket, can_frame* out_frame){
    return read(socket, out_frame, sizeof(can_frame));
}
ssize_t write_bus(int socket, can_frame frame){
    return write(socket, &frame, sizeof(can_frame));
}

//---------------------------------------------------------------------------------------------------------

}
}
