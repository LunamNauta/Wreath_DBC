#include "wreath/can/serial.hpp"

namespace Wreath{
namespace CAN{
namespace Serial{

void direct_deserial(void* dest, const can_frame& src){
    std::memcpy(dest, src.data, src.len);
}
void direct_serial(can_frame* dest, void* src, const DBC::Message& message){
    std::memcpy(dest->data, src, message.length);
    dest->len = message.length;
    dest->can_id = message.id;
}

void direct_request_serial(can_frame* dest, void* src, const DBC::Message& message){
    dest->can_id = message.id | CAN_RTR_FLAG;
    if (src){
        direct_serial(dest, src, message);
        return;
    }
    dest->len = 0;
}

}
}
}
