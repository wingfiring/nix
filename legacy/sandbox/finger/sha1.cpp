#include <xirang2/io/file.h>
#include <xirang2/sha1.h>

#include <iostream>
#include <iomanip>

#include <xirang2/io/s11n.h>
#include <xirang2/io/memory.h>

int main(int argc, char** argv){
	for (int i = 1; i < argc; ++i){
		auto path = xirang2::file_path(xirang2::string((const char*)argv[i]), xirang2::pp_utf8check);
		xirang2::io::file_reader file(path);
		auto view = file.view_rd(xirang2::ext_heap::handle(0, file.size()));
		auto addr = view->address();
		xirang2::sha1 sha;
		sha.process_block(addr.begin(), addr.begin() + 10);
		sha.process_block(addr.begin() + 10, addr.end());
		auto & dig = sha.get_digest();

		std::cout << dig;
		std::cout << "  " << argv[i] << std::endl;

		auto dig2 = xirang2::sha1_digest(dig.to_string());
		std::cout << dig2;
		std::cout << "  " << argv[i] << std::endl;

		xirang2::io::mem_archive mar;
		auto serializer = xirang2::io::local::as_sink(mar);
		serializer & dig;

		xirang2::io::mem_archive mar2;
		auto serializer2 = xirang2::io::exchange::as_sink(mar2);
		serializer2 & dig2;
	}

	return 0;
}
