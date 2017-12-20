
#ifndef COMMAND_UTILS_H__
#define COMMAND_UTILS_H__ 

#include <xirang2/type/xirang.h>

namespace CS{

class CommandUtils
{
public:
    static void response_error(std::ostream& os, int code, const xirang2::file_path& path );
    static void response_text(std::ostream& fout);
    static void response_file(std::ostream& fout, xirang2::iref<xirang2::io::read_map>& file);
    static void response_header(std::ostream& os, int code = 200);
	static xirang2::type::CommonObject locateObject(xirang2::type::Namespace root, xirang2::sub_file_path path);
    static xirang2::type::ConstCommonObject locateConstObject(xirang2::type::Namespace root, xirang2::sub_file_path path);
};

}//namespace

#endif  //COMMAND_UTILS_H__
