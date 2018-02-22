#include "precompile.h"
#include <nix/context_except.h>

BOOST_AUTO_TEST_SUITE(context_exception)
NIX_EXCEPTION_TYPE(test_context_exception);

void test_throw(){
	NIX_THROW(test_context_exception) << " The answer is " << 42;
}
BOOST_AUTO_TEST_CASE(context_exception_case){
	nix::throw_exception<test_context_exception> e("file", 1, "");
	BOOST_CHECK_THROW(e.apply_throw(), test_context_exception);
	BOOST_CHECK_THROW(test_throw(), test_context_exception);
}

BOOST_AUTO_TEST_SUITE_END()

