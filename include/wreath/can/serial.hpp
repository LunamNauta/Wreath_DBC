#ifndef WREATH_CAN_SERIALIZATION_HEADER
#define WREATH_CAN_SERIALIZATION_HEADER

#include <cstring>

#include <linux/can/raw.h>

#include "wreath/dbc/database.hpp"

namespace Wreath{
namespace CAN{
namespace Serial{

//Warning: Incredibly easy to screw up while using this. 
//Only use this if you know the exact format of data on your platform
void direct_deserial(void* dest, const can_frame& src);
void direct_serial(can_frame* dest, void* src, const DBC::Message& message);
void direct_request_serial(can_frame* dest, void* src, const DBC::Message& message);

}
}
}

#endif
