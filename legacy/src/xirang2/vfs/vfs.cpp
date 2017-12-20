#include <xirang2/vfs.h>
#include <xirang2/string_algo/string.h>
#include <xirang2/utility/make_reverse_iterator.h>
#include <xirang2/vfs/vfs_common.h>
#include <xirang2/serialize/exchs11n.h>
#include <xirang2/serialize/path.h>
#include <xirang2/type/xirang.h>
#include <xirang2/type/itypebinder.h>
#include <xirang2/vfs/metadatacache.h>
#include <xirang2/serialize/versiontype.h>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include <unordered_map>
#include <chrono>

namespace xirang2{ namespace vfs{
	///////////// MetadataValue  ////////////////////
	MetadataValue::MetadataValue(){};
	MetadataValue::MetadataValue(file_path path, type::CommonObject obj)
		: file_type(std::move(path)), m_metadata(obj)
	{ }
	MetadataValue::MetadataValue(const MetadataValue& rhs)
		: file_type(rhs.file_type)
		  , m_metadata(cloneObject(rhs.m_metadata))
	{}
	MetadataValue::MetadataValue(MetadataValue&& rhs)
		: file_type(std::move(rhs.file_type))
		  , m_metadata(rhs.m_metadata)
	{
		rhs.m_metadata = type::CommonObject();
	}
	MetadataValue& MetadataValue::operator=(MetadataValue rhs){
		file_type = std::move(rhs.file_type);
		m_metadata = rhs.m_metadata;
		rhs.m_metadata = type::CommonObject();
		return *this;
	}

	MetadataValue::~MetadataValue(){
		clean_metadata();
	}
	bool MetadataValue::empty() const{
		return file_type.empty() && !m_metadata.valid();
	}
	type::CommonObject MetadataValue::metadata() const{
		return m_metadata;
	}
	void MetadataValue::take_metadata(type::CommonObject from){
		clean_metadata();
		m_metadata = from;
	}
	void MetadataValue::assign_metadata(type::ConstCommonObject from){
		auto temp = type::cloneObject(from);
		clean_metadata();
		m_metadata = temp;
	}
	void MetadataValue::clean_metadata(){
		if (m_metadata.valid())
			xirang2::type::ObjectDeletor(xirang2::memory::get_global_heap()).destroy(m_metadata);
	}

	void save(io::writer& wr, const MetadataValue& v){
		auto sink = io::exchange::as_sink(wr);
		bool is_empty = v.empty();
		sink & is_empty;

		if (is_empty)	return;
		
		// order:
		// file type, metadata valid flag
		// if the flag is true:
		// type path, type version, 
		// obj blob
		sink & v.file_type;

		bool is_valid = v.metadata().valid();
		sink & is_valid;
		if (is_valid){
			auto type = v.metadata().type();
			sink & type.fullName() & type.version();
			type.methods().serialize(wr, v.metadata());
		}
	}
	void load(io::reader& rd, MetadataValue& value, type::Xirang& xr, type::Namespace temp_root){
		auto source = xirang2::io::exchange::as_source(rd);
		bool is_empty = false;
		source & is_empty;
		if (is_empty){
			value.file_type = file_path();
			value.clean_metadata();
			return;
		}

		bool is_valid = false;
		source & value.file_type & is_valid;

		if (is_valid){
			file_path type_path;
			version_type version;
			source & type_path & version;
			auto type = xr.resolveType(type_path, version, temp_root);
			if (!type.valid()) 
				XR_THROW(failed_resolve_type_exception);
			xirang2::type::ScopedObjectCreator obj_creator(type, xr);
			type.methods().deserialize(rd, obj_creator.get(), xr.get_heap(), xr.get_ext_heap());
			value.take_metadata(obj_creator.release());
		}
	}

