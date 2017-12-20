#include <xirang2/type/typebinder.h>
#include <xirang2/type/type.h>
#include <xirang2/type/typeio.h>
#include <xirang2/serialize/versiontype.h>
#include <xirang2/serialize/sha1.h>
#include <xirang2/serialize/s11nbasetype.h>

#include <cstring>

#include <boost/functional/hash.hpp>

#include <iostream>

namespace xirang2 { namespace type{

	namespace {
		TypeMethods defaultMethods;
		std::size_t align_lcm(std::size_t p1, std::size_t p2)
		{
			return p1 < p2 ? p2 : p1;
		}
	}

	version_type calculateObjVersion(ConstCommonObject obj){
		XR_PRE_CONDITION(obj.valid());
		sha1 sha;
		iref<io::writer> sink(sha);
		obj.type().methods().serialize(sink.get<io::writer>(), obj);

		version_type ver;
		ver.id = sha.get_digest();
		return ver;
	}

	version_type calculateTypeVersion(Type t){
		sha1 sha;
		iref<io::writer> sink(sha);
		saveType(sink.get<io::writer>(), t);
		return version_type(sha.get_digest());
	}

	TypeMethods& DefaultMethods()
	{
		return defaultMethods;
	}

	void TypeMethods::construct(CommonObject obj, heap& al, ext_heap& eh) const
	{
		XR_PRE_CONDITION(obj.valid());

		Type t = obj.type();
		byte* p = reinterpret_cast<byte*>(obj.data());

		char* pos = reinterpret_cast<char*>(p);
		TypeItemRange members = t.members();
		for (TypeItemRange::iterator itr(members.begin()); itr != members.end(); ++itr)
		{
			Type ti = (*itr).type();
			XR_PRE_CONDITION(ti.valid() && ti.isMemberResolved());
			ti.methods().construct(CommonObject(ti, pos + (*itr).offset()), al, eh);
		}
	}

	void TypeMethods::destruct(CommonObject obj) const
	{
		XR_PRE_CONDITION(obj.valid());

		Type t = obj.type();

		if (t.isPod())
			return;

		SubObjRange members = obj.members();
		typedef std::reverse_iterator<SubObjRange::iterator> iterator_type;
		for (SubObjRange::iterator itr (iterator_type(members.end())), end(iterator_type(members.begin()));
				itr != end; ++itr)
		{
			SubObject ti = *itr;
			Type t2 = ti.type();
			if (!t2.isPod())
				t2.methods().destruct(ti.asCommonObject());
		}
	}

	TypeMethods::~TypeMethods()
	{
	}

	void TypeMethods::assign(ConstCommonObject src, CommonObject dest) const
	{
		XR_PRE_CONDITION(src.valid() && dest.valid() );
		XR_PRE_CONDITION(src.type() == dest.type() );

		Type t = src.type();

		if (t.isPod())
		{
			std::memcpy(dest.data(), src.data(), t.payload());
			return;
		}

		ConstSubObjRange rng_src = src.members();
		SubObjRange rng_dest = dest.members();

		ConstSubObjRange::iterator itr_src = rng_src.begin();
		SubObjRange::iterator itr_dest = rng_dest.begin();
		for (; itr_src != rng_src.end(); ++itr_src, ++itr_dest)
		{
			ConstSubObject tis = *itr_src;
			SubObject tid = *itr_dest;
			tid.asCommonObject().assign( tis.asCommonObject());
		}
	}

	void TypeMethods::beginLayout(std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
	{
		payload = 0;
		offset = 0;
		align = 1;
		pod = true;
	}
	void TypeMethods::nextLayout(TypeItem& item, std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
	{
		if (payload != Type::no_size)
		{
			if (!item.type().valid())
			{
				payload = Type::no_size;
				offset = Type::no_size;
				align = Type::no_size;
			}
			else
			{
				XR_PRE_CONDITION(item.type().isComplete());

				std::size_t padding = payload % item.type().align();
				if (padding != 0)
					padding = item.type().align() - padding;
				payload += item.type().payload() + padding;
				offset = payload;
				pod = pod && item.type().isPod();
				align = align_lcm(align, item.type().align());
			}
		}
	}
	void TypeMethods::serialize(io::writer& wr, ConstCommonObject obj){
		XR_PRE_CONDITION(obj.valid());
		XR_PRE_CONDITION(&obj.type().methods() == this );

		for (auto &i : obj.members()){
			i.type().methods().serialize(wr, i.asCommonObject());
		}
	}
	void TypeMethods::deserialize(io::reader& rd, CommonObject obj, heap& inner, ext_heap& outer){
		XR_PRE_CONDITION(obj.valid());
		XR_PRE_CONDITION(&obj.type().methods() == this );

		for (auto &i : obj.members()){
			i.type().methods().deserialize(rd, i.asCommonObject(), inner, outer);
		}
	}
    int generic_compare(ConstCommonObject lhs,ConstCommonObject rhs){
        XR_PRE_CONDITION(lhs.valid() && rhs.valid() );
        XR_PRE_CONDITION(lhs.type() == rhs.type() );
        
        ConstSubObjRange rng_left = lhs.members();
        ConstSubObjRange rng_right = rhs.members();
        
        int result = 0;
        auto itr_left = rng_left.begin();
        auto itr_right = rng_right.begin();
        for (; itr_left != rng_left.end(); ++itr_left, ++itr_right)
        {
            auto item_left = itr_left->asCommonObject();
            auto item_right = itr_right->asCommonObject();
            auto extension = item_left.type().methods().extension();
            XR_PRE_CONDITION(extension->compare);
            result = extension->compare(item_left, item_right);
            if (result != 0)
                break;
        }
        return result;
    }
    
    /// return the hash code for given object
    std::size_t generic_hash(ConstCommonObject obj){
        XR_PRE_CONDITION(obj.valid());
        
        std::size_t hash = 2166136261U;
        ConstSubObjRange members = obj.members();
        for (auto& item : obj.members()){
            auto sub_obj = item.asCommonObject();
            auto extension = sub_obj.type().methods().extension();
            
            XR_PRE_CONDITION(extension && extension->hash);
            hash = hash * 16777619U ^ extension->hash(sub_obj);
        }
        
        return hash;
    }
    
    const MethodsExtension K_generic_extension = {
        &generic_compare,
        &generic_hash
    };
    
	const MethodsExtension* TypeMethods::extension() const
	{
		return &K_generic_extension;
	}

	static const string K_GenericType(".sys.type..generic");
	const string& TypeMethods::internalID() const{
		return K_GenericType;
	}
	bool TypeMethods::isPrimitive() const {
		return false;
	}

	size_t hasher<string>::apply(ConstCommonObject obj)
	{
		const string& v = uncheckBind<string>(obj);
		return v.hash();
	}
}}

