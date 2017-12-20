#include "../precompile.h"
#if 0
#include <xirang2/io/memory.h>
#include <xirang2/vfs/zip.h>
#include <xirang2/vfs/inmemory.h>
#include <xirang2/io/adaptor.h>

#include "./vfs.h"

BOOST_AUTO_TEST_SUITE(vfs_suite)
BOOST_AUTO_TEST_CASE(zipfs_case)
{
    using namespace xirang2::fs;
    using namespace xirang2;
    using xirang2::io::open_flag;

    xirang2::io::mem_read_write_archive ar;
    {
        InMemory cache;
        ZipFs vfs(ar, cache, "test zipfs");

        xirang2::any ret = vfs.getopt(ao_readonly);
        BOOST_CHECK(!ret.empty() && !xirang2::any_cast<bool>(ret));

        VfsTester tester;
        tester.test_mount(vfs);
        tester.test_on_empty(vfs);
        tester.test_modification(vfs);
    }
    {
        using namespace xirang2::io;
        typedef multiplex_deletor<multiplex_reader<multiplex_random<multiplex_base<reader, xirang2::io::random, xirang2::ideletor> > > > multiplex_read_archive;
        multiplex_read_archive ar2(&ar);
        InMemory cache;
        ZipFs vfs(ar2, cache);

        xirang2::any ret = vfs.getopt(ao_readonly);
        BOOST_CHECK(!ret.empty() && xirang2::any_cast<bool>(ret));

        VfsTester tester;
        tester.test_readonly(vfs);
    }

}


BOOST_AUTO_TEST_SUITE_END()

#endif
