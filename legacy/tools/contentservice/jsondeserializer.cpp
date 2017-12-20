
#include "jsondeserializer.h"
#include "commandutils.h"
#include "contentservicert.h"
#include "predefinedtypes.h"
#include "xirang2/string_algo/uri.h"
#include "xirang2/type/object.h"
#include <xirang2/protein/datatype.h>
#include <xirang2/type/nativetypeversion.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <unordered_map>
#include <functional>

namespace CS{

using namespace xirang2;
using namespace xirang2::type;
using namespace xirang2::protein;
using boost::property_tree::ptree;

typedef std::unordered_map<xirang2::type::Type,
	std::function<void(const xirang2::string&, xirang2::type::CommonObject&, const ptree&)>,
	hash_type> VisitTable;

static void build_type_visitor_(xirang2::type::Namespace root, VisitTable& vistable);
static VisitTable visitTable;
static bool tableInitialized = false;

static VisitTable& get_visittable_()
{
	assert(tableInitialized);
	return visitTable;
}

void JSONDeserializer::build_type_visitor(Namespace root)
{
	build_type_visitor_(root, visitTable);
	tableInitialized = true;
}

void JSONDeserializer::convert_obj(std::ostream& os, const std::string& path, const std::string& js, ContentServiceRT& rt)
{
	xirang2::string js_decoded = xirang2::uri::decode_string(js);
	std::string js_str(js_decoded.c_str());
	std::stringstream sstr(js_str);
	ptree pt;
	boost::property_tree::json_parser::read_json(sstr, pt);

	xirang2::file_path filepath(path.c_str());
	auto obj = CommandUtils::locateObject(rt.xr.root(), filepath);
	if (obj.valid())
	{
		VisitTable& vt = get_visittable_();
		for (ptree::iterator iter = pt.begin(); iter != pt.end(); ++iter)
		{
			std::string key(iter->first);
			xirang2::string node(key.c_str());
			ptree child = iter->second;

			SubObject prop = obj.getMember(node);
			Type t = prop.type();
			auto p = vt.find(t);
			(p->second)(node, obj, pt);  //TODO: need to decide to use pt or child
		}
	}


	os << "\r\nThis is from original str passed from REST API:" << js_str << "\r\n";

	os << "\r\nPut is successfully commit!\r\n";
}


void build_type_visitor_(Namespace root, VisitTable& vistable){
	typedef std::function<void(const xirang2::string&, CommonObject&, const ptree&)> FB_;
	
	vistable[root.locateType(file_path("/sys/type/int8"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		*Ref<int8_t>(obj.getMember(node).asCommonObject()) = pt.get<bool>("Hidden") == true ? 1 : 0;
	}
	);
	vistable[root.locateType(file_path("/sys/type/int32"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		*Ref<int32_t>(obj.getMember(node).asCommonObject()) = pt.get<int32_t>(node.c_str());
	}
	);
	vistable[root.locateType(file_path("/sys/type/array"))] = FB_(		//gradient only, it's int
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		Array& arr = *Ref<Array>(obj.getMember(node).asCommonObject());
		ptree int_array = pt.get_child(node.c_str());
		BOOST_FOREACH(ptree::value_type &v, int_array)
		{
			ptree child = v.second;
			int32_t value = child.get_value<int32_t>();
			arr.push_back(ConstCommonObject(arr.type(), &value));
		}
	}
	);
	vistable[root.locateType(file_path("/sys/type/run/array_double"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		Array& arr = *Ref<Array>(obj.getMember(node).asCommonObject());
		ptree double_array = pt.get_child(node.c_str());
		BOOST_FOREACH(ptree::value_type &v, double_array)
		{
			ptree child = v.second;
			double value = child.get_value<double>();
			arr.push_back(ConstCommonObject(arr.type(), &value));
		}
	}
	);
	vistable[root.locateType(file_path("/sys/type/double"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		*Ref<double>(obj.getMember(node).asCommonObject()) = pt.get<double>(node.c_str());
	}
	);
	vistable[root.locateType(file_path("/sys/type/run/array_string"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{ 
		Array& arr = *Ref<Array>(obj.getMember(node).asCommonObject());
		for (auto& i : arr){
			*Ref<xirang2::string>(i);
		}

		//Array arrOriginal = arr;
		arr.clear();
		ptree string_array = pt.get_child(node.c_str());
		BOOST_FOREACH(ptree::value_type &v, string_array)
		{
			xirang2::string str(v.second.data());
			arr.push_back(ConstCommonObject(arr.type(), &str));
		}

		for (auto& k : arr){
			*Ref<xirang2::string>(k);
		}
	}
	);
	vistable[root.locateType(file_path("/sys/type/string"))] = FB_(
		[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
		{
			*Ref<xirang2::string>(obj.getMember(node).asCommonObject()) = pt.get<std::string>(node.c_str()).c_str();
		}
	);
	vistable[root.locateType(file_path("/sys/type/path"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		file_path path(pt.get<std::string>(node.c_str()).c_str());
		*Ref<file_path>(obj.getMember(node).asCommonObject()) = path;
	}
	);
	vistable[root.locateType(file_path("/sys/type/run/array_path"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		Array& arr = *Ref<Array>(obj.getMember(node).asCommonObject());
		//Array arrOriginal = arr;
		arr.clear();
		ptree string_array = pt.get_child(node.c_str());
		BOOST_FOREACH(ptree::value_type &v, string_array)
		{
			ptree child = v.second;
			std::string str = child.get_value<std::string>();
			file_path path(str.c_str());
			arr.push_back(ConstCommonObject(arr.type(), &path));
		}
	}
	);
	vistable[root.locateType(file_path("/sys/type/dynamic_link"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		// TODO:
#ifdef WIN32
		node; obj; pt;
#endif
		//print_dlink(os, *Ref<DynamicLink>(obj));
	}
	);
	vistable[root.locateType(file_path("/sys/type/run/array_dynamic_link"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		// TODO:
#ifdef WIN32
		node; obj; pt;
#endif
		//print_array_link(os, *Ref<Array>(obj));
	}
	);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/vector3"))] = FB_(
	[](const xirang2::string& node, CommonObject& obj, const ptree& pt)
	{
		Array& arr = *Ref<Array>(obj.getMember(node).asCommonObject());
		//Array arrOriginal = arr;
		arr.clear();
		ptree double_array = pt.get_child(node.c_str());
		BOOST_FOREACH(ptree::value_type &v, double_array)
		{
			ptree child = v.second;
			double value = child.get_value<double>();
			arr.push_back(ConstCommonObject(arr.type(), &value));
		}
	}
	);
	/*
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/array_color4"))] = FB_(
	[](ConstCommonObject obj){ *Ref<Array>(obj); },
	[](std::ostream&os, ConstCommonObject obj){ print_array_color4(os, *Ref<Array>(obj));}
	);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/color4"))] = FB_(
	[](ConstCommonObject obj){ *Ref<color4>(obj);},
	[](std::ostream&os, ConstCommonObject obj){ print_color4(os, *Ref<color4>(obj)); }
	);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/general_unit"))] = FB_(
	[](ConstCommonObject obj){ *Ref<general_unit>(obj); },
	[](std::ostream&os, ConstCommonObject obj){ print_unit(os, *Ref<general_unit>(obj));}
	);

	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/color4_link"))] = FB_(
	[](ConstCommonObject obj){
	*Ref<color4>(obj.getMember(0).asCommonObject());
	*Ref<DynamicLink>(obj.getMember(1).asCommonObject());
	},
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
	[](ConstCommonObject obj){
	*Ref<double>(obj.getMember(0).asCommonObject());
	*Ref<DynamicLink>(obj.getMember(1).asCommonObject());
	},
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
	[](ConstCommonObject obj){
	*Ref<unit_distance>(obj);
	},
	[](std::ostream&os, ConstCommonObject obj){
	os << "{\"value\":";
	print_distance(os, *Ref<unit_distance>(obj));
	os << "}";
	}
	);

	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/distance_bounds"))] = FB_(
	[](ConstCommonObject obj){
	auto&  b = *Ref<const_bounds_value>(obj);
	*Ref<unit_distance>(b.getValue());
	},
	[](std::ostream&os, ConstCommonObject obj){
	auto&  b = *Ref<const_bounds_value>(obj);
	print_distance(os, *Ref<unit_distance>(b.getValue()));
	}
	);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/unit_bounds"))] = FB_(
	[](ConstCommonObject obj){
	auto&  b = *Ref<const_bounds_value>(obj);
	*Ref<general_unit>(b.getValue());
	},
	[](std::ostream&os, ConstCommonObject obj){
	auto&  b = *Ref<const_bounds_value>(obj);
	print_unit(os, *Ref<general_unit>(b.getValue()));
	}
	);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/int_bounds"))] = FB_(
	[](ConstCommonObject obj){
	*Ref<const_bounds_value>(obj);
	},
	[](std::ostream&os, ConstCommonObject obj){
	auto& b = *Ref<const_bounds_value>(obj);
	os << *Ref<int32_t>(b.getValue());
	}
	);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/double_bounds"))] = FB_(
	[](ConstCommonObject obj){
	*Ref<const_bounds_value>(obj);
	},
	[](std::ostream&os, ConstCommonObject obj){
	auto& b = *Ref<const_bounds_value>(obj);
	os << *Ref<double>(b.getValue()); }
	);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/double_link_bounds"))] = FB_(
	[](ConstCommonObject obj){
	*Ref<const_bounds_value>(obj.getMember(0).asCommonObject());
	*Ref<DynamicLink>(obj.getMember(1).asCommonObject());
	},
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
	[](ConstCommonObject obj){ *Ref<ConstStaticLinkRef>(obj); },
	[](std::ostream&os, ConstCommonObject obj){ print_string(os, (*Ref<ConstStaticLinkRef>(obj)).path().str()); }
	);
	vistable[root.locateType(file_path("/lib/adsk/cm/sys/type/choice_value"))] = FB_(
	[](ConstCommonObject obj){ Ref<int32_t>(obj.getMember(0).asCommonObject()); },
	[](std::ostream&os, ConstCommonObject obj){
	os << *Ref<int32_t>(obj.getMember(0).asCommonObject());
	}
	);
	*/
}

}//namespace
