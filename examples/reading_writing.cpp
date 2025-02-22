#include <iostream>
#include <fstream>

#include "wreath/dbc/static_checks.hpp"
#include "wreath/dbc/database.hpp"
#include "wreath/can/serial.hpp"
#include "wreath/can/can.hpp"

struct odrive_adc_voltage{
    float adc_voltage;
} __attribute__((packed));
static_assert(
    sizeof(odrive_adc_voltage) == 4,
    "Error: Sizeof 'odrive_adc_voltage' must be 4 bytes\n"
);

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
    "Error: Sizeof 'odrive_heartbeat' must be 8 bytes\n"
);

int main(int argc, char** argv){
    Wreath::DBC::Message adc_voltage_msg;
    Wreath::DBC::Message heartbeat_msg;
    Wreath::DBC::Database dbc_db;
    odrive_adc_voltage adc_voltage;
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
    if (dbc_db.get_message_bname("Axis2_Get_ADC_Voltage", &adc_voltage_msg)){
        std::cerr << "Error: Failed to find 'Axis2_Get_ADC_Voltage' in DBC database\n";
        return 1;
    }
    if (dbc_db.get_message_bname("Axis2_Heartbeat", &heartbeat_msg)){
        std::cerr << "Error: Failed to find 'Axis2_Heartbeat' in DBC database\n";
        return 1;
    }
    if ((can_socket = Wreath::CAN::create_socket(CAN_RAW)) < 0){
        std::cerr << "Error: Failed to create CAN socket\n";
        return 1;
    }
    if (Wreath::CAN::bind_socket(can_socket, "can0") < 0){
        std::cerr << "Error: Failed to bind CAN socket to 'can0'\n";
        return 1;
    }

    while (true){
        ssize_t bytes_read = Wreath::CAN::read_bus(can_socket, &frame);
        if (bytes_read != sizeof(frame)) continue;
        if (frame.can_id != heartbeat_msg.id) continue;

        Wreath::CAN::Serial::direct_deserial(&heartbeat, frame);
        std::cout << "Axis0_Heartbeat: ";
        std::cout << "    Axis_Error: "            << heartbeat.axis_error << " ";
        std::cout << "    Axis_State: "            << heartbeat.axis_state << " ";
        std::cout << "    Motor_Error_Flag: "      << heartbeat.motor_error_flag << " ";
        std::cout << "    Encoder_Error_Flag: "    << heartbeat.encoder_error_flag << " ";
        std::cout << "    Controller_Error_Flag: " << heartbeat.controller_error_flag << " ";
        std::cout << "    Trajectory_Done_Flag: "  << heartbeat.trajectory_done_flag << " ";
        std::cout << "\n";

        Wreath::CAN::Serial::direct_request_serial(&frame, nullptr, adc_voltage_msg);
        Wreath::CAN::write_bus(can_socket, frame);
    }

    Wreath::CAN::close_socket(can_socket);
}
