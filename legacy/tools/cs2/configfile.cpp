#include "configfile.h"
#include <xirang2/io/file.h>
#include <xirang2/type/xirang.h>
#include <xirang2/vfs/local.h>
#include <xirang2/type/typeio.h>
#include <xirang2/versionedvfs.h>
#include <xirang2/protein/datatype.h>
#include <xirang2/vfs/inmemory.h>

#include <xirang2/serialize/exchs11n.h>
#include <xirang2/serialize/path.h>

#ifdef MSVC_COMPILER_
#pragma warning( disable : 4995 )
#endif
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/searching/boyer_moore.hpp>


#include <thread>

#include <iostream>

namespace cs2{
    
    CMContext::CMContext()
    : runtime("cm", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap())
    , idx_file(nullptr)
    {
        SetupXirang(runtime);
        xirang2::protein::SetupSysTypes(runtime);
        
        std::shared_ptr<xirang2::vfs::IVfs> dummy_root(new xirang2::vfs::InMemory);
        vfs_deque.push_back(dummy_root);
        
        runtime.rootFs().mount(xirang2::file_path("/"), *dummy_root);
    }
    
    CMContext::~CMContext(){
        while(!vfs_deque.empty())
            vfs_deque.pop_back();
    }
    
    void CMContext::init_idx(const xirang2::file_path& file)
	{
		XR_PRE_CONDITION(!idx_file);
		idx_file.reset(new xirang2::io::file_reader(file));

		idx_view = idx_file->view_rd(xirang2::ext_heap::handle(0, idx_file->size()));
		auto address = idx_view->address();

		unsigned int nthreads = std::thread::hardware_concurrency();
		nthreads = nthreads > 0 ? nthreads : 1;
		std::size_t steps = idx_file->size() / nthreads;
		const char* start = reinterpret_cast<const char*>(address.begin());
		const char* end = reinterpret_cast<const char*>(address.end());

		typedef boost::algorithm::boyer_moore<const char*> bm_type;
		const char dim[2] = { 0,0 };
		bm_type bm_dim(dim, dim + sizeof(dim));
		auto pre_end = start;
		for (std::size_t i = 0; i < nthreads; ++i) {
			auto pos = pre_end + steps - sizeof(dim);
			pos = std::min(end - sizeof(dim), pos);

			auto last = bm_dim(pos, end);
			if (last != end) last += sizeof(dim);

			std::pair<const char*, const char*> section(pre_end, last);
			idx_table.push_back(section);

			pre_end = last;
		}
	}

    void LoadConfig(xirang2::fcgi::Context& ctx, const xirang2::file_path& config_path){
        xirang2::io::file_reader fin(config_path);
        auto view = fin.view_rd(xirang2::ext_heap::handle(0, fin.size()));
        
        auto beg = reinterpret_cast<const char*>(view->address().begin());
        auto end = reinterpret_cast<const char*>(view->address().end());
        
        std::string text(beg, end);
        std::stringstream sstr(text);
        
        using boost::property_tree::ptree;
        ptree pt;
        read_info(sstr, pt);
        
        auto error_pages = pt.get_child_optional("error_pages");
        if(error_pages){
            xirang2::file_path config_dir = config_path.parent();
            for(auto &page : error_pages.get()){
                auto code = boost::lexical_cast<int>(page.first);
                auto file_str = page.second.get_value<std::string>();
                
                xirang2::file_path path(file_str.c_str());
                if (!path.is_absolute())
                    path = config_dir / path;
                
                xirang2::io::file_reader f(path);
                auto fview = f.view_rd(xirang2::ext_heap::handle(0, f.size()));
                
                auto b = reinterpret_cast<const char*>(fview->address().begin());
                auto e = reinterpret_cast<const char*>(fview->address().end());

                ctx.error_pages[code] = std::string(b, e);
            }
        }
        
        auto mount_table = pt.get_child_optional("mount_table");
        if (mount_table){
            for(auto &mtab : mount_table.get()){
                xirang2::fcgi::VFSEntry entry;
                
                entry.path = mtab.first;
                entry.type = mtab.second.get<std::string>("type", "");
                entry.flag = mtab.second.get<std::string>("flag", "");
                entry.args = mtab.second.get<std::string>("args", "");
                
                ctx.mount_table.push_back(entry);
            }
        }
    }
    const xirang2::file_path path_metabase(".metabase");
    
