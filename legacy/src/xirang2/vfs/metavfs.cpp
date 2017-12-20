#include <xirang2/vfs/metavfs.h>
#include <xirang2/vfs/metadatacache.h>

namespace xirang2{ namespace vfs{
    
    struct MetadataVfs::MetadataVfsImp {
        RootFs* root = nullptr;
        IVfs*  underling = nullptr;
		MetadataCache* cache = nullptr;
        file_path resource;
        DefaultMetadataCacheHandler handler;
        explicit MetadataVfsImp(type::Xirang& xr) : handler(xr){}
	};

	MetadataVfs::MetadataVfs(type::Xirang& xr, IVfs& underling_, sub_file_path res, MetadataCache& metacache)
        : m_imp(new MetadataVfs::MetadataVfsImp(xr))
	{
		m_imp->underling = &underling_;
		m_imp->cache = &metacache;
		m_imp->resource = res;
        metacache.register_fs(m_imp->underling, m_imp->handler);
	}

	MetadataVfs::~MetadataVfs(){
		m_imp->cache->clear_fs(this);
	}

	fs_error MetadataVfs::do_remove(sub_file_path path){
		auto ret = m_imp->underling->remove(path);
		if (ret == fs::er_ok)
			m_imp->cache->remove_item(this, path);
		return ret;
	}

	fs_error MetadataVfs::do_createDir(sub_file_path path){
		return  m_imp->underling->createDir(path);
	}

	fs_error MetadataVfs::do_copy(sub_file_path from, sub_file_path to){
		auto ret = m_imp->underling->copy(from, to);
		if (ret != fs::er_ok)	return ret;

		auto meta = m_imp->cache->try_fetch(m_imp->underling, from);
		if (meta)
			m_imp->cache->set(m_imp->underling, to,  *meta);

		return ret;
	}

	fs_error MetadataVfs::do_move(sub_file_path from, sub_file_path to){
        auto meta = m_imp->cache->try_fetch(m_imp->underling, from);
		auto ret = m_imp->underling->move(from, to);
		if (ret != fs::er_ok)	return ret;

        if (meta){
            m_imp->cache->set(m_imp->underling, to,  *meta);
            m_imp->cache->remove_item(m_imp->underling, from);
        }
        
		return ret;
	}

	fs_error MetadataVfs::do_truncate(sub_file_path path, long_size_t s){
		return m_imp->underling->truncate(path, s);
	}

	void MetadataVfs::do_sync(){
		m_imp->cache->flush();
		m_imp->underling->sync();
	}

	const string& MetadataVfs::do_resource() const { return m_imp->resource.str();}

	RootFs* MetadataVfs::do_root() const{ return m_imp->root;}

	bool MetadataVfs::do_mounted() const{ return m_imp->root != nullptr;}

	file_path MetadataVfs::do_mountPoint() const { 
		return m_imp->root ? m_imp->root->mountPoint(*this) : file_path();
	}
    
    namespace {
        struct metadatavfs_state_selector{
            typedef const VfsState value_type;
            typedef const VfsState* pointer;
            typedef const VfsState& reference;
            
			MetadataVfs* vfs;
            file_path dir;
            io_option option;
            mutable VfsState fst;
            
            metadatavfs_state_selector(MetadataVfs* vfs_, file_path d, io_option opt)
                : vfs(vfs_), dir(d), option(opt)
            {
			}
            const VfsState& operator()(const VfsState & item) const{
				fst = item;

                fst.path = item.path;
				fst.owner_fs = vfs;
				fst.metadata = MetadataValue();
                
                if (option == io::ao_metadata){
                    if (auto meta = vfs->getImp().cache->try_fetch(&vfs->underling(), dir / fst.path)){
                        fst.metadata = *meta;
                    }
                }
                return fst;
            }
        };
    }

	VfsNodeRange MetadataVfs::do_children(sub_file_path path, io_option option) const{
        auto result = underling().children(path, option);
        if (result.empty())
            return result;
        
        metadatavfs_state_selector sel(const_cast<MetadataVfs*>(this), path.parent(), option);
        return VfsNodeRange(
                            VfsNodeRange::iterator(make_select_iterator(result.begin(), sel)),
                            VfsNodeRange::iterator(make_select_iterator(result.end(), sel))
                            );
        
        
	}

	VfsState MetadataVfs::do_state(sub_file_path path, io_option option) const{
		auto st = underling().state(path, option);
        st.owner_fs = const_cast<MetadataVfs*>(this);
        if (st.state < fs::st_error_placeholder){
            if (auto meta = m_imp->cache->try_fetch(m_imp->underling, path))
				st.metadata = *meta;
        }
        return st;
	}

	IVfs& MetadataVfs::underling() const{
		return *m_imp->underling;
	}

	void** MetadataVfs::do_create(unsigned long long mask,
                                  void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
        return underling().raw_create(mask, ret, owner, path, flag);
    }

	any MetadataVfs::do_getopt(int id, const any & optdata) const {
        if (id == io::ao_metadata || id == io::ao_file_type){
            const file_path* path = xirang2::any_cast<const file_path>(&optdata);
            if (!path)  return any();
            auto meta = m_imp->cache->try_fetch(m_imp->underling, *path);
			if (!meta) return any();
			if (id == io::ao_metadata) return any(*meta);
			return any(meta->file_type);
        }
        
        return underling().getopt(id, optdata);

	}
    any MetadataVfs::do_setopt(int id, const any & optdata,  const any & indata){
        if (id == io::ao_metadata || id == io::ao_file_type){
            const file_path* path = xirang2::any_cast<const file_path>(&optdata);
            if (!path) return any();
			if (underling().state(*path).state == fs::st_not_found)	return any();
            
			if (id == io::ao_metadata){
				const MetadataValue * meta = xirang2::any_cast<const MetadataValue>(&indata);
				if (!meta) return any();

				m_imp->cache->set(m_imp->underling, *path, *meta);
			}
			else {
				const file_path *file_type = xirang2::any_cast<const file_path>(&indata);
				if (!file_type) return any();
				MetadataValue new_meta;
				if (auto meta = m_imp->cache->try_fetch(m_imp->underling, *path))
					new_meta = *meta;
				new_meta.file_type = *file_type;
				m_imp->cache->set(m_imp->underling, *path, std::move(new_meta));
			}
            return true;
        }
        
        return underling().setopt(id, optdata, indata);
        
    }
	void MetadataVfs::do_setRoot(RootFs* r){
		XR_PRE_CONDITION(!mounted() || r == nullptr);
		m_imp->root = r;
	}
    MetadataVfs::MetadataVfsImp& MetadataVfs::getImp() { return *m_imp;}

}}


