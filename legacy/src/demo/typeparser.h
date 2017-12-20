#ifndef DEMO_TYPE_PARSER_H
#define DEMO_TYPE_PARSER_H

#include <aio/common/buffer.h>
#include <aio/xirang2/xirang.h>

class TypeLoader
{
	public:
		void load(const aio::buffer<unsigned char>& buf, xirang2::Xirang& xr);
};
#endif //end DEMO_TYPE_PARSER_H
