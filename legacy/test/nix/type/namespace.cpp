#include "../precompile.h"
#include <xirang2/type/xirang.h>
#include <xirang2/type/namespace.h>
#include <xirang2/type/typebinder.h>
#include <xirang2/type/binder.h>
#include <xirang2/type/object.h>
#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang2;
using namespace xirang2::type;

BOOST_AUTO_TEST_SUITE(namespace_suite)

BOOST_AUTO_TEST_CASE(namespace_case)
{
	Xirang xi("test_namespace", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());
    BOOST_CHECK(xi.root().name().empty());
    BOOST_CHECK(xi.root().fullName() == file_path("/"));

	SetupXirang(xi);

    Namespace user = NamespaceBuilder()
        .name(file_path("user"))
        .adoptBy(xi.root());
    BOOST_REQUIRE(user.valid());

    Namespace user_include = NamespaceBuilder()
        .name(file_path("include"))
        .adoptBy(user);

    Namespace sys = user.locateNamespace(file_path("/sys"));
    BOOST_REQUIRE(sys.valid() && "locate with absolute path");
    Namespace sys3 = user.locateNamespace(file_path("sys"));
    BOOST_CHECK(sys == sys3 && "locate with relative path which in parent namespace");

    Namespace user_include2 = user.locateNamespace(file_path("include"));
    BOOST_REQUIRE(user_include2 == user_include && "locate in current namespace");
    BOOST_CHECK(user_include.fullName() == file_path("/user/include"));
    BOOST_CHECK(user_include.empty());
    BOOST_CHECK(user_include.parent() == user);
    BOOST_CHECK(user_include != user);
    BOOST_CHECK(user_include.root() == xi.root());


    Type type_int = sys.locateType(file_path("int"));
    BOOST_CHECK(type_int.valid() && "locate type via relative type name");
    BOOST_CHECK(type_int == sys.locateType(file_path("sys/type/int32")) && "locate via relative type path.");
    BOOST_CHECK(type_int == user_include.locateType(file_path("sys/type/int32")) && "locate via relative type path.");
    BOOST_CHECK(type_int == user_include.locateType(file_path("/sys/type/int32")) && "locate via absolute type path.");

    BOOST_CHECK(!xi.root().findRealType(file_path("int")).valid());
    BOOST_CHECK(sys.findNamespace(file_path("type")).findRealType(file_path("int32")).valid());

    BOOST_CHECK(xi.root().findType(file_path("int32")).valid() && "find an alias in root");

    TypeAlias int_alias = xi.root().findAlias(file_path("int"));
    BOOST_REQUIRE(int_alias.valid());
    BOOST_CHECK(int_alias.type() == type_int);


    BOOST_CHECK(user_include.types().begin() == user_include.types().end());

    TypeBuilder()
        .name(file_path("test"))
        .addMember("m1", file_path("int"), type_int)
        .endBuild()
        .adoptBy(user);

    size_t type_counter = 0;
    TypeSynonymRange types = user.types();
    for (TypeSynonymRange::iterator itr = types.begin(); itr !=types.end(); ++itr)
        ++type_counter;

    BOOST_CHECK(type_counter == 1);

    TypeAliasBuilder().name(file_path("test_alias"))
        .setType(type_int)
        .typeName(file_path("int"))
        .adoptBy(user);
    size_t alias_counter = 0;
   TypeAliasRange alias = user.alias();
    for (TypeAliasRange::iterator itr = alias.begin(); itr != alias.end(); ++itr)
        ++alias_counter;
    BOOST_CHECK(alias_counter == 1);

    size_t children_counter = 0;
    NamespaceRange children = user.namespaces();
    for (NamespaceRange::iterator itr = children.begin(); itr != children.end(); ++itr)
        ++children_counter;
    BOOST_CHECK(children_counter == 1);


    ObjectFactory(xi).create(type_int, user, file_path("int_obj"));
    size_t obj_counter = 0;
    ObjectRange objects = user.objects();
    for (ObjectRange::iterator itr = objects.begin(); itr != objects.end(); ++itr)
        ++obj_counter;
    BOOST_CHECK(obj_counter == 1);
}

BOOST_AUTO_TEST_CASE(namespace_builder_case)
{
    Xirang xi("test_namespace_builder", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());
	SetupXirang(xi);

    NamespaceBuilder()
        .name(file_path("user"))
        .adoptBy(xi.root());
    Namespace user = xi.root().findNamespace(file_path("user"));
    BOOST_REQUIRE(user.valid());

    NamespaceBuilder()
        .name(file_path("include"))
        .parent(user)
        .adoptBy();

    NamespaceBuilder tb;
    tb.parent(user);
    BOOST_CHECK(tb.getParent() == user);

    tb.name(file_path("test_name"));
    BOOST_CHECK(tb.getName() == file_path("test_name"));

    Namespace user_include = user.findNamespace(file_path("include"));
    BOOST_CHECK(user_include.valid());

    NamespaceBuilder testBuilder;

    Type type_int = xi.root().locateType(file_path("int"));
    BOOST_REQUIRE(type_int.valid());

    TypeBuilder type_b;
    type_b.name(file_path("test_type"))
        .addMember("m1", file_path("int"), type_int)
        .endBuild();
    testBuilder.adopt(type_b);

    NamespaceBuilder ns_b;
    ns_b.name(file_path("test_ns"));
    testBuilder.adopt(ns_b);

    TypeAliasBuilder alias_b;
    alias_b.name(file_path("test_alias"))
        .setType(type_int)
        .typeName(file_path("int"));
    testBuilder.adopt(alias_b);

    ScopedObjectCreator(type_int, xi).adoptBy(testBuilder.get(), file_path("int_obj"));

    NamespaceBuilder ns_new;
    ns_new.swap(testBuilder);


    ns_new.adoptChildrenBy(user);

    BOOST_CHECK(user.findNamespace(file_path("include")).valid());
    BOOST_CHECK(user.findNamespace(file_path("test_ns")).valid());
    BOOST_CHECK(user.findType(file_path("test_type")).valid());
    BOOST_CHECK(user.findAlias(file_path("test_alias")).valid());
    BOOST_CHECK(user.findObject(file_path("int_obj")).name != 0);

    NamespaceBuilder().createChild(file_path("x")).adoptChildrenBy(user);

    NamespaceBuilder().createChild(file_path("a/b/c")).adoptChildrenBy(user);

    Namespace a = user.findNamespace(file_path("a"));
    BOOST_REQUIRE(a.valid());

    Namespace b = a.findNamespace(file_path("b"));
    BOOST_REQUIRE(b.valid());

    Namespace c = b.findNamespace(file_path("c"));
    BOOST_REQUIRE(c.valid());
}
BOOST_AUTO_TEST_SUITE_END()