	///////////// VfsState  ////////////////////
	VfsState::VfsState()
		: owner_fs(nullptr)
	{
	}
	VfsState::VfsState(IVfs* owner, file_state st, file_path p, bool withTime)
		: owner_fs(owner)
	{
		path = p;
		state = st;

		if (withTime){
			create_time = std::chrono::nanoseconds(std::chrono::system_clock::now().time_since_epoch()).count();
			modified_time = create_time;
		}

	}
	VfsState::~VfsState(){
	}
	VfsState::VfsState(const VfsState& rhs)
		: fstate(rhs)
		, owner_fs(rhs.owner_fs)
		  ,  metadata(rhs.metadata)
	{
	}

	VfsState::VfsState(VfsState&& rhs)
		: fstate(std::move(rhs))
		  , owner_fs(rhs.owner_fs)
		  , metadata(std::move(rhs.metadata))
	{
	}
	VfsState& VfsState::operator=(const VfsState& rhs){
		return assign(rhs, io_option::ao_metadata);
	}
	VfsState& VfsState::operator=(VfsState&& rhs){
		if (this != &rhs){
		 	path = std::move(rhs.path);
			owner_fs = rhs.owner_fs;
		    state = rhs.state;
			size = rhs.size;
		    create_time = rhs.create_time; 
			modified_time = rhs.modified_time;
			metadata = std::move(rhs.metadata);
		}
		return *this;
	}
	VfsState& VfsState::assign(const VfsState& rhs, io_option option){
		if (this != &rhs){
		 	path = rhs.path;
			owner_fs = rhs.owner_fs;
		    state = rhs.state;
			size = rhs.size;
		    create_time = rhs.create_time; 
			modified_time = rhs.modified_time;
			if (option == io_option::ao_metadata)
				metadata = rhs.metadata;
			else{
				metadata.file_type = rhs.metadata.file_type;
				metadata.clean_metadata();
			}
		}
		return *this;
	}

	IVfs::~IVfs(){}

    any IVfs::do_getopt(int, const any &) const
    {
        return any();
    }
    any IVfs::do_setopt(int, const any &,  const any &)
    {
        return any();
    }
    class RootFsImp
	{
        typedef boost::bimap<boost::bimaps::set_of<file_path, path_less>, boost::bimaps::unordered_set_of<IVfs*> > mount_map;
		typedef mount_map::value_type mount_value;
		struct mount_info_selector{
			typedef const MountInfo value_type;
			typedef const MountInfo* pointer;
			typedef const MountInfo& reference;

			mutable MountInfo info;

			const MountInfo& operator()(const mount_map::left_map::value_type& item) const{
				info.path = item.first;
				info.fs = item.second;
				return info;
			}
		};
	public:
		RootFsImp(const string& res, RootFs* host)
			: m_resource(res), m_host(host)
		{
			XR_PRE_CONDITION(host);
		}

		fs_error mount(sub_file_path dir, xirang2::unique_ptr<IVfs>& vfs){
			XR_PRE_CONDITION(vfs);
			auto ret = mount(dir, *vfs);
			if (ret == fs::er_ok){
				m_owned_fs[dir.str()] = std::move(vfs);
			}
			return ret;
		}
		// \pre is_absolute(dir) && is_normalized(dir)
		fs_error mount(sub_file_path dir, IVfs& vfs)
		{
			XR_PRE_CONDITION(dir.is_absolute() && dir.is_normalized());
			XR_PRE_CONDITION(!vfs.mounted());
			XR_PRE_CONDITION(0 == mountPoint(dir));

			VfsState st = locate(dir);

			if (st.state == fs::st_dir || (dir.is_root() && m_mount_map.empty()) )
			{
				m_mount_map.insert(mount_value(dir, &vfs));
				vfs.setRoot(m_host);
				return fs::er_ok;
			}

			return fs::er_not_a_mount_point;
		}

		fs_error unmount(sub_file_path dir)
		{
			if (containMountPoint(dir))
				return fs::er_busy_mounted;

			mount_map::left_iterator pos = m_mount_map.left.find(dir);
			XR_PRE_CONDITION(pos != m_mount_map.left.end());
            pos->second->setRoot(0);
			m_mount_map.left.erase(pos);
			m_owned_fs.erase(dir.str());
			return fs::er_ok;
		}

