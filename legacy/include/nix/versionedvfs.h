#ifndef XIRANG2_VERSIONED_VFS_H__
#define XIRANG2_VERSIONED_VFS_H__

#include <xirang2/vfs.h>
#include <xirang2/versiontype.h>
#include <xirang2/vfs/metadatacache.h>

namespace xirang2{ namespace vfs{

	enum BlobType{
		bt_none,	///< not a valid blob type
		bt_file,	///< regular file type
		bt_tree,	///< directory
		bt_submission,
		bt_metadata
	};

	typedef BiRangeT<const_itr_traits<version_type> > VersionList;
	struct Submission {
		static constexpr uint32_t flag = bt_submission;
		version_type tree;
		version_type version;	///< current submission id
		int64_t time;			///< fixed to int64 for different platform
		string author, submitter, description;
		version_type prev;	///< previous submission id, may need to support multiple prev
	};

	// history is a list of submission and file version
	struct FileHistoryItem{
		version_type submission;	///< submission id
		version_type version;		///< file id
	};

	struct TreeItemInfo{
		uint32_t flag;		// can be bt_file, bt_tree, or bt_metadata
		uint64_t size;
		int64_t ctime;
		int64_t mtime;
		version_type version;
	};

	struct TreeItem : public TreeItemInfo{
		file_path name;
	};

	struct BlobMetadata{
		static constexpr uint32_t	flag = bt_metadata;	
		uint32_t tflag;
		version_type target;
		MetadataValue metadata;	// usually, should not store an empty metadata
	};


	typedef BiRangeT<const_itr_traits<FileHistoryItem> > FileHistory;
	typedef BiRangeT<const_itr_traits<TreeItem> > TreeItemList;
	typedef ForwardRangeT<const_itr_traits<file_path> > RemovedList;
	typedef ForwardRangeT<const_itr_traits<MetadataElement> > MetadataChangeList;

	class IWorkspace;
	class IRepository;

	// IRepoManager will hold returned IRepository. IRepoManager may release an IRepoManager automatically if no other one need this repository.
	class XR_INTERFACE IRepoManager{
	public:

		/// create an IRepository object by given vfs and path.
		/// \param vfs given virtual file system
		/// \param repoPath root entry of given repository.
		/// \return null if the repoPath is not a repository. and it will throw exception is the repo is corrupted.
		virtual std::shared_ptr<IRepository> getRepo(IVfs& vfs, sub_file_path repoPath) = 0;

		/// \param path resource path for detect
		/// \param repoPath  return the path of repo. if the path doesn't contain repo, repoPath will be most match part.
		/// \param pathInRepo if path contains a repo, it return the rest part of path. if it doesn't contain repo, it's the mismatch part.
		/// \return return true if path contains a repo, if not return false
		virtual bool extractRepoPath(IVfs& vfs, sub_file_path path, file_path* repoPath, file_path* pathInRepo) const = 0;

		virtual ~IRepoManager(){}
	};

	XR_EXCEPTION_TYPE(bad_repository_exception);
	XR_EXCEPTION_TYPE(repository_coruppted_exception);

	// the IVfs part works on repository only.
	// a/~repo/b/c/#ver
	// a/~repo/b/c/#ver/d
	// IVfs should avoid forward the call to IRepository. init IRepository maybe expensive.
	class XR_INTERFACE IRepository : public IVfs{
		public:
		// history of root will get all submission;
		// \p should has no version part
		// ver can be a tree or commit. for commit, it's will use the root of the commit.
		// if ver refer to a metadata, and the metadata's target is a tree, it's also valid
		virtual	unique_ptr<IVfs> getVfs(const version_type& ver) const = 0;

		virtual const Submission* getSubmission(const version_type& ver) const = 0;
		virtual BlobType blobType(const version_type& id) const = 0;

		virtual TreeItemList treeItems(const version_type& id) const = 0;

		/// TODO: add fundamental methods
		/// version_type add_file(io::read_map& );
		/// version_type add_tree(const tree_blob& );
		/// version_type add_metadata(const MetadataValue& meta, uint32_t target_flag, const version_type& ver);
		/// version_type add_submission(const Submission& );
		/// void set_head(const version_type& ver);

		// include metadata if possible
		virtual version_type getFileVersion(const version_type& ver, sub_file_path p) const = 0;
		virtual void push() = 0;
		// advanced API
		virtual void pull() = 0;

		//TODO: need more detail design
		virtual void fetch(const version_type& ver, int level) = 0;

		virtual IVfs& underlying() const = 0;
		virtual const file_path& prefix() const = 0;

		// return commit id;
		// it'll commit and earse workspace;
		// default Workspace will be recreated automatically.
		// after commit, getBase() will return commit id
		// if the IVfs is empty and affectedRemove(const file_path& p) is empty too, this method do nothing and just return base;
		virtual const Submission* commit(IWorkspace& wk, const string& description, const version_type& base) = 0;
		protected:
			virtual ~IRepository(){};
	};

