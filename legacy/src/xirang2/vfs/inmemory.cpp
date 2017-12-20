#include <xirang2/vfs/inmemory.h>
#include <xirang2/io/memory.h>

#include <xirang2/vfs/vfs_common.h>
#include <xirang2/string_algo/string.h>
#include <xirang2/buffer.h>

#include "file_tree.h"

namespace xirang2{ namespace vfs{ 
	namespace {
		typedef private_::file_node<buffer<byte> > node_type;
		struct ftree_state_selector{
			typedef const VfsState value_type;
			typedef const VfsState* pointer;
			typedef const VfsState& reference;

			IVfs* vfs;
			io_option option;
			mutable VfsState fst;

			ftree_state_selector(IVfs* vfs_, io_option opt) : vfs(vfs_), option(opt){}
			const VfsState& operator()(const std::pair<const file_path, std::unique_ptr<node_type> >& item) const{
				fst.assign(item.second->state, option);
				fst.path = item.first;
				fst.size = item.second->state.state == fs::st_dir ? 0 : item.second->data.size();
				fst.owner_fs = vfs;
				return fst;
			}
		};
	}
	class InMemoryImp
	{
		typedef private_::file_node<buffer<byte> > file_node;
		public:

		InMemoryImp(const string& res, IVfs* host)
			: m_resource(res), m_host(host), m_root(0), m_readonly(false)
		{
			m_root_node.state.owner_fs = m_host;
		}

		~InMemoryImp()
		{ }

		fs_error remove(sub_file_path path)
		{
            XR_PRE_CONDITION(!path.is_absolute());
            if (m_readonly)
                return fs::er_permission_denied;

            fs_error ret = remove_check(*host(), path);
			if (ret == fs::er_ok)
			{
				auto pos = locate(m_root_node, path);
				XR_PRE_CONDITION(pos.node);
                return pos.not_found.empty()
					? removeNode(pos.node)
					: fs::er_not_found;
			}

			return ret;
		}
		// dir operations
		// \pre !absolute(path)
		fs_error createDir(sub_file_path path)
		{
			XR_PRE_CONDITION(!path.is_absolute());
            if (m_readonly)
                return fs::er_permission_denied;

			auto pos = locate(m_root_node, path);
			if (pos.not_found.empty())
				return fs::er_exist;
			if (pos.node->state.state != fs::st_dir)
				return fs::er_not_dir;

			if (!pos.not_found.parent().empty())
				return fs::er_not_found;

			auto ret = create_node(m_host, pos, fs::st_dir, false);
			XR_POST_CONDITION(ret);
			unuse(ret);
			return fs::er_ok;
		}

		// file operations
		io::buffer_io create(sub_file_path path, int flag)
		{
			XR_PRE_CONDITION(!path.is_absolute());

            if ( m_readonly) XR_THROW(fs::permission_denied_exception);

            auto pos = locate(m_root_node, path);
            if((flag & io::of_open_create_mask) == io::of_create && pos.not_found.empty()) XR_THROW(fs::exist_exception);
			if((flag & io::of_open_create_mask) == io::of_open && !pos.not_found.empty()) XR_THROW(fs::not_found_exception);
			if (!pos.not_found.parent().empty())
				XR_THROW(fs::not_found_exception);
			if (pos.node->state.state != fs::st_dir && !pos.not_found.empty())
				XR_THROW(fs::not_found_exception);

			if (pos.node->state.state != fs::st_regular && pos.not_found.empty())
				XR_THROW(fs::not_regular_exception);

			if (!pos.not_found.empty()) {
				auto res = create_node(m_host, pos, fs::st_regular, false);
				return io::buffer_io(res->data);
			}
			else
				return io::buffer_io(pos.node->data);
		}

		io::buffer_in readOpen(sub_file_path path){
			XR_PRE_CONDITION(!path.is_absolute());

            auto pos = locate(m_root_node, path);
			if(!pos.not_found.empty()) XR_THROW(fs::not_found_exception);
			if (pos.node->state.state != fs::st_regular)
				XR_THROW(fs::file_type_exception);

			return io::buffer_in(pos.node->data);
		}

