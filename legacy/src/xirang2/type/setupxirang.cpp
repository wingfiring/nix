#include <xirang2/type/xirang.h>
#include <xirang2/type/typebinder.h>
#include <xirang2/type/nativetype.h>
#include <xirang2/type/array.h>
#include <xirang2/type/itypebinder.h>
#include <xirang2/type/nativetypeversion.h>
#include <xirang2/serialize/path.h>
#include <xirang2/type/typeio.h>
#include <xirang2/type/objectlink.h>
#include <xirang2/type/primitivetype.h>

#include <assert.h>
namespace xirang2{ namespace type{

    namespace {
	    template<typename T>
        TypeMethods* getPrimitiveMethods(const string& str)
        {
            static PrimitiveMethods<T> methods(str);
            return &methods;
        }

        PrimitiveMethods<bool> methods_bool(".sys.type.bool");
        PrimitiveMethods<float> methods_float(".sys.type.float");
        PrimitiveMethods<double> methods_double(".sys.type.double");

        PrimitiveMethods<int8_t> methods_int8_t(".sys.type.int8");
        PrimitiveMethods<uint8_t> methods_uint8_t(".sys.type.uint8");
        PrimitiveMethods<int16_t> methods_int16_t(".sys.type.int16");
        PrimitiveMethods<uint16_t> methods_uint16_t(".sys.type.uint16");
        PrimitiveMethods<int32_t> methods_int32_t(".sys.type.int32");
        PrimitiveMethods<uint32_t> methods_uint32_t(".sys.type.uint32");
        PrimitiveMethods<int64_t> methods_int65_t(".sys.type.int64");
        PrimitiveMethods<uint64_t> methods_uint64_t(".sys.type.uint64");

        PrimitiveMethods<byte> methods_byte(".sys.type.byte");

        PrimitiveMethods<void*> methods_void_pointer(".sys.type.pointer");
        PrimitiveMethods<const void*> methods_const_void_pointer(".sys.type.const_pointer");

        PrimitiveMethods<string> methods_string(".sys.type.string");
        PrimitiveMethods<file_path> methods_file_path(".sys.type.path");
        PrimitiveMethods<Type> methods_type(".sys.type.type");
        PrimitiveMethods<DynamicLink> methods_dynamic_link(".sys.type.dynamic_link");

        PrimitiveMethods<byte_buffer> methods_byte_buffer(".sys.type.byte_buffer");

		struct TypeDesc
		{
			file_path name;
			TypeMethods* methods;
		};

		const TypeDesc g_systype_table[] =
		{
			{file_path("bool"), 	&methods_bool},

			{file_path("float"),	&methods_float},
			{file_path("double"),	&methods_double },

			{file_path("int8"), 	&methods_int8_t},
			{file_path("uint8"), 	&methods_uint8_t },
			{file_path("int16"),	&methods_int16_t },
			{file_path("uint16"),	&methods_uint16_t },
			{file_path("int32"),	&methods_int32_t },
			{file_path("uint32"),	&methods_uint32_t },
			{file_path("int64"),	&methods_int65_t },
			{file_path("uint64"),	&methods_uint64_t },

			{file_path("byte"),	&methods_byte },

			{file_path("pointer"),	&methods_void_pointer },
			{file_path("const_pointer"),	&methods_const_void_pointer },

			{file_path("string"),	&methods_string },
			{file_path("path"),		&methods_file_path },
			{file_path("type"),		&methods_type },
			{file_path("dynamic_link"),		&methods_dynamic_link },

            {file_path("byte_buffer"), &methods_byte_buffer },

            {file_path(""),0}
		};

        PrimitiveMethods<type_dir> methods_type_dir(".sys.type.fs.dir");
        PrimitiveMethods<type_file> methods_type_file(".sys.type.fs.file");
        PrimitiveMethods<type_unknown> methods_type_unknown(".sys.type.unknown");

        const TypeDesc g_systype_noalias_table[] = {

            { file_path("type_dir"), &methods_type_dir},
            { file_path("type_file"), &methods_type_file},
            { file_path("type_unknown"), &methods_type_unknown},

            { file_path(""),0 }
        };

