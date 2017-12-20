#include <xirang2/vfs/vfs_common.h>

namespace xirang2{ namespace vfs{ 
    fs_error remove_check(IVfs& fs, sub_file_path path)
	{
		XR_PRE_CONDITION(!path.is_absolute());

		if (fs.mounted())
		{
			auto mount_point = fs.root()->mountPoint(fs);
			if (fs.root()->containMountPoint(mount_point / path))
				return fs::er_busy_mounted;
		}

		VfsState st = fs.state(path);

		if (st.state == fs::st_invalid)
			return fs::er_invalid;

		if (st.state == fs::st_not_found)
			return fs::er_not_found;

		return fs::er_ok;
	}

	fs_error copyFile(const VfsState& from, const VfsState& to)
	{
		using io::reader;
		using io::writer;
		using io::sequence;

		try{
			auto src = from.owner_fs->create<reader, sequence>(from.path, io::of_open);
			auto dest = to.owner_fs->create<writer>(to.path, io::of_create_or_open);

			long_size_t copied_size = copy_data(src.get<reader>(), dest.get<writer>());
			if (copied_size != src.get<sequence>().size())
				return fs::er_system_error;

			auto file_type = from.owner_fs->getopt(io_option::ao_file_type, from.path);
			if (!file_type.empty())
				to.owner_fs->setopt(io_option::ao_file_type, to.path, file_type);

			auto meta = from.owner_fs->getopt(io_option::ao_metadata, from.path);
			if (!meta.empty())
				to.owner_fs->setopt(io_option::ao_metadata, to.path, meta);
			return fs::er_ok;
		}
		catch(fs::not_found_exception&){
			return fs::er_not_found;
		}
		catch(fs::system_error_exception&){
			return fs::er_system_error;
		}
		catch(fs::open_failed_exception&){
			return fs::er_open_failed;
		}
		catch(fs::permission_denied_exception&){
			return fs::er_permission_denied;
		}
		catch(fs::create_exception&){
			return fs::er_create;
		}
		catch(xirang2::exception&){
		}
		catch(...){
		}

		return fs::er_invalid;
	}

	fs_error moveFile(const VfsState& from, const VfsState& to)
	{
		auto ret1 = copyFile(from, to);
		if (ret1 == fs::er_ok)
			return from.owner_fs->remove(from.path);

		if (to.owner_fs->state(to.path).state == fs::st_not_found)
			return ret1;

		auto ret2 = to.owner_fs->remove(to.path);

		return ret2 == fs::er_ok ? ret1 : ret2;
	}
}}


