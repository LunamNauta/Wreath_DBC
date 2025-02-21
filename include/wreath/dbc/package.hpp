#ifndef WREATH_DBC_PACKAGE_HEADER
#define WREATH_DBC_PACKAGE_HEADER

#include <linux/can/raw.h>

#include "wreath/dbc/static_checks.hpp"
#include "wreath/dbc/database.hpp"

namespace Wreath{
namespace DBC{
namespace Package{

//---------------------------------------------------------------------------------------------------------

void memcpy_bits(__u8* dest, const __u8* src, std::size_t sbyte, std::size_t sbit, std::size_t blen);
void reverse_memcpy_bits(__u8* dest, const __u8* src, std::size_t sbyte, std::size_t sbit, std::size_t blen);

//---------------------------------------------------------------------------------------------------------

int package_dbc_message(const Message& message, int can_flags, can_frame* out_frame, ...);
int unpackage_dbc_message(const Message& message, const can_frame* frame, ...);

//---------------------------------------------------------------------------------------------------------

}
}
}

#endif
