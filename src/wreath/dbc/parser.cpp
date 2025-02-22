#include <algorithm>
#include <iostream>
#include <string>

#include "wreath/dbc/parser.hpp"

namespace Wreath{
namespace DBC{
namespace Parser{

//---------------------------------------------------------------------------------------------------------

std::string_view::const_iterator absorb_spaces(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end){
    return std::find_if(beg, end, [](char c){return !std::isspace(c);});
}
std::string_view::const_iterator absorb_non_spaces(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end){
    return std::find_if(beg, end, [](char c){return std::isspace(c);});
}
std::string_view::const_iterator absorb_unsigned(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end){
    return std::find_if(beg, end, [](char c){return !std::isdigit(c);});
}
std::string_view::const_iterator absorb_signed(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end){
    std::string_view::const_iterator it = beg;
    if (*it == '+' || *it == '-') it++;
    return std::find_if(it, end, [](char c){return !std::isdigit(c);});
}
std::string_view::const_iterator absorb_float(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end){
    std::string_view::const_iterator it = absorb_signed(beg, end);
    if (*it != '.') return it;
    it = absorb_unsigned(++it, end);
    return it;
}
std::string_view::const_iterator absorb_until(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end, char val){
    return std::find_if(beg, end, [&val](char c){return c == val;});
}

//---------------------------------------------------------------------------------------------------------

#define DBC_ParError_Null(type, line, field){std::cerr << "Error (Wreath::DBC::Parse, " << type << ", Line #" << line << "): Field '" << field << "' has no length\n"; return 1;}
#define DBC_ParError_Unex(type, line, expec, val){std::cerr << "Error (Wreath::DBC::Parse, " << type << ", Line #" << line << "): Expected '" << expec << "', found '" << val << "'\n"; return 1;}
#define DBC_ParError_Other(type, line, error){std::cerr << "Error (Wreath::DBC::Parse, " << type << ", Line #" << line << "): " << error << "\n"; return 1;}

int parse_bo(const std::string_view& line, std::size_t line_number, Message* out_message){
    std::string_view::const_iterator it1, it2, it3;

    it1 = absorb_spaces(line.begin(), line.end());
    it2 = absorb_non_spaces(it1, line.end());
    if (it1 == it2) return 2;
    if (std::string(it1, it2) != "BO_") return 2;
    if (*(it1 = it2) != ' ') return 2;

    it1 = absorb_spaces(it1, line.end());
    it2 = absorb_unsigned(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("BO_", line_number, "id");
    out_message->id = std::stoull(std::string(it1, it2));

    it1 = absorb_spaces(it2, line.end());
    it2 = absorb_until(it1, line.end(), ':');
    it3 = absorb_non_spaces(it1, line.end());
    if (*it2 != ':') DBC_ParError_Unex("BO_", line_number, ":", *it2);
    if (it1 == std::min(it2, it3)) DBC_ParError_Null("BO_", line_number, "name");
    out_message->name = std::string(it1, std::min(it2, it3));

    it1 = absorb_spaces(it2+1, line.end());
    it2 = absorb_unsigned(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("BO_", line_number, "length");
    out_message->length = std::stoull(std::string(it1, it2));

    it1 = absorb_spaces(it2, line.end());
    it2 = absorb_non_spaces(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("BO_", line_number, "sender");
    out_message->sender = std::string(it1, it2);

    return 0;
}
int parse_sg(const std::string_view& line, std::size_t line_number, Signal* output){
    std::string_view::const_iterator it1, it2, it3;

    it1 = absorb_spaces(line.begin(), line.end());
    it2 = absorb_non_spaces(it1, line.end());
    if (it1 == it2) return 2;
    if (std::string(it1, it2) != "SG_") return 2;
    if (*(it1 = it2) != ' ') return 2;

    it1 = absorb_spaces(it1, line.end());
    it2 = absorb_until(it1, line.end(), ':');
    it3 = absorb_non_spaces(it1, line.end());
    if (*it2 != ':') DBC_ParError_Unex("SG_", line_number, ":", *it2);
    if (it1 == std::min(it2, it3)) DBC_ParError_Null("SG_", line_number, "name");
    output->name = std::string(it1, std::min(it2, it3));

    it1 = absorb_spaces(it2+1, line.end());
    it2 = absorb_unsigned(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "bit_start");
    output->bit_start = std::stoull(std::string(it1, it2));

    if (*(it1 = it2) != '|') DBC_ParError_Unex("SG_", line_number, "|", *it1);
    it2 = absorb_unsigned(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "bit_length");
    output->bit_length = std::stoull(std::string(it1, it2));

    if (*(it1 = it2) != '@') DBC_ParError_Unex("SG_", line_number, "@", *it1);
    if (*(++it1) != '1' && *it1 != '0') DBC_ParError_Unex("SG_", line_number, "0|1", *it1);
    output->is_little_endian = *it1 == '1';

    if (*(++it1) != '+' && *it1 != '-') DBC_ParError_Unex("SG_", line_number, "+|-", *it1);
    output->is_signed = *it1 == '-';

    it1 = absorb_spaces(++it1, line.end());
    if (*it1 != '(') DBC_ParError_Unex("SG_", line_number, "(", *it1);
    it2 = absorb_float(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "factor");
    output->factor = std::stof(std::string(it1, it2));
    if (*(it1 = it2) != ',') DBC_ParError_Unex("SG_", line_number, ",", *it1);
    it2 = absorb_float(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "offset");
    output->offset = std::stof(std::string(it1, it2));
    if (*(it1 = it2) != ')') DBC_ParError_Unex("SG_", line_number, ")", *it1);

    it1 = absorb_spaces(++it1, line.end());
    if (*it1 != '[') DBC_ParError_Unex("SG_", line_number, "[", *it1);
    it2 = absorb_unsigned(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "min");
    output->min = std::stof(std::string(it1, it2));
    if (*(it1 = it2) != '|') DBC_ParError_Unex("SG_", line_number, "|", *it1);
    it2 = absorb_unsigned(++it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "max");
    output->max = std::stof(std::string(it1, it2));
    if (*(it1 = it2) != ']') DBC_ParError_Unex("SG_", line_number, "]", *it1);

    it1 = absorb_spaces(++it1, line.end());
    if (*it1 != '\"') DBC_ParError_Unex("SG_", line_number, "\"", *it1);
    it2 = absorb_until(++it1, line.end(), '\"');
    if (*it2 != '\"') DBC_ParError_Unex("SG_", line_number, "\"", *it1)
    if (it1 != it2) output->unit = std::string(it1, it2);

    it1 = absorb_spaces(it2, line.end());
    it2 = absorb_non_spaces(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("SG_", line_number, "receivers");
    output->receivers.push_back(std::string(it1, it2));
    while (it1 != line.end()){
        it1 = absorb_spaces(it2, line.end());
        it2 = absorb_non_spaces(it1, line.end());
        if (it1 == it2) break;
        output->receivers.push_back(std::string(it1, it2));
    }

    return 0;
}
int parse_val(const std::string_view& line, std::size_t line_number, Val_Decl* output){
    std::string_view::const_iterator it1, it2;

    it1 = absorb_spaces(line.begin(), line.end());
    it2 = absorb_non_spaces(it1, line.end());
    if (it1 == it2) return 2;
    if (std::string(it1, it2) != "VAL_") return 2;
    if (*(it1 = it2) != ' ') return 2;

    it1 = absorb_spaces(it1, line.end());
    it2 = absorb_unsigned(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("VAL_", line_number, "object_id");
    output->object_id = std::stoull(std::string(it1, it2));

    it1 = absorb_spaces(it2, line.end());
    it2 = absorb_non_spaces(it1, line.end());
    if (it1 == it2) DBC_ParError_Null("VAL_", line_number, "signal_name");
    output->signal_name = std::string(it1, it2);

    while (it1 != line.end()){
        it1 = absorb_spaces(it2+1, line.end());
        it2 = absorb_unsigned(it1, line.end());
        if (it1 == it2) break;
        std::size_t val = std::stoull(std::string(it1, it2));

        it1 = absorb_spaces(it2, line.end());
        if (*it1 != '\"') break;
        it2 = absorb_until(++it1, line.end(), '\"');
        if (it1 == it2) break;
        if (*it2 != '\"') break;
        output->value_enum.push_back({val, std::string(it1, it2)});
    }
    if (!output->value_enum.size()) return 1;

    return 0;
}

//---------------------------------------------------------------------------------------------------------

}
}
}