	class XR_INTERFACE IWorkspace : public IVfs{
		public:
		// mark p will be removed in next commit.
		// the function only remove the file from next commit, not remove it from workspace
		// if syncWorkspace, it'll remove the file from workspace.
		// after this call
		virtual bool markRemove(const file_path& p) = 0;
		virtual bool unmarkRemove(const file_path& p) = 0;
		virtual RemovedList allRemoved() const = 0;
		virtual bool isMarkedRemove(const file_path& p) const = 0;

		virtual bool markMetadataChange(const file_path& p, const MetadataValue& m) = 0;
		virtual bool unmarkMetadataChange(const file_path& p) = 0;
		virtual const MetadataValue* getMetadataChange(const file_path& p) = 0;
		virtual MetadataChangeList allMetadataChange() = 0;
		protected:
		virtual ~IWorkspace(){}
	};

	class LocalRepositoryImp;
	class XR_API LocalRepository : public IRepository{
	public:
		LocalRepository(IVfs& vfs, const file_path& prefix, type::Xirang& xr);
		// from IVfs
		// remove, createDir, copy, move, and truncate should be failed since history is not editable.
		virtual fs_error do_remove(sub_file_path path);
		virtual fs_error do_createDir(sub_file_path path);
		virtual fs_error do_copy(sub_file_path from, sub_file_path to);
		virtual fs_error do_move(sub_file_path from, sub_file_path to);
		virtual fs_error do_truncate(sub_file_path path, long_size_t s);

		virtual void do_sync();
		virtual const string& do_resource() const;
		virtual RootFs* do_root() const;
		virtual bool do_mounted() const;
		virtual file_path do_mountPoint() const;

		/// if path doesn't end with version string (#<sha1 digest>), it should return sub items in given HEAD.
		/// if with version, it only return the children in given version directory.
		virtual VfsNodeRange do_children(sub_file_path path, io_option option) const;

		virtual VfsState do_state(sub_file_path path, io_option option) const;
        virtual any do_getopt(int id, const any & optdata) const ;
        virtual any do_setopt(int id, const any & optdata,  const any & indata);
		virtual void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag);
		virtual void do_setRoot(RootFs* r);

		// from IRepository
		virtual unique_ptr<IVfs> getVfs(const version_type& ver) const;
		virtual const Submission* getSubmission(const version_type& ver) const;
		virtual BlobType blobType(const version_type& id) const;
		virtual TreeItemList treeItems(const version_type& id) const;
		virtual version_type getFileVersion(const version_type& commit_id, sub_file_path p) const;
		virtual void push();
		virtual void pull();
		virtual void fetch(const version_type& ver, int level);
		virtual IVfs& underlying() const;
		virtual const file_path& prefix() const;
		virtual const Submission* commit(IWorkspace& wk, const string& description, const version_type& base);
	private:
		unique_ptr<LocalRepositoryImp> m_imp;

	};

	XR_API extern fs_error initRepository(IVfs& vfs, sub_file_path dir);


	class WorkspaceImp;

	class XR_API Workspace : public IWorkspace {
	public:
		explicit Workspace(IVfs& ws, const string& resource);
		// IVfs API
		virtual fs_error do_remove(sub_file_path path);
		virtual fs_error do_createDir(sub_file_path path);
		virtual fs_error do_copy(sub_file_path from, sub_file_path to);
		virtual fs_error do_move(sub_file_path from, sub_file_path to);
		virtual fs_error do_truncate(sub_file_path path, long_size_t s);
		virtual void do_sync();
		virtual const string& do_resource() const;
		virtual RootFs* do_root() const;
		virtual bool do_mounted() const;
		virtual file_path do_mountPoint() const;

		/// \param path can be "~repo/a/b/#version"
		/// FUTURE: or  "~repo/<#submission or root version>/a/b" or "~repo/a/<#tree version>/b"
		virtual VfsNodeRange do_children(sub_file_path path, io_option option) const;
		virtual VfsState do_state(sub_file_path path, io_option option) const;
        virtual any do_getopt(int id, const any & optdata) const ;
        virtual any do_setopt(int id, const any & optdata, const any & indata);
		virtual void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag);

		// the p can be regular file or directory. if it's a directory, it means all children need to be removed recursively.
		virtual bool markRemove(const file_path& p);

		// should remove the parent directory if the parent are not affected.
		virtual bool unmarkRemove(const file_path& p);

		virtual RemovedList allRemoved() const;

		// return true if user added a same path as p exactly  via markRemove;
		virtual bool isMarkedRemove(const file_path& p) const;

		virtual bool markMetadataChange(const file_path& p, const MetadataValue& m);
		virtual bool unmarkMetadataChange(const file_path& p);
		virtual const MetadataValue* getMetadataChange(const file_path& p);
		virtual MetadataChangeList allMetadataChange();
	private:
		virtual void do_setRoot(RootFs* r);
		unique_ptr<WorkspaceImp> m_imp;
	};

}}

#endif //end XIRANG2_VERSIONED_VFS_H__

