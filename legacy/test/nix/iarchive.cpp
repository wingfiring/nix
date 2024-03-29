/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"

#include <algorithm>
//BOOST
#include <boost/mpl/list.hpp>

#include "iarchive.h"

BOOST_AUTO_TEST_SUITE(archive_suite)
using namespace xirang2;


ArchiveTester& ArchiveTester::check_reader(iref<io::reader> ar)
{
	io::reader* rd = &ar.get<io::reader>();

	BOOST_REQUIRE(rd->readable());

	buffer<xirang2::byte> buf;
	buf.resize(128);
	auto reset = block_read(*rd, to_range(buf));
	BOOST_CHECK( reset.empty() || !rd->readable());
	BOOST_CHECK( !rd->readable() || reset.empty());

	return *this;
}
ArchiveTester& ArchiveTester::check_reader_random(iref<io::reader, io::random> ar)
{
	io::reader* rd = &ar.get<io::reader>();

	BOOST_REQUIRE(rd->readable());

	buffer<xirang2::byte> buf;
	buf.resize(128);
	auto reset = block_read(*rd, to_range(buf));
	BOOST_CHECK( reset.empty() || !rd->readable());
	BOOST_CHECK( !rd->readable() || reset.empty());

	io::random* rnd = &ar.get<io::random>();
	if (rnd)
	{
		size_t buf_size = std::min(buf.size(), (size_t)rnd->size());

		BOOST_REQUIRE(buf_size > 1);

		buf.resize(buf_size/2);

		rnd->seek(0);
		reset = block_read(*rd, to_range(buf));

		BOOST_CHECK(reset.empty());
		BOOST_CHECK(rd->readable());
		BOOST_CHECK(rnd->offset() == buf.size());

		rnd->seek(rnd->size() - buf.size() + 1);
		reset = block_read(*rd, to_range(buf));

		BOOST_CHECK(reset.empty());
		BOOST_CHECK(!rd->readable());
	}
	return *this;

}

ArchiveTester& ArchiveTester::check_writer(iref<io::writer> ar)
{
	io::writer* wr = &ar.get<io::writer>();
	BOOST_REQUIRE(wr->writable());

	buffer<xirang2::byte> buf(128, xirang2::byte('X'));

	auto reset = block_write(*wr, to_range(buf));
	BOOST_CHECK(reset.empty());
	BOOST_REQUIRE(wr->writable());

	return *this;
}

ArchiveTester& ArchiveTester::check_writer_random(iref<io::writer, io::random> ar)
{
	io::writer* wr = &ar.get<io::writer>();
	BOOST_REQUIRE(wr->writable());

	buffer<xirang2::byte> buf(128, xirang2::byte('X'));

	auto reset = block_write(*wr, to_range(buf));
	BOOST_CHECK(reset.empty());
	BOOST_REQUIRE(wr->writable());

	io::random* rnd = &ar.get<io::random>();
	if (rnd)
	{
		size_t buf_size = std::min (buf.size(), (size_t)rnd->size());

		BOOST_REQUIRE(buf_size > 1);

		buf.resize(buf_size/2);

		rnd->seek(0);
		reset = block_write(*wr, to_range(buf));

		BOOST_CHECK(reset.empty());
		BOOST_CHECK(wr->writable());
		BOOST_CHECK(rnd->offset() == buf.size());
        using size_type = buffer<xirang2::byte>::size_type;
		size_type old_size = size_type(rnd->size());

		rnd->seek(rnd->size() - buf.size() + 1);

		reset = block_write(*wr, to_range(buf));

		BOOST_CHECK(reset.empty());
		BOOST_CHECK(wr->writable());
		BOOST_CHECK(old_size + 1 == rnd->size());
	}
	return *this;

}
ArchiveTester& ArchiveTester::check_sequence(iref<io::sequence> ar)
{
	io::sequence* seq = &ar.get<io::sequence>();

	BOOST_CHECK(seq->size() == io::unknow_size || seq->offset() <= seq->size());
	return *this;
}

ArchiveTester& ArchiveTester::check_forward(iref<io::forward> ar)
{
	io::forward* fwd = &ar.get<io::forward>();

	iref<io::sequence> sar(ar);
	check_sequence(sar);

	xirang2::long_size_t size = fwd->size();

	xirang2::long_size_t off = fwd->offset();

	xirang2::long_size_t step = (fwd->size() - fwd->offset()) / 2;

	step = std::min<long_size_t>(step, 128);

	xirang2::long_size_t new_off = fwd->seek(off + step);
	BOOST_CHECK(new_off == fwd->offset());
	BOOST_CHECK(new_off == off + step);

	new_off = fwd->seek(fwd->size() + step);
	bool can_over_end = new_off == size + step;
	if (can_over_end)
	{
		BOOST_CHECK(new_off == size + step);
		BOOST_CHECK(new_off == fwd->size() + step);
	}
	else
	{
		BOOST_CHECK(new_off == size);
	}
	BOOST_CHECK(new_off == fwd->offset());
	return *this;
}

ArchiveTester& ArchiveTester::check_random(iref<io::random> ar)
{
	io::random* rnd = &ar.get<io::random>();
	iref<io::forward> sar(ar);
	check_forward(sar);

	xirang2::long_size_t off = rnd->offset();
	BOOST_REQUIRE(off > 1);
	BOOST_REQUIRE(off > 1);
	rnd->seek(off - 1);	//seek back
	BOOST_CHECK(rnd->offset() == off - 1);
	rnd->seek(off);	//seek forward
	BOOST_CHECK(rnd->offset() == off);

	xirang2::long_size_t size = rnd->size(); //random pos
	BOOST_CHECK(size / 2 == rnd->seek(size / 2));
	BOOST_CHECK(rnd->offset() == size / 2);

	BOOST_CHECK(0 == rnd->seek(0));
	BOOST_CHECK(0 == rnd->offset());

	size = rnd->size();
	BOOST_CHECK(size == rnd->seek(size));
	BOOST_CHECK(size == rnd->offset());
	return *this;
}


BOOST_AUTO_TEST_SUITE_END()
