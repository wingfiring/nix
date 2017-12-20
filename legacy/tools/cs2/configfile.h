#ifndef XIRANG2_CS2_CONFIG_FILE_H_
#define XIRANG2_CS2_CONFIG_FILE_H_
#include <fcgigear/fcgiserver.h>

// new xirang2
#include <xirang2/type/xirang.h>
#include <xirang2/versionedvfs.h>
#include <xirang2/io/file.h>
#include <vector>
#include <map>

namespace cs2{
    
    extern void LoadConfigAndInit(xirang2::fcgi::Context& ctx, const xirang2::file_path& config);
    
    typedef std::unordered_map<xirang2::file_path, std::tuple<xirang2::file_path, xirang2::version_type>, xirang2::hash_file_path> t_object_type_table;
    typedef std::map<xirang2::file_path, xirang2::file_path> t_object_path_table;

    class CMContext : public xirang2::fcgi::UserContext {
    public:
        std::vector<std::shared_ptr<xirang2::vfs::IVfs> >  vfs_deque;

        xirang2::type::Xirang runtime;
        std::shared_ptr<xirang2::vfs::LocalRepository> cm_repo;
        std::shared_ptr<xirang2::vfs::Workspace> cm_workspace;
        
		t_object_type_table object_type_table;
		t_object_path_table object_path_table;
		std::unique_ptr<xirang2::io::file_reader> idx_file;
		xirang2::unique_ptr<xirang2::io::read_view> idx_view;
		std::vector<std::pair<const char*, const char*>> idx_table;

		CMContext();
		~CMContext();
		void init_idx(const xirang2::file_path& file);
	};

}

#endif //end XIRANG2_CS2_CONFIG_FILE_H_
