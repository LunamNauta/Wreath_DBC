#ifndef WREATH_DBC_PARSE_HEADER
#define WREATH_DBC_PARSE_HEADER

#include <fstream>

#include "wreath/dbc/database.hpp"

namespace Wreath{
namespace DBC{
namespace Parse{

//---------------------------------------------------------------------------------------------------------

std::string_view::const_iterator absorb_spaces(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end);
std::string_view::const_iterator absorb_non_spaces(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end);
std::string_view::const_iterator absorb_unsigned(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end);
std::string_view::const_iterator absorb_signed(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end);
std::string_view::const_iterator absorb_float(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end);
std::string_view::const_iterator absorb_until(const std::string_view::const_iterator& beg, const std::string_view::const_iterator& end, char val);

//---------------------------------------------------------------------------------------------------------

#define DBC_ParError_Null(type, line, field){std::cerr << "Error (Wreath::DBC::Parse, " << type << ", Line #" << line << "): Field '" << field << "' has no length\n"; return 1;}
#define DBC_ParError_Unex(type, line, expec, val){std::cerr << "Error (Wreath::DBC::Parse, " << type << ", Line #" << line << "): Expected '" << expec << "', found '" << val << "'\n"; return 1;}
#define DBC_ParError_Other(type, line, error){std::cerr << "Error (Wreath::DBC::Parse, " << type << ", Line #" << line << "): " << error << "\n"; return 1;}

int parse_bo(const std::string_view& line, std::size_t line_number, Message* out_message);
int parse_sg(const std::string_view& line, std::size_t line_number, Signal* output);
int parse_val(const std::string_view& line, std::size_t line_number, Val_Decl* output);

//---------------------------------------------------------------------------------------------------------

int from_file(std::ifstream& dbc_file, Database* out_database);

//---------------------------------------------------------------------------------------------------------

}
}
}

#endif
