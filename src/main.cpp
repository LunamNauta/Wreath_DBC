#include <algorithm>
#include <bit>
#include <cstddef>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>
#include <cstdarg>
#include <cstring>
#include <climits>

struct DBC_Signal{
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
    std::vector<std::pair<std::size_t, std::string>> value_descriptions;

    void Set_Value_Descriptions(const std::vector<std::pair<std::size_t, std::string>>& val){
        value_descriptions = val;
    }
};

struct DBC_Message{
    std::size_t id;
    std::string name;
    std::size_t length;
    std::string sender;
    std::vector<DBC_Signal> signals;

    void Add_Signal(const DBC_Signal& signal){
        std::vector<DBC_Signal>::const_iterator it = std::upper_bound(signals.begin(), signals.end(), signal, [](const DBC_Signal& lhs, const DBC_Signal& rhs){return lhs.bit_start < rhs.bit_start;});
        signals.insert(it, signal);
    }
    DBC_Signal Get_Signal_By_Name(const std::string& name) const{
        std::vector<DBC_Signal>::const_iterator it = std::find_if(signals.begin(), signals.end(), [&name](const DBC_Signal& signal){return signal.name == name;});
        if (it == signals.end()) throw std::invalid_argument("");
        return *it;
    }
    DBC_Signal& Get_Signal_By_Name_Ref(const std::string& name){
        std::vector<DBC_Signal>::iterator it = std::find_if(signals.begin(), signals.end(), [&name](const DBC_Signal& signal){return signal.name == name;});
        if (it == signals.end()) throw std::invalid_argument("");
        return *it;
    }
};

struct DBC_Val_Decl{
    std::size_t object_id;
    std::string signal_name;
    std::vector<std::pair<std::size_t, std::string>> value_descriptions;
};

struct DBC_Database{
    std::string version;
    std::vector<std::string> nodes;
    std::vector<DBC_Message> objects;

    void Add_Message(const DBC_Message& object){
        std::vector<DBC_Message>::const_iterator it = std::upper_bound(objects.begin(), objects.end(), object, [](const DBC_Message& lhs, const DBC_Message& rhs){return lhs.id < rhs.id;});
        objects.insert(it, object);
    }
    DBC_Message Get_Message_By_ID(std::size_t id) const{
        std::vector<DBC_Message>::const_iterator it = std::lower_bound(objects.begin(), objects.end(), DBC_Message{}, [&id](const DBC_Message& lhs, const DBC_Message&){return lhs.id < id;});
        if (it == objects.end()) throw std::invalid_argument("");
        if (it->id != id) throw std::invalid_argument("");
        return *it;
    }
    DBC_Message& Get_Message_By_ID_Ref(std::size_t id){
        std::vector<DBC_Message>::iterator it = std::lower_bound(objects.begin(), objects.end(), DBC_Message{}, [&id](const DBC_Message& lhs, const DBC_Message&){return lhs.id < id;});
        if (it == objects.end()) throw std::invalid_argument("");
        if (it->id != id) throw std::invalid_argument("");
        return *it;
    }
    DBC_Message Get_Message_By_Name(std::string name) const{
        std::vector<DBC_Message>::const_iterator it = std::find_if(objects.begin(), objects.end(), [&name](const DBC_Message& lhs){return lhs.name == name;});
        if (it == objects.end()) throw std::invalid_argument("");
        return *it;
    }
    DBC_Message& Get_Message_By_Name_Ref(std::string name){
        std::vector<DBC_Message>::iterator it = std::find_if(objects.begin(), objects.end(), [&name](const DBC_Message& lhs){return lhs.name == name;});
        if (it == objects.end()) throw std::invalid_argument("");
        return *it;
    }
};

std::string_view::const_iterator Absorb_Spaces(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end){
    return std::find_if(beg, end, [](char c){return !std::isspace(c);});
}
std::string_view::const_iterator Absorb_Non_Spaces(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end){
    return std::find_if(beg, end, [](char c){return std::isspace(c);});
}
std::string_view::const_iterator Absorb_Unsigned(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end){
    return std::find_if(beg, end, [](char c){return !std::isdigit(c);});
}
std::string_view::const_iterator Absorb_Float(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end){
    std::string_view::const_iterator it = Absorb_Unsigned(beg, end);
    if (*it != '.') return it;
    it = Absorb_Unsigned(++it, end);
    return it;
}
std::string_view::const_iterator Absorb_Until(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end, char val){
    return std::find_if(beg, end, [&val](char c){return c == val;});
}

