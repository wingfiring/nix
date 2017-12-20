#include <xirang2/vfs/local.h>
#include <xirang2/type/xrbase.h>
#include <xirang2/io/file.h>
#include <xirang2/string_algo/utf8.h>
#include <xirang2/vfs/vfs_common.h>

// BOOST
#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

#ifndef MSVC_COMPILER_
#include <unistd.h>
#else
#include <cstdint>
#include <allocators>
#endif
namespace xirang2{ namespace vfs{
	namespace {
		struct local_state_selector{
			typedef const VfsState value_type;
			typedef const VfsState* pointer;
			typedef const VfsState& reference;

			IVfs* vfs;
			mutable VfsState fst;

			local_state_selector(IVfs* vfs_) : vfs(vfs_){}
			const VfsState& operator()(const boost::filesystem::directory_entry & item) const{
				static_cast<fs::fstate&>(fst) = fs::state(file_path(item.path().string(boost::filesystem::detail::utf8_codecvt_facet()).c_str()));
				fst.path = fst.path.filename();
				fst.owner_fs = vfs;
				return fst;
			}
		};
	}

	LocalFs::LocalFs(const file_path& dir)
		: m_root(0), m_resource(dir)
	{
	}
	LocalFs::~LocalFs()
	{
	}

	// common operations of dir and file
	// \pre !absolute(path)
	fs_error LocalFs::do_remove(sub_file_path path) {
        XR_PRE_CONDITION(!path.is_absolute());
        fs_error ret = remove_check(*this, path);
        if (ret != fs::er_ok)
            return ret;
        if (path.empty())
            return fs::er_invalid;
        return fs::remove(m_resource / path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error LocalFs::do_createDir(sub_file_path path){
        XR_PRE_CONDITION(!path.is_absolute());
        if (path.empty())
            return fs::er_invalid;
		return fs::create_dir(m_resource / path);
	}

	io::file LocalFs::writeOpen(sub_file_path path, int flag) {
        XR_PRE_CONDITION(!path.is_absolute());
        return io::file(m_resource / path, flag);

	}
	io::file_reader LocalFs::readOpen(sub_file_path path){
        XR_PRE_CONDITION(!path.is_absolute());
        return io::file_reader(m_resource / path);
	}
	void** LocalFs::do_create(unsigned long long mask,
			void** base, unique_ptr<void>& owner, sub_file_path path, int flag){
		using namespace io;

		void** ret = 0;
		if (mask & detail::get_mask<io::writer, io::write_map>::value ){ //write open
			unique_ptr<io::file> ar(new io::file(writeOpen(path, flag)));
			iref<reader, writer, io::random, ioctrl, read_map, write_map> ifile(*ar);
			ret = copy_interface<reader, writer, io::random, ioctrl, read_map, write_map >::apply(mask, base, ifile, (void*)ar.get());
			owner = std::move(ar);
		}
		else{ //read open
			unique_ptr<io::file_reader> ar(new io::file_reader(readOpen(path)));
			iref<reader, io::random, read_map> ifile(*ar);
			ret = copy_interface<reader, io::random, read_map>::apply(mask, base, ifile, (void*)ar.get());
			owner = std::move(ar);
		}
		return ret;
	}

	// \pre !absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	fs_error LocalFs::do_copy(sub_file_path from, sub_file_path to){
		XR_PRE_CONDITION(!to.is_absolute());
		XR_PRE_CONDITION(!from.is_absolute());

		auto from_st = state(from);
		if (from_st.state != fs::st_regular)
		{
			if (from_st.state == fs::st_not_found)	return fs::er_not_found;
			return fs::er_unsupport;
		}

		boost::filesystem::path psrc((m_resource/from).str().c_str(), boost::filesystem::detail::utf8_codecvt_facet());
		boost::filesystem::path pdest((m_resource/to).str().c_str(), boost::filesystem::detail::utf8_codecvt_facet());
		boost::system::error_code ec;
		//FIXME: the boost didn't build with flag -std=c++11
		//boost::filesystem::copy_file(psrc, pdest, boost::filesystem::copy_option::overwrite_if_exists, ec);
		boost::filesystem::copy(psrc, pdest, ec);
		if(!ec) return fs::er_ok;
		return fs::er_system_error;
	}
	fs_error LocalFs::do_move(sub_file_path from, sub_file_path to){
		XR_PRE_CONDITION(!to.is_absolute());
		XR_PRE_CONDITION(!from.is_absolute());

		auto from_st = state(from);
		if (from_st.state == fs::st_not_found)	return fs::er_not_found;

		boost::filesystem::path psrc((m_resource/from).str().c_str(), boost::filesystem::detail::utf8_codecvt_facet());
		boost::filesystem::path pdest((m_resource/to).str().c_str(), boost::filesystem::detail::utf8_codecvt_facet());
		boost::system::error_code ec;
		boost::filesystem::rename(psrc, pdest, ec);
		if(!ec) return fs::er_ok;
		return fs::er_system_error;
	}

	fs_error LocalFs::do_truncate(sub_file_path path, long_size_t s) {
		XR_PRE_CONDITION(!path.is_absolute());
		return fs::truncate(m_resource / path, s);
	}

	void LocalFs::do_sync() {
#ifndef MSVC_COMPILER_
        ::sync();
#endif
    }

	// query
	const string& LocalFs::do_resource() const { return m_resource.str(); }

	// volume
	// if !mounted, return null
	RootFs* LocalFs::do_root() const { return m_root; }

	// \post mounted() && root() || !mounted() && !root()
	bool LocalFs::do_mounted() const {
		return m_root != 0;
	}

	// \return mounted() ? absolute() : empty()
	file_path LocalFs::do_mountPoint() const { return m_root ? m_root->mountPoint(*this) : file_path();}

	// \pre !absolute(path)
	VfsNodeRange LocalFs::do_children(sub_file_path path, io_option  option ) const{
		XR_PRE_CONDITION(!path.is_absolute());
		VfsState st = do_state(path, option);
		if (st.state == fs::st_dir)
		{
			typedef boost::filesystem::directory_iterator iterator;;
			file_path real_path = m_resource/path;
			iterator first(
#ifdef WIN32_OS_
					(real_path.is_network() || real_path.is_pure_disk())
					? wstring(real_path.native_wstr() << literal("/")).c_str()
					: real_path.native_wstr().c_str()
#else
			real_path.str().c_str()
#endif
			), last;
			local_state_selector sel(const_cast<LocalFs*>(this));
			return VfsNodeRange(
					VfsNodeRange::iterator(make_select_iterator(first, sel)),
					VfsNodeRange::iterator(make_select_iterator(last, sel))
					);
		}
		return VfsNodeRange();
	}

	// \pre !absolute(path)
	VfsState LocalFs::do_state(sub_file_path path, io_option /* option */) const {
		XR_PRE_CONDITION(!path.is_absolute());

		VfsState fst;
		static_cast<fs::fstate&>(fst) = fs::state(m_resource/path);
		fst.owner_fs = const_cast<LocalFs*>(this);
		fst.path = path;

		return fst;
	}

	// if r == null, means unmount
	void LocalFs::do_setRoot(RootFs* r) {
		XR_PRE_CONDITION(!mounted() || r == 0);
		m_root = r;
	}

}
}