    void load_runtime(CMContext& ctx, xirang2::type::Namespace temp_root, xirang2::vfs::RootFs& rootfs, xirang2::type::Xirang& xr, const xirang2::file_path& dir)
    {
        auto res = rootfs.locate(dir);
        XR_PRE_CONDITION(res.owner_fs);
        
        std::unordered_map<xirang2::file_path, std::tuple<xirang2::file_path, xirang2::version_type>, xirang2::hash_file_path> type_table;
        
        auto metar = rootfs.create<xirang2::io::reader>(dir / path_metabase, xirang2::io::of_open | xirang2::io::of_nothrow);
        if (!metar) {
            return;
        }
        
        auto& rd = metar.get<xirang2::io::reader>();
        auto source = xirang2::io::exchange::as_source(rd);
        xirang2::file_path obj_name;
        while(rd.readable()){
            source & obj_name;
            auto& tp = type_table[obj_name];
            source & std::get<0>(tp) & std::get<1>(tp);
            
            ctx.object_type_table[dir / obj_name] = tp;
        }
        
        auto dest_ns = getOrCreateNamespace(temp_root, dir);
        
        xirang2::type::BinaryTypeLoader loader(rootfs, xr.root());
        for (auto&i : res.owner_fs->children(res.path)){
            auto path_in_this_vfs = res.path /  i.path;
            auto state = res.owner_fs->state(path_in_this_vfs);
            if (state.state == xirang2::fs::st_regular){
                auto pos = type_table.find(i.path);
                if (pos == type_table.end()) continue;
                
                auto& tp = pos->second;
                auto objar = res.owner_fs->create<xirang2::io::reader>(path_in_this_vfs, xirang2::io::of_open);
                xirang2::type::Type t = loader.load(std::get<0>(tp), std::get<1>(tp), temp_root);
                XR_PRE_CONDITION(t);
                xirang2::type::ScopedObjectCreator obj_creator(t, xr);
                t.methods().deserialize(objar.get<xirang2::io::reader>(), obj_creator.get(), xr.get_heap(), xr.get_ext_heap());
                obj_creator.adoptBy(dest_ns, i.path);
                
            }else{
                XR_PRE_CONDITION(state.state == xirang2::fs::st_dir);
                load_runtime(ctx, temp_root, rootfs, xr, dir/i.path);
            }
        }
    }
    
    void create_obj_path_map(xirang2::type::Namespace ns, const xirang2::file_path& prefix, t_object_path_table& table)
	{
		for (auto object : ns.objects())
			table.emplace(*object.name, prefix / *object.name);

		for (auto sub_ns : ns.namespaces())
			create_obj_path_map(sub_ns, prefix/sub_ns.name(), table);
	}

    void LoadConfigAndInit(xirang2::fcgi::Context& ctx, const xirang2::file_path& config_path){
        LoadConfig(ctx, config_path);
        
		auto user_context = std::make_shared<CMContext>();
		ctx.user_context = user_context;

		xirang2::file_path idx = config_path.parent()/xirang2::file_path("json.idx");
		user_context->init_idx(idx);

		for (auto& mtab : ctx.mount_table){
			xirang2::file_path mount_point(mtab.path);
			if (mtab.type == xirang2::literal("local_repo")){

				std::shared_ptr<xirang2::vfs::IVfs> vfs(new xirang2::vfs::LocalFs(config_path.parent() / xirang2::file_path(mtab.args)));
				user_context->cm_repo.reset(new xirang2::vfs::LocalRepository(*vfs, xirang2::file_path(), user_context->runtime));

				xirang2::file_path mount_point(mtab.path);
				auto relative_path  = mount_point;
				relative_path.remove_absolute();

				xirang2::vfs::recursive_create_dir(*user_context->runtime.rootFs().mountPoint(xirang2::file_path("/")), relative_path);
				user_context->runtime.rootFs().mount(mount_point, *user_context->cm_repo);

				user_context->vfs_deque.push_back(vfs);
				user_context->vfs_deque.push_back(user_context->cm_repo);


				std::shared_ptr<xirang2::vfs::IVfs> vfs_workspace(new xirang2::vfs::LocalFs(config_path.parent() / xirang2::file_path(mtab.args) / xirang2::file_path("stage")));
				user_context->vfs_deque.push_back(vfs_workspace);

				user_context->cm_workspace.reset(new xirang2::vfs::Workspace(*vfs_workspace, xirang2::string()));


				auto f = vfs->create<xirang2::io::reader>(xirang2::file_path("wkremove"), xirang2::io::of_open | xirang2::io::of_nothrow);
				if (f){
					auto source = xirang2::io::exchange::as_source(f.get<xirang2::io::reader>());
					using namespace xirang2;
					while(f.get<xirang2::io::reader>().readable()){
						user_context->cm_workspace->markRemove(load<xirang2::file_path>(source));
					}
				}
			}
			else {
				std::cout << "Unsupport fs type\n";
			}
		}

		load_runtime(*user_context, user_context->runtime.root(), user_context->runtime.rootFs(), user_context->runtime, xirang2::file_path("/lib/adsk/cm/1.0"));

		xirang2::file_path path("/lib/adsk/cm/1.0/prism/materialappearance");
		auto ns = user_context->runtime.root().locateNamespace(path);
		if (ns.valid())
			create_obj_path_map(ns, path, user_context->object_path_table);
	}
}
