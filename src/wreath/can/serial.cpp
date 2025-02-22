#include "wreath/can/serial.hpp"

namespace Wreath{
namespace CAN{
namespace Serial{

void direct_deserial(void* dest, const can_frame& src){
    std::memcpy(dest, src.data, src.len);
}
void direct_serial(can_frame* dest, void* src, std::size_t len){
    std::memcpy(dest->data, src, len);
    dest->len = len;
}

}
}
}
