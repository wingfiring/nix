//XIRANG2_LICENSE_PLACE_HOLDER

#ifndef XR_TEST_HELPER__
#define XR_TEST_HELPER__

namespace xirang2
{
	template<typename T, int ID = 0>
	struct test_helper;
};

#define XR_ENABLE_TEST template<typename T, int> friend struct ::xirang2::test_helper

#endif //end XR_TEST_HELPER__