		void sync()
		{
            mount_map::right_iterator itr = m_mount_map.right.begin();
            mount_map::right_iterator iend = m_mount_map.right.end();
			for (;
					itr != iend; ++itr)
			{
				itr->first->sync();
			}
		}

		// query
		const string& resource() const {
			return m_resource;
		}

		// volume
		// return absolute name
		file_path mountPoint(const IVfs& p) const
		{
			IVfs* fs = const_cast<IVfs*>(&p);
			mount_map::right_const_iterator pos = m_mount_map.right.find(fs);
			return pos == m_mount_map.right.end()
                ? file_path()
				: pos->second;
		}
		IVfs* mountPoint(sub_file_path p) const
		{
			XR_PRE_CONDITION(p.is_absolute());
			mount_map::left_const_iterator pos = m_mount_map.left.find(p);
			return pos == m_mount_map.left.end()
				? 0
				: pos->second;
		}

		bool containMountPoint(sub_file_path path) const
		{
			XR_PRE_CONDITION(path.is_absolute());

            mount_map::left_const_iterator pos = m_mount_map.left.lower_bound(path);
			for(; pos != m_mount_map.left.end(); ++pos){
				if (pos->first.under(path))
					return true;
			}
			return false;
		}

		VfsRange mountedFS() const
		{
			mount_info_selector sel;
			return VfsRange(
                VfsRange::iterator(make_select_iterator(m_mount_map.left.begin(), sel)),
				VfsRange::iterator(make_select_iterator(m_mount_map.left.end(), sel))
					);
		}

		VfsState locate(sub_file_path path) const
		{
			XR_PRE_CONDITION(path.is_absolute());

            mount_map::left_const_iterator pos = m_mount_map.left.lower_bound(path);
			if (pos != m_mount_map.left.end()
					&& path == pos->first)
			{
				static const file_path mount_point_type_path(sub_file_path(literal("/sys/type/fs/mount_point")));

				VfsState st = pos->second->state(sub_file_path());
				st.state = fs::st_mount_point;;
				st.metadata.file_type = mount_point_type_path;
				return st;
			}

            auto rpos = xirang2::make_reverse_iterator(pos);
            auto rend = xirang2::make_reverse_iterator(m_mount_map.left.begin());
			for(; rpos != rend; ++rpos)
			{
				if ((rpos->first == path || rpos->first.contains(path)))
				{
					file_path rest(path);
					rest.replace_prefix(rpos->first, sub_file_path());
					rest.remove_absolute();
					return rpos->second->state(rest);

				}
			}

			return VfsState();
		}
	private:
		mount_map m_mount_map;
		std::unordered_map<string, xirang2::unique_ptr<IVfs>, hash_string> m_owned_fs;
		string m_resource;
		RootFs* m_host;
	};

	RootFs::RootFs(const string& res )
		: m_imp(new RootFsImp(res, this))
	{
	}

	RootFs::~RootFs()
	{
		check_delete(m_imp);
	}

	// \pre dir must be absolute name
	fs_error RootFs::mount(sub_file_path dir, IVfs& vfs)
	{
		return m_imp->mount(dir, vfs);
	}

	fs_error RootFs::mount(sub_file_path dir, xirang2::unique_ptr<IVfs>& vfs)
	{
		return m_imp->mount(dir, vfs);
	}

	fs_error RootFs::unmount(sub_file_path dir)
	{
		return m_imp->unmount(dir);
	}

	void RootFs::sync()
	{
		m_imp->sync();
	}

	// query
	const string& RootFs::resource() const
	{
		return m_imp->resource();
	}

	// volume
	// return absolute name
	file_path RootFs::mountPoint(const IVfs& p) const
	{
		return m_imp->mountPoint(p);
	}
	IVfs* RootFs::mountPoint(sub_file_path p) const
	{
		return m_imp->mountPoint(p);
	}

