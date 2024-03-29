/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang2/heap/file_mapping_heap.h>
#include <xirang2/io/file.h>
#include <xirang2/fsutility.h>

BOOST_AUTO_TEST_SUITE(suite_file_mapping_heap)
using namespace xirang2;


BOOST_AUTO_TEST_CASE(case_file_mapping_file)
{
	typedef ext_heap::handle handle;
    file_path temp_path = fs::temp_dir(sub_file_path(literal("tfmap_")));
    file_path file_name =  temp_path / fs::private_::gen_temp_name(sub_file_path(literal("fa")));
    {
        io::file file0(file_name, io::of_create_or_open);
		typedef file_mapping_heap::ar_type ar_type;
		ar_type file(file0);

        handle ch;

        {
            file_mapping_heap eheap(file, 0, memory::multi_thread);

            void * p = eheap.malloc(1000, 1, 0);
            BOOST_CHECK(p != 0);

            eheap.free(p, 1000, 1);

            handle h = eheap.allocate(1000, 1, handle());
            BOOST_CHECK(h.size() >= 1000);
            ch = eheap.allocate(2000, 1, handle());

            p = eheap.track_pin(h);
            BOOST_CHECK(eheap.track_pin_count(h) == 1);
            BOOST_CHECK(eheap.view_pin_count(h) == 1);
            void * p2 = eheap.pin(h);

            char * s = (char*)p2;
            std::fill(s, s + 1000, 'F');

            void* p3 = eheap.pin(ch);
            char* s3 = (char*)p3;
            std::fill(s3, s3 + 2000, 'S');
            eheap.unpin(p3);

            BOOST_CHECK(p == p2);
            BOOST_CHECK(eheap.view_pin_count(h) == 2);
#ifndef NDEBUG	
            BOOST_CHECK(eheap.track_pin_count(h) == 2);
#else
            BOOST_CHECK(eheap.track_pin_count(h) == 1);
#endif	

            eheap.track_unpin(p);
            BOOST_CHECK(eheap.view_pin_count(h) == 1);
#ifndef NDEBUG	
            BOOST_CHECK(eheap.track_pin_count(h) == 1);
#else
            BOOST_CHECK(eheap.track_pin_count(h) == 0);
#endif	
            eheap.unpin(p2);
            BOOST_CHECK(eheap.track_pin_count(h) == 0);
            BOOST_CHECK(eheap.view_pin_count(h) == 0);

            eheap.deallocate(h);

            eheap.set_limit(10240, 20480);

            const file_mapping_heap_info& info = eheap.get_info();

            BOOST_CHECK(10240 == info.soft_limit);
            BOOST_CHECK(20480 == info.hard_limit);
        }
        {
            file_mapping_heap eheap(file, 0, memory::multi_thread);
            void* p = eheap.track_pin(ch);
            const char* pc = reinterpret_cast<const char*>(p);
            bool pass = true;
            for (int i = 0; i < 2000; ++i)
            {
                if (*pc++ != 'S')
                {
                    pass = false;
                    break;
                }
            }
            BOOST_CHECK(pass);
            eheap.track_unpin(p);
        }
    }
    xirang2::fs::recursive_remove(temp_path);
}

BOOST_AUTO_TEST_SUITE_END()
