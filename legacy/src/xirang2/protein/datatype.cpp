#include <xirang2/protein/datatype.h>
namespace xirang2{ namespace protein{

	static const file_path K_protein_sys_type_path ("lib/adsk/cm/1.0/sys/type");
	void SetupSysTypes(type::Xirang& xr)
	{
		using namespace xirang2::type;

		Namespace root = xr.root();

		NamespaceBuilder().createChild(K_protein_sys_type_path).adoptChildrenBy(root);
		auto type_ns = root.locateNamespace(K_protein_sys_type_path);
		XR_PRE_CONDITION(type_ns.valid());

		Namespace systype_ns = root.locateNamespace(file_path("/sys/type"));
		Type t_double = systype_ns.locateType(file_path("double"));
		Type t_int32 = systype_ns.locateType(file_path("int32"));
		Type t_int8 = systype_ns.locateType(file_path("int8"));
		//Type t_pointer = systype_ns.locateType(file_path("pointer"));
		Type t_string = systype_ns.locateType(file_path("string"));
		Type t_path = systype_ns.locateType(file_path("path"));
		Type t_static_link = systype_ns.locateType(file_path("link"));
		Type t_dynamic_link = systype_ns.locateType(file_path("dynamic_link"));
		Type t_array = systype_ns.locateType(file_path("array"));
		Type t_string_array = systype_ns.locateType(file_path("run/array_string"));
		Type t_path_array = systype_ns.locateType(file_path("run/array_path"));

		//Type t_double_link =
		TypeBuilder()
			.name(file_path("double_link"))
			.addMember(literal("value"), file_path("double"), t_double)
			.addMember(literal("link"), file_path("dynamic_link"), t_dynamic_link)
			.endBuild()
			.adoptBy(type_ns);

		Type t_color4 = TypeBuilder()
			.name(file_path("color4"))
			.addMember(literal("r"), file_path("double"), t_double)
			.addMember(literal("g"), file_path("double"), t_double)
			.addMember(literal("b"), file_path("double"), t_double)
			.addMember(literal("a"), file_path("double"), t_double)
			.endBuild()
			.adoptBy(type_ns);

		TypeBuilder()
			.name(file_path("array_color4"))
			.modelFrom(t_array)
			.setArg("value_type", t_color4.name(), t_color4)
			.endBuild()
			.adoptBy(type_ns);

		TypeBuilder()
			.name(file_path("color4_link"))
			.addMember(literal("value"), file_path("color4"), t_color4)
			.addMember(literal("link"), file_path("dynamic_link"), t_dynamic_link)
			.endBuild()
			.adoptBy(type_ns);

		Type t_unit =
		TypeBuilder()
			.name(file_path("general_unit"))
			.addMember(literal("unit"), file_path("int32_t"), t_int32)
			.addMember(literal("value"), file_path("double"), t_double)
			.endBuild()
			.adoptBy(type_ns);

		Type t_distance =
		TypeBuilder()
			.name(file_path("unit_distance"))
			.modelFrom(t_unit)
			.endBuild()
			.adoptBy(type_ns);

		//Type t_vector3 =
		TypeBuilder()
			.name(file_path("vector3"))
			.addMember(literal("x"), file_path("double"), t_double)
			.addMember(literal("y"), file_path("double"), t_double)
			.addMember(literal("z"), file_path("double"), t_double)
			.endBuild()
			.adoptBy(type_ns);

		Type t_metadata =
		TypeBuilder()
			.name(file_path("metadata"))
			.addMember(literal("BaseSchema"), file_path("string"), t_string)
			.addMember(literal("Hidden"), file_path("int8"), t_int8)
			.addMember(literal("UIName"), file_path("string"), t_string)
			.addMember(literal("VersionGUID"), file_path("string"), t_string)
			.addMember(literal("category"), file_path("array_string"), t_string_array)
			.addMember(literal("description"), file_path("string"), t_string)
			.addMember(literal("keyword"), file_path("array_string"), t_string_array)
			.addMember(literal("thumbnail"), file_path("array_path"), t_path_array)
			.addMember(literal("tags"), file_path("array_string"), t_string_array)
			.addMember(literal("dict"), file_path("path"), t_path)
			.endBuild()
			.adoptBy(type_ns);

		// Type t_bounds =
		TypeBuilder()
			.name(file_path("bounds"))
			.addMember(literal("has_min"), file_path("int8"), t_int8)
			.addMember(literal("min_inclusive"), file_path("int8"), t_int8)
			.addMember(literal("has_max"), file_path("int8"), t_int8)
			.addMember(literal("max_inclusive"), file_path("int8"), t_int8)
			.addMember(literal("is_percetage"), file_path("int8"), t_int8)
			.addMember(literal("min"), file_path("double"), t_double)
			.addMember(literal("max"), file_path("double"), t_double)
			.addMember(literal("increment"), file_path("double"), t_double)
			.endBuild()
			.adoptBy(type_ns);

		Type t_bounds_value = TypeBuilder()
			.name(file_path("bounds_value"))
			.setArg(literal("value_type"), file_path(""), Type())
			.addMember(literal("value"), file_path("value_type"), Type())
			.endBuild()
			.adoptBy(type_ns);

		Type t_double_bounds =
		TypeBuilder()
			.name(file_path("double_bounds"))
			.modelFrom(t_bounds_value)
			.setArg("value_type", t_double.name(), t_double)
			.endBuild()
			.adoptBy(type_ns);

		TypeBuilder()
			.name(file_path("int_bounds"))
			.modelFrom(t_bounds_value)
			.setArg("value_type", t_int32.name(), t_int32)
			.endBuild()
			.adoptBy(type_ns);

		TypeBuilder()
			.name(file_path("unit_bounds"))
			.modelFrom(t_bounds_value)
			.setArg("value_type", t_unit.name(), t_unit)
			.endBuild()
			.adoptBy(type_ns);

		TypeBuilder()
			.name(file_path("distance_bounds"))
			.modelFrom(t_bounds_value)
			.setArg("value_type", t_distance.name(), t_distance)
			.endBuild()
			.adoptBy(type_ns);

		TypeBuilder()
			.name(file_path("double_link_bounds"))
			.addMember(literal("value"), file_path("double_bounds"), t_double_bounds)
			.addMember(literal("link"), file_path("dynamic_link"), t_dynamic_link)
			.endBuild()
			.adoptBy(type_ns);



		// Type t_choice_value =
		TypeBuilder()
			.name(file_path("choice_value"))
			.setArg(literal("enum"), file_path(""), Type())
			.addMember(literal("value"), file_path("int32"), t_int32)
			.endBuild()
			.adoptBy(type_ns);

		TypeBuilder()
			.name(file_path("meta_link"))
			.modelFrom(t_static_link)
			.setArg(literal("value_type"), file_path("metadata"), t_metadata)
			.endBuild()
			.adoptBy(type_ns);
	}
}}

