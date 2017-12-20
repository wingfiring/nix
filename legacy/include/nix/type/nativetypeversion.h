#ifndef XIRANG2_TYPE_NATIVE_TYPE_VERSION_OF_H__
#define XIRANG2_TYPE_NATIVE_TYPE_VERSION_OF_H__
#include <xirang2/type/itypebinder.h>
#include <xirang2/type/array.h>
#include <xirang2/type/type.h>
#include <xirang2/type/objectlink.h>

#include <xirang2/serialize/buffer.h>
#include <xirang2/serialize/string.h>

namespace xirang2{ namespace type{
	XIRANG2_DEFINE_TYPE_VERSION_OF("e92c627a87cbb999142e5c40868d4978e9a2c048",              Array, ".sys.type.array");
	XIRANG2_DEFINE_TYPE_VERSION_OF("8f6f2494c348158f7c0228a71fa644a7a2f3d142",               bool, ".sys.type.bool");
	XIRANG2_DEFINE_TYPE_VERSION_OF("38808b928d5b1725623493b71315a296a42d9255",               byte, ".sys.type.byte");
	XIRANG2_DEFINE_TYPE_VERSION_OF("53f9c90a60a43ad96331e23dfed079309752ea6f",        byte_buffer, ".sys.type.byte_buffer");
	XIRANG2_DEFINE_TYPE_VERSION_OF("19720db6e59c96e10a443af2ed9d179a5b125506",        const void*, ".sys.type.const_pointer");
	XIRANG2_DEFINE_TYPE_VERSION_OF("f388dcc9a036c7fe98e67e91f91237eff4944fd7",             double, ".sys.type.double");
	XIRANG2_DEFINE_TYPE_VERSION_OF("833af57eece3ee6cb5c77551238fb97bdeb047c4",        DynamicLink, ".sys.type.dynamic_link");
	XIRANG2_DEFINE_TYPE_VERSION_OF("5fcb06335ed36351225d9540768165d8bc949923",              float, ".sys.type.float");
	XIRANG2_DEFINE_TYPE_VERSION_OF("0cb73e695beae3de9229703997464772af94e341",            int16_t, ".sys.type.uint16");
	XIRANG2_DEFINE_TYPE_VERSION_OF("3bc75688dc46b1a6591ed30b97d9cccc460d4073",            int32_t, ".sys.type.int32");
	XIRANG2_DEFINE_TYPE_VERSION_OF("5a32820cbe3f6a169050c6cba05a68b0146f2deb",            int64_t, ".sys.type.int64");
	XIRANG2_DEFINE_TYPE_VERSION_OF("6f3fae8c082942fd546273714b0b9ff729b15b50",             int8_t, ".sys.type.int8");
	XIRANG2_DEFINE_TYPE_VERSION_OF("39c50a2366aa3386b5f45ac82d96507780d096b4",      StaticLinkRef, ".sys.type..generic");
	XIRANG2_DEFINE_TYPE_VERSION_OF("39c50a2366aa3386b5f45ac82d96507780d096b4", ConstStaticLinkRef, ".sys.type..generic");
	XIRANG2_DEFINE_TYPE_VERSION_OF("19bd8235ddb8b96de530677d08cd7b3bba3fffc7",          file_path, ".sys.type.path");
	XIRANG2_DEFINE_TYPE_VERSION_OF("f885443c8851c593ea26ad9e56a4f9c4ef3cdb22",              void*, ".sys.type.pointer");
	XIRANG2_DEFINE_TYPE_VERSION_OF("47e36fd0d8f8ad9382a51d650054e29b9a0c86c3",             string, ".sys.type.string");
	XIRANG2_DEFINE_TYPE_VERSION_OF("9cf80fd076bc6ca56d0e662b1c5d5dccfcbad89f",               Type, ".sys.type.type");
	XIRANG2_DEFINE_TYPE_VERSION_OF("0cb73e695beae3de9229703997464772af94e341",           uint16_t, ".sys.type.uint16");
	XIRANG2_DEFINE_TYPE_VERSION_OF("cf3713e23270b06d836f03dfd6628422af1f502d",           uint32_t, ".sys.type.uint32");
	XIRANG2_DEFINE_TYPE_VERSION_OF("059775b243a4a8de93219d6e05018efd99a018f3",           uint64_t, ".sys.type.uint64");
	XIRANG2_DEFINE_TYPE_VERSION_OF("9ba01bf2538643423bab5ba7724f4b8a34892ee2",            uint8_t, ".sys.type.uint8");
}}
#endif //end XIRANG2_TYPE_NATIVE_TYPE_VERSION_OF_H__

