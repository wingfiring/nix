
#ifndef JSON_SERIALIZER_H__
#define JSON_SERIALIZER_H__

#include <functional>
#include <unordered_map>
#include <xirang2/type/xirang.h>

namespace CS{

class JSONSerializer
{
public:
    void response_obj(std::ostream& os, const xirang2::type::ConstCommonObject& obj);
    void response_type(std::ostream& os, const xirang2::type::Type& t);
    void response_object_list (std::ostream& os, xirang2::type::Namespace root, const xirang2::file_path& path);
    void response_schema_list(std::ostream& os, xirang2::type::Namespace root);
    static void build_type_visitor(xirang2::type::Namespace root);
};

}//namespace

#endif  //JSON_SERIALIZER_H__
