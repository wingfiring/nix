
#include "jsonserializer.h"
#include "predefinedtypes.h"
#include "assert.h"
#include <xirang2/protein/datatype.h>
#include <xirang2/type/nativetypeversion.h>


namespace CS{

using namespace xirang2;
using namespace xirang2::type;
using namespace xirang2::protein;

typedef std::unordered_map<xirang2::type::Type,
	std::function<void(std::ostream&, xirang2::type::ConstCommonObject)> ,
	hash_type> VisitTable;

static void build_type_visitor_(xirang2::type::Namespace root, VisitTable& vistable);
static VisitTable visitTable;
static bool tableInitialized = false;

static VisitTable& get_visittable_()
{
    assert(tableInitialized);
    return visitTable;
}

static void visit_property_(std::ostream& os, const ConstCommonObject& obj){
	bool need_comma = false;
	os << "{";
	VisitTable& vt = get_visittable_();
	for (auto& pro : obj.members()){
		if (need_comma) os << ',';
		else need_comma = true;

		auto p = vt.find(pro.type());
		if (p == vt.end() && pro.type().model().valid())
			p =vt.find(pro.type().model());

		XR_PRE_CONDITION(p != vt.end());

		os << '"' << pro.ownerType().member(pro.index()).name() << "\":";
		//(std::get<1>(p->second))(os, pro.asCommonObject());
		(p->second)(os, pro.asCommonObject());
	}
	os << "}";
}

template<typename T>
void print_array(std::ostream& os, const Array& arr){
	os << "[";
	bool need_comma = false;
	for(auto& i : arr){
		if (need_comma) os << ",";
		else need_comma = true;
		os << *Ref<T>(i);
	}
	os << "]";
}

xirang2::string escape_json(const xirang2::string& text){
	std::string result;
	for (auto&i : text){
		if(i == '"' || i == '\\'){
			result.push_back('\\');
			result.push_back(i);
		}
		else if (i =='\n'){
			result.push_back('\\');
			result.push_back('n');
		}
		else if (i =='\r'){
			result.push_back('\\');
			result.push_back('r');
		}
		else
			result.push_back(i);
	}
	return result.c_str();
}


void print_string(std::ostream& os, const xirang2::string& s){
	os << "\"" << escape_json(s) << "\"";
}
void print_array_string(std::ostream& os, const Array& arr){
	os << "[";
	bool need_comma = false;
	for(auto& i : arr){
		if (need_comma) os << ",";
		else need_comma = true;
		print_string(os, *Ref<xirang2::string>(i));
	}
	os << "]";
}

const static file_path K_prefix("/cm/lib/adsk/cm/resource/");
void print_path(std::ostream& os, const file_path& path){
	if (path.empty() || K_prefix.contains(path))
		print_string(os, path.str());
	else
		print_string(os, (K_prefix / path).str());
}
void print_array_path(std::ostream& os, const Array& arr){
	os << "[";
	bool need_comma = false;
	for(auto& i : arr){
		if (need_comma) os << ",";
		else need_comma = true;
		print_path(os, *Ref<file_path>(i));
	}
	os << "]";
}

void print_dlink(std::ostream& os, const DynamicLink& link){
	os << '"' << escape_json(link.path.str()) << '"';
}
void print_array_link(std::ostream& os, const Array& arr){
	os << "[";
	bool need_comma = false;
	for(auto& i : arr){
		if (need_comma) os << ",";
		else need_comma = true;
		print_dlink(os, *Ref<DynamicLink>(i));
	}
	os << "]";
}

void print_vector3(std::ostream& os, const vector3& v){
	os << "{\"x\":" << v.data[0] << ",\"y\":" << v.data[1] << ",\"z\":" << v.data[2] << "}";
}

void print_array_vector3(std::ostream& os, const Array& arr){
	os << "[";
	bool need_comma = false;
	for(auto& i : arr){
		if (need_comma) os << ",";
		else need_comma = true;
		print_vector3(os, *Ref<vector3>(i));
	}
	os << "]";
}

void print_color4(std::ostream& os, const color4& v){
	os << "{\"r\":" << v.r << ",\"g\":" << v.g << ",\"b\":" << v.b << ",\"a\":" << v.a << "}";
}

void print_array_color4(std::ostream& os, const Array& arr){
	os << "[";
	bool need_comma = false;
	for(auto& i : arr){
		if (need_comma) os << ",";
		else need_comma = true;
		print_color4(os, *Ref<color4>(i));
	}
	os << "]";
}

void print_unit(std::ostream& os, const general_unit& v){
	os << "{\"unit\":" << v.unit << ",\"value\":" << v.value << "}";
}

void print_distance(std::ostream& os, const unit_distance& v){
	os << "{\"unit\":" << v.unit << ",\"value\":" << v.value << "}";
}

void JSONSerializer::build_type_visitor(Namespace root)
{
    build_type_visitor_(root, visitTable);
    tableInitialized = true;
}

void JSONSerializer::response_obj(std::ostream& os, const ConstCommonObject& obj){
	visit_property_(os, obj);
}

void JSONSerializer::response_type(std::ostream& os, const Type& t){
	os << "{";

	os << "\"fullName\":"  << "\"" << t.fullName().str() << "\",";
	os << "\"model\":\"";
	if (t.model().valid())
		os << t.model().fullName().str();
	os << "\",";

	os << "\"args\":[";
	bool first_arg  = true;
	for (auto&i : t.args()){
		if (!first_arg) os << ", ";
		else first_arg = false;
		os << "{\"name\":\"" << i.name() << "\","
			<< "\"typename\":\""  << i.typeName().str() << "\","
			<< "\"type\":\"";
		if (i.type().valid())
			os << i.type().fullName().str();
		os << "\"}";
	}
	os << "],";

	os << "\"members\":[";
	bool first_member  = true;
	for (auto&i : t.members()){
		if (!first_member) os << ", ";
		else first_member = false;
		os << "{\"name\":\"" << i.name() << "\","
			<< "\"typename\":\""  << i.typeName().str() << "\","
			<< "\"type\":\"";
		if (i.type().valid())
			os << i.type().fullName().str();
		os << "\"}";
	}
	os << "],";

	os << "\"static_data_type\":\"";
	auto static_type = t.staticType();
	if (static_type.valid())
		os << static_type.fullName().str();
	os << "\",";

	os << "\"static_data\":";
	auto static_data = t.staticData();
	if (!static_data.valid())
		os << "{}";
	else
		response_obj(os, static_data);
	os << ",";

	os << "\"default\":";
	auto prototype = t.prototype();
	if (!prototype.valid())
		os << "{}";
	else
		response_obj(os, prototype);

	os << "}";

}

void JSONSerializer::response_object_list (std::ostream& os,Namespace root, const file_path& path){
	auto ns = root.locateNamespace(sub_file_path(literal("/lib/adsk/cm/type")));
	if (!ns.valid())
		return;

	os << "{"
		"    \"path\":";
	print_string(os, path.str());

	os << ",   \"items\":";
	os << "[";
	bool need_comma = false;
	for (auto&i : root.locateNamespace(path/file_path("metadata")).objects()){
		if (need_comma){
			os << ", ";
		}
		else
			need_comma = true;
		response_obj(os, i.value);
	}


	os << "],   \"assets\":";
	os << "[";
	need_comma = false;
	for (auto&i : root.locateNamespace(path/file_path("metadata")).objects()){
		if (need_comma){
			os << ", ";
		}
		else
			need_comma = true;
		print_string(os, i.name->str());
	}

	os << "]}";
}

void JSONSerializer::response_schema_list(std::ostream& os,Namespace root){
	auto ns = root.locateNamespace(sub_file_path(literal("/lib/adsk/cm/type")));
	XR_PRE_CONDITION(ns.valid());

	os << "{"
		"    \"path\":\"/lib/adsk/cm/type\","
		"    \"items\":";
	os << "[";
	bool need_comma = false;
	for (auto&i : ns.types()){
		if (need_comma){
			os << ", ";
		}
		else
			need_comma = true;
		os << "\""<< i.current().name().str() << "\"";
	}

	os << "]}";

}

void build_type_visitor_(Namespace root, VisitTable& vistable){
	typedef std::function<void(std::ostream&, ConstCommonObject)> FB_;

	vistable[root.locateType(file_path("/sys/type/int8"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ os << std::boolalpha << (*Ref<int8_t>(obj) == 1); }
			);
	vistable[root.locateType(file_path("/sys/type/int32"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ os << *Ref<int32_t>(obj); }
			);
	vistable[root.locateType(file_path("/sys/type/array"))] = FB_(		//gradient only, it's int
			[](std::ostream&os, ConstCommonObject obj){ print_array<int32_t>(os, *Ref<Array>(obj)); }
			);
	vistable[root.locateType(file_path("/sys/type/run/array_double"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_array<double>(os, *Ref<Array>(obj));}
			);
	vistable[root.locateType(file_path("/sys/type/double"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ os << *Ref<double>(obj); }
			);
	vistable[root.locateType(file_path("/sys/type/run/array_string"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_array_string(os, *Ref<Array>(obj)); }
			);
	vistable[root.locateType(file_path("/sys/type/string"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_string(os, *Ref<xirang2::string>(obj)); }
			);
	vistable[root.locateType(file_path("/sys/type/path"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_path(os, *Ref<file_path>(obj)); }
			);
	vistable[root.locateType(file_path("/sys/type/run/array_path"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_array_path(os, *Ref<Array>(obj));}
			);
	vistable[root.locateType(file_path("/sys/type/dynamic_link"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_dlink(os, *Ref<DynamicLink>(obj));}
			);
	vistable[root.locateType(file_path("/sys/type/run/array_dynamic_link"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_array_link(os, *Ref<Array>(obj));}
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/vector3"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_vector3(os, *Ref<vector3>(obj));}
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/array_color4"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_array_color4(os, *Ref<Array>(obj));}
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/color4"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_color4(os, *Ref<color4>(obj)); }
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/general_unit"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_unit(os, *Ref<general_unit>(obj));}
			);

	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/color4_link"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){
				os << "{\"value\":";
				auto& c = *Ref<color4>(obj.getMember(0).asCommonObject());
				print_color4(os, c);

				os << ",\"path\":" ;
				auto& link = *Ref<DynamicLink>(obj.getMember(1).asCommonObject());
				print_dlink(os, link);
				os << "}";
			}
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/double_link"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){
				os << "{\"value\":";
				os << *Ref<double>(obj.getMember(0).asCommonObject());

				os << ",\"path\":" ;
				auto& link = *Ref<DynamicLink>(obj.getMember(1).asCommonObject());
				print_dlink(os, link);
				os << "}";
			}
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/unit_distance"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){
				os << "{\"value\":";
				print_distance(os, *Ref<unit_distance>(obj));
				os << "}";
			}
			);

	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/distance_bounds"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){
				auto&  b = *Ref<const_bounds_value>(obj);
				print_distance(os, *Ref<unit_distance>(b.getValue()));
			}
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/unit_bounds"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){
				auto&  b = *Ref<const_bounds_value>(obj);
				print_unit(os, *Ref<general_unit>(b.getValue()));
			}
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/int_bounds"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){
				auto& b = *Ref<const_bounds_value>(obj);
				os << *Ref<int32_t>(b.getValue());
			}
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/double_bounds"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){
			auto& b = *Ref<const_bounds_value>(obj);
			os << *Ref<double>(b.getValue()); }
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/double_link_bounds"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){
				os << "{\"value\":";
				auto& b = *Ref<const_bounds_value>(obj.getMember(0).asCommonObject());
				os << *Ref<double>(b.getValue());

				os << ",\"path\":" ;
				auto& link = *Ref<DynamicLink>(obj.getMember(1).asCommonObject());
				print_dlink(os, link);
				os << "}";
			}
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/meta_link"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){ print_string(os, (*Ref<ConstStaticLinkRef>(obj)).path().str()); }
			);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/choice_value"))] = FB_(
			[](std::ostream&os, ConstCommonObject obj){
				os << *Ref<int32_t>(obj.getMember(0).asCommonObject());
			}
			);
}

} //namespace
