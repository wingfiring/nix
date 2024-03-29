/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang2/io/memory.h>
#include <xirang2/serialize/s11n.h>
#include "./iarchive.h"

BOOST_AUTO_TEST_SUITE(archive_suite)
using namespace xirang2;

BOOST_AUTO_TEST_CASE(buffer_in_case)
{
	xirang2::buffer<xirang2::byte> buf(128, xirang2::byte('X'));

	io::buffer_in ar(buf);
	ArchiveTester tester;

	tester.check_reader(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(buffer_out_case)
{
	xirang2::buffer<xirang2::byte> buf(128, xirang2::byte('X'));

	io::buffer_out ar(buf);
	ArchiveTester tester;

	tester.check_writer(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(buffer_io_case)
{
	xirang2::buffer<xirang2::byte> buf(128, xirang2::byte('X'));

	io::buffer_io ar(buf);
	ArchiveTester tester;

	tester.check_reader(ar);
	tester.check_writer(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(mem_read_archive_case)
{
	xirang2::buffer<xirang2::byte> buf(128, xirang2::byte('X'));

	io::mem_reader ar(buf);
	ArchiveTester tester;

	tester.check_reader(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(mem_write_archive_case)
{
	io::mem_writer ar;
	ArchiveTester tester;

	tester.check_writer(ar);
	tester.check_random(ar);
}

BOOST_AUTO_TEST_CASE(mem_read_write_archive_case)
{
	io::mem_archive ar;
	ArchiveTester tester;

	tester.check_writer(ar);
	ar.seek(0);
	tester.check_reader(ar);
	tester.check_random(ar);
}


BOOST_AUTO_TEST_CASE(mem_archive_case)
{
	io::mem_archive ar;
	iref<io::reader, io::writer> iar(ar);

	io::writer& wr = iar.get<io::writer>();
	io::reader& rd = iar.get<io::reader>();

	int i = 3;
	auto sink = io::local::as_sink(wr);
	sink & i;

	BOOST_CHECK(ar.size() == sizeof(int));

	ar.seek(0);
	BOOST_CHECK(ar.offset() == 0);
	int j;
	auto source = io::local::as_source(rd);
	source & j;
	BOOST_CHECK(j == 3);

	ar.truncate(sizeof(int)*2);
	BOOST_CHECK(ar.size() == sizeof(int)*2);

	ar.truncate(0);
	BOOST_CHECK(ar.size() == 0);

}

BOOST_AUTO_TEST_SUITE_END()
