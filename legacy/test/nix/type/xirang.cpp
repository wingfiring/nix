#include "../precompile.h"
#include <xirang2/type/xirang.h>
#include <xirang2/type/typebinder.h>
#include <xirang2/type/binder.h>
#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang2;
using namespace xirang2::type;

BOOST_AUTO_TEST_SUITE(xirang_suites)

BOOST_AUTO_TEST_CASE(xirang_smoke_case)
{
	Xirang xi("test", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());
    BOOST_CHECK(xi.name() == literal("test"));

    BOOST_CHECK(xi.root().valid());
    BOOST_CHECK(xi.root().name().empty());

    BOOST_CHECK(xi.get_heap().equal_to(xirang2::memory::get_global_heap()));

    BOOST_CHECK(xi.get_ext_heap().equal_to(xirang2::memory::get_global_ext_heap()));
}


BOOST_AUTO_TEST_CASE(xirang_setup_case)
{
	Xirang xi("test", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());
	SetupXirang(xi);

    BOOST_CHECK(xi.root().locateNamespace(file_path("/sys/type")).findType(file_path("int32")).valid());
	BOOST_CHECK(xi.root().findType(file_path("int")).valid());
	BOOST_CHECK(xi.root().findAlias(file_path("int")).valid());

	BOOST_CHECK(xi.root().findType(file_path("int")) == xi.root().findAlias(file_path("int")).type());
	BOOST_CHECK(xi.root().findType(file_path("int")) == xi.root().locateNamespace(file_path("/sys/type")).findType(file_path("int32")));

	Type tint = xi.root().findType(file_path("int"));

	BOOST_CHECK(tint.fullName() == file_path("/sys/type/int32"));
}

BOOST_AUTO_TEST_SUITE_END()
