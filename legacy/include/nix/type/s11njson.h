#ifndef XIRANG2_TYPE_S11N_JSON_H_
#define XIRANG2_TYPE_S11N_JSON_H_

#include <xirang2/serialize/s11nbase.h>
#include <xirang2/type/object.h>
#include <xirang2/type/itypebinder.h>

namespace xirang2{ namespace io{ namespace json_obj{
    enum json_content_control{
        jcc_none = 0,
        jcc_type_of_root            = 1,
        jcc_type_of_link            = 1 << 2,
        jcc_embed_link              = 1 << 3,
        jcc_link_version            = 1 << 4,
    };
    
    class XR_INTERFACE json_formatter{
    public:
        virtual std::size_t write(io::writer& wr, type::ConstCommonObject obj, int flag) = 0;
    protected:
        virtual ~json_formatter(){}
    };
    
    XR_EXCEPTION_TYPE(json_formatter_not_found_exception);
    
    XR_API extern json_formatter* find_formatter(const string& name);
    XR_API extern json_formatter* register_formatter(const string& name, json_formatter* fmt);
    XR_API extern json_formatter* unregister_formatter(const string& name);
    XR_API extern void init_default_formatter_once();
    XR_API extern std::size_t write_type(io::writer& wr, type::Type type, int flag);
    XR_API extern std::size_t write_object(io::writer& wr, type::ConstCommonObject obj, int flag);
    XR_API extern string to_json(type::ConstCommonObject obj, int flag);
    XR_API extern string to_json(type::Type type, int flag);
    
    
    namespace{
        struct call_init_default_formatter_once{
            call_init_default_formatter_once(){ init_default_formatter_once();}
        };
        call_init_default_formatter_once k_call_init_default_formatter_once;
    }
    
    struct json_formatter_entry{
        string name;
        json_formatter* formatter;
    };
    
    typedef ForwardRangeT<const_itr_traits<json_formatter_entry> > json_formatter_range;
    XR_API extern json_formatter_range list_formatter();
    
    inline json_formatter& get_formatter(const string& name){
        auto ret = find_formatter(name);
        if (!ret)
            XR_THROW(json_formatter_not_found_exception)(name);
        return *ret;
    }
    
    
    template<typename Ar> struct serializer : public s11n::serializer_base<Ar>{
        explicit serializer(Ar& ar, int flag)
            : s11n::serializer_base<Ar>(ar)
            , m_flag(flag)
        {};
        int flag() const{ return m_flag;}
    protected:
        int m_flag;
    };
    
    template<typename Ar> serializer<Ar> as_sink(Ar& ar, int flag = jcc_none){ return serializer<Ar>(ar, flag);}
    
    template<typename Ar,
    typename =  typename std::enable_if<s11n::is_serializer<Ar>::value>::type>
    Ar& save(Ar& wr, type::ConstCommonObject obj)
    {
        write_object(wr.get(), obj, wr.flag());
        return wr;
    }
    
    
#if 0
    template<typename Ar> struct deserializer : public s11n::deserializer_base<Ar>{
        explicit deserializer(Ar& ar) : s11n::deserializer_base<Ar>(ar){};
    };

    template<typename Ar> deserializer<Ar> as_source(Ar& ar){ return deserializer<Ar>(ar);}
    
    template<typename Ar,
    typename = typename std::enable_if<s11n::is_deserializer<Ar>::value>::type>
    Ar& load(Ar& rd, type::CommonObject obj)
    {
        return rd;
    }
#endif
    
}}}

#endif //end XIRANG2_TYPE_S11N_JSON_H_
