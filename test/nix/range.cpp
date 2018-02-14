#include "precompile.h"
#include <nix/range.h>
#include <vector>

BOOST_AUTO_TEST_SUITE(range)
BOOST_AUTO_TEST_CASE(range_basic_case){
	nix::range<char*> rng1;
	BOOST_CHECK(rng1.empty());
	BOOST_CHECK(rng1.size() == 0);
	BOOST_CHECK(rng1.begin() == rng1.end());
	BOOST_CHECK(std::begin(rng1) == std::end(rng1));

	nix::range<char*> rng2;
	BOOST_CHECK(rng1 == rng2);
}

BOOST_AUTO_TEST_CASE(range_make_case){
	std::vector<int> vi;
	auto rng = nix::to_range(vi);
	BOOST_CHECK(rng.empty());

	vi.emplace_back(42);
	vi.emplace_back(42);
	rng = nix::to_range(vi);
	BOOST_CHECK(rng.size() == 2);
	rng.remove_prefix(1);
	BOOST_CHECK(rng.size() == 1);
	rng.remove_suffix(1);
	BOOST_CHECK(rng.empty());


}
BOOST_AUTO_TEST_SUITE_END()
