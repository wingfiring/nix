#include <xirang2/type/s11njson.h>
#include <xirang2/serialize/exchs11n.h>
#include <xirang2/type/array.h>
#include <xirang2/type/objectlink.h>
#include <xirang2/type/nativetypeversion.h>
#include <xirang2/io/memory.h>
#include <xirang2/string_algo/json.h>

#include <unordered_map>
#include <sstream>

namespace xirang2{ namespace io{ namespace json_obj{
    namespace {
        
        std::unordered_map<string, json_formatter*, hash_string>* K_json_formatter_table = 0;
        const std::locale& K_c_locale() {
            static std::locale locale("C");
            return locale;
        }
        const xirang2::string K_null("null");
        const xirang2::string K_true("true");
        const xirang2::string K_false("false");
        
        struct clean_table{
            ~clean_table(){
                if (K_json_formatter_table) delete K_json_formatter_table;
            };
            
        };
        clean_table K_clean_table;
    }
    
    json_formatter* find_formatter(const string& name){
        auto pos = K_json_formatter_table->find(name);
        return pos == K_json_formatter_table->end() ? nullptr : pos->second;
    }
    json_formatter* register_formatter(const string& name, json_formatter* fmt){
        XR_PRE_CONDITION(fmt);
        auto old = find_formatter(name);
        (*K_json_formatter_table)[name] = fmt;
        
        return old;
    }
    
    json_formatter* unregister_formatter(const string& name){
        auto old = find_formatter(name);
        
        K_json_formatter_table->erase(name);
        return old;
    }

    
    struct json_formatter_iterator_imp
    {
        typedef std::unordered_map<string, json_formatter*, hash_string>::iterator RealIterator;
        
        json_formatter_iterator_imp(const RealIterator& itr) : rpos(itr){}
        
        const json_formatter_entry& operator*() const { cache = json_formatter_entry{rpos->first, rpos->second}; return cache;}
        const json_formatter_entry* operator->() const { return &**this;}
        void operator++ () { ++rpos;}
        
        bool operator== (const json_formatter_iterator_imp& rhs) const { return rpos == rhs.rpos;}
        
        RealIterator rpos;
        mutable json_formatter_entry cache;
    };
    
    json_formatter_range list_formatter(){
        return json_formatter_range (
                               json_formatter_range::iterator(json_formatter_iterator_imp(K_json_formatter_table->begin())),
                               json_formatter_range::iterator(json_formatter_iterator_imp(K_json_formatter_table->end()))
                               );
        
    }
    
    template<typename Container>
    range<writer::iterator> to_buffer_range(const Container& cont){
        
        if(cont.empty())
            return range<writer::iterator>();
        
        static_assert(sizeof(*cont.begin()) == 1, "only support char type");
        const xirang2::byte* first = (const xirang2::byte*)&*cont.begin();
        const xirang2::byte* last =first + cont.size();
        
        return make_range(first, last);
    }
    
    template<typename Container>
    size_t block_write_bytes(io::writer& wr, const Container& t){
        auto buf = to_buffer_range(t);
        block_write(wr, buf);
        return buf.size();
    }
    
    std::size_t write_type_str(io::writer& wr, type::Type t){
        if (t.valid()){
            auto version = sha1_to_string(t.version().id);
            auto str = xirang2::json::escape_string(t.versionedFullName().str());
            block_write_bytes(wr, str);
            
            return str.size();
        }

        block_write(wr, to_buffer_range(K_null));
        return K_null.size();
    }
    
    static const string K_type_left("{\"type\":");
    static const string K_value_left(",\"value\":");
    
    std::size_t write_with_type(io::writer& wr, type::ConstCommonObject obj, int& /*flag*/){
        block_write(wr, to_buffer_range(K_type_left));
        auto size = write_type_str(wr, obj.type());
        block_write(wr, to_buffer_range(K_value_left));
        
        auto s = xirang2::io::exchange::as_sink(wr);
        s & char('}');
        return size + K_type_left.size() + K_value_left.size() + 1; //1 for end '}'
    }
    
