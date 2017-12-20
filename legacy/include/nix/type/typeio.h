#ifndef XIRANG2_TYPE_TYPE_IO_H__
#define XIRANG2_TYPE_TYPE_IO_H__
#include <xirang2/type/namespace.h>
#include <xirang2/type/typebinder.h>

namespace xirang2{ namespace type{
	XR_API extern void saveType(io::writer&wr, Type t);
	XR_API extern Namespace getOrCreateNamespace(Namespace root, sub_file_path path);
	///\note This save method will save type reference only.
	template<typename Ar> Ar& save(Ar& ar, Type t){
		ar & uint8_t(t.valid()? 1 : 0);
		if (t.valid()){
			ar & t.fullName().str() & t.version();
		}
		return ar;
	}

	/// \note the data member layout must be in accord with save and saveType
	struct TypeRefData{
		uint8_t valid = 0;
		file_path fullName;
		version_type version;
	};

	XR_EXCEPTION_TYPE(failed_to_located_type_exception);
	template<typename Ar> Ar& load(Ar& ar, TypeRefData& t){
		ar & t.valid;
		if (t.valid){
			ar & t.fullName & t.version;
		}
		return ar;
	}

	template<> struct constructor<Type>{
		static void apply(CommonObject obj, heap& hp, ext_heap& ehp);
	};

	template<> struct hasher<Type> {
		static size_t apply(ConstCommonObject obj);
	};

	template<> struct comparison<Type> {
		static int apply(ConstCommonObject lhs,ConstCommonObject rhs) ;
	};

	template<> struct serializer<Type> {
		static void apply(io::writer& wr, ConstCommonObject obj);
	};
	template<> struct deserializer<Type> {
		static void apply(io::reader& rd, CommonObject obj, heap& inner, ext_heap& outer);
	};


	struct ITypeSaver{
		virtual ~ITypeSaver(){}
		virtual void saveType(Type t) = 0;
		virtual iauto<io::writer> openWrite(sub_file_path type_path_in_runtime, const version_type& ver) = 0;
		virtual void onDepends(Type t) = 0;
	};

	struct XR_INTERFACE ITypeLoader{		// for different formats: binary, text, json, xml etc.
		virtual ~ITypeLoader(){}

		virtual Type load(sub_file_path type_file_in_fs, const version_type& ver, Namespace temp_root) = 0 ;
		virtual iauto<io::reader> openRead(sub_file_path type_file_in_fs, const version_type& ver) = 0;
	};
	XR_API extern Type load(ITypeLoader& loader, sub_file_path type_file_in_fs, Namespace temp_root);

#if 0	// not implemented
	struct XR_API BinaryTypeSaver : public ITypeSaver{
		BinaryTypeSaver(vfs::RootFs& fs);

		virtual void saveType(Type t);
		virtual iauto<io::writer> openWrite(sub_file_path type_path_in_runtime, const version_type& ver);
		virtual void onDepends(Type t);
	};
#endif
	struct XR_API BinaryTypeLoader : public ITypeLoader{
		BinaryTypeLoader(vfs::RootFs& fs, Namespace reference );

		virtual Type load(sub_file_path type_file_in_fs, const version_type& ver, Namespace temp_root);
		virtual iauto<io::reader> openRead(sub_file_path type_file_in_fs, const version_type& ver);
		private:
		vfs::RootFs& fs_;
		Namespace root_;
	};

	XR_EXCEPTION_TYPE(fail_to_load_type_exception);
	XR_EXCEPTION_TYPE(unrecognized_type_format_exception);
	XR_EXCEPTION_TYPE(type_version_mismatch_exception);
}}
#endif //end XIRANG2_TYPE_TYPE_IO_H__