#define DBC_ParError_Null(type, line, field){std::cerr << "Error (DBC_Parse, " << type << ", Line #" << line << "): Field '" << field << "' has no length\n"; return 1;}
#define DBC_ParError_Unex(type, line, expec, val){std::cerr << "Error (DBC_Parse, " << type << ", Line #" << line << "): Expected '" << expec << "', found '" << val << "'"; return 1;}

int Parse_DBC_BO_(const std::string_view& line, std::size_t line_number, DBC_Message* output){
    std::string_view::const_iterator it1, it2, it3;

    it1 = Absorb_Spaces(line.begin(), line.end());
    it2 = Absorb_Non_Spaces(it1, line.end());
    if (it1 == it2) return 1;
    if (std::string(it1, it2) != "BO_") return 1;
    if (*(it1 = it2) != ' ') return 1;

    it1 = Absorb_Spaces(it1, line.end());
    it2 = Absorb_Unsigned(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("BO_", line_number, "id");
    output->id = std::stoull(std::string(it1, it2));

    it1 = Absorb_Spaces(it2, line.end());
    it2 = Absorb_Until(it1, line.end(), ':');
    it3 = Absorb_Non_Spaces(it1, line.end());
    if (*it2 != ':') DBC_ParError_Unex("BO_", line_number, ":", *it2);
    if (it1 == std::min(it2, it3)) DBC_ParError_Null("BO_", line_number, "name");
    output->name = std::string(it1, std::min(it2, it3));

    it1 = Absorb_Spaces(it2+1, line.end());
    it2 = Absorb_Unsigned(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("BO_", line_number, "length");
    output->length = std::stoull(std::string(it1, it2));

    it1 = Absorb_Spaces(it2, line.end());
    it2 = Absorb_Non_Spaces(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("BO_", line_number, "sender");
    output->sender = std::string(it1, it2);

    return 0;
}
int Parse_DBC_SG_(const std::string_view& line, std::size_t line_number, DBC_Signal* output){
    std::string_view::const_iterator it1, it2, it3;

    it1 = Absorb_Spaces(line.begin(), line.end());
    it2 = Absorb_Non_Spaces(it1, line.end());
    if (it1 == it2) return 1;
    if (std::string(it1, it2) != "SG_") return 1;
    if (*(it1 = it2) != ' ') return 1;

    it1 = Absorb_Spaces(it1, line.end());
    it2 = Absorb_Until(it1, line.end(), ':');
    it3 = Absorb_Non_Spaces(it1, line.end());
    if (*it2 != ':') DBC_ParError_Unex("SG_", line_number, ":", *it2);
    if (it1 == std::min(it2, it3)) DBC_ParError_Null("SG_", line_number, "name");
    output->name = std::string(it1, std::min(it2, it3));

    it1 = Absorb_Spaces(it2+1, line.end());
    it2 = Absorb_Unsigned(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "bit_start");
    output->bit_start = std::stoull(std::string(it1, it2));

    if (*(it1 = it2) != '|') DBC_ParError_Unex("SG_", line_number, "|", *it1);
    it2 = Absorb_Unsigned(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "bit_length");
    output->bit_length = std::stoull(std::string(it1, it2));

    if (*(it1 = it2) != '@') DBC_ParError_Unex("SG_", line_number, "@", *it1);
    if (*(++it1) != '1' && *it1 != '0') DBC_ParError_Unex("SG_", line_number, "0|1", *it1);
    output->is_little_endian = *it1 == '1';

    if (*(++it1) != '+' && *it1 != '-') DBC_ParError_Unex("SG_", line_number, "+|-", *it1);
    output->is_signed = *it1 == '-';

    it1 = Absorb_Spaces(++it1, line.end());
    if (*it1 != '(') DBC_ParError_Unex("SG_", line_number, "(", *it1);
    it2 = Absorb_Float(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "factor");
    output->factor = std::stof(std::string(it1, it2));
    if (*(it1 = it2) != ',') DBC_ParError_Unex("SG_", line_number, ",", *it1);
    it2 = Absorb_Float(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "offset");
    output->offset = std::stof(std::string(it1, it2));
    if (*(it1 = it2) != ')') DBC_ParError_Unex("SG_", line_number, ")", *it1);

    it1 = Absorb_Spaces(++it1, line.end());
    if (*it1 != '[') DBC_ParError_Unex("SG_", line_number, "[", *it1);
    it2 = Absorb_Unsigned(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "min");
    output->min = std::stof(std::string(it1, it2));
    if (*(it1 = it2) != '|') DBC_ParError_Unex("SG_", line_number, "|", *it1);
    it2 = Absorb_Unsigned(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "max");
    output->max = std::stof(std::string(it1, it2));
    if (*(it1 = it2) != ']') DBC_ParError_Unex("SG_", line_number, "]", *it1);

    it1 = Absorb_Spaces(++it1, line.end());
    if (*it1 != '\"') DBC_ParError_Unex("SG_", line_number, "\"", *it1);
    it2 = Absorb_Until(++it1, line.end(), '\"');
    if (*it2 != '\"') DBC_ParError_Unex("SG_", line_number, "\"", *it1)
    if (it1 != it2) output->unit = std::string(it1, it2);

    it1 = Absorb_Spaces(it2, line.end());
    it2 = Absorb_Non_Spaces(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "receivers");
    output->receivers.push_back(std::string(it1, it2));
    while (it1 != line.end()){
        it1 = Absorb_Spaces(it2, line.end());
        it2 = Absorb_Non_Spaces(it1, line.end());
        if (it1 == it2) break;
        output->receivers.push_back(std::string(it1, it2));
    }

    return 0;
}
int Parse_DBC_VAL_(const std::string_view& line, std::size_t line_number, DBC_Val_Decl* output){
    std::string_view::const_iterator it1, it2;

    it1 = Absorb_Spaces(line.begin(), line.end());
    it2 = Absorb_Non_Spaces(it1, line.end());
    if (it1 == it2) return 1;
    if (std::string(it1, it2) != "VAL_") return 1;
    if (*(it1 = it2) != ' ') return 1;

    it1 = Absorb_Spaces(it1, line.end());
    it2 = Absorb_Unsigned(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("VAL_", line_number, "object_id");
    output->object_id = std::stoull(std::string(it1, it2));

    it1 = Absorb_Spaces(it2, line.end());
    it2 = Absorb_Non_Spaces(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("VAL_", line_number, "signal_name");
    output->signal_name = std::string(it1, it2);

    while (it1 != line.end()){
        it1 = Absorb_Spaces(it2+1, line.end());
        it2 = Absorb_Unsigned(it1, line.end());
        if (it1 == it2) break;
        std::size_t val = std::stoull(std::string(it1, it2));

        it1 = Absorb_Spaces(it2, line.end());
        if (*it1 != '\"') break;
        it2 = Absorb_Until(++it1, line.end(), '\"');
        if (it1 == it2) break;
        if (*it2 != '\"') break;
        output->value_descriptions.push_back({val, std::string(it1, it2)});
    }
    if (!output->value_descriptions.size()) return 1;

    return 0;
}

DBC_Database Parse_DBC_File(std::ifstream& dbc_file){
    std::size_t last_message_id = std::numeric_limits<std::size_t>::max();
    DBC_Database output;
    DBC_Message message;
    DBC_Signal signal;
    DBC_Val_Decl val;

    std::string line;
    std::size_t line_number = 1;
    while (std::getline(dbc_file, line)){
        message = {};
        if (!Parse_DBC_BO_(line, line_number, &message)){
            output.Add_Message(message);
            last_message_id = message.id;
            goto next_line;
        }

        signal = {};
        if (!Parse_DBC_SG_(line, line_number, &signal)){
            if (last_message_id == std::numeric_limits<std::size_t>::max()) goto next_line;
            output.Get_Message_By_ID_Ref(last_message_id).Add_Signal(signal);
            goto next_line;
        }

        val = {};
        if (!Parse_DBC_VAL_(line, line_number, &val)){
            if (last_message_id == std::numeric_limits<std::size_t>::max()) goto next_line;
            output.Get_Message_By_ID_Ref(val.object_id).Get_Signal_By_Name_Ref(val.signal_name).Set_Value_Descriptions(val.value_descriptions);
            goto next_line;
        }

        next_line:
        line_number++;
    }
    
    return output;
}

int memcpy_bits(std::uint8_t* dest, std::uint8_t* src, std::size_t sbyte, std::size_t sbit, std::size_t blen){
    if (sbit != 0){ // TODO: Make this more efficient. How? Idk. Bytewise copies somewhere?
        for (size_t i = 0; i < blen; ++i) {
            size_t src_bit_index = i;
            size_t src_byte_index = src_bit_index / 8;
            size_t src_bit_pos = src_bit_index % 8;
            uint8_t bit_value = (src[src_byte_index] >> src_bit_pos) & 1;

            size_t dest_bit_index = sbit + i;
            size_t dest_byte_index = sbyte + dest_bit_index / 8;
            size_t dest_bit_pos = dest_bit_index % 8;

            if (bit_value) dest[dest_byte_index] |= (1 << dest_bit_pos);
            else dest[dest_byte_index] &= ~(1 << dest_bit_pos);
        }
        return 0;
    }
    std::memcpy(dest+sbyte, src, blen/8);
    if (blen % 8) dest[sbyte+blen/8] ^= src[blen/8] & ((1 << blen % 8) - 1);
    return 0;
}
int Package_CAN_Message(const DBC_Message& message, std::vector<std::uint8_t>* bytes, ...){
    if (std::endian::native != std::endian::little && std::endian::native != std::endian::big){
        std::cerr << "Warning (Package_CAN_Message): Cannot determine endianness. Package may be malformed\n";
        return 1;
    }
    bytes->resize(message.length);
    va_list args;
    va_start(args, bytes);
    std::size_t current_byte = 0;
    std::size_t current_bit = 0;
    for (std::size_t a = 0; a < message.signals.size(); a++){
        if (message.signals[a].is_single_float){
            if (sizeof(float) != 4 || CHAR_BIT != 8){
                std::cerr << "Error (Package_CAN_Message): Failed to package CAN message. 'float' is not IEEE-754 32-bit float\n";
                return 1;
            }
            float val = va_arg(args, double);
            std::size_t current_byte = message.signals[a].bit_start / 8;
            std::size_t current_bit = message.signals[a].bit_start % 8;
            if (memcpy_bits(bytes->data(), (std::uint8_t*)&val, current_byte, current_bit, message.signals[a].bit_length)){
                std::cerr << "Error (Package_CAN_Message): Failed to package CAN message\n";
                return 1;
            }
        } else if (message.signals[a].is_double_float){
            if (sizeof(double) != 8 || CHAR_BIT != 8){
                std::cerr << "Error (Package_CAN_Message): Failed to package CAN message. 'double' is not IEEE-754 64-bit float\n";
                return 1;
            }
            double val = va_arg(args, double);
            std::size_t current_byte = message.signals[a].bit_start / 8;
            std::size_t current_bit = message.signals[a].bit_start % 8;
            if (memcpy_bits(bytes->data(), (std::uint8_t*)&val, current_byte, current_bit, message.signals[a].bit_length)){
                std::cerr << "Error (Package_CAN_Message): Failed to package CAN message\n";
                return 1;
            }
        }
        else if (message.signals[a].is_signed){
            std::intmax_t val = va_arg(args, std::intmax_t);
            if (message.signals[a].is_little_endian && std::endian::native == std::endian::big || !message.signals[a].is_little_endian && std::endian::native == std::endian::little){
                val = std::byteswap(val);
            }
            std::size_t current_byte = message.signals[a].bit_start / 8;
            std::size_t current_bit = message.signals[a].bit_start % 8;
            if (memcpy_bits(bytes->data(), (std::uint8_t*)&val, current_byte, current_bit, message.signals[a].bit_length)){
                std::cerr << "Error (Package_CAN_Message): Failed to package CAN message\n";
                return 1;
            }
        } else{
            std::uintmax_t val = va_arg(args, std::uintmax_t);
            if (message.signals[a].is_little_endian && std::endian::native == std::endian::big || !message.signals[a].is_little_endian && std::endian::native == std::endian::little){
                val = std::byteswap(val);
            }
            std::size_t current_byte = message.signals[a].bit_start / 8;
            std::size_t current_bit = message.signals[a].bit_start % 8;
            if (memcpy_bits(bytes->data(), (std::uint8_t*)&val, current_byte, current_bit, message.signals[a].bit_length)){
                std::cerr << "Error (Package_CAN_Message): Failed to package CAN message\n";
                return 1;
            }
        }
    }
    return 0;
}

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
    DBC_Database dbc_db = Parse_DBC_File(file);

    DBC_Message message = dbc_db.Get_Message_By_Name("Axis0_Heartbeat");
    std::vector<std::uint8_t> packed_message;
    Package_CAN_Message(message, &packed_message, 1, 2, 1, 1, 1, 1);
    for (std::uint8_t byte : packed_message) std::cout << +byte << "\n";

    /*
    for (const DBC_Message& message : dbc_db.objects){
        std::cout << message.name << " " << message.id << "\n";
        for (const DBC_Signal& signal : message.signals){
            std::cout << "  " << signal.name << " " << signal.bit_start << " " << signal.bit_length << "\n";
            if (!signal.value_descriptions.size()) continue;
            for (const std::pair<std::size_t, std::string>& val_decl : signal.value_descriptions){
                std::cout << "    " << val_decl.first << " \"" << val_decl.second << "\"\n"; 
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    */
}
