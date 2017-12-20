#include "../precompile.h"
#include <xirang2/type/xirang.h>
#include <xirang2/type/object.h>
#include <xirang2/type/typebinder.h>
#include <xirang2/type/binder.h>
#include <xirang2/type/nativetypeversion.h>

#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang2;
using namespace xirang2::type;

BOOST_AUTO_TEST_SUITE(xirang_object_suites)

BOOST_AUTO_TEST_CASE(object_case)
{
    Xirang xi("object_case", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());

    SetupXirang(xi);

    Namespace test = NamespaceBuilder().name(file_path("test")).adoptBy(xi.root());


    Type sub_pair = TypeBuilder().name(file_path("sub_pair"))
        .addMember("first", file_path("int"), xi.root().findType(file_path("int")))
        .addMember("second", file_path("string"), xi.root().findType(file_path("string")))
        .endBuild()
        .adoptBy(test);
	BOOST_CHECK(sub_pair.members().size() == 2);

    Type triple = TypeBuilder().name(file_path("triple"))
        .addMember("first", file_path("int"), xi.root().findType(file_path("int")))
        .addMember("second", file_path("sub_pair"), test.findType(file_path("sub_pair")))
        .addMember("third", file_path("double"), xi.root().findType(file_path("double")))
        .endBuild()
        .adoptBy(test);


    ScopedObjectCreator holder(triple, xi);
    CommonObject obj_triple_0 = holder.get();
    BOOST_REQUIRE(obj_triple_0);

    BOOST_CHECK(obj_triple_0.type() == triple);
    BOOST_CHECK(obj_triple_0.getMember(1) == obj_triple_0.getMember("second"));
    BOOST_CHECK(obj_triple_0.asSubObject().asCommonObject() == obj_triple_0);

    BOOST_CHECK(obj_triple_0.members().size() == 3);

    SubObject subobj2 = obj_triple_0.getMember(2);
    BOOST_CHECK(subobj2.valid() && subobj2.type() == xi.root().findType(file_path("double")));

    BOOST_CHECK(subobj2 == obj_triple_0.getMember("third"));
    BOOST_CHECK(subobj2.asCommonObject().asSubObject().asCommonObject() == subobj2.asCommonObject());
    BOOST_CHECK(subobj2.data() != 0);
    BOOST_CHECK(subobj2.data() == reinterpret_cast<const xirang2::byte*>(obj_triple_0.data()) + triple.member(2).offset());
    BOOST_CHECK(bind<double>(subobj2) == 0.0);
    BOOST_CHECK(subobj2.ownerType() == triple);
    BOOST_CHECK(subobj2.ownerData() == obj_triple_0.data());
    BOOST_CHECK(subobj2.index() == 2);
    BOOST_CHECK(obj_triple_0.asSubObject().index() == ConstSubObject::PhonyIndex);

    //get second.first
    SubObject second_first = obj_triple_0.getMember(1).asCommonObject().getMember(0);
    BOOST_CHECK(second_first.valid());
    BOOST_CHECK(second_first.index() == 0);
    bind<int>(second_first) = 42;
    obj_triple_0.getMember(0).asCommonObject().assign(second_first.asCommonObject());
    BOOST_CHECK(bind<int>(obj_triple_0.getMember(0)) == 42);

    ConstCommonObject const_obj_triple_0 = obj_triple_0;

    BOOST_REQUIRE(const_obj_triple_0.valid());
    BOOST_CHECK(const_obj_triple_0.type() == triple);
    BOOST_CHECK(const_obj_triple_0.getMember(1) == const_obj_triple_0.getMember("second"));
    BOOST_CHECK(const_obj_triple_0.asSubObject().asCommonObject() == const_obj_triple_0);

    BOOST_CHECK(const_obj_triple_0.type().members().size() == 3);
    BOOST_CHECK(const_obj_triple_0.members().size() == 3);

    ConstSubObject subobj = const_obj_triple_0.getMember(2);
    BOOST_CHECK(subobj.valid() && subobj.type() == xi.root().findType(file_path("double")));

    BOOST_CHECK(subobj == const_obj_triple_0.getMember("third"));
    BOOST_CHECK(subobj.asCommonObject().asSubObject().asCommonObject() == subobj.asCommonObject());
    BOOST_CHECK(subobj.data() != 0);
    BOOST_CHECK(subobj.data() == reinterpret_cast<const xirang2::byte*>(const_obj_triple_0.data()) + triple.member(2).offset());
    BOOST_CHECK(bind<double>(subobj) == 0.0);
    BOOST_CHECK(subobj.ownerType() == triple);
    BOOST_CHECK(subobj.ownerData() == const_obj_triple_0.data());
    BOOST_CHECK(subobj.index() == 2);
    BOOST_CHECK(const_obj_triple_0.asSubObject().index() == ConstSubObject::PhonyIndex);

    holder.adoptBy(test, file_path("obj_triple_0"));
    BOOST_CHECK(test.findObject(file_path("obj_triple_0")).name != 0);
    BOOST_CHECK(test.findObject(file_path("obj_triple_0")).value == obj_triple_0);

    BOOST_CHECK((obj_triple_0.members().begin()++)->asCommonObject().valid());

}

BOOST_AUTO_TEST_SUITE_END()
