#ifndef JSON_DESERIALIZER_H__
#define JSON_DESERIALIZER_H__

#include <xirang2/type/xirang.h>
#include "contentservicefwd.h"

namespace CS{

class JSONDeserializer
{
public:
    void convert_obj(std::ostream& os, const std::string& path, const std::string& js, ContentServiceRT& rt);
    static void build_type_visitor(xirang2::type::Namespace root);
};

}//namespace

#endif  //JSON_DESERIALIZER_H__
