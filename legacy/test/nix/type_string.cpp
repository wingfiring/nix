/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang2/type_string.h>
#include <iostream>

BOOST_AUTO_TEST_SUITE(type_string_suite)
using namespace xirang2;

struct test;
BOOST_AUTO_TEST_CASE(type_string_debug)
{
	xirang2::string s = type_string<test>();
	std::cout << s << std::endl;
}
BOOST_AUTO_TEST_SUITE_END()
