#ifndef XIRANG2_PRIMITIVE_TYPE_H__
#define XIRANG2_PRIMITIVE_TYPE_H__

#include <xirang2/type/type.h>
#include <xirang2/type/object.h>
#include <xirang2/type/binder.h>
#include <xirang2/path.h>
namespace xirang2 {    namespace type {

        // specialize for basic_string
        template<> struct hasher<file_path> {
            static size_t apply(ConstCommonObject obj) {
                auto& data = *Ref<file_path>(obj, TypeMismatchPolicy::tmp_nocheck);
                return data.str().hash();
            }
        };

        // TODO: for fs, should move to header files
        struct type_dir {};
        int compare(type_dir, type_dir) { return 0; }
        DEFINE_COMPARE(type_dir);
        template<> struct hasher<type_dir> {
            static size_t apply(ConstCommonObject) { return 0; }
        };

        struct type_file {};
        int compare(type_file, type_file) { return 0; }
        DEFINE_COMPARE(type_file);
        template<> struct hasher<type_file> {
            static size_t apply(ConstCommonObject) { return 0; }
        };

        struct type_unknown {};
        int compare(type_unknown, type_unknown) { return 0; }
        DEFINE_COMPARE(type_unknown);
        template<> struct hasher<type_unknown> {
            static size_t apply(ConstCommonObject) { return 0; }
        };
}}
#endif //end XIRANG2_PRIMITIVE_TYPE_H__
