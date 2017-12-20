#include "../precompile.h"
#include <xirang2/vfs/inmemory.h>
#include "./vfs.h"

BOOST_AUTO_TEST_SUITE(vfs_suite)
BOOST_AUTO_TEST_CASE(inmemfs_case)
{
    using namespace xirang2::vfs;
    using namespace xirang2;
    using xirang2::io::open_flag;

    InMemory vfs;
    xirang2::any ret = vfs.getopt(io_option::ao_readonly);
    BOOST_CHECK(!ret.empty() && !xirang2::any_cast<bool>(ret));

    VfsTester tester;
    tester.test_mount(vfs);
    tester.test_on_empty(vfs);
    tester.test_modification(vfs);

    ret = vfs.setopt(io_option::ao_readonly, true);
    BOOST_REQUIRE(!ret.empty() && xirang2::any_cast<bool>(ret));
    tester.test_readonly(vfs);
}
BOOST_AUTO_TEST_SUITE_END()
