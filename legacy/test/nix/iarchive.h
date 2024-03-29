#ifndef XR_COMMON_TEST_IARCHIVE_H
#define XR_COMMON_TEST_IARCHIVE_H
#include <xirang2/io.h>
#include <xirang2/interface.h>

BOOST_AUTO_TEST_SUITE(archive_suite)

using namespace xirang2;

class ArchiveTester
{
	public:
	ArchiveTester& check_reader(iref<io::reader>);
	ArchiveTester& check_writer(iref<io::writer>);
	ArchiveTester& check_sequence(iref<io::sequence>);
	ArchiveTester& check_forward(iref<io::forward>);
	ArchiveTester& check_random(iref<io::random>);
	ArchiveTester& check_reader_random(iref<io::reader, io::random> ar);
	ArchiveTester& check_writer_random(iref<io::writer, io::random> ar);

	ArchiveTester& check_rd_view(iref<io::read_view>);
	ArchiveTester& check_wr_view(iref<io::write_view>);
};
BOOST_AUTO_TEST_SUITE_END()

#endif //end XR_COMMON_TEST_IARCHIVE_H
