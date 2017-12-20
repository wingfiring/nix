#include <xirang2/vfs/subvfs.h>

namespace xirang2{ namespace vfs{

	SubVfs::SubVfs(IVfs& parent_, sub_file_path dir)
		: m_root(0), parent(parent_), m_resource(dir)
	{
	}

	SubVfs::~SubVfs()
	{	
	}

	// common operations of dir and file
	// \pre !absolute(path)
	fs_error SubVfs::do_remove(sub_file_path path) { 
        XR_PRE_CONDITION(!path.is_absolute());
        return parent.remove(m_resource / path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error SubVfs::do_createDir(sub_file_path path){
        XR_PRE_CONDITION(!path.is_absolute());
        return parent.createDir(m_resource / path);
	}

	// \pre !absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	fs_error SubVfs::do_copy(sub_file_path from, sub_file_path to){
        XR_PRE_CONDITION(!to.is_absolute());
        XR_PRE_CONDITION(!from.is_absolute());
		return parent.copy(from, m_resource / to);
	}

	fs_error SubVfs::do_move(sub_file_path from, sub_file_path to){
        XR_PRE_CONDITION(!to.is_absolute());
        XR_PRE_CONDITION(!from.is_absolute());
		return parent.move(from, m_resource / to);
	}

	fs_error SubVfs::do_truncate(sub_file_path path, long_size_t s) {
        XR_PRE_CONDITION(!path.is_absolute());
        return parent.truncate(m_resource / path, s);
	}

	void SubVfs::do_sync() { parent.sync();  }

	// query
	const string& SubVfs::do_resource() const { return m_resource.str(); }

	// volume
	// if !mounted, return null
	RootFs* SubVfs::do_root() const { return m_root; }

	// \post mounted() && root() || !mounted() && !root()
	bool SubVfs::do_mounted() const { 
        return m_root != 0;
    }

	// \return mounted() ? absolute() : empty() 
    file_path SubVfs::do_mountPoint() const { return m_root ? m_root->mountPoint(*this) : file_path();}

	namespace {
		struct subfs_state_selector{
			typedef const VfsState value_type;
			typedef const VfsState* pointer;
			typedef const VfsState& reference;

			IVfs* vfs;
			mutable VfsState fst;

			subfs_state_selector(IVfs* vfs_) : vfs(vfs_){}
			const VfsState& operator()(const VfsState& item) const{
				fst = item;
				fst.owner_fs = vfs;
				return fst;
			}
		};
	}

	// \pre !absolute(path)
	VfsNodeRange SubVfs::do_children(sub_file_path path, io_option option) const{
        XR_PRE_CONDITION(!path.is_absolute());
		subfs_state_selector sel(const_cast<SubVfs*>(this));
        auto children =  parent.children(m_resource / path, option);
		return VfsNodeRange(
					VfsNodeRange::iterator(make_select_iterator(children.begin(), sel)),
					VfsNodeRange::iterator(make_select_iterator(children.end(), sel))
				);
	}

	// \pre !absolute(path)
	VfsState SubVfs::do_state(sub_file_path path, io_option option) const {
        XR_PRE_CONDITION(!path.is_absolute());
        auto st =  parent.state(m_resource / path, option);
		st.owner_fs = const_cast<SubVfs*>(this);
		st.path = path;

		return st;
	}

	// if r == null, means unmount
	void SubVfs::do_setRoot(RootFs* r) { 
        XR_PRE_CONDITION(!mounted() || r == 0);
        m_root = r;
	}
	void** SubVfs::do_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
        XR_PRE_CONDITION(!path.is_absolute());
        return parent.raw_create(mask, ret, owner, m_resource / path,  flag);
	}

} }

