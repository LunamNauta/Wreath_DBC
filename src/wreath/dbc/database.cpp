#include <algorithm>
#include <iostream>
#include <fstream>
#include <limits>

#include "wreath/dbc/database.hpp"
#include "wreath/dbc/parser.hpp"

namespace Wreath{
namespace DBC{

//---------------------------------------------------------------------------------------------------------

void Signal::set_value_enum(const std::vector<std::pair<std::size_t, std::string>>& val){
    value_enum = val;
}
std::string Signal::get_value_str(std::size_t val){
    std::vector<std::pair<std::size_t, std::string>>::const_iterator it = std::find_if(value_enum.begin(), value_enum.end(), [&val](const std::pair<std::size_t, std::string>& desc){return desc.first == val;});
    if (it == value_enum.end()) return "";
    return it->second;
}

//---------------------------------------------------------------------------------------------------------

void Message::add_signal(const Signal& signal){
    std::vector<Signal>::const_iterator it = std::upper_bound(signals.begin(), signals.end(), signal, [](const Signal& lhs, const Signal& rhs){return lhs.bit_start < rhs.bit_start;});
    signals.insert(it, signal);
}
int Message::get_signal_bname(const std::string& name, Signal* out_signal) const{
    std::vector<Signal>::const_iterator it = std::find_if(signals.begin(), signals.end(), [&name](const Signal& signal){return signal.name == name;});
    if (it == signals.end()) return 1;
    *out_signal = *it;
    return 0;
}
int Message::get_signal_bname(const std::string& name, Signal** out_signal){
    std::vector<Signal>::iterator it = std::find_if(signals.begin(), signals.end(), [&name](const Signal& signal){return signal.name == name;});
    if (it == signals.end()) return 1;
    *out_signal = &*it;
    return 0;
}

//---------------------------------------------------------------------------------------------------------

int Database::from_file(std::ifstream& dbc_file){
    std::size_t last_message_id = std::numeric_limits<std::size_t>::max();
    Message* message_ref;
    Signal* signal_ref;
    Message message;
    Signal signal;
    Val_Decl val;
    int res = 0;

    std::string line;
    std::size_t line_number = 1;
    while (std::getline(dbc_file, line)){
        message = {};
        if (!(res = Parser::parse_bo(line, line_number, &message))){
            add_message(message);
            last_message_id = message.id;
            goto next_line;
        } else if (res != 2) return res;

        signal = {};
        if (!(res = Parser::parse_sg(line, line_number, &signal))){
            if (last_message_id == std::numeric_limits<std::size_t>::max()) DBC_ParError_Other("SG_", line_number, "SG_ line appears before first BO_ line");
            get_message_bid(last_message_id, &message_ref);
            message_ref->add_signal(signal);
            goto next_line;
        } else if (res != 2) return res;

        val = {};
        if (!(res = Parser::parse_val(line, line_number, &val))){
            if (last_message_id == std::numeric_limits<std::size_t>::max()) DBC_ParError_Other("VAL_", line_number, "VAL_ line appears before first BO_ line");
            get_message_bid(val.object_id, &message_ref);
            if (message_ref->get_signal_bname(val.signal_name, &signal_ref)) DBC_ParError_Other("VAL_", line_number, "VAL_ line references SG_ that has not been defined");
            signal_ref->set_value_enum(val.value_enum);
            goto next_line;
        } else if (res != 2) return res;

        next_line:
        line_number++;
    }
    
    return 0;
}

void Database::add_message(const Message& object){
    std::vector<Message>::const_iterator it = std::upper_bound(objects.begin(), objects.end(), object, [](const Message& lhs, const Message& rhs){return lhs.id < rhs.id;});
    objects.insert(it, object);
}
int Database::get_message_bid(std::size_t id, Message* out_message) const{
    std::vector<Message>::const_iterator it = std::lower_bound(objects.begin(), objects.end(), Message{}, [&id](const Message& lhs, const Message&){return lhs.id < id;});
    if (it == objects.end()) return 1;
    if (it->id != id) return 1;
    *out_message = *it;
    return 0;
}
int Database::get_message_bid(std::size_t id, Message** out_message){
    std::vector<Message>::iterator it = std::lower_bound(objects.begin(), objects.end(), Message{}, [&id](const Message& lhs, const Message&){return lhs.id < id;});
    if (it == objects.end()) return 1;
    if (it->id != id) return 1;
    *out_message = &*it;
    return 0;
}
int Database::get_message_bname(std::string name, Message* out_message) const{
    std::vector<Message>::const_iterator it = std::find_if(objects.begin(), objects.end(), [&name](const Message& lhs){return lhs.name == name;});
    if (it == objects.end()) return 1;
    *out_message = *it;
    return 0;
}
int Database::get_message_bname(std::string name, Message** out_message){
    std::vector<Message>::iterator it = std::find_if(objects.begin(), objects.end(), [&name](const Message& lhs){return lhs.name == name;});
    if (it == objects.end()) return 1;
    *out_message = &*it;
    return 0;
}

//---------------------------------------------------------------------------------------------------------

}
}
