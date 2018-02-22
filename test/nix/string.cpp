/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <nix/contract.h>
#include <nix/string.h>
#include <nix/range.h>
#include <nix/utility.h>

//BOOST
#include <boost/mpl/list.hpp>

BOOST_AUTO_TEST_SUITE(string_suite)
using namespace nix;

typedef boost::mpl::list<char, wchar_t> test_types;

template<typename T>
struct test_data2;
template<>
struct test_data2<const char>
{
	typedef basic_range_string<const char> result;
	static result abcd() { return literal("abcd");}
	static result qwerty(){ return literal("qwerty");}
	static result abcd_qwerty(){ return literal("abcdqwerty");}
	
};
template<>
struct test_data2<const wchar_t>
{
	typedef basic_range_string<const wchar_t> result;
	static result abcd() { return literal(L"abcd");}
	static result qwerty(){ return literal(L"qwerty");}
	static result abcd_qwerty(){ return literal(L"abcdqwerty");}	
};

BOOST_AUTO_TEST_CASE_TEMPLATE(basic_range_string_case, T, test_types)
{
	typedef basic_range_string<const T> cstring;
	typedef test_data2<const T> data;

	cstring empty;

	BOOST_CHECK(empty.size() == 0);
	BOOST_CHECK(empty.empty());
	BOOST_CHECK(empty.begin() == empty.end());

	cstring abcd = data::abcd();
	BOOST_CHECK(abcd.size() == 4);
	BOOST_CHECK(!abcd.empty());
	BOOST_CHECK(abcd.data() != 0);
	BOOST_CHECK(abcd.begin() + 4 == abcd.end());

	cstring qwerty = data::qwerty();

	BOOST_CHECK(abcd == abcd);
	BOOST_CHECK(abcd != qwerty);
	BOOST_CHECK(abcd < qwerty);
	BOOST_CHECK(qwerty > abcd);
	BOOST_CHECK(abcd < data::qwerty());
	BOOST_CHECK(qwerty == qwerty);

	swap(abcd, qwerty);

	BOOST_CHECK(abcd == data::qwerty());

	cstring range_ctor(data::abcd_qwerty().begin(), data::abcd_qwerty().begin() + 4);
	BOOST_CHECK(range_ctor == data::abcd());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(basic_string_case, T, test_types)
{
	typedef basic_string<T> cstring;
	typedef test_data2<const T> data;

	cstring empty;

	BOOST_CHECK(empty.size() == 0);
	BOOST_CHECK(empty.empty());
	BOOST_CHECK(empty.c_str() != 0);
	BOOST_CHECK(empty.begin() == empty.end());

	cstring abcd = data::abcd();
	BOOST_CHECK(abcd.size() == 4);
	BOOST_CHECK(!abcd.empty());
	BOOST_CHECK(abcd.c_str() != 0);
	BOOST_CHECK(abcd.begin() + 4 == abcd.end());

	cstring qwerty = data::qwerty();

	BOOST_CHECK(abcd != qwerty);
	BOOST_CHECK(!(abcd < abcd));
	BOOST_CHECK(abcd < qwerty);
	BOOST_CHECK(abcd < data::qwerty());
	BOOST_CHECK(qwerty == qwerty);
	BOOST_CHECK(cstring(abcd << qwerty) == data::abcd_qwerty());
	BOOST_CHECK(cstring(abcd << data::qwerty()) == data::abcd_qwerty());
	BOOST_CHECK(cstring(data::abcd() << qwerty) == data::abcd_qwerty());

	swap(abcd, qwerty);

	BOOST_CHECK(abcd == data::qwerty());

	cstring range_ctor(make_range(data::abcd_qwerty().begin(), data::abcd_qwerty().begin() + 4));
	BOOST_CHECK(range_ctor == qwerty);

	cstring abcd_qwerty4 = data::abcd_qwerty();
	abcd_qwerty4 = abcd_qwerty4  << abcd_qwerty4;
	abcd_qwerty4 = abcd_qwerty4  << abcd_qwerty4;

	auto abcd_qwerty4_dup = abcd_qwerty4;

	unused(abcd_qwerty4_dup);
	
}

BOOST_AUTO_TEST_SUITE_END()

