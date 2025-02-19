#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>

struct DBC_Signal{
    std::string name;
    std::size_t bit_start;
    std::size_t bit_length;
    bool is_little_endian;
    bool is_signed;
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

struct DBC_Object{
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
    std::vector<DBC_Object> objects;

    void Add_Object(const DBC_Object& object){
        std::vector<DBC_Object>::const_iterator it = std::upper_bound(objects.begin(), objects.end(), object, [](const DBC_Object& lhs, const DBC_Object& rhs){return lhs.id < rhs.id;});
        objects.insert(it, object);
    }
    DBC_Object Get_Object_By_ID(std::size_t id) const{
        std::vector<DBC_Object>::const_iterator it = std::lower_bound(objects.begin(), objects.end(), DBC_Object{}, [&id](const DBC_Object& lhs, const DBC_Object&){return lhs.id < id;});
        if (it == objects.end()) throw std::invalid_argument("");
        if (it->id != id) throw std::invalid_argument("");
        return *it;
    }
    DBC_Object& Get_Object_By_ID_Ref(std::size_t id){
        std::vector<DBC_Object>::iterator it = std::lower_bound(objects.begin(), objects.end(), DBC_Object{}, [&id](const DBC_Object& lhs, const DBC_Object&){return lhs.id < id;});
        if (it == objects.end()) throw std::invalid_argument("");
        if (it->id != id) throw std::invalid_argument("");
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

int Parse_DBC_BO_(const std::string_view& line, std::size_t line_number, DBC_Object* output){
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
    std::size_t last_object_id = std::numeric_limits<std::size_t>::max();
    DBC_Database output;
    DBC_Object object;
    DBC_Signal signal;
    DBC_Val_Decl val;

    std::string line;
    std::size_t line_number = 1;
    while (std::getline(dbc_file, line)){
        object = {};
        if (!Parse_DBC_BO_(line, line_number, &object)){
            output.Add_Object(object);
            last_object_id = object.id;
            goto next_line;
        }

        signal = {};
        if (!Parse_DBC_SG_(line, line_number, &signal)){
            if (last_object_id == std::numeric_limits<std::size_t>::max()) goto next_line;
            output.Get_Object_By_ID_Ref(last_object_id).Add_Signal(signal);
            goto next_line;
        }

        val = {};
        if (!Parse_DBC_VAL_(line, line_number, &val)){
            if (last_object_id == std::numeric_limits<std::size_t>::max()) goto next_line;
            output.Get_Object_By_ID_Ref(val.object_id).Get_Signal_By_Name_Ref(val.signal_name).Set_Value_Descriptions(val.value_descriptions);
            goto next_line;
        }

        next_line:
        line_number++;
    }
    
    return output;
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

    for (const DBC_Object& object : dbc_db.objects){
        std::cout << object.name << " " << object.id << "\n";
        for (const DBC_Signal& signal : object.signals){
            std::cout << "  " << signal.name << " " << signal.bit_start << " " << signal.bit_length << "\n";
            if (!signal.value_descriptions.size()) continue;
            for (const std::pair<std::size_t, std::string>& val_decl : signal.value_descriptions){
                std::cout << "    " << val_decl.first << " \"" << val_decl.second << "\"\n"; 
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}
