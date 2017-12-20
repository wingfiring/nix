
#include "commandutils.h"

namespace CS{

using namespace xirang2;
using namespace xirang2::type;

void CommandUtils::response_error(std::ostream& os, int code, const file_path& path ){
	os << "Status: " << code << "\r\n"
		"Content-type: text/html\r\n"
		"\r\n"
		"Request file: "
		<< path.str()
		<< " not found\r\n";
}
void CommandUtils::response_text(std::ostream& fout){
	fout << "Content-type: text/html\r\n"
		"\r\n"
		<< "Test Done\n";
}

void CommandUtils::response_file(std::ostream& fout, iref<io::read_map>& file){
	auto & rmap = file.get<xirang2::io::read_map>();
	auto view = rmap.view_rd(xirang2::ext_heap::handle(0, rmap.size()));
	auto address = view.get<xirang2::io::read_view>().address();

	fout << "Cache-Control: public, max-age=7776000\r\n\r\n";
	fout.write((const char*)address.begin(), address.size());
}

void CommandUtils::response_header(std::ostream& os, int code){
	os << "Status: " << code << "\r\n"	
		"Content-type: text/html\r\n"
		"\r\n";
}

CommonObject CommandUtils::locateObject(Namespace root, sub_file_path path){
	Namespace ns = root.locateNamespace(path.parent());
	return  ns.valid() ? ns.findObject(path.filename()).value : CommonObject();
}

ConstCommonObject CommandUtils::locateConstObject(Namespace root, sub_file_path path){
	Namespace ns = root.locateNamespace(path.parent());
	return  ns.valid() ?  ns.findObject(path.filename()).value : ConstCommonObject();
}

} //namespace
