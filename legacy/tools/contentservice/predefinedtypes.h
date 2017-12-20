#ifndef PREDEFINED_TYPES_H__
#define PREDEFINED_TYPES_H__

namespace CS{

struct hash_type{

	std::size_t operator()(xirang2::type::Type t) const{
		return (size_t)*(void**)&t;
	}
};

}//namespace

#endif  //PREDEFINED_TYPES_H__
