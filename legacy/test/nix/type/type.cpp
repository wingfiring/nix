#include "../precompile.h"
#include <xirang2/type/xirang.h>
#include <xirang2/type/type.h>

#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang2;
using namespace xirang2::type;

BOOST_AUTO_TEST_SUITE(xirang_type_suites)

BOOST_AUTO_TEST_CASE(type_case)
{
	Xirang xi("type_case", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());

    SetupXirang(xi);

    NamespaceBuilder().name(file_path("test")).adoptBy(xi.root());
    Namespace test = xi.root().findNamespace(file_path("test"));

    Type pair = TypeBuilder().name(file_path("pair"))
        .setArg("first_type", file_path(""), Type())
        .setArg("second_type", file_path(""), Type())
        .addMember("first", file_path("first_type"), Type())
        .addMember("second", file_path("second_type"), Type())
        .endBuild()
        .adoptBy(test);

    BOOST_REQUIRE(pair.valid());
    BOOST_REQUIRE(pair);
    BOOST_CHECK(pair.name() == file_path("pair"));
    BOOST_CHECK(pair.fullName() == file_path("/test/pair"));
    BOOST_CHECK(!pair.isMemberResolved());
    BOOST_CHECK(pair.unresolvedArgs() == 2);
    BOOST_CHECK(!pair.isComplete());
    BOOST_CHECK(!pair.hasModel());
    BOOST_CHECK(!pair.model().valid());

    BOOST_CHECK(pair.memberCount() == 2);

    TypeItemRange::iterator member_pos = pair.members().begin();
    BOOST_REQUIRE(member_pos != pair.members().end());

    BOOST_CHECK(*member_pos == pair.member(0));
    TypeItem first_member = *member_pos;
    BOOST_REQUIRE(first_member.valid());
    BOOST_CHECK(!first_member.type().valid());
    BOOST_CHECK(first_member.name() == literal("first"));
    BOOST_CHECK(first_member.typeName() == file_path("first_type"));
    BOOST_CHECK(first_member.index() == 0);
    BOOST_CHECK(first_member.offset() == Type::no_size);
    BOOST_CHECK(!first_member.isResolved());
    BOOST_CHECK(first_member == pair.member(0));
    BOOST_CHECK(first_member != pair.member(1));

    ++member_pos;
    BOOST_CHECK(member_pos != pair.members().end());
    BOOST_CHECK(++member_pos == pair.members().end());

    BOOST_CHECK(pair.argCount() == 2);

    TypeArgRange::iterator arg_pos = pair.args().begin();
    BOOST_REQUIRE(arg_pos != pair.args().end());

    BOOST_CHECK(*arg_pos == pair.arg(0));
    TypeArg first_arg = *arg_pos;
    BOOST_REQUIRE(first_arg.valid());
    BOOST_CHECK(!first_arg.type().valid());
    BOOST_CHECK(first_arg.name() == literal("first_type"));
    BOOST_CHECK(first_arg.typeName().empty());
    BOOST_CHECK(!first_arg.isBound());
    BOOST_CHECK(first_arg == pair.arg(0));
    BOOST_CHECK(first_arg != pair.arg(1));

    Type type_int = xi.root().locateType(file_path("int32"));
    BOOST_CHECK(type_int.valid());
    Type pair_int = TypeBuilder().name(file_path("pair_int"))
        .modelFrom(pair)
        .setArg("first_type", file_path("int"), type_int)
        .endBuild()
        .adoptBy(test);

    TypeArg first_int_arg = pair_int.arg(0);
    BOOST_REQUIRE(first_int_arg.valid());
    BOOST_CHECK(first_int_arg.type().valid());
    BOOST_CHECK(first_int_arg.type() == type_int);
    BOOST_CHECK(first_int_arg.name() == literal("first_type"));
    BOOST_CHECK(first_int_arg.typeName() == file_path("int"));
    BOOST_CHECK(first_int_arg.isBound());

    TypeItem first_int_member = pair_int.member(0);
    BOOST_REQUIRE(first_int_member.valid());
    BOOST_CHECK(first_int_member.type() == type_int);
    BOOST_CHECK(first_int_member.typeName() == file_path("first_type"));
    BOOST_CHECK(first_int_member.offset() == 0);
    BOOST_CHECK(first_int_member.isResolved());

    BOOST_CHECK(!pair_int.isComplete());
    BOOST_CHECK(!pair_int.isMemberResolved());
    BOOST_CHECK(pair_int.unresolvedArgs() == 1);
    BOOST_CHECK(pair_int.hasModel());
    BOOST_CHECK(pair_int.model() == pair);
    BOOST_CHECK(pair_int.modelName() == file_path("pair"));

    Type type_long = xi.root().locateType(file_path("int64"));
    Type pair_int_long = TypeBuilder().name(file_path("pair_int_long"))
        .modelFrom(pair_int)
        .setArg("second_type", file_path("long"), type_long)
        .endBuild()
        .adoptBy(test);

    struct IntLongPair_{
        int32_t m1;
        int64_t m2;
    };

    BOOST_CHECK(pair_int_long.isComplete());
    BOOST_CHECK(pair_int_long.isMemberResolved());
    BOOST_CHECK(pair_int_long.unresolvedArgs() == 0);
    BOOST_CHECK(pair_int_long.hasModel());
    BOOST_CHECK(pair_int_long.model() == pair);
    BOOST_CHECK(pair_int_long.modelName() == file_path("pair"));
/*
    BOOST_CHECK(pair_int_long.align() == STDTR1::alignment_of<IntLongPair_>::value);
 */
    BOOST_CHECK(pair_int_long.payload() == sizeof(IntLongPair_));
    BOOST_CHECK(pair_int_long.isPod());

    first_int_member = pair_int_long.member(0);
    BOOST_REQUIRE(first_int_member.valid());
    BOOST_CHECK(first_int_member.type() == type_int);
    BOOST_CHECK(first_int_member.offset() == 0);
    BOOST_CHECK(first_int_member.isResolved());

    TypeItem second_long_member = pair_int_long.member(1);
    BOOST_REQUIRE(second_long_member.valid());
    BOOST_CHECK(second_long_member.type() == type_long);
    BOOST_CHECK(second_long_member.isResolved());
    BOOST_CHECK(second_long_member.offset() == sizeof(int));
    BOOST_CHECK(second_long_member.index() == 1);

    //
    Type pair_int_string = TypeBuilder().name(file_path("pair_int_string"))
        .modelFrom(pair_int)
        .setArg("second_type", file_path("string"), xi.root().locateType(file_path("string")))
        .endBuild()
        .adoptBy(test);
    BOOST_CHECK(pair_int_string.valid() && pair_int_string.isMemberResolved() && !pair_int_string.isPod());

    //test LocateType for nested type
    Type triple_int_pair_long = TypeBuilder().name(file_path("triple_int_pair_long"))
        .modelFrom(pair_int)
        .setArg("second_type", file_path("pair_int_long"), pair_int_long)
        .addMember("thrid", file_path("second_type/second_type"), Type())
        .endBuild()
        .adoptBy(test);

    BOOST_REQUIRE(triple_int_pair_long.valid() && triple_int_pair_long.isMemberResolved());
    BOOST_CHECK(triple_int_pair_long.member(1).type() == pair_int_long);
    BOOST_CHECK(triple_int_pair_long.member(2).type() == type_long);
}


BOOST_AUTO_TEST_SUITE_END()
