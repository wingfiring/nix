
#include <xirang2/string_algo/uri.h>
#include <xirang2/type/object.h>
#include <xirang2/protein/datatype.h>
#include <xirang2/type/nativetypeversion.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

#include <unordered_map>
#include <functional>

#include "commands.h"

namespace cs2{
    
    using namespace xirang2;
    using namespace xirang2::type;
    using namespace xirang2::protein;
    using boost::property_tree::ptree;
    
    struct hash_type{
        
        std::size_t operator()(xirang2::type::Type t) const{
            return xirang2::hash_sha1()(t.version().id);
        }
    };
    
    typedef std::unordered_map<xirang2::type::Type,
    std::function<void(const xirang2::string&, xirang2::type::CommonObject&, const ptree&)>, hash_type> VisitTable;
    
    static void build_type_visitor_(xirang2::type::Namespace root, VisitTable& vistable);
    
    static VisitTable& get_visittable()
    {
        static VisitTable visitTable;

        return visitTable;
    }
    
    void init_json_load_table(xirang2::type::Namespace root){
        build_type_visitor_(root, get_visittable());
    }
    
    void convert_obj(const std::string& js, xirang2::type::CommonObject obj)
    {
        xirang2::string js_decoded = xirang2::uri::decode_string(js);
        std::string js_str(js_decoded.c_str());
        std::stringstream sstr(js_str);
        ptree pt;
        boost::property_tree::json_parser::read_json(sstr, pt);
        
        VisitTable& vt = get_visittable();
        for (ptree::iterator iter = pt.begin(); iter != pt.end(); ++iter)
        {
            std::string key(iter->first);
            xirang2::string node(key.c_str());
            ptree child = iter->second;
            
            SubObject prop = obj.getMember(node);
            Type t = prop.type();
            auto p = vt.find(t);
            if (p != vt.end())
                (p->second)(node, obj, pt);
        }
        
    }
    
    
    void build_type_visitor_(Namespace root, VisitTable& vistable){
        typedef std::function<void(const xirang2::string&, CommonObject&, const ptree&)> FB_;
        
        vistable[root.locateType(file_path("/sys/type/int8"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                *Ref<bool>(obj.getMember(node).asCommonObject()) = pt.get<bool>("Hidden");
            }
            );
        vistable[root.locateType(file_path("/sys/type/int32"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                *Ref<int32_t>(obj.getMember(node).asCommonObject()) = pt.get<int32_t>(node.c_str());
            }
            );
        vistable[root.locateType(file_path("/sys/type/array"))] =
        FB_(		//gradient only, it's int
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                Array& arr = *Ref<Array>(obj.getMember(node).asCommonObject());
                ptree int_array = pt.get_child(node.c_str());
                for(auto &v : int_array)
                {
                    ptree child = v.second;
                    int32_t value = child.get_value<int32_t>();
                    arr.push_back(ConstCommonObject(arr.type(), &value));
                }
            }
            );
        vistable[root.locateType(file_path("/sys/type/run/array_double"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                Array& arr = *Ref<Array>(obj.getMember(node).asCommonObject());
                ptree double_array = pt.get_child(node.c_str());
                for(auto &v : double_array)
                {
                    ptree child = v.second;
                    double value = child.get_value<double>();
                    arr.push_back(ConstCommonObject(arr.type(), &value));
                }
            }
            );
        vistable[root.locateType(file_path("/sys/type/double"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                *Ref<double>(obj.getMember(node).asCommonObject()) = pt.get<double>(node.c_str());
            }
            );
        vistable[root.locateType(file_path("/sys/type/run/array_string"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                Array& arr = *Ref<Array>(obj.getMember(node).asCommonObject());
                for (auto& i : arr){
                    *Ref<xirang2::string>(i);
                }
                
                //Array arrOriginal = arr;
                arr.clear();
                ptree string_array = pt.get_child(node.c_str());
                for(auto &v : string_array)
                {
                    xirang2::string str(v.second.data());
                    arr.push_back(ConstCommonObject(arr.type(), &str));
                }
                
                for (auto& k : arr){
                    *Ref<xirang2::string>(k);
                }
            }
            );
        vistable[root.locateType(file_path("/sys/type/string"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                *Ref<xirang2::string>(obj.getMember(node).asCommonObject()) = pt.get<std::string>(node.c_str()).c_str();
            }
            );
        vistable[root.locateType(file_path("/sys/type/path"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                file_path path(pt.get<std::string>(node.c_str()).c_str());
                *Ref<file_path>(obj.getMember(node).asCommonObject()) = path;
            }
            );
        vistable[root.locateType(file_path("/sys/type/run/array_path"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                Array& arr = *Ref<Array>(obj.getMember(node).asCommonObject());
                //Array arrOriginal = arr;
                arr.clear();
                ptree string_array = pt.get_child(node.c_str());
                for(auto &v : string_array)
                {
                    ptree child = v.second;
                    std::string str = child.get_value<std::string>();
                    file_path path(str.c_str());
                    arr.push_back(ConstCommonObject(arr.type(), &path));
                }
            }
            );
        vistable[root.locateType(file_path("/sys/type/dynamic_link"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                // TODO:
#ifdef WIN32
                node; obj; pt;
#endif
                //print_dlink(os, *Ref<DynamicLink>(obj));
            }
            );
        vistable[root.locateType(file_path("/sys/type/run/array_dynamic_link"))] =
        FB_(
            [](const xirang2::string& node, CommonObject& obj, const ptree& pt)
            {
                // TODO:
#ifdef WIN32
                node; obj; pt;
#endif
                //print_array_link(os, *Ref<Array>(obj));
            }
            );
    }
    
}//namespace
