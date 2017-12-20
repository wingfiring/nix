#include <xirang2/string_algo/json.h>

namespace xirang2{ namespace json{
    namespace{
        const char* K_hex_table= "0123456789abcdef";
    }
    string escape_buffer(const buffer<byte>& buf){
        
        string_builder sb;
        sb.push_back('"');
        
        for(auto b : buf){
            uint8_t ch = static_cast<unsigned char>(b);
            if (ch < 0x20){
                sb.append(literal("\\u00"));
                sb.push_back(K_hex_table[(ch >> 4)]);
                sb.push_back(K_hex_table[(ch & 0x0f)]);
            }
            else {
                if(ch == '\\' || ch == '"'){
                    sb.push_back('\\');
                }
                sb.push_back(char(ch));
            }
        }
        sb.push_back('"');
        return string(sb);
    }
    
    string escape_char(char b){
        uint8_t ch = b;
        string_builder sb;
        sb.push_back('"');
        if (ch < 0x20){
            sb.append(literal("\\u00"));
            sb.push_back(K_hex_table[(ch >> 4)]);
            sb.push_back(K_hex_table[(ch & 0x0f)]);
        }
        else {
            if(ch == '\\' || ch == '"'){
                sb.push_back('\\');
            }
            sb.push_back(char(ch));
        }
        sb.push_back('"');
        return string(sb);
    }
    
    string escape_string(const string& str){
        string_builder sb;
        sb.push_back('"');
        
        for(auto c : str){
            unsigned int ch = static_cast<unsigned char>(c);
            if (ch < 0x20){
                sb.append(literal("\\u00"));
                sb.push_back(K_hex_table[(ch >> 4)]);
                sb.push_back(K_hex_table[(ch & 0x0f)]);
            }
            else {
                if(ch == '\\' || ch == '"'){
                    sb.push_back('\\');
                }
                sb.push_back(char(ch));
            }
        }
        sb.push_back('"');
        return string(sb);
        
    }

    
    
}}
