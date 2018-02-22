
/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <nix/contract.h>
#include <nix/context_except.h>

//STL
#include <string>

BOOST_AUTO_TEST_SUITE(contract_suite)

using namespace nix;

BOOST_AUTO_TEST_CASE(test_contract_tag)
{
	exception_reporter except_engin;
	contract_handler_saver saver(&except_engin);
	BOOST_REQUIRE(contract_handler::get_handle() == &except_engin); //test contract_handler_saver

	BOOST_CHECK_THROW(contract_handler::process(contract_category::expect, 0, 0, 0, 0, 0),  expects_exception);
	BOOST_CHECK_THROW(contract_handler::process(contract_category::ensure, 0, 0, 0, 0, 0),  ensures_exception);

	//pre_ post_ and invariant_exception are derived from contract_exception.
	BOOST_CHECK_THROW(contract_handler::process(contract_category::expect, 0, 0, 0, 0, 0),  contract_exception);

}

BOOST_AUTO_TEST_SUITE_END()
