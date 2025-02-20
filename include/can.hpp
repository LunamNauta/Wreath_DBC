#ifndef WREATH_CAN_HEADER
#define WREATH_CAN_HEADER

#include <iostream>
#include <cstring>
#include <cstdarg>
#include <climits>
#include <bit>

#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

#include "dbc.hpp"

namespace Wreath{
namespace CAN{

//---------------------------------------------------------------------------------------------------------

int create_socket(int protocol){
    return socket(PF_CAN, SOCK_RAW, protocol);
}
int close_socket(int socket){
    return close(socket);
}
int bind_socket(int socket, const char* can_id, sockaddr_can* out_addr = nullptr){
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
ssize_t write_bus(int socket, const can_frame& frame){
    return write(socket, &frame, sizeof(can_frame));
}

//---------------------------------------------------------------------------------------------------------

void memcpy_bits(__u8* dest, const __u8* src, std::size_t sbyte, std::size_t sbit, std::size_t blen){
    if (sbit == 0){
        std::memcpy(dest + sbyte, src, blen / 8);
        if (blen % 8) dest[sbyte + blen / 8] ^= src[blen / 8] & ((1 << blen % 8) - 1);
        return;
    }

    size_t bit_idx = 0;
    size_t dst_byte_idx = sbyte;
    size_t dst_bit_idx = sbit;
    
    while (bit_idx < blen) {
        size_t src_byte_idx = bit_idx / 8;
        size_t src_bit_idx = bit_idx % 8;
        
        __u8 src_bit = (src[src_byte_idx] >> src_bit_idx) & 1;
        
        dest[dst_byte_idx] &= ~(1 << dst_bit_idx);
        dest[dst_byte_idx] |= (src_bit << dst_bit_idx);
        
        bit_idx++;
        dst_bit_idx++;
        
        if (dst_bit_idx == 8) {
            dst_bit_idx = 0;
            dst_byte_idx++;
        }
    }
}
void reverse_memcpy_bits(__u8* dest, const __u8* src, std::size_t sbyte, std::size_t sbit, std::size_t blen){
    if (sbit == 0){
        std::memcpy(dest, src + sbyte, blen / 8);
        if (blen % 8) dest[blen / 8] ^= (src[sbyte + blen / 8] & ((1 << (blen % 8)) - 1));
        return;
    }

    size_t bit_idx = 0;
    size_t src_byte_idx = sbyte;
    size_t src_bit_idx = sbit;
    
    while (bit_idx < blen) {
        size_t dst_byte_idx = bit_idx / 8;
        size_t dst_bit_idx = bit_idx % 8;
        
        __u8 src_bit = (src[src_byte_idx] >> src_bit_idx) & 1;
        
        dest[dst_byte_idx] &= ~(1 << dst_bit_idx); // Clear the bit
        dest[dst_byte_idx] |= (src_bit << dst_bit_idx); // Set the bit
        
        bit_idx++;
        src_bit_idx++;
        
        if (src_bit_idx == 8) {
            src_bit_idx = 0;
            src_byte_idx++;
        }
    }
}
int package_dbc_message(const DBC::Message& message, int can_flags, can_frame* out_frame, ...){
    if (std::endian::native != std::endian::little && std::endian::native != std::endian::big){
        std::cerr << "Warning (Package_CAN_Message): Cannot determine endianness. Package may be malformed\n";
    }
    out_frame->can_id = message.id | can_flags;
    out_frame->len = message.length;
    va_list args;
    va_start(args, out_frame);
    for (std::size_t a = 0; a < message.signals.size(); a++){
        if (message.signals[a].is_single_float){
            if (sizeof(float) != 4 || CHAR_BIT != 8){
                std::cerr << "Error (Package_CAN_Message): Failed to package CAN message. 'float' is not IEEE-754 32-bit float\n";
                return 1;
            }
            float val = va_arg(args, double);
            std::size_t sbyte = message.signals[a].bit_start / 8;
            std::size_t sbit = message.signals[a].bit_start % 8;
            std::size_t blen = message.signals[a].bit_length;
            memcpy_bits(out_frame->data, (__u8*)&val, sbyte, sbit, blen);
        } else if (message.signals[a].is_double_float){
            if (sizeof(double) != 8 || CHAR_BIT != 8){
                std::cerr << "Error (Package_CAN_Message): Failed to package CAN message. 'double' is not IEEE-754 64-bit float\n";
                return 1;
            }
            double val = va_arg(args, double);
            std::size_t sbyte = message.signals[a].bit_start / 8;
            std::size_t sbit = message.signals[a].bit_start % 8;
            std::size_t blen = message.signals[a].bit_length;
            memcpy_bits(out_frame->data, (__u8*)&val, sbyte, sbit, blen);
        }
        else if (message.signals[a].is_signed){
            std::intmax_t val = va_arg(args, std::intmax_t);
            if (message.signals[a].is_little_endian && std::endian::native == std::endian::big || !message.signals[a].is_little_endian && std::endian::native == std::endian::little){
                val = std::byteswap(val);
            }
            std::size_t sbyte = message.signals[a].bit_start / 8;
            std::size_t sbit = message.signals[a].bit_start % 8;
            std::size_t blen = message.signals[a].bit_length;
            memcpy_bits(out_frame->data, (__u8*)&val, sbyte, sbit, blen);
        } else{
            std::uintmax_t val = va_arg(args, std::uintmax_t);
            if (message.signals[a].is_little_endian && std::endian::native == std::endian::big || !message.signals[a].is_little_endian && std::endian::native == std::endian::little){
                val = std::byteswap(val);
            }
            std::size_t sbyte = message.signals[a].bit_start / 8;
            std::size_t sbit = message.signals[a].bit_start % 8;
            std::size_t blen = message.signals[a].bit_length;
            memcpy_bits(out_frame->data, (__u8*)&val, sbyte, sbit, blen);
        }
    }
    return 0;
}
int unpackage_dbc_message(const DBC::Message& message, const can_frame* frame, ...){
    if (std::endian::native != std::endian::little && std::endian::native != std::endian::big){
        std::cerr << "Warning (Unpackage_CAN_Message): Cannot determine endianness. Package may be malformed\n";
    }
    va_list args;
    va_start(args, frame);
    for (std::size_t a = 0; a < message.signals.size(); a++){
        if (message.signals[a].is_single_float){
            if (sizeof(float) != 4 || CHAR_BIT != 8){
                std::cerr << "Error (Unpackage_CAN_Message): Failed to unpackage CAN message. 'float' is not IEEE-754 32-bit float\n";
                return 1;
            }
            float* val = va_arg(args, float*);
            std::size_t sbyte = message.signals[a].bit_start / 8;
            std::size_t sbit = message.signals[a].bit_start % 8;
            std::size_t blen = message.signals[a].bit_length;
            reverse_memcpy_bits((__u8*)&val, frame->data, sbyte, sbit, blen);
        } else if (message.signals[a].is_double_float){
            if (sizeof(double) != 8 || CHAR_BIT != 8){
                std::cerr << "Error (Unpackage_CAN_Message): Failed to unpackage CAN message. 'double' is not IEEE-754 64-bit float\n";
                return 1;
            }
            double* val = va_arg(args, double*);
            std::size_t sbyte = message.signals[a].bit_start / 8;
            std::size_t sbit = message.signals[a].bit_start % 8;
            std::size_t blen = message.signals[a].bit_length;
            reverse_memcpy_bits((__u8*)&val, frame->data, sbyte, sbit, blen);
        } else if (message.signals[a].is_signed){
            std::intmax_t* val = va_arg(args, std::intmax_t*);
            std::size_t sbyte = message.signals[a].bit_start / 8;
            std::size_t sbit = message.signals[a].bit_start % 8;
            std::size_t blen = message.signals[a].bit_length;
            reverse_memcpy_bits((__u8*)&val, frame->data, sbyte, sbit, blen);
            if (message.signals[a].is_little_endian && std::endian::native == std::endian::big || !message.signals[a].is_little_endian && std::endian::native == std::endian::little){
                *val = std::byteswap(*val);
            }
        } else{
            std::uintmax_t* val = va_arg(args, std::uintmax_t*);
            std::size_t sbyte = message.signals[a].bit_start / 8;
            std::size_t sbit = message.signals[a].bit_start % 8;
            std::size_t blen = message.signals[a].bit_length;
            reverse_memcpy_bits((__u8*)&val, frame->data, sbyte, sbit, blen);
            if (message.signals[a].is_little_endian && std::endian::native == std::endian::big || !message.signals[a].is_little_endian && std::endian::native == std::endian::little){
                *val = std::byteswap(*val);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------

}
}

#endif
