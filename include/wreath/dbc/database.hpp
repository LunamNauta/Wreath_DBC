#ifndef WREATH_DBC_HEADER
#define WREATH_DBC_HEADER

#include <string>
#include <vector>

namespace Wreath{
namespace DBC{

struct Signal{
    std::vector<std::pair<std::size_t, std::string>> value_enum;
    std::vector<std::string> receivers;
    std::string name;
    std::string unit;
    std::size_t bit_start;
    std::size_t bit_length;
    float factor;
    float offset;
    float min;
    float max;
    bool is_little_endian;
    bool is_signed;
    bool is_single_float;
    bool is_double_float;

    void set_value_enum(const std::vector<std::pair<std::size_t, std::string>>& val);
    std::string get_value_str(std::size_t val);
};

struct Message{
    std::size_t id;
    std::string name;
    std::size_t length;
    std::string sender;
    std::vector<Signal> signals;

    void add_signal(const Signal& signal);
    int get_signal_bname(const std::string& name, Signal* out_signal) const;
    int get_signal_bname(const std::string& name, Signal** out_signal);
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

    void add_message(const Message& object);
    int get_message_bid(std::size_t id, Message* out_message) const;
    int get_message_bid(std::size_t id, Message** out_message);
    int get_message_bname(std::string name, Message* out_message) const;
    int get_message_bname(std::string name, Message** out_message);
};

}
}

#endif
