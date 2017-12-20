#ifndef XIRANG2_PROTEIN_DATATYPE_H__
#define XIRANG2_PROTEIN_DATATYPE_H__
#include <xirang2/type/itypebinder.h>
#include <xirang2/type/xirang.h>
#include <xirang2/type/array.h>
#include <xirang2/type/type.h>
#include <xirang2/type/objectlink.h>

namespace xirang2{ namespace protein{
	struct color4{
		double r,g,b,a;
	};

	struct general_unit{
		int32_t unit;
		double value;
	};
	struct unit_distance{
		int32_t unit;
		double value;
	};

	struct vector3{
		double data[3];
	};

	struct bounds{
		int8_t has_min = 0, min_inclusive = 0, has_max = 0, max_inclusive = 0, is_percetage = 0;
		double min = 0, max = 0, increment = 1;
	};

	struct choice_value{
		int value;
	};

	struct color4_link{
		color4	value;
		type::DynamicLink link;
	};

	struct scalar_link{
		double	value;
		type::DynamicLink link;
	};

	struct bounds_value{
		type::CommonObject obj;
		explicit bounds_value(type::CommonObject obj_) : obj(obj_){}
		public:
		type::Type type() const{ return obj.type();}
		type::Type valueType()const{ return obj.getMember(0).type();}
		type::CommonObject getValue()const{ return obj.getMember(0).asCommonObject();}
		void setValue(type::CommonObject obj_){
			XR_PRE_CONDITION(obj_.valid() && obj_.type() == valueType());
			getValue().assign(obj_);
		}
		template<typename Type, typename ObjType> friend class type::ObjectRefModelBase;
	};

	struct const_bounds_value{
		type::ConstCommonObject obj;
		explicit const_bounds_value(type::ConstCommonObject obj_) : obj(obj_){}
		public:
		type::Type type() const{ return obj.type();}
		type::Type valueType()const{ return obj.type().arg(0).type();}
		type::ConstCommonObject getValue()const{ return type::ConstCommonObject(valueType(), obj.data());}
		template<typename Type, typename ObjType> friend class type::ObjectRefModelBase;
	};
    
    // Defined but not used types of Protein's
    
    struct color3{
        double r,g,b;
    };
    struct matrix44{
        double data[16];
    };
    
    struct vector4{
        double data[4];
    };

#if 0
	struct metadata_file{
		uint64_t size;
		uint64_t creation_time;
		uint32_t mime;
		uint32_t type;
		string name;
	};
#endif

	struct metadata{
		string schema;
		int8_t hidden;
		string 	UIName;
		string	VersionGUID;		// sys/type/run/array_string
		type::Array 	category;		// sys/type/run/array_string
		string 	description;
		type::Array	keyword;		// sys/type/run/array_string
		type::Array	thumbnail;		// sys/type/run/array_string
		type::Array	tags;
		file_path 	dict;
	};

	void XR_API SetupSysTypes(type::Xirang& xr);
}}
namespace xirang2{ namespace type{
	SPECIALIZE_OBJECT_REF_ALL(protein::bounds_value, ObjectRefModelBase);
	SPECIALIZE_CONST_OBJECT_REF(protein::const_bounds_value, ObjectRefModelBase);

	XIRANG2_DEFINE_TYPE_VERSION_OF("4ef9e60c04587ce047b0fd17f189c110a166d22c",             protein::bounds, ".sys.type..generic");
	XIRANG2_DEFINE_TYPE_VERSION_OF("d8edd5a4ceda2dfe6d0fd34b896b8ba8bafc4a8f",       protein::bounds_value, ".sys.type..generic");
	XIRANG2_DEFINE_TYPE_VERSION_OF("d8edd5a4ceda2dfe6d0fd34b896b8ba8bafc4a8f", protein::const_bounds_value, ".sys.type..generic");
	XIRANG2_DEFINE_TYPE_VERSION_OF("a1aeb584c57688a7f17a76180a20141a2db5def8",             protein::color4, ".sys.type..generic");
	XIRANG2_DEFINE_TYPE_VERSION_OF("4dc47d64448688c49d2560d068ab9bc64aac7d5a",       protein::general_unit, ".sys.type..generic");
	XIRANG2_DEFINE_TYPE_VERSION_OF("1d846240395e372c3d6a29cc405b6d3c96242537",           protein::metadata, ".sys.type..generic");  // TODO: remove all
	XIRANG2_DEFINE_TYPE_VERSION_OF("608831e92746f704b5cbf5776c4d5f4e47756552",            protein::vector3, ".sys.type..generic");
	XIRANG2_DEFINE_TYPE_VERSION_OF("89c624c10fe4cd23caf8cb90bb02deb372c7aed9",      protein::unit_distance, ".sys.type..generic");

}}

#endif //end XIRANG2_PROTEIN_DATATYPE_H__

