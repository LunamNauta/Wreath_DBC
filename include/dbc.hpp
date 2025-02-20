#ifndef WREATH_DBC_HEADER
#define WREATH_DBC_HEADER

#include <algorithm>
#include <string>
#include <vector>

#include <iostream> //TODO: Remove

namespace Wreath{
namespace DBC{

struct Signal{
    std::string name;
    std::size_t bit_start;
    std::size_t bit_length;
    bool is_little_endian;
    bool is_signed;
    bool is_single_float;
    bool is_double_float;
    float factor;
    float offset;
    float min;
    float max;
    std::string unit;
    std::vector<std::string> receivers;
    std::vector<std::pair<std::size_t, std::string>> value_enum;

    void set_value_enum(const std::vector<std::pair<std::size_t, std::string>>& val){
        value_enum = val;
    }
    std::string get_value_str(std::size_t val){
        std::vector<std::pair<std::size_t, std::string>>::const_iterator it = std::find_if(value_enum.begin(), value_enum.end(), [&val](const std::pair<std::size_t, std::string>& desc){return desc.first == val;});
        if (it == value_enum.end()) return "";
        return it->second;
    }
};

struct Message{
    std::size_t id;
    std::string name;
    std::size_t length;
    std::string sender;
    std::vector<Signal> signals;

    void add_signal(const Signal& signal){
        std::vector<Signal>::const_iterator it = std::upper_bound(signals.begin(), signals.end(), signal, [](const Signal& lhs, const Signal& rhs){return lhs.bit_start < rhs.bit_start;});
        signals.insert(it, signal);
    }
    int get_signal_bname(const std::string& name, Signal* out_signal) const{
        std::vector<Signal>::const_iterator it = std::find_if(signals.begin(), signals.end(), [&name](const Signal& signal){return signal.name == name;});
        if (it == signals.end()) return 1;
        *out_signal = *it;
        return 0;
    }
    int get_signal_bname(const std::string& name, Signal** out_signal){
        std::vector<Signal>::iterator it = std::find_if(signals.begin(), signals.end(), [&name](const Signal& signal){return signal.name == name;});
        if (it == signals.end()) return 1;
        *out_signal = &*it;
        return 0;
    }
};

struct Val_Decl{
    std::size_t object_id;
    std::string signal_name;
    std::vector<std::pair<std::size_t, std::string>> value_enum;
};

struct Database{
    std::string version;
    std::vector<std::string> nodes;
    std::vector<Message> objects;

    void add_message(const Message& object){
        std::vector<Message>::const_iterator it = std::upper_bound(objects.begin(), objects.end(), object, [](const Message& lhs, const Message& rhs){return lhs.id < rhs.id;});
        objects.insert(it, object);
    }
    int get_message_bid(std::size_t id, Message* out_message) const{
        //std::vector<Message>::const_iterator it = std::lowed_bound(objects.begin(), objects.end(), Message{}, [&id](const Message& lhs, const Message&){return lhs.id < id;});
        auto it = std::find_if(objects.begin(), objects.end(), [&id](const Message& message){return message.id == id;});
        if (it == objects.end()) return 1;
        if (it->id != id) return 1;
        *out_message = *it;
        return 0;
    }
    int get_message_bid(std::size_t id, Message** out_message){
        std::vector<Message>::iterator it = std::lower_bound(objects.begin(), objects.end(), Message{}, [&id](const Message& lhs, const Message&){return lhs.id < id;});
        if (it == objects.end()) return 1;
        if (it->id != id) return 1;
        *out_message = &*it;
        return 0;
    }
    int get_message_bname(std::string name, Message* out_message) const{
        std::vector<Message>::const_iterator it = std::find_if(objects.begin(), objects.end(), [&name](const Message& lhs){return lhs.name == name;});
        if (it == objects.end()) return 1;
        *out_message = *it;
        return 0;
    }
    int get_message_bname(std::string name, Message** out_message){
        std::vector<Message>::iterator it = std::find_if(objects.begin(), objects.end(), [&name](const Message& lhs){return lhs.name == name;});
        if (it == objects.end()) return 1;
        *out_message = &*it;
        return 0;
    }
};

}
}

#endif
