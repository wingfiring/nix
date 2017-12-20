/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/
#include "precompile.h"
#include <xirang2/lrumap.h>
#include <xirang2/string.h>

#include <random>

BOOST_AUTO_TEST_SUITE(lrumap_suite)
using namespace xirang2;

BOOST_AUTO_TEST_CASE(empty_lrumap_case)
{
	lru_map<string, string, hash_string> string_lru(200);
	BOOST_CHECK(string_lru.capacity() == 200);
    BOOST_CHECK(string_lru.size() == 0);
    
    lru_map<int, string> int_lru;
    BOOST_CHECK(int_lru.missed() == 0);
    BOOST_CHECK(int_lru.hitted() == 0);
}

BOOST_AUTO_TEST_CASE(basic_lrumap_case)
{
    lru_map<int, int> int_lru(20);
    
    for(int i = 0; i < 20; ++i)
        int_lru.set(i, i);
    
    BOOST_CHECK(int_lru.size() == 20);
    BOOST_CHECK(int_lru.missed() == 0);
    BOOST_CHECK(int_lru.hitted() == 0);
    
    lru_map<int, int>::list_type output;
    int_lru.capacity(15, output);   // [0,5) should be removed
    BOOST_CHECK(int_lru.size() == 15);
    BOOST_CHECK(int_lru.missed() == 0);
    BOOST_CHECK(int_lru.hitted() == 0);
    BOOST_CHECK(output.size() == 5);
    
    int_lru.fetch(5);   // 5 should be move to the head
    BOOST_CHECK(int_lru.missed() == 0);
    BOOST_CHECK(int_lru.hitted() == 1);
    
    int_lru.try_fetch(4);   // 5 should be move to the head, 6 is at the tail
    BOOST_CHECK(int_lru.missed() == 1);
    BOOST_CHECK(int_lru.hitted() == 1);
    
    int_lru.set(20, 20);    // 6 should be removed
    BOOST_CHECK(int_lru.size() == 15);
    BOOST_CHECK(int_lru.missed() == 1);
    BOOST_CHECK(int_lru.hitted() == 1);
    
    int_lru.try_fetch(6);   //
    BOOST_CHECK(int_lru.missed() == 2);
    BOOST_CHECK(int_lru.hitted() == 1);
    
}
BOOST_AUTO_TEST_CASE(basic_lrumap_perf)
{
    const std::size_t K_size = 2000;
    lru_map<int, int> int_lru(K_size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    //std::uniform_int_distribution<std::size_t> d(0, K_size * 2);
    
    //std::normal_distribution<> d(K_size, K_size/4);
    std::poisson_distribution<> d(static_cast<int>(K_size));
    for (std::size_t i = 0; i < 100 * K_size; ++i){
        auto v =  int(d(gen));
        if (!int_lru.try_fetch(v))
            int_lru.set(v, v);
    }
    BOOST_TEST_MESSAGE("Hitted: " << int_lru.hitted());
    BOOST_TEST_MESSAGE("Missed: " << int_lru.missed());
    BOOST_TEST_MESSAGE("Hit(%): " << double(int_lru.hitted())/double((int_lru.hitted() + int_lru.missed())) << "%");
}
BOOST_AUTO_TEST_SUITE_END()
