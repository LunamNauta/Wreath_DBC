#include <iostream>
#include <fstream>

#include "dbc_parse.hpp"
#include "dbc.hpp"
#include "can.hpp"

struct odrive_heartbeat{
    std::uint32_t axis_error;
    std::uint8_t axis_state;
    std::uint8_t motor_error_flag;
    std::uint8_t encoder_error_flag;
    std::uint8_t controller_error_flag;
    std::uint8_t trajectory_done_flag;
};

int main(int argc, char** argv){
    if (argc < 2){
        std::cerr << "Error: Please supply a path to a .dbc file\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file.is_open()){
        std::cerr << "Error: Invalid file path\n";
        return 1;
    }

    Wreath::DBC::Database dbc_db;
    if (Wreath::DBC::Parse::from_file(file, &dbc_db)){
        std::cerr << "Error: Failed to parse DBC file\n";
        return 1;
    }

    int can_socket = Wreath::CAN::create_socket(CAN_RAW);
    if (can_socket < 0){
        std::cerr << "Error: Failed to create CAN socket\n";
        return -1;
    }
    if (Wreath::CAN::bind_socket(can_socket, "can0") < 0){
        std::cerr << "Error: Failed to bind CAN socket\n";
        return -1;
    }
 
    Wreath::DBC::Message set_controller_mode_msg;
    Wreath::DBC::Message get_adc_voltage_msg;
    Wreath::DBC::Message set_input_vel_msg;
    Wreath::DBC::Message heartbeat_msg;
    if (dbc_db.get_message_bname("Axis2_Set_Controller_Mode", &set_controller_mode_msg)){
        std::cerr << "Error: Failed to find 'Axis2_Set_Controller_Mode' in DBC database\n";
        return 1;
    }
    if (dbc_db.get_message_bname("Axis2_Get_ADC_Voltage", &get_adc_voltage_msg)){
        std::cerr << "Error: Failed to find 'Axis2_Get_ADC_Voltage' in DBC database\n";
        return 1;
    }
    if (dbc_db.get_message_bname("Axis2_Set_Input_Vel", &set_input_vel_msg)){
        std::cerr << "Error: Failed to find 'Axis2_Set_Input_Vel' in DBC database\n";
        return 1;
    }
    if (dbc_db.get_message_bname("Axis2_Heartbeat", &heartbeat_msg)){
        std::cerr << "Error: Failed to find 'Axis2_Heartbeat' in DBC database\n";
        return 1;
    }
 
    can_frame frame;
    odrive_heartbeat heartbeat;
    Wreath::CAN::package_dbc_message(set_controller_mode_msg, 0, &frame, 2, 1);
    Wreath::CAN::write_bus(can_socket, frame);
    Wreath::CAN::package_dbc_message(set_input_vel_msg, 0, &frame, 1.0f, 1.0f);
    Wreath::CAN::write_bus(can_socket, frame);

    while (true){
        //Wreath::CAN::package_dbc_message(get_adc_voltage_msg, CAN_RTR_FLAG, &frame);
        //Wreath::CAN::write_bus(can_socket, frame);
 
        ssize_t bytes_read = Wreath::CAN::read_bus(can_socket, &frame);
        if (bytes_read != sizeof(can_frame)) continue; //TODO: Remove?
        
        Wreath::DBC::Message message;
        if (dbc_db.get_message_bid(frame.can_id, &message)){
            std::cerr << "Warning: Received CAN message that is not in DBC database\n";
            continue;
        }
        if (message.id != heartbeat_msg.id) continue;

        Wreath::CAN::unpackage_dbc_message(
            message, &frame, 
            &heartbeat.axis_error, 
            &heartbeat.axis_state,
            &heartbeat.motor_error_flag,
            &heartbeat.encoder_error_flag,
            &heartbeat.controller_error_flag,
            &heartbeat.trajectory_done_flag
        );

        std::cout << heartbeat_msg.signals[0].get_value_str(heartbeat.axis_error) << " ";
        std::cout << heartbeat_msg.signals[1].get_value_str(heartbeat.axis_state) << " ";
        std::cout << heartbeat.motor_error_flag << " ";
        std::cout << heartbeat.encoder_error_flag << " ";
        std::cout << heartbeat.controller_error_flag << " ";
        std::cout << heartbeat.trajectory_done_flag << " ";
        std::cout << "\n";
    }

    Wreath::CAN::close_socket(can_socket);
}
