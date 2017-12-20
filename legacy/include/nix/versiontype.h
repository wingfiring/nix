#ifndef XIRANG2_VERSION_TYPE_H__
#define XIRANG2_VERSION_TYPE_H__
#include <xirang2/sha1.h>
namespace xirang2{
	enum class hash_algorithm{
		ha_sha1 = 1,
	};
	struct version_type : totally_ordered<version_type>{
		uint16_t protocol_version = 1;
		uint16_t algorithm = uint16_t(hash_algorithm::ha_sha1);
		sha1_digest id;
		uint32_t conflict_id = 0;

		version_type() {};
		explicit version_type(const_range_string s){ sha1_from_string(id, s); }
		explicit version_type(const sha1_digest& s) : id(s){}
	};
	inline bool is_empty(const version_type& ver){ return is_empty(ver.id);}

	inline bool operator==(const version_type& lhs, const version_type& rhs){
		return lhs.id == rhs.id
			&& lhs.protocol_version == rhs.protocol_version
			&& lhs.algorithm == rhs.algorithm
			&& lhs.conflict_id == rhs.conflict_id;
	}
	inline bool operator<(const version_type& lhs, const version_type& rhs){
		return lhs.id < rhs.id
			|| (lhs.id == rhs.id &&  lhs.protocol_version < rhs.protocol_version)
			|| (lhs.protocol_version == rhs.protocol_version && lhs.algorithm < rhs.algorithm)
			|| (lhs.algorithm == rhs.algorithm && lhs.conflict_id < rhs.conflict_id);
	}

	struct hash_version_type{
		std::size_t operator()(const version_type& ver) const{
			return hash_sha1()(ver.id);
		}
	};

	typedef int32_t revision_type;
	const revision_type no_revision = -1;
}

#endif //end XIRANG2_VERSION_TYPE_H__