		// \pre !absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		fs_error copy(sub_file_path from, sub_file_path to)
		{
            XR_PRE_CONDITION(!to.is_absolute());
            XR_PRE_CONDITION(!from.is_absolute());
            if (m_readonly)
                return fs::er_permission_denied;

			auto from_st = state(from, io::io_option::ao_default);
			if (from_st.state == fs::st_not_found)
				return fs::er_not_found;
			if (from_st.state != fs::st_regular)
				return fs::er_not_regular;

			auto from_pos = locate(m_root_node, from);
			XR_INVARIANT(from_pos.node && from_pos.node->state.state == fs::st_regular);

			auto to_pos = locate(m_root_node, to);
			if (!to_pos.not_found.empty() && to_pos.node->state.state != fs::st_dir)
				return fs::er_not_dir;

			auto node = to_pos.node;
			if (!to_pos.not_found.empty())
				node = create_node(m_host, to_pos, fs::st_regular, true);

			if (node->state.state != fs::st_regular)
				return fs::er_not_regular;

			node->data = from_pos.node->data;
			node->state.metadata= from_pos.node->state.metadata;
			node->state.modified_time = from_pos.node->state.modified_time;
			return fs::er_ok;
		}

		fs_error move(sub_file_path from, sub_file_path to)
		{
            XR_PRE_CONDITION(!from.is_absolute());
            XR_PRE_CONDITION(!to.is_absolute());
            if (m_readonly)
                return fs::er_permission_denied;

			if (from.contains(to))
				return fs::er_invalid;

			auto from_pos = locate(m_root_node, from);
			if (!from_pos.not_found.empty())
				return fs::er_not_found;
			auto from_node = from_pos.node;

			auto to_pos = locate(m_root_node, to);
			auto to_node = to_pos.node;
			if (to_pos.not_found.empty()){	// target exists
				 if ((to_node->state.state != from_node->state.state) // if target is dir, can't move
					|| (to_node->state.state == fs::st_dir))
					return fs::er_exist;
			}
			else {
				to_node = create_node(m_host, to_pos, from_node->state.state, true);
			}

			to_node->state = from_node->state;
			to_node->state.path = to.filename();

			for(auto& i : from_node->children)
				i.second->parent = to_node;
			to_node->children.swap(from_node->children);
			to_node->data.swap(from_node->data);

			removeNode(from_node);

			return fs::er_ok;
		}

		fs_error truncate(sub_file_path path, long_size_t s)
		{
            XR_PRE_CONDITION(!path.is_absolute());
            if (m_readonly)
                return fs::er_permission_denied;

			auto pos = locate(m_root_node, path);
			if (!pos.not_found.empty())
				return fs::er_not_found;
			if (pos.node->state.state != fs::st_regular)
				return fs::er_not_regular;

			pos.node->data.resize(s);
			return fs::er_ok;

		}

		void sync() { }

		// query
		const string& resource() const { return m_resource;}

		// volume
		// if !mounted, return null
		RootFs* root() const{ return m_root;}

		// \post mounted() && root() || !mounted() && !root()
		bool mounted() const { return m_root != 0; }

		// \return mounted() ? absolute() : empty() 
		file_path mountPoint() const 
		{
			return m_root ? m_root->mountPoint(*m_host) : file_path();
		}


		// \pre !absolute(path)
		VfsNodeRange children(sub_file_path path, io_option option) const 
		{
			auto pos = locate(const_cast<file_node&>(m_root_node), path);
			if (pos.not_found.empty() && pos.node->state.state == fs::st_dir)
			{
                auto beg = pos.node->children.begin();
                auto end = pos.node->children.end();
				ftree_state_selector sel(m_host, option);
				return VfsNodeRange(
                    VfsNodeRange::iterator(make_select_iterator(beg, sel)),
					VfsNodeRange::iterator(make_select_iterator(end, sel))
						);
			}
			return VfsNodeRange();
		}


		// \pre !absolute(path)
		VfsState state(sub_file_path path, io_option option) const
		{
			XR_PRE_CONDITION(!path.is_absolute());

			auto pos = locate(const_cast<file_node&>(m_root_node), path);
			if (!pos.not_found.empty() || pos.node == nullptr)
				return VfsState(m_host, fs::st_not_found, path);

			VfsState fst;
			fst.assign(pos.node->state, io_option::ao_default);
			if (option == io_option::ao_metadata)
				fst.metadata = pos.node->state.metadata;
			fst.size = pos.node->data.size();
			fst.path = path;
			return fst;
		}

		void setRoot(RootFs* r) 
		{
			XR_PRE_CONDITION(!mounted() || r == 0);
			m_root = r;
		}
		IVfs* host() const { return m_host;}

