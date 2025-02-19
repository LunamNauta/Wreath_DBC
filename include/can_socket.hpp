#ifndef WREATH_CAN_SOCKET_HEADER
#define WREATH_CAN_SOCKET_HEADER

#include <cstring>
#include <string>

#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

void Clear_CAN_Bus(int can_socket){
    can_frame frame;
    while (read(can_socket, &frame, sizeof(frame)) == sizeof(can_frame));
}
ssize_t Read_CAN_Bus(int can_socket, can_frame* out_frame){
    return read(can_socket, out_frame, sizeof(can_frame));
}
ssize_t Write_CAN_Bus(int can_socket, const can_frame& in_frame){
    return write(can_socket, &in_frame, sizeof(can_frame));
}

int Create_CAN_Socket(){
    return socket(PF_CAN, SOCK_RAW, CAN_RAW);
}
int Bind_CAN_Socket(int can_socket, const std::string& can_id, sockaddr_can* out_addr = nullptr){
    sockaddr_can addr;
    ifreq if_req;

    std::strcpy(if_req.ifr_name, can_id.c_str());
    ioctl(can_socket, SIOCGIFINDEX, &if_req);

    std::memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = if_req.ifr_ifindex;

    if (out_addr) *out_addr = addr;
    return bind(can_socket, (sockaddr*)&addr, sizeof(addr));
}
int Close_CAN_Socket(int can_socket){
    return close(can_socket);
}

#endif