    std::size_t write_root_with_type(io::writer& wr, type::ConstCommonObject obj, int& flag){
        XR_PRE_CONDITION(((flag & int(jcc_type_of_root))) != 0);
        
        flag &= ~jcc_type_of_root;
        return write_with_type(wr, obj, flag);
    }
    
    class json_formatter_base : public json_formatter{
    public:
        virtual std::size_t write(io::writer& wr, type::ConstCommonObject obj, int flag){
            auto s = xirang2::io::exchange::as_sink(wr);
            
            size_t count = 0;
            bool need_write_type = (flag & jcc_type_of_root) != 0;
            if (need_write_type)
                count += write_root_with_type(wr, obj, flag);
            
            count += do_write(wr, obj, flag);
            
            if(need_write_type)
                s & char('}');
            
            return count;
        }
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int flag) = 0;
    protected:
        virtual ~json_formatter_base(){}
    };
    
    struct null_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
			unuse(obj);
            XR_PRE_CONDITION(!obj.valid());
            
            block_write(wr, to_buffer_range(K_null));

            return K_null.size();
        }
        
        static null_formatter instance;
    };
    null_formatter null_formatter::instance;
    
    struct generic_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int flag){
            auto s = xirang2::io::exchange::as_sink(wr);
            
            size_t count = 2;
            
            s & char('{');
            
            bool first = true;

            for(auto& i : obj.members()){
                if (first)
                    first = false;
                else {
                    s & char(',');
                    ++count;
                }
                
                auto escaped_name = xirang2::json::escape_string(i.itemInfo().name());

                block_write(wr, to_buffer_range(escaped_name));
                s & char(':');
                count += escaped_name.size() + 1;
                
                auto m = i.asCommonObject();
                XR_PRE_CONDITION(m.valid());
                
                auto internal_id = m.type().methods().internalID();
                
                count += get_formatter(internal_id).write(wr, m, flag);
            }
            
            s & char('}');
            
            return count;
        }
        
        static generic_formatter instance;
    };
    generic_formatter generic_formatter::instance;
    
    struct array_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int flag){
            auto s = xirang2::io::exchange::as_sink(wr);
            
            size_t count = 2;
            
            s & char('[');
            
            bool first = true;
            auto& arr = *type::Ref<type::Array>(obj);
            
            for(auto& i : arr){
                if (first)
                    first = false;
                else {
                    s & char(',');
                    ++count;
                }

                XR_PRE_CONDITION(i.valid());
                
                auto internal_id = i.type().methods().internalID();
                
                count += get_formatter(internal_id).write(wr, i, flag);
                
            }
            
            s & char(']');

            return count;
        }
        
        static array_formatter instance;
    };
    array_formatter array_formatter::instance;
    
    struct bool_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
            auto & str = *type::Ref<bool>(obj)? K_true : K_false;
            block_write(wr, to_buffer_range(str));

            return str.size();
        }
        
        static bool_formatter instance;
    };
    bool_formatter bool_formatter::instance;
    
    
    struct byte_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
            auto str = xirang2::json::escape_char(static_cast<char>(*type::Ref<byte>(obj)));
            block_write(wr, to_buffer_range(str));

            return str.size();
        }
        
        static byte_formatter instance;
    };
    byte_formatter byte_formatter::instance;
    
    
    struct byte_buffer_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
            auto str = xirang2::json::escape_buffer(*type::Ref<buffer<byte>>(obj));
            block_write(wr, to_buffer_range(str));
            
            return str.size();
        }
        
        static byte_buffer_formatter instance;
    };
    byte_buffer_formatter byte_buffer_formatter::instance;
    
    
    template<typename T>
    struct number_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
            std::stringstream sstr;
            sstr.imbue(K_c_locale());
            sstr << *type::Ref<T>(obj);
            auto str = sstr.str();
            block_write(wr, to_buffer_range(str));
            
            return str.size();
        }
        
        static number_formatter instance;
    };
    
    template<typename T>
    number_formatter<T> number_formatter<T>::instance;
    
    template<>
    struct number_formatter<int8_t> : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
            std::stringstream sstr;
            sstr.imbue(K_c_locale());
            sstr << int(*type::Ref<int8_t>(obj));
            auto str = sstr.str();
            block_write(wr, to_buffer_range(str));
            
            return str.size();
        }
        
        static number_formatter instance;
    };
    
    number_formatter<int8_t> number_formatter<int8_t>::instance;
    
    template<>
    struct number_formatter<uint8_t> : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
            std::stringstream sstr;
            sstr.imbue(K_c_locale());
            sstr << int(*type::Ref<uint8_t>(obj));
            auto str = sstr.str();
            block_write(wr, to_buffer_range(str));
            
            return str.size();
        }
        
        static number_formatter instance;
    };
    
    number_formatter<uint8_t> number_formatter<uint8_t>::instance;

    
    struct string_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
            auto str = xirang2::json::escape_string(*type::Ref<string>(obj));
            block_write(wr, to_buffer_range(str));
            
            return str.size();
        }
        
        static string_formatter instance;
    };
    
    string_formatter string_formatter::instance;
    
    struct path_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
            auto str = xirang2::json::escape_string(type::Ref<file_path>(obj)->str());
            block_write(wr, to_buffer_range(str));
            
            return str.size();
        }
        
        static path_formatter instance;
    };
    
    path_formatter path_formatter::instance;

    struct type_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int /*flag*/){
            auto t = *type::Ref<type::Type>(obj);
            return write_type_str(wr, t);
        }
        
        static type_formatter instance;
    };
    type_formatter type_formatter::instance;
    
    static const string K_target_left(",\"target\":");
    
    struct dynamic_link_formatter : public json_formatter_base{
        virtual std::size_t do_write(io::writer& wr, type::ConstCommonObject obj, int flag){
            auto& src = *type::Ref<type::DynamicLink>(obj);
            if (flag & jcc_type_of_link){
                block_write(wr, to_buffer_range(K_type_left));
                auto size = write_type_str(wr, src.target.type());
                block_write(wr, to_buffer_range(K_target_left));
                auto str = xirang2::json::escape_string(src.path.str());
                block_write(wr, to_buffer_range(str));
                
                auto s = xirang2::io::exchange::as_sink(wr);
                s & char('}');
                
                return size + K_type_left.size() + K_target_left.size() + str.size() + 1;
            }
            else {
                auto str = xirang2::json::escape_string(src.path.str());
                block_write(wr, to_buffer_range(str));
                
                return str.size();
            }
        }
        
        static dynamic_link_formatter instance;
    };
    dynamic_link_formatter dynamic_link_formatter::instance;

    
    void init_default_formatter_once(){
        if (!K_json_formatter_table){
            K_json_formatter_table = new std::unordered_map<string, json_formatter*, hash_string>;
            
            (*K_json_formatter_table)["null"] = &null_formatter::instance;
            (*K_json_formatter_table)[".sys.type..generic"] = &generic_formatter::instance;
            (*K_json_formatter_table)[".sys.type.array"] = &array_formatter::instance;
            (*K_json_formatter_table)[".sys.type.bool"] = &bool_formatter::instance;
            (*K_json_formatter_table)[".sys.type.byte"] = &byte_formatter::instance;
            (*K_json_formatter_table)[".sys.type.byte_buffer"] = &byte_buffer_formatter::instance;
            (*K_json_formatter_table)[".sys.type.dynamic_link"] = &dynamic_link_formatter::instance;
            (*K_json_formatter_table)[".sys.type.string"] = &string_formatter::instance;
            (*K_json_formatter_table)[".sys.type.path"] = &path_formatter::instance;
            (*K_json_formatter_table)[".sys.type.type"] = &type_formatter::instance;
            
            (*K_json_formatter_table)[".sys.type.double"] = &number_formatter<double>::instance;
            (*K_json_formatter_table)[".sys.type.float"] = &number_formatter<float>::instance;
            (*K_json_formatter_table)[".sys.type.int16"] = &number_formatter<int16_t>::instance;
            (*K_json_formatter_table)[".sys.type.int32"] = &number_formatter<int32_t>::instance;
            (*K_json_formatter_table)[".sys.type.int64"] = &number_formatter<int64_t>::instance;
            (*K_json_formatter_table)[".sys.type.int8"] = &number_formatter<int8_t>::instance;
            (*K_json_formatter_table)[".sys.type.uint16"] = &number_formatter<uint16_t>::instance;
            (*K_json_formatter_table)[".sys.type.uint32"] = &number_formatter<uint32_t>::instance;
            (*K_json_formatter_table)[".sys.type.uint64"] = &number_formatter<uint64_t>::instance;
            (*K_json_formatter_table)[".sys.type.uint8"] = &number_formatter<uint8_t>::instance;
        }
    }
    XR_API std::size_t write_object(io::writer& wr, type::ConstCommonObject obj, int flag){
        xirang2::string internal_id = "null";
        
        if (obj.valid()){
            internal_id = obj.type().methods().internalID();
        }
        
        return get_formatter(internal_id).write(wr, obj, flag);
    }
    
    XR_API string to_json(type::ConstCommonObject obj, int flag){
        xirang2::io::mem_archive mar;
        xirang2::iref<xirang2::io::writer> dest(mar);
        xirang2::io::json_obj::get_formatter(obj.type().methods().internalID()).write(dest.get<xirang2::io::writer>(), obj, flag);
        const char* first = reinterpret_cast<const char*>(mar.data().begin());
        const char* last = reinterpret_cast<const char*>(mar.data().end());
        
        return string(const_range_string(first, last));
    }
    
    std::size_t write_type(io::writer& wr, type::Type t, int flag){
        std::stringstream os;
        os.imbue(K_c_locale());
        
        auto s = block_write_bytes(wr, literal("{\"fullName\":"));
        s += write_type_str(wr, t);
        s += block_write_bytes(wr, literal(",\"model\":"));
        s += write_type_str(wr, t.model());
        
        s += block_write_bytes(wr, literal(",\"args\":["));
        
        bool first_arg  = true;
        for (auto&i : t.args()){
            if (!first_arg)
                s += block_write_bytes(wr, literal(","));
            else
                first_arg = false;
            
            s += block_write_bytes(wr, literal("{\"name\":"));
            s += block_write_bytes(wr, xirang2::json::escape_string(i.name()));
            
            s += block_write_bytes(wr, literal(",\"typename\":"));
            s += block_write_bytes(wr, xirang2::json::escape_string(i.typeName().str()));
            
            s += block_write_bytes(wr, literal(",\"type\":"));
            s += write_type_str(wr, i.type());
            
            s += block_write_bytes(wr, literal("}"));
        }
        s += block_write_bytes(wr, literal("],\"members\":["));
        

        bool first_member  = true;
        for (auto&i : t.members()){
            if (!first_member)
                s += block_write_bytes(wr, literal(","));
            else
                first_member = false;
            s += block_write_bytes(wr, literal("{\"name\":"));
            s += block_write_bytes(wr, xirang2::json::escape_string(i.name()));
            
            s += block_write_bytes(wr, literal(",\"typename\":"));
            s += block_write_bytes(wr, xirang2::json::escape_string(i.typeName().str()));
            
            s += block_write_bytes(wr, literal(",\"type\":"));
            s += write_type_str(wr, i.type());
            
            s += block_write_bytes(wr, literal("}"));
        }
        s += block_write_bytes(wr, literal("],\"static_data_type\":"));

        s += write_type_str(wr, t.staticType());
        
        s += block_write_bytes(wr, literal(",\"static_data\":"));
        
        s += write_object(wr, t.staticData(), flag);
        
        s += block_write_bytes(wr, literal(",\"default\":"));
        
        s += write_object(wr, t.prototype(), flag);
        
        s += block_write_bytes(wr, literal("}"));
        
        return s;
    }
    
    XR_API string to_json(type::Type type, int flag){
        xirang2::io::mem_archive mar;
        xirang2::iref<xirang2::io::writer> dest(mar);
        write_type(dest.get<xirang2::io::writer>(), type, flag);
        
        const char* first = reinterpret_cast<const char*>(mar.data().begin());
        const char* last = reinterpret_cast<const char*>(mar.data().end());
        
        return string(const_range_string(first, last));
    }
}}}
