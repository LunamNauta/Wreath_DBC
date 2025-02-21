#ifndef WREATH_CAN_HEADER
#define WREATH_CAN_HEADER

#include <linux/can/raw.h>
#include <unistd.h>

namespace Wreath{
namespace CAN{

//---------------------------------------------------------------------------------------------------------

int create_socket(int protocol);
int close_socket(int socket);
int bind_socket(int socket, const char* can_id, sockaddr_can* out_addr = nullptr);

//---------------------------------------------------------------------------------------------------------

ssize_t read_bus(int socket, can_frame* out_frame);
ssize_t write_bus(int socket, can_frame frame);

//---------------------------------------------------------------------------------------------------------

}
}

#endif
