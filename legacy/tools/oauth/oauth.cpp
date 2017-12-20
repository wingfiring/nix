#include <xirang2/sha1.h>
#include <xirang2/byteorder.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace xirang2{

	const char * base64_table = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	string base64_encode(const char* in, size_t bytes){

		string_builder sb;
		auto size = bytes;
		int rest = size % 3;

		size_t group = size - rest;
		uint8_t*  pos = (uint8_t*)in;
		auto g_end = pos + group;
		for (; pos != g_end; pos += 3){
			uint32_t v = (uint32_t(pos[0]) << 16) + (uint32_t(pos[1]) << 8) + uint32_t(pos[2]);
			for (int i = 0; i < 4; ++i){
				sb.push_back(base64_table[(v >> 18) & 0x3F]);
				v <<= 6;
			}
		}
		if (rest == 1){
			sb.push_back(base64_table[(*pos) >> 2]);
			sb.push_back(base64_table[((*pos) & 3) << 4]);
			sb.push_back('=');
			sb.push_back('=');
		}
		else if (rest == 2){
			uint32_t v = (uint32_t(pos[0]) << 16) + (uint32_t(pos[1]) << 8);
			sb.push_back(base64_table[(v >> 18) & 0x3F]);
			sb.push_back(base64_table[(v >> 12) & 0x3F]);
			sb.push_back(base64_table[(v >> 6) & 0x3F]);
			sb.push_back('=');
		}

		return string(sb);
	}
}

using namespace xirang2;

const unsigned K_Block_Length = 64;
sha1_digest hmac_sha1(const string& s, const string& data){
	std::vector<char> key;
	key.resize(64);

	if (s.size() > K_Block_Length){
		sha1 sha;
		sha.write(string_to_c_range(s));
		auto dig = sha.get_digest();

		std::copy((const char*)dig.v, (const char*)dig.v + sizeof(dig.v), key.begin());
	}
	else{
		std::copy(s.begin(), s.end(), key.begin());
	}
	
	for (auto& v : key)
		v = char(v) ^ 0x36;

	sha1 inner;
	inner.write(xirang2::make_range((xirang2::byte*)&key[0], (xirang2::byte*)&key[0] + key.size()));
	inner.write(string_to_c_range(data));
	auto inner_dig = inner.get_digest();

	sha1 outer;
	for (auto& v : key)
		v = char(v) ^ (0x36 ^ 0x5c);
	outer.write(xirang2::make_range((xirang2::byte*)&key[0], (xirang2::byte*)&key[0] + key.size()));
	
	outer.write(xirang2::make_range((xirang2::byte*)inner_dig.v, (xirang2::byte*)inner_dig.v + sizeof(inner_dig.v)));

	return outer.get_digest();
}

//std::string make_oauth_string(const std::string& method,
//	const std::string& url,
//	const std::map<std::string, std::string>& parameters){
//	std::string ret;
//	ret = method + "&" + url + "&";
//	bool is_first = true;
//	for (auto& i : parameters){
//		if (is_first) is_first = false;
//		else
//			ret += "%26";
//		ret += i.first + "%3D" + i.second ;
//	}
//}

int test_oauth(){
	sha1 sha;
	string s_abc("abc");
	sha.write(string_to_c_range(s_abc));
	std::cout << sha.get_digest() << std::endl;

	std::cout << "Expect: effcdf6ae5eb2fa2d27416d5f184df9c259a7c79\n";
	std::cout << hmac_sha1("Jefe", "what do ya want for nothing?") << std::endl;
	std::cout << hmac_sha1("key", "The quick brown fox jumps over the lazy dog") << std::endl;
	xirang2::string text("GET&http%3A%2F%2Fvenus-staging.autodesk.com%2FAsset%2FCloudSearch&oauth_consumer_key%3D62e8bac8-76b1-4ebc-8c1e-0bdd249a9329%26oauth_nonce%3DGJ8TGH%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D1418630594%26oauth_token%3DipiF6j7CQDrtWe%252FJ1YvAQA0hT2w%253D%26oauth_version%3D1.0");
	auto s = hmac_sha1("2ffdadda-8386-46dd-bfa3-fb877bfff070&%2B0C4k5AEd0lZtZ2LtFR2x07cru8%3D", text);
	std::cout << s << std::endl;
	std::cout << base64_encode((const char*)s.v, sizeof(s.v)).c_str() << std::endl;
	
	return 0;
}
void test_login();

int main() {
    //test_oauth();
    test_login();
}