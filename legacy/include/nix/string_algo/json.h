#ifndef XR_COMMON_STRING_ALGO_JSON_H__
#define XR_COMMON_STRING_ALGO_JSON_H__

#include <xirang2/config.h>
#include <xirang2/buffer.h>
#include <xirang2/context_except.h>

namespace xirang2{ namespace json{
    
    XR_API extern string escape_buffer(const buffer<byte>& buf);
    XR_API extern string escape_char(char b);
    XR_API extern string escape_string(const string& str);
}}



#endif //endif XR_COMMON_STRING_ALGO_JSON_H__

