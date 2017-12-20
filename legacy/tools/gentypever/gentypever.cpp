#include <xirang2/type/xirang.h>
#include <xirang2/type/type.h>
#include <xirang2/type/typebinder.h>
#include <xirang2/type/nativetype.h>
#include <xirang2/serialize/string.h>
#include <xirang2/serialize/buffer.h>
#include <xirang2/type/array.h>
#include <xirang2/type/typeio.h>
#include <xirang2/type/objectlink.h>

#include <xirang2/protein/datatype.h>

#include <iostream>
#include <iomanip>

#include <map>

using namespace xirang2;
using namespace xirang2::type;

void print(Type t) {
	static const std::map<string, string> g_type_map = {
		{"bool", 			"bool"},
		{"float",			"float"},
		{"double", 			"double"},
		{"int8",			"int8_t"},
		{"int16",			"int16_t"},
		{"int32",			"int32_t"},
		{"int64",			"int64_t"},
		{"uint8",			"uint8_t"},
		{"uint16",			"uint16_t"},
		{"uint32",			"uint32_t"},
		{"uint64",			"uint64_t"},

		{"byte",			"byte"},
		{"pointer",			"void*"},
		{"const_pointer",	"const void*"},
		{"type",			"Type"},
		{"array",			"Array"},
		{"string",			"string"},
		{"path",			"file_path"},
		{"byte_buffer",	 	"byte_buffer"},
		{"link",			"StaticLinkRef"},
		{"dynamic_link",	"DynamicLink"},

	//unused
		{"ref",				""},
		{"dynamic_ref",		""},
	};

	auto name = t.name().str();
	auto pos = g_type_map.find(name);
	if (pos != g_type_map.end())	name = pos->second;
	if (name.empty())	return;

	std::cout << "XIRANG2_DEFINE_TYPE_VERSION_OF(\""
		<< (sha1_to_string(t.version().id)) << "\", "
		<< std::setw(18) << name << ", \"" << t.methods().internalID() <<"\");\n";
}

int main(){
	sha1 sha;
	std::cout << sha.get_digest() << std::endl;
	Xirang xr("demo", memory::get_global_heap(), memory::get_global_ext_heap());
	SetupXirang(xr);
	auto sys = xr.root().locateNamespace(file_path("sys/type"));
	for (auto ts : sys.types()){
		Type t = ts.current();
		print(t);
	}

	std::cout << std::endl << std::endl;
	auto run = xr.root().locateNamespace(file_path("sys/type/run"));
	for (auto ts : run.types()){
		Type t = ts.current();
		print(t);
	}

	xirang2::protein::SetupSysTypes(xr);

	auto xrtype_ns = xr.root().locateNamespace(file_path("lib/adsk/cm/1.0/sys/type"));
	std::cout << std::endl << std::endl;
	for (auto ts : xrtype_ns.types()){
		Type t = ts.current();
		print(t);
	}

}
