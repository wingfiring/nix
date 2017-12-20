#include <xirang2/type/typebinder.h>
#include <xirang2/type/type.h>
#include <xirang2/type/typeio.h>
#include <xirang2/serialize/versiontype.h>
#include <xirang2/serialize/sha1.h>
#include <xirang2/serialize/s11nbasetype.h>
#include <xirang2/serialize/path.h>
#include <xirang2/vfs.h>
#include <xirang2/type/nativetypeversion.h>

#include <xirang2/io/file.h>

//STL
#include <vector>

namespace xirang2{ namespace type{

		struct TypeMember{
			string name;
			file_path typeName;
			TypeRefData type;
		};

		struct TypeData{
			TypeRefData modelType;

			std::vector<TypeMember> args;
			std::vector<TypeMember> members;
			string internalID;

			TypeRefData staticDataType;
		};

	void constructor<Type>:: apply(CommonObject, heap& , ext_heap& ){
		XR_INVARIANT(false && "Must not create a type");
	}

	int comparison<Type>::apply(ConstCommonObject lhs,ConstCommonObject rhs) {
		return (*Ref<Type>(lhs)).compare(*Ref<Type>(rhs));
	}

	void serializer<Type>::apply(io::writer& wr, ConstCommonObject obj){
		auto s = io::exchange::as_sink(wr);
		save(s, *Ref<Type>(obj));
	}
	void deserializer<Type>::apply(io::reader& rd, CommonObject obj, heap& /*inner*/, ext_heap& /*outer*/){
		auto & t = *Ref<Type>(obj);

		auto s = io::exchange::as_source(rd);
		TypeRefData data;
		s & data;
		if (data.valid == 1){
			auto type = obj.type().parent().root().findRealType(data.fullName, data.version);
			if (!type.valid())
				XR_THROW(failed_to_located_type_exception);
			t = type;
		}
		else
			t = Type();
	}


	size_t hasher<Type>::apply(ConstCommonObject obj){
		auto& data = uncheckBind<Type>(obj);
		return (size_t)data.m_imp;
	}


	void saveType(io::writer& wr, Type t){
		XR_PRE_CONDITION(t.valid());
		auto s = io::exchange::as_sink(wr);

		s & uint8_t(1);			// IMPORTANT! type serialized version is fixed to 1. it'll only be changed if we use new serialized structure in the future.

		s & t.methods().internalID();
		s & t.model();

		save_size_t(s, t.args().size());
		for (auto& i : t.args())
			s & i.name() & i.typeName() & i.type();

		save_size_t(s, t.members().size());
		for (auto& i : t.members())
			s & i.name() & i.typeName() & i.type();

		s & t.staticType();
		auto static_data = t.staticData();
		uint8_t has_data_flag = static_data.valid() ? 1 : 0;
		s & has_data_flag;
		if (has_data_flag == 1){
			static_data.type().methods().serialize(wr, static_data);
		}

		auto prototype = t.prototype();
		uint8_t has_prototype_flag = prototype.valid() ? 1 : 0;
		s & has_prototype_flag;
		if (has_prototype_flag == 1){
			t.methods().serialize(wr, prototype);
		}
	}


	void loadTypeData(io::reader& rd, TypeData& d){
		auto s = io::exchange::as_source(rd);

		if (load<uint8_t>(s) != 1)
			XR_THROW(unrecognized_type_format_exception);

		s & d.internalID;
		s & d.modelType;

		auto args = load_size_t(s);
		while(args--){
			TypeMember m;
			s & m.name & m.typeName & m.type;
			d.args.push_back(m);
		}
		args = load_size_t(s);
		while(args--){
			TypeMember m;
			s & m.name & m.typeName & m.type;
			d.members.push_back(m);
		}
		s & d.staticDataType;
	}


	BinaryTypeLoader::BinaryTypeLoader(vfs::RootFs& fs, Namespace reference)
		: fs_(fs), root_(reference)
	{}
	Namespace getOrCreateNamespace(Namespace root, sub_file_path path){
		XR_PRE_CONDITION(path.is_absolute());
		auto itr = path.begin(); ++itr;	//skip root;
		auto end = path.end();
		Namespace ret = root;
		for (; itr != end && ret.valid(); ++itr){
			auto res = ret.findNamespace(*itr);
			if (!res.valid()){
				ret = NamespaceBuilder().name(*itr).adoptBy(ret);
			}
			else
				ret = res;
		}
		return ret;
	}

	Type BinaryTypeLoader::load(sub_file_path type_file_in_fs, const version_type& ver, Namespace temp_root){
		auto type = root_.locateRealType(type_file_in_fs, ver);
		if (type.valid())	return type;

		type = temp_root.locateRealType(type_file_in_fs, ver);
		if (type.valid())	return type;


        auto rd = openRead(type_file_in_fs, ver);
        TypeData data;
		auto& reader = rd.get<io::reader>();
		loadTypeData(reader, data);

		TypeBuilder tb;
		tb.name(type_file_in_fs.filename());
		if (data.modelType.valid){
			Type model_type = load(data.modelType.fullName, data.modelType.version, temp_root);
			XR_POST_CONDITION(model_type.valid());
			tb.modelFrom(model_type);
		}

		for (auto&i : data.args){
			Type t;
			if (i.type.valid)
				t = load(i.type.fullName, i.type.version, temp_root);
			tb.setArg(i.name, i.typeName, t);
		}
		// XXX: if it has model, all members has been created by tb.modelFrom,
		// so may we don't need to store the members if type has a model type.
		// currently, it's unreasonable to add member for a generated type
		if (!data.modelType.valid){
			for (auto&i : data.members){
				Type t;
				if (i.type.valid)
					t = load(i.type.fullName, i.type.version, temp_root);
				tb.addMember(i.name, i.typeName, t);
			}
		}

		if (data.staticDataType.valid){
			Type static_type = load(data.staticDataType.fullName, data.staticDataType.version, temp_root);
			XR_POST_CONDITION(static_type.valid());
			tb.setStaticType(static_type);
		}
		
		auto ret = tb.get();

		auto s = io::exchange::as_source(reader);
		if (io::load<uint8_t>(s) == 1){	//has static_data
			auto static_data = tb.createStaticData();
			ret.staticType().methods().deserialize(reader, static_data, memory::get_global_heap(), memory::get_global_ext_heap());
		}
		auto target_ns = getOrCreateNamespace(temp_root, type_file_in_fs.parent());
		if (io::load<uint8_t>(s) == 1){	// has prototype
			auto prototype = tb.createPrototype();
			ret.methods().deserialize(reader, prototype, memory::get_global_heap(), memory::get_global_ext_heap());
		}
        tb.endBuild();
		tb.adoptBy(target_ns);		// before get prototype, the type must be built finished and adopted by some namespace.
        if (ret.version() != ver)
			XR_THROW(type_version_mismatch_exception);
        

		return ret;
	}

	iauto<io::reader> BinaryTypeLoader::openRead(sub_file_path type_file_in_fs, const version_type&  ver){
        file_path path(type_file_in_fs);
        if (!is_empty(ver.id)){
            auto str_ver = sha1_to_string(ver.id);
            path.replace_version(sub_file_path(str_ver));
        }
		auto ret = fs_.create<io::reader>(path, io::of_open);
		if (!ret.valid())
			XR_THROW(fail_to_load_type_exception);
		return ret;
	}
}}
