#include "../precompile.h"
#include <xirang2/type/xirang.h>
#include <xirang2/type/typealias.h>

#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang2;
using namespace xirang2::type;

BOOST_AUTO_TEST_SUITE(xirang_type_suites)

BOOST_AUTO_TEST_CASE(typealias_case)
{
	Xirang xi("typealias_case", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());

    SetupXirang(xi);

    TypeAliasBuilder builder;
    builder.name(file_path("int_alias"))
        .typeName(file_path("/sys/int"))
        .setType(xi.root().findType(file_path("int")));
    TypeAlias tmp = builder.get();

    BOOST_CHECK(tmp.valid());
    BOOST_CHECK(builder.getName() == file_path("int_alias"));
    BOOST_CHECK(builder.getTypeName() == file_path("/sys/int"));

    TypeAlias int_alias = builder.adoptBy(xi.root());

    BOOST_REQUIRE(int_alias.valid());
    BOOST_CHECK(int_alias == tmp);

}


BOOST_AUTO_TEST_SUITE_END()
