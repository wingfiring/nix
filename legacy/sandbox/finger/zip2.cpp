#include <xirang2/zip.h>
#include <xirang2/vfs/local.h>
#include <xirang2/vfs/zipfs.h>
#include <xirang2/vfs/inmemory.h>
#include <xirang2/io/file.h>


#include <iostream>

void usage(){
	std::cout <<
		"zip add <zip file> <folder> ... \n"
		"zip list <zip file> \n"
		"zip extract <zip file> [dest]\n";
	exit(1);
}

using xirang2::vfs::IVfs;
using namespace xirang2;

void copy_fs(IVfs& from, IVfs& to, const file_path& sdir, const file_path& ddir){
	auto children = from.children(sdir);
	for (auto &i : children){
		file_path spath = sdir / i.path;
		file_path tpath = ddir / i.path;

		auto st = from.state(spath);
		if (st.state == xirang2::fs::st_dir){
			to.createDir(tpath);
			std::cout << spath.str() << "/" << std::endl;
			copy_fs(from, to, spath, tpath);
		}
		else {
			auto src = from.create<io::read_map>(spath, xirang2::io::of_open);
			auto dest = to.create<io::write_map>(tpath, xirang2::io::of_create_or_open);
			dest.get<io::write_map>().truncate(0);
			auto size = io::copy_data(src.get<io::read_map>(), dest.get<io::write_map>());
			std::cout << spath.str()<< "\t" << size << std::endl;
		}
	}
}

void add(const char* dest, char** src){
	auto path = xirang2::file_path(xirang2::string(dest), xirang2::pp_utf8check);
	auto file = xirang2::fs::recursive_create(path, xirang2::io::of_create_or_open);
	xirang2::iref<xirang2::io::read_map, xirang2::io::write_map> ifile(file);
	xirang2::vfs::InMemory cache;
	xirang2::vfs::ZipFs zfs(ifile, &cache);

	for (;*src; ++src){
		const char* s = *src;
		auto input = xirang2::file_path(xirang2::string(s), xirang2::pp_utf8check);
		auto st = xirang2::fs::state(input).state;
		if (st == xirang2::fs::st_regular){
			xirang2::io::file_reader  f(input);
			xirang2::iref<xirang2::io::read_map> fmap(f);
			auto dest = zfs.create<io::write_map>(input.filename(), io::of_create_or_open);
			copy_data(fmap.get<io::read_map>(), dest.get<io::write_map>());
		}
		else if (st == xirang2::fs::st_dir){
			xirang2::vfs::LocalFs vfs(input);
			zfs.createDir(input.filename());
			copy_fs(vfs, zfs, xirang2::file_path(), input.filename());
		}
		else {
			std::cerr << "Failed to read file " << input.str() << std::endl;
		}
	}

	zfs.sync();
}

void list2(xirang2::vfs::IVfs& zfs, const xirang2::file_path& path){
	auto children = zfs.children(path);
	for (auto& i : children){
		auto p = path / i.path;
		auto st = zfs.state(p);
		std::cout << p.str() ;
		if(st.state == xirang2::fs::st_dir){
			std::cout << "/" << std::endl;
			list2(zfs, p); 
		}
		else if (st.state == xirang2::fs::st_regular){
			std::cout << "\t" << st.size << std::endl;
		}
		else
			std::cout << " error\n";
	}
}

void list(const char* src)
{
	auto path = xirang2::file_path(xirang2::string(src), xirang2::pp_utf8check);
	xirang2::io::file_reader file(path);
	xirang2::iref<xirang2::io::read_map> file_map(file);
	xirang2::vfs::ZipFs zfs(file_map, 0);
	
	list2(zfs, xirang2::file_path());
}

void extract(const char* src, const char* dest_dir){
	auto path = xirang2::file_path(xirang2::string(src), xirang2::pp_utf8check);
	auto dest_path = xirang2::file_path(xirang2::string(dest_dir == 0 ? "." : dest_dir), xirang2::pp_utf8check);
	if (!xirang2::fs::exists(dest_path) && xirang2::fs::recursive_create_dir(dest_path) != xirang2::fs::er_ok){
		std::cerr << "Failed to create dir " << dest_path.str() << "\n";
		exit(2);
	}
		
	xirang2::vfs::LocalFs local_fs(dest_path);

	xirang2::io::file_reader file(path);
	xirang2::iref<xirang2::io::read_map> file_map(file);
	xirang2::vfs::InMemory cache;
	xirang2::vfs::ZipFs zfs(file_map, 0);

	copy_fs(zfs, local_fs, file_path(), file_path());
}

int main(int argc, char** argv){
	if (argc < 3) usage();
	if (argv[1] == std::string("add")){
		//if (argc < 4) usage();
		add (argv[2], argv + 3);
	}
	else if (argv[1] == std::string("list")){
		list (argv[2]);
	}
	else if (argv[1] == std::string("extract")){
		extract (argv[2], argv[3]);
	}

	return 0;
}


// 
// g++ -g -std=c++11 -o z2 zip2.cpp -I../../ -I/usr/local/include -L../../build_dir/debug/lib/ -L/usr/local/lib -lxirang -lz -lboost_system -lboost_filesystem
//
