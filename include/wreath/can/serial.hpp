#ifndef WREATH_CAN_SERIALIZATION_HEADER
#define WREATH_CAN_SERIALIZATION_HEADER

#include <cstring>

#include <linux/can/raw.h>

namespace Wreath{
namespace CAN{
namespace Serial{

//Warning: Incredibly easy to screw up while using this. 
//Only use this if you know the exact format of data on your platform
void direct_deserial(void* dest, const can_frame& src);
void direct_serial(can_frame* dest, void* src, std::size_t len);

}
}
}

#endif
