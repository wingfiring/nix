//XIRANG2_LICENSE_PLACE_HOLDER

#ifndef XR_COMMON_FS_UTILITY_H__
#define XR_COMMON_FS_UTILITY_H__

#include <xirang2/config.h>
#include <xirang2/string.h>
#include <xirang2/exception.h>
#include <xirang2/io.h>
#include <xirang2/iterator.h>
#include <xirang2/io/file.h>
#include <xirang2/path.h>

namespace xirang2{ namespace fs{
    namespace private_{
        /// \note never use the returned name as file or dir name without check, otherwise may encouter security risk.
        /// It intends to be used to implement temp file/dir only.
        /// \post return start with template_
		XR_API file_path gen_temp_name(sub_file_path template_);
    }

//********************** begin core fs functions
    enum fs_error
    {
        er_ok,		///< no error
		er_invalid, 	///< invalid parameter
		er_busy_mounted,	
		er_not_found,
		er_exist,
		er_used_mount_point,
		er_not_a_mount_point,
		er_unmount_root,
		er_fs_not_found,
		er_system_error,
		er_open_failed,
		er_file_busy,
		er_not_regular,
		er_not_dir,
		er_permission_denied,
        er_not_empty,
		er_data_error,
		er_file_type,
		er_create,
		er_is_root,
        er_temp_unvaliable,
		er_unsupport,
    };
	XR_EXCEPTION_TYPE(invalid_exception);
	XR_EXCEPTION_TYPE(busy_mounted_exception);
	XR_EXCEPTION_TYPE(not_found_exception);
	XR_EXCEPTION_TYPE(exist_exception);
	XR_EXCEPTION_TYPE(used_mount_point_exception);
	XR_EXCEPTION_TYPE(not_a_mount_point_exception);
	XR_EXCEPTION_TYPE(unmount_root_exception);
	XR_EXCEPTION_TYPE(fs_not_found_exception);
	XR_EXCEPTION_TYPE(system_error_exception);
	XR_EXCEPTION_TYPE(open_failed_exception);
	XR_EXCEPTION_TYPE(file_busy_exception);
	XR_EXCEPTION_TYPE(not_regular_exception);
	XR_EXCEPTION_TYPE(not_dir_exception);
	XR_EXCEPTION_TYPE(permission_denied_exception);
	XR_EXCEPTION_TYPE(not_empty_exception);
	XR_EXCEPTION_TYPE(file_type_exception);
	XR_EXCEPTION_TYPE(create_exception);
	XR_EXCEPTION_TYPE(is_root_exception);
	XR_EXCEPTION_TYPE(temp_unvaliablei_exception);
	XR_EXCEPTION_TYPE(unsupport_exception);

    enum file_state
	{
		st_regular = 0,
		st_dir,
		st_symbol,
		st_socket,
		st_pipe,
		st_mount_point,
        
        st_error_placeholder,
        
        st_not_found,
        st_unknown,
        st_invalid,
        st_temp_unvaliable
	};

	XR_API fs_error remove(const file_path& path);
	XR_API fs_error create_dir(const file_path& path);

	XR_EXCEPTION_TYPE(file_copy_error);
	XR_API void copy(const file_path& from, const file_path& to);
	XR_API fs_error move(const file_path& from, const file_path& to);

	XR_API fs_error truncate(const file_path& path, long_size_t s);

    struct XR_API fstate
    {
        file_path path;
        file_state state;
		long_size_t  size;
		uint64_t create_time;
		uint64_t modified_time;

		fstate();
		~fstate();
    };

	XR_API fstate state(const file_path& path);

    typedef range<bidir_iterator<const_itr_traits<file_path> > > file_range;
	XR_API file_range children(const file_path& path);

// temp file related 

	// get OS temp dir
	XR_API file_path sys_temp_dir();
    // \throw xirang2::io::create_failed
	XR_API io::file temp_file(const file_path& template_ = file_path(literal("tmpf")), int flag = io::of_remove_on_close, file_path* result_path = 0);

    // \throw xirang2::io::create_failed
	XR_API io::file temp_file(const file_path& template_, const file_path& parent_dir, int flag = io::of_remove_on_close, file_path* result_path = 0);

    // \throw xirang2::io::create_failed
	XR_API file_path temp_dir(const file_path& template_ = file_path(literal("tmpd")));
    
    // \throw xirang2::io::create_failed
	XR_API file_path temp_dir(const file_path& template_, const file_path& parent_dir);

//********************** end core fs functions

//********************** begin helpers
	XR_API bool exists(const file_path& file);

    // if path is not a dir, remove it.
    // if the path is dir, remove the dir and children, recursilvey
	XR_API fs_error recursive_remove(const file_path& path);

    // create a dir, if the some dir of the paths are not exist, create them
	XR_API fs_error recursive_create_dir(const file_path& path);

    // create a file, if the some dir of the paths are not exist, create them
    // \throw xirang2::io::create_failed
	XR_API io::file recursive_create(const file_path& path, int flag);
}}

#endif //end XR_COMMON_FS_UTILITY_H__
