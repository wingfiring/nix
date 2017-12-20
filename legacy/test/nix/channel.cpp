/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang2/channel.h>
#include <future>
#include <vector>

BOOST_AUTO_TEST_SUITE(channel_suite)
static const int K_steps = 10000;
BOOST_AUTO_TEST_CASE(channel_test)
{
    using namespace xirang2;

    channel<int> ch;
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 50; i++) {
        futures.emplace_back(std::async(std::launch::async, [i, &ch] {
            auto start = i * K_steps;
            auto end = start + K_steps;
            for (int j = start; j != end; ++j) 
                ch.push(j);

            std::cout << i << " producer exit" << std::endl;
        }));

        futures.emplace_back(std::async(std::launch::async, [i, &ch] {
            for (int j = 0; j != K_steps; ++j)
                ch.pull();

            std::cout << i << " consumer exit" << std::endl;
        }));
    }

    for (auto&i : futures) {
        //std::cout << ".";
        i.get();
    }

}
BOOST_AUTO_TEST_SUITE_END()