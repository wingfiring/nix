/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#ifndef NIX_TEST_NIX_PRECOMPILE_H
#define NIX_TEST_NIX_PRECOMPILE_H
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>

namespace nix
{
	inline void disable_warning () // disable boost unused warning for gcc
	{

	}
}

#endif // end NIX_TEST_NIX_PRECOMPILE_H