	VfsRange RootFs::mountedFS() const
	{
		return m_imp->mountedFS();
	}
	VfsState RootFs::locate(sub_file_path path) const
	{
		return m_imp->locate(path);
	}
	bool RootFs::containMountPoint(sub_file_path path) const
	{
		return m_imp->containMountPoint(path);
	}
	fs_error RootFs::remove(sub_file_path path){
		auto ret = locate(path);
		if (!ret.owner_fs)
			return fs::er_not_found;
		return ret.owner_fs->remove(ret.path);
	}
	fs_error RootFs::createDir(sub_file_path path){
		auto ret = locate(path);
		if (!ret.owner_fs)
			return fs::er_not_found;
		return ret.owner_fs->createDir(ret.path);
	}
	fs_error RootFs::copy(sub_file_path from, sub_file_path to) {
		auto ret = locate(from);
		if (!ret.owner_fs)
			return fs::er_not_found;

		auto ret2 = locate(to);
		if (ret.owner_fs == ret2.owner_fs)
			return ret.owner_fs->copy(ret.path, ret2.path);

		auto err =  copyFile(ret, ret2);
		return err;
	}

	fs_error RootFs::move(sub_file_path from, sub_file_path to) {
		auto ret = locate(from);
		if (!ret.owner_fs)
			return fs::er_not_found;

		auto ret2 = locate(to);
		if (ret.owner_fs == ret2.owner_fs)
			return ret.owner_fs->move(ret.path, ret2.path);

		return moveFile(ret, ret2);
	}

	fs_error RootFs::truncate(sub_file_path path, long_size_t s){
		auto ret = locate(path);
		if (!ret.owner_fs)
			return fs::er_not_found;
		return ret.owner_fs->truncate(ret.path, s);
	}
	VfsNodeRange RootFs::children(sub_file_path path, io_option option) const {
		auto ret = locate(path);
		if (!ret.owner_fs)
			return VfsNodeRange();
		return ret.owner_fs->children(ret.path, option);
	}
	VfsState RootFs::state(sub_file_path path, io_option option) const{
		auto ret = locate(path);
		if (!ret.owner_fs)
			return ret;
		return ret.owner_fs->state(ret.path, option);

	}
	void** RootFs::raw_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
		auto result = locate(path);
		if (!result.owner_fs)
			return 0;
		return result.owner_fs->raw_create(mask, ret, owner, result.path, flag);
	}

    file_path temp_dir(IVfs& vfs, sub_file_path template_, sub_file_path parent_dir)
    {
		auto ret = vfs.state(parent_dir).state;
		if (ret == fs::st_not_found)
            XR_THROW(fs::not_found_exception)("failed to locate the temp directory:")(parent_dir.str());

        if (ret != fs::st_dir)
            XR_THROW(fs::not_dir_exception)("failed to locate the temp directory:")(parent_dir.str());

		file_path prefix = parent_dir / template_;

        const int max_try = 100;
        for(int i = 0; i < max_try ; ++i)
        {
            file_path fpath = fs::private_::gen_temp_name(prefix);
            if (vfs.createDir(fpath) == fs::er_ok)
                return fpath;
        }

        XR_THROW(fs::create_exception)("failed to create temp file in directory:")(parent_dir.str());
    }

    fs_error recursive_remove(IVfs&vfs, sub_file_path path)
    {
        if (vfs.state(path).state == fs::st_dir)
        {
            auto rf = vfs.children(path);
            std::vector<VfsState> files;
            for (auto n : rf)
            files.push_back(n);

            for (auto& node : files)
            {
                fs_error ret = recursive_remove(*node.owner_fs, path / node.path);
                if (ret != fs::er_ok)
                    return ret;
            }
        }

        return vfs.remove(path);
    }

    fs_error recursive_create_dir(IVfs& vfs, sub_file_path path)
    {
		if (path.empty())
			return fs::er_ok;

		file_path current;
		for (auto & p : path){
			current /= p;

            fs::file_state st = vfs.state(current).state;
            if (st == fs::st_not_found)
            {
                fs_error err = vfs.createDir(current);
                if (err != fs::er_ok)
                    return err;
            }
            else if (st != fs::st_dir)
            {
                return fs::er_invalid;
            }
		}

		return fs::er_ok;
    }

}	//vfs
}	//xirang2