        any getopt(int id, const any & optdata) const {

			switch(id){
				case io_option::ao_readonly:
					return any(m_readonly);
				case io_option::ao_file_type:
					{
						auto path = any_cast<sub_file_path>(&optdata);
						if (path){
							auto pos = locate(const_cast<file_node&>(m_root_node), *path);
							if (pos.not_found.empty())
								return any(pos.node->state.metadata.file_type);
						}
					}
					break;
				case io_option::ao_metadata:
					break;
				default:
					;
			}
            return any();
        }

        any setopt(int id, const any & optdata,  const any & /*indata*/){
            if (id == io_option::ao_readonly){
                m_readonly = any_cast<bool>(optdata);
                return any(m_readonly);
            }
            return any();
        }

		private:

		file_node m_root_node;
		string m_resource;
		IVfs* m_host;
		RootFs* m_root;

        bool m_readonly;
	};


	InMemory::InMemory(const string& resource)
		: m_imp(new InMemoryImp(resource, this))
	{}

	InMemory::~InMemory() { check_delete(m_imp);}

	// common operations of dir and file
	// \pre !absolute(path)
	fs_error InMemory::do_remove(sub_file_path path)
	{
		return m_imp->remove(path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error InMemory::do_createDir(sub_file_path path) { 
		return m_imp->createDir(path);
	}

	// file operations
	io::buffer_io InMemory::writeOpen(sub_file_path path, int flag){
		return m_imp->create(path, flag);
	}
	io::buffer_in InMemory::readOpen(sub_file_path path){
		return m_imp->readOpen(path);
	}

	// \pre !absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	// otherwise, from should be a
	fs_error InMemory::do_copy(sub_file_path from, sub_file_path to) {
		return m_imp->copy(from, to);
	}

	fs_error InMemory::do_move(sub_file_path from, sub_file_path to) {
		return m_imp->move(from, to);
	}

	fs_error InMemory::do_truncate(sub_file_path path, long_size_t s)
	{
		return m_imp->truncate(path, s);
	}

	void InMemory::do_sync() { return m_imp->sync();}

	// query
	const string& InMemory::do_resource() const { return m_imp->resource();}

	// volume
	// if !mounted, return null
	RootFs* InMemory::do_root() const { return m_imp->root();}

	// \post mounted() && root() || !mounted() && !root()
	bool InMemory::do_mounted() const { return m_imp->mounted(); }

	// \return mounted() ? absolute() : empty() 
	file_path InMemory::do_mountPoint() const { return m_imp->mountPoint();}

	// \pre !absolute(path)
	VfsNodeRange InMemory::do_children(sub_file_path path, io_option option) const { 
		return m_imp->children(path, option); 
	}

	// \pre !absolute(path)
	VfsState InMemory::do_state(sub_file_path path, io_option option) const { 
		return m_imp->state(path, option); 
	}
	// if r == null, means unmount
	void InMemory::do_setRoot(RootFs* r) { return m_imp->setRoot(r); }

	any InMemory::do_getopt(int id, const any & optdata /*= any() */) const 
	{
		return m_imp->getopt(id, optdata);
	}
	any InMemory::do_setopt(int id, const any & optdata,  const any & indata/*= any()*/)
	{
		return m_imp->setopt(id, optdata, indata);
	}
	void** InMemory::do_create(unsigned long long mask,
			void** base, unique_ptr<void>& owner, sub_file_path path, int flag){

		void** ret = 0;
		if (mask & detail::get_mask<io::writer, io::write_map>::value ){ //write open
			unique_ptr<io::buffer_io> ar(new io::buffer_io(writeOpen(path, flag)));
			iref<io::reader, io::writer, io::random, io::ioctrl, io::read_map, io::write_map> ifile(*ar);
			ret = copy_interface<io::reader, io::writer, io::random, io::ioctrl, io::read_map, io::write_map>::apply(mask, base, ifile, (void*)ar.get()); 
			unique_ptr<void>(std::move(ar)).swap(owner);
		}
		else{ //read open
			unique_ptr<io::buffer_in> ar(new io::buffer_in(readOpen(path)));
			iref<io::reader, io::random, io::read_map> ifile(*ar);
			ret = copy_interface<io::reader, io::random, io::read_map>::apply(mask, base, ifile, (void*)ar.get()); 
			unique_ptr<void>(std::move(ar)).swap(owner);
		}
		return ret;
	}
}}


