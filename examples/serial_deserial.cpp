#include <iostream>
#include <fstream>

#include "wreath/can/serial.hpp"
#include "wreath/dbc/database.hpp"

struct odrive_heartbeat{
    std::uint32_t axis_error;
    std::uint8_t axis_state;
    std::uint8_t motor_error_flag;
    std::uint8_t encoder_error_flag;
    std::uint8_t controller_error_flag : 7;
    std::uint8_t trajectory_done_flag : 1;
} __attribute__((packed));
static_assert(
    sizeof(odrive_heartbeat) == 8,
    "Error: Sizeof odrive_heartbeat must be 8 bytes\n"
);

int main(int argc, char** argv){
    Wreath::DBC::Message heartbeat_msg;
    Wreath::DBC::Database dbc_db;
    odrive_heartbeat heartbeat;
    std::ifstream dbc_file;
    can_frame frame;
    int can_socket;

    if (argc <= 1){
        std::cerr << "Error: Please provide the path to a DBC file\n";
        return 1;
    }
    dbc_file.open(argv[1]);
    if (!dbc_file.is_open()){
        std::cerr << "Error: Failed to open file at path '" << argv[1] << "'\n";
        return 1;
    }
    if (dbc_db.from_file(dbc_file)){
        std::cerr << "Error: Failed to parse DBC file\n";
        return 1;
    }
    if (dbc_db.get_message_bname("Axis0_Heartbeat", &heartbeat_msg)){
        std::cerr << "Error: Failed to find 'Axis0_Heartbeat' in DBC database\n";
        return 1;
    }

    heartbeat.axis_error = 1;
    heartbeat.axis_state = 2;
    heartbeat.motor_error_flag = 0;
    heartbeat.encoder_error_flag = 1;
    heartbeat.controller_error_flag = 0;
    heartbeat.trajectory_done_flag = 1;

    std::cout << "Original Data: ";
    std::cout << +heartbeat.axis_error << " ";
    std::cout << +heartbeat.axis_state << " ";
    std::cout << +heartbeat.motor_error_flag << " ";
    std::cout << +heartbeat.encoder_error_flag << " ";
    std::cout << +heartbeat.controller_error_flag << " ";
    std::cout << +heartbeat.trajectory_done_flag << " ";
    std::cout << "\n";

    Wreath::CAN::Serial::direct_serial(&frame, &heartbeat, heartbeat_msg);
    std::cout << "Encoded Data: ";
    for (__u8 byte : frame.data) std::cout << +byte << " ";
    std::cout << "\n";
    
    Wreath::CAN::Serial::direct_deserial(&heartbeat, frame);
    std::cout << "Decoded Data: ";
    std::cout << +heartbeat.axis_error << " ";
    std::cout << +heartbeat.axis_state << " ";
    std::cout << +heartbeat.motor_error_flag << " ";
    std::cout << +heartbeat.encoder_error_flag << " ";
    std::cout << +heartbeat.controller_error_flag << " ";
    std::cout << +heartbeat.trajectory_done_flag << " ";
    std::cout << "\n";
}
