#ifndef SRC_XIRANG2_VFS_VFS_COMMON_H__
#define SRC_XIRANG2_VFS_VFS_COMMON_H__

#include <xirang2/vfs.h>
#include <xirang2/type/object.h>
namespace xirang2{ namespace vfs{

	XR_API extern fs_error remove_check(IVfs& fs, sub_file_path path);
	XR_API extern fs_error copyFile(const VfsState& from, const VfsState& to);
	XR_API extern fs_error moveFile(const VfsState& from, const VfsState& to);
}}
#endif //end SRC_XIRANG2_VFS_VFS_COMMON_H__