        struct AliasPair{
			file_path name;
			file_path type;
		};
		AliasPair g_alias_table[] =
		{
            {file_path("short"), 		file_path("/sys/type/int16")},
            {file_path("ushort"), 	file_path("/sys/type/uint16")},
            {file_path("int"), 		file_path("/sys/type/int32")},
            {file_path("uint"), 		file_path("/sys/type/uint32")},
            {file_path("long"), 		file_path("/sys/type/int64")},
            {file_path("ulong"), 		file_path("/sys/type/uint64")},
            {file_path("llong"), 		file_path("/sys/type/int64")},
            {file_path("ullong"), 	file_path("/sys/type/uint64")},
		};
        PrimitiveMethods<Array> methods_array(".sys.type.array");
	}
    
	XR_API void SetupXirang(Xirang& xr)
	{
        Namespace root = xr.root();

        Namespace sys = NamespaceBuilder().name(file_path("sys")).adoptBy(root);
        Namespace typeNs = NamespaceBuilder().name(file_path("type")).adoptBy(sys);

		for (const TypeDesc* itr(g_systype_table); !itr->name.empty(); ++itr)
		{
            Type t = TypeBuilder(itr->methods)
                .name(itr->name)
                .endBuild()
                .adoptBy(typeNs);

			if (!itr->name.empty())
				TypeAliasBuilder()
					.name(itr->name)
					.typeName(t.fullName())
					.setType(t)
					.adoptBy(root);
		}

        for (const TypeDesc* itr(g_systype_noalias_table); !itr->name.empty(); ++itr)
        {
            TypeBuilder(itr->methods)
                .name(itr->name)
                .endBuild()
                .adoptBy(typeNs);
        }
        

		//static reference type
		{
            Type t = TypeBuilder()
                .name(file_path("ref"))
                .setArg(literal("ref_type"), file_path(""), Type())
                .addMember(literal("to"), file_path("pointer"), typeNs.findType(file_path("pointer")))
                .endBuild()
                .adoptBy(typeNs);

            TypeAliasBuilder()
                .name(file_path("ref"))
                .typeName(file_path("/sys/type/ref"))
                .setType(t)
                .adoptBy(root);
		}

		//dynamic reference type
		{
            Type t = TypeBuilder()
                .name(file_path("dynamic_ref"))
                .addMember(literal("type"), file_path("type"), typeNs.findType(file_path("type")))
                .addMember(literal("to"), file_path("pointer"), typeNs.findType(file_path("pointer")))
                .endBuild()
                .adoptBy(typeNs);
			XR_POST_CONDITION(t.isComplete());

            TypeAliasBuilder()
                .name(file_path("dynamic_ref"))
                .typeName(file_path("/sys/type/dynamic_ref"))
                .setType(t)
                .adoptBy(root);
		}
		//static link type
		{
            Type t = TypeBuilder()
                .name(file_path("link"))
                .setArg(literal("value_type"), file_path(""), Type())
                .addMember(literal("path"), file_path("path"), typeNs.findType(file_path("path")))
                .addMember(literal("to"), file_path("pointer"), typeNs.findType(file_path("pointer")))
                .endBuild()
                .adoptBy(typeNs);

            TypeAliasBuilder()
                .name(file_path("link"))
                .typeName(file_path("/sys/type/link"))
                .setType(t)
                .adoptBy(root);
		}

		{
            Type t = TypeBuilder(&methods_array)
                .name(file_path("array"))
                .setArg(literal("value_type"), file_path(""), Type())
                .endBuild()
                .adoptBy(typeNs);

            TypeAliasBuilder()
                .name(file_path("array"))
                .typeName(file_path("/sys/type/array"))
                .setType(t)
                .adoptBy(root);
		}
		{
			for (auto& i : g_alias_table){
				Type t = root.locateType(i.type);
				XR_PRE_CONDITION(t.valid());
				TypeAliasBuilder()
					.name(i.name)
					.typeName(i.type)
					.setType(t)
					.adoptBy(root);
			}
		}
		{
			Namespace typeRun = NamespaceBuilder().name(file_path("run")).adoptBy(typeNs);
			Type t_array = typeNs.findType(file_path("array"));
			for (auto ts : typeNs.types()){
				Type t = ts.current();
				if (t.isComplete()){
					TypeBuilder()
						.name(file_path(string(literal("array_") << t.name().str())))
						.modelFrom(t_array)
						.setArg("value_type", t.name(), t)
						.endBuild()
						.adoptBy(typeRun);
				}
			}
		}
	}
}}

