/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

/** @file

	@author wingfire mailto:wing.fire@gmail.com http://blog.csdn.net/wingfiring
 */

#ifndef XR_XIRANG2_UNIT_TEST_PRE_COMPILE_HEAD_H
#define XR_XIRANG2_UNIT_TEST_PRE_COMPILE_HEAD_H

#include <xirang2/config.h>


//#define BOOST_TEST_NO_AUTO_LINK 1

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>

#define XR_KEEP_CONTRACT_ASSERT
#include <xirang2/utility/unused.h>
#include <iostream>

namespace
{
	inline void disable_warning () // disable boost unused warning for gcc
	{
#if defined GNUC_COMPILER_
		xirang2::unused(boost::test_tools::check_is_close);
		xirang2::unused(boost::test_tools::check_is_small);
#endif

	}
}

#endif // end XR_XIRANG2_UNIT_TEST_PRE_COMPILE_HEAD_H
