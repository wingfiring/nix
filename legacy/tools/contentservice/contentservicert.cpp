#include "contentservicert.h"
#include <xirang2/serialize/s11nbasetype.h>
#include <xirang2/type/typeio.h>
#include <xirang2/protein/datatype.h>
#include "commandutils.h"
#include "jsonserializer.h"
#include "jsondeserializer.h"
#include <map>
#include <iostream>

namespace CS{

using namespace xirang2;
using namespace xirang2::type;
using namespace xirang2::protein;

void load_fs(vfs::IVfs& to, io::reader& rd){
	auto s = io::exchange::as_source(rd);
	while(rd.readable()){
		file_path path;
		s & path;
		auto dest = vfs::recursive_create<io::writer>(to, path, io::of_create_or_open);
		std::size_t size = io::exchange::load_size_t(s);
		copy_data(rd, dest.get<io::writer>(), size);
	}
}

void load_runtime(Namespace temp_root, vfs::RootFs& rootfs, Xirang& xr, const file_path& dir, const file_path& metabase){
	auto res = rootfs.locate(dir);
	XR_PRE_CONDITION(res.owner_fs);
	std::map<file_path, std::tuple<file_path, version_type>> type_table;
	try {
	    auto metar = rootfs.create<io::reader>(dir/metabase, io::of_open);
	    auto& rd = metar.get<io::reader>();
	    auto source = io::exchange::as_source(rd);
	    file_path obj_name;
	    while(rd.readable()){
		source & obj_name;
		auto& tp = type_table[obj_name];
		source & std::get<0>(tp) & std::get<1>(tp);
	    }
	}catch(...){}

	auto dest_ns = getOrCreateNamespace(temp_root, dir);

	BinaryTypeLoader loader(rootfs,  xr.root());
	for (auto&i : res.owner_fs->children(res.path)){
		auto path_in_this_vfs = res.path /  i.path;
		auto state = res.owner_fs->state(path_in_this_vfs);
		if (state.state == fs::st_regular){
			auto pos = type_table.find(i.path);
			if (pos == type_table.end()) continue;

			auto& tp = pos->second;
			auto objar = res.owner_fs->create<io::reader>(path_in_this_vfs, io::of_open);
			Type t = loader.load(std::get<0>(tp), std::get<1>(tp), temp_root);
			XR_PRE_CONDITION(t);
			ScopedObjectCreator obj_creator(t, xr);
			t.methods().deserialize(objar.get<io::reader>(), obj_creator.get(), xr.get_heap(), xr.get_ext_heap());
			obj_creator.adoptBy(dest_ns, i.path);

		}else{
			XR_PRE_CONDITION(state.state == fs::st_dir);
			load_runtime(temp_root, rootfs, xr, dir/i.path, metabase);
		}
	}
}

ContentServiceRT::ContentServiceRT(const ContentServiceRTParam& param):
xr(param.xrName, memory::get_global_heap(), memory::get_global_ext_heap()),
rootFs(param.rootfsName),
resourceFs(file_path(param.workDir)/file_path("resource")),
params(param)
{
	SetupXirang(xr);
	xirang2::protein::SetupSysTypes(xr);

	file_path work_dir(param.workDir);
	file_path root_slash("/");
	rootFs.mount(root_slash, memFs);

	recursive_create_dir(memFs, file_path(param.resPath));
	rootFs.mount(root_slash/file_path(param.resPath), resourceFs);

	{	// release file affer loaded
		io::file_reader f(work_dir/file_path(param.dataFile));
		iref<io::reader> ireader(f);
		load_fs(memFs, ireader.get<io::reader>());
	}
	NamespaceBuilder nb;
	load_runtime(nb.get(), rootFs, xr, root_slash, file_path(param.metabasePath));
	nb.adoptChildrenBy(xr.root());
    JSONSerializer::build_type_visitor(xr.root());
	JSONDeserializer::build_type_visitor(xr.root());
}

} //namespace
