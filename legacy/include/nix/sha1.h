// boost/uuid/sha1.hpp header file  ----------------------------------------------//

// Copyright 2007 Andy Tompkins.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Revision History
//  29 May 2007 - Initial Revision
//  25 Feb 2008 - moved to namespace boost::uuids::detail
//  10 Jan 2012 - can now handle the full size of messages (2^64 - 1 bits)

// This is a byte oriented implementation

#ifndef XIRANG2_BOOST_UUID_SHA1_H__
#define XIRANG2_BOOST_UUID_SHA1_H__

#include <xirang2/context_except.h>
#include <xirang2/operators.h>
#include <xirang2/io.h>

#include <ostream>

namespace xirang2 {

class XR_API sha1;
XR_EXCEPTION_TYPE(sha1_runtime_exception);


struct sha1_digest{

	sha1_digest(){
		for (auto& i : v)
			i = 0;
	};

	sha1_digest(no_initialize){}

	bool operator==(const sha1_digest& rhs) const{
		return v[0] == rhs.v[0]
			&& v[1] == rhs.v[1]
			&& v[2] == rhs.v[2]
			&& v[3] == rhs.v[3]
			&& v[4] == rhs.v[4];
	}
	bool operator<(const sha1_digest& rhs) const{
		 return v[0] < rhs.v[0]
			 || (v[0] == rhs.v[0] && v[1] < rhs.v[1])
			 || (v[1] == rhs.v[1] && v[2] < rhs.v[2])
			 || (v[2] == rhs.v[2] && v[3] < rhs.v[3])
			 || (v[3] == rhs.v[3] && v[4] < rhs.v[4]);
	}
	uint32_t v[5];
};

XR_API sha1_digest sha1_from_string(const_range_string d);
XR_API sha1_digest& sha1_from_string(sha1_digest& s, const_range_string d);
XR_API string sha1_to_string(const sha1_digest& s);


inline bool is_empty(const sha1_digest& dig){
		for (auto i : dig.v) if (i != 0) return false;
		return true;
}

XR_API bool is_valid_sha1_str(const_range_string d);

struct sha1_digest_compare_ : totally_ordered<sha1_digest>{};

struct hash_sha1{
	template<typename T, size_t N> struct to_hash;
	template<typename T> struct to_hash<T, 4>{
		static T apply(const sha1_digest& d){
			return d.v[0];
		}
	};
	template<typename T> struct to_hash<T, 8>{
		static T apply(const sha1_digest& d){
			return (T(d.v[0]) << 32 ) + d.v[1];
		}
	};
	size_t operator()(const sha1_digest& d) const{
		return to_hash<size_t, sizeof(size_t)>::apply(d);
	}
};

XR_EXCEPTION_TYPE(bad_sha1_string_exception);

inline std::ostream& operator<<(std::ostream& outs, const sha1_digest& dig){
	return outs << sha1_to_string(dig);
}

enum class sha1_digest_represent{
	sdr_regular,
	sdr_internal
};


class XR_API sha1
{
public:
    sha1();
	// io::writer
	range<const byte*> write(const range<const byte*>& r);
	bool writable() const;
	void sync();

    void reset();

    void process_byte(byte b);

	sha1_digest get_digest(sha1_digest_represent sdr = sha1_digest_represent::sdr_regular) const;
	struct sha1_data{
		sha1_digest h_;
		uint8_t block_[64];
		std::size_t block_byte_index_;
		std::size_t bit_count_low;
		std::size_t bit_count_high;
	};

private:
	sha1_data data;

};

} // namespace xirang2

#endif

