#ifndef XIRANG2_TYPE_OBJECT_LINK_H__
#define XIRANG2_TYPE_OBJECT_LINK_H__
#include <xirang2/type/itypebinder.h>
#include <xirang2/type/binder.h>
#include <xirang2/serialize/path.h>

namespace xirang2{ namespace type{
	struct StaticLinkImp{
		file_path path;
		void* data;
		StaticLinkImp() : data(0){}
	};

	class StaticLinkRef{
		CommonObject obj_;
		StaticLinkImp& getImp_() const{
			return *static_cast<StaticLinkImp*>(obj_.data());
		}
		explicit StaticLinkRef(CommonObject obj) : obj_(obj){}
		public:
		file_path& path() const{ return getImp_().path;}
		Type type() const{ return obj_.type();}
		Type targetType() const{ return type().arg(0).type();}
		CommonObject getTarget() const{ return CommonObject(targetType(), getImp_().data);}
		void setValue(CommonObject obj, const file_path& path) const{
			XR_PRE_CONDITION(obj.valid() && obj.type() == targetType());
			getImp_().data = obj.data();
			getImp_().path = path;
		}
		template<typename Type, typename ObjType> friend class ObjectRefModelBase;
	};

	class ConstStaticLinkRef{
		ConstCommonObject obj_;
		const StaticLinkImp& getImp_() const{
			return *static_cast<const StaticLinkImp*>(obj_.data());
		}
		explicit ConstStaticLinkRef(ConstCommonObject obj) : obj_(obj){}
		public:
		const file_path& path() const{ return getImp_().path;}
		Type type() const{ return obj_.type();}
		Type targetType() const{ return type().arg(0).type();}
		ConstCommonObject getTarget() const{ return ConstCommonObject(targetType(), obj_.data());}
		template<typename Type, typename ObjType> friend class ObjectRefModelBase;
	};

SPECIALIZE_OBJECT_REF(StaticLinkRef, ObjectRefModelBase);
SPECIALIZE_CONST_OBJECT_REF(ConstStaticLinkRef, ObjectRefModelBase);

	template<> struct hasher<StaticLinkImp> {
		static size_t apply(ConstCommonObject obj){
			return static_cast<const StaticLinkImp*>(obj.data())->path.str().hash();
		}
	};

	template<> struct comparison<StaticLinkImp> {
		static int apply(ConstCommonObject lhs,ConstCommonObject rhs) {
			auto& lpath = static_cast<const StaticLinkImp*>(lhs.data())->path;
			auto& rpath = static_cast<const StaticLinkImp*>(rhs.data())->path;
			return lpath < rpath
				? -1
				: lpath < rpath
				? 1
				: 0;
		}
	};

	template<> struct serializer<StaticLinkImp> {
		static void apply(io::writer& wr, ConstCommonObject obj){
			auto s = io::exchange::as_sink(wr);
			s & static_cast<const StaticLinkImp*>(obj.data())->path;
		}
	};
	template<> struct deserializer<StaticLinkImp> {
		static void apply(io::reader& rd, CommonObject obj, heap& /*inner*/, ext_heap& /*outer*/){
			auto s = io::exchange::as_source(rd);
			s & static_cast<StaticLinkImp*>(obj.data())->path;
		}
	};

	struct DynamicLink{
		file_path path;
		CommonObject target;
	};

	template<> struct hasher<DynamicLink> {
		static size_t apply(ConstCommonObject obj){
			return static_cast<const DynamicLink*>(obj.data())->path.str().hash();
		}
	};

	template<> struct comparison<DynamicLink> {
		static int apply(ConstCommonObject lhs,ConstCommonObject rhs) {
			auto& lpath = static_cast<const DynamicLink*>(lhs.data())->path;
			auto& rpath = static_cast<const DynamicLink*>(rhs.data())->path;
			return lpath < rpath
				? -1
				: lpath < rpath
				? 1
				: 0;
		}
	};

	template<> struct serializer<DynamicLink> {
		static void apply(io::writer& wr, ConstCommonObject obj){
			auto s = io::exchange::as_sink(wr);
			s & static_cast<const DynamicLink*>(obj.data())->path;
		}
	};

	template<> struct deserializer<DynamicLink> {
		static void apply(io::reader& rd, CommonObject obj, heap& , ext_heap& ){
			auto s = io::exchange::as_source(rd);
			s & static_cast<DynamicLink*>(obj.data())->path;
		}
	};
}}
#endif //end XIRANG2_TYPE_OBJECT_LINK_H__


