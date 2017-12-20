/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/
#include "precompile.h"
#include <xirang2/fsutility.h>
#include <sys/stat.h>

BOOST_AUTO_TEST_SUITE(tempfile_suite)
using namespace xirang2::fs;
using namespace xirang2;

BOOST_AUTO_TEST_CASE(tempfile_case)
{
    file_path prefix(literal("test"));
    file_path test_name = fs::private_::gen_temp_name(prefix);

    BOOST_CHECK(prefix.str().size() < test_name.str().size());
    BOOST_CHECK(std::equal(prefix.str().begin(), prefix.str().end(), test_name.str().begin()));

    file_path tmppath;
    {   
        xirang2::io::file file = temp_file(prefix, xirang2::io::of_remove_on_close, &tmppath);
        BOOST_CHECK(!tmppath.empty());
        BOOST_CHECK(state(tmppath).state == xirang2::fs::st_regular);
    }
    BOOST_CHECK(state(tmppath).state == xirang2::fs::st_not_found);

    BOOST_CHECK_THROW((temp_file(file_path(literal("test")), file_path(literal("path/not/exist")))),  fs::not_found_exception);

    BOOST_CHECK_THROW((temp_dir(file_path(literal("test")), file_path(literal("path/not/exist")))),  fs::not_found_exception);

    file_path tmpfilepath2;
    {
        tmppath  = temp_dir(prefix);
        BOOST_CHECK(!tmppath.empty());
        BOOST_CHECK_NO_THROW((temp_file(file_path(literal("test")), tmppath)));
        BOOST_CHECK(state(tmppath).state == xirang2::fs::st_dir);

        temp_file(sub_file_path(literal("test")), tmppath, 0, &tmpfilepath2);
    }
    BOOST_CHECK(state(tmpfilepath2).state == xirang2::fs::st_regular);
    xirang2::fs::remove(tmpfilepath2);
    BOOST_CHECK(state(tmpfilepath2).state == xirang2::fs::st_not_found);

    xirang2::fs::remove(tmppath);
    BOOST_CHECK(state(tmppath).state == xirang2::fs::st_not_found);

}

BOOST_AUTO_TEST_CASE(recursive_create_dir_case)
{
    file_path tmpdir  = temp_dir(sub_file_path(literal("tmp")));
    BOOST_CHECK(recursive_create_dir(tmpdir / sub_file_path(literal("/a/b/c"))) == xirang2::fs::er_ok);

    BOOST_CHECK_NO_THROW((recursive_create(tmpdir / sub_file_path(literal("/x/y/z")), xirang2::io::of_create)));

    recursive_remove(tmpdir);
}

BOOST_AUTO_TEST_CASE(recreate_file_case)
{
    file_path tmppath;
	{
		xirang2::io::file file = temp_file(file_path(literal("test")), 0, &tmppath);
		xirang2::buffer<xirang2::byte> data;
		data.resize(100);
		BOOST_CHECK(file.write(to_range(data)).empty());

		BOOST_CHECK_NO_THROW((xirang2::io::file_reader)(tmppath));

		BOOST_CHECK_NO_THROW((xirang2::io::file(tmppath, xirang2::io::of_open)));
	}
	xirang2::fs::remove(tmppath);
}
BOOST_AUTO_TEST_SUITE_END()

