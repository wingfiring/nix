#include <xirang2/versionedvfs.h>
#include <xirang2/vfs/vfs_common.h>
#include <xirang2/serialize/exchs11n.h>
#include <xirang2/serialize/versiontype.h>
#include <xirang2/serialize/path.h>
#include <xirang2/io/adaptor.h>
#include <xirang2/versionhelper.h>
#include <xirang2/serialize/s11nbasetype.h>
#include <xirang2/vfs/inmemory.h>

#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <chrono>
#include <vector>

namespace xirang2{ namespace vfs{
	constexpr uint32_t Submission::flag;
	constexpr uint32_t BlobMetadata::flag;

	///// tree_blob ////////////////////
    struct tree_blob : public std::map<file_path, TreeItemInfo> {
        static const uint32_t flag = bt_tree;
    };
	const uint32_t tree_blob::flag;
    
	template<typename Ar> Ar& save(Ar& ar, const TreeItemInfo& i){
		return ar & i.flag & i.size & i.ctime & i.mtime & i.version;
	}
	template<typename Ar> Ar& load(Ar& ar, TreeItemInfo& i){
		return ar & i.flag & i.size & i.ctime & i.mtime & i.version;
	}
    
	template<typename Ar> Ar& load(Ar& ar, tree_blob& tb){
		uint32_t flag = bt_none;

		ar & flag;
		if (flag != tree_blob::flag)
			XR_THROW(bad_repository_exception)("bad tree blob flag");

		size_t s = load_size_t(ar) ;
		string name;
		TreeItemInfo info;
		for (size_t i = 0; i < s; ++i){
			ar & name & info;
			tb.emplace(file_path(name), std::move(info));
		}
		return ar;
	}

	template<typename Ar> Ar& load(Ar& ar, tree_blob& tb, type::Xirang&){
		return load(ar, tb);
	}

	template<typename Ar> Ar& save(Ar& ar, const tree_blob& tb){
		ar & tb.flag;
		save_size_t(ar, tb.size());
		for (auto &i : tb)
		  ar & i.first & i.second;
		return ar;
	}

	///// blob_info ////////////////////
	/// Total record size: 28 ( 2+2+20+4, version) + 4 + 8 + 8 = 48
	struct blob_info{
		blob_info(blob_info&& rhs)
			: flag(rhs.flag)
			, size(rhs.size)
			, offset(rhs.offset)
			, cache(std::move(rhs.cache))
		{ }

		blob_info(const blob_info&) = delete;
		blob_info() = default;

		uint32_t flag;
		uint64_t size;
		long_size_t offset;	// -1 means separated blob file.
        mutable unique_ptr<void> cache;
	};

	struct blob_info_map : std::unordered_map<version_type, blob_info, hash_version_type>
	{ 
	};

	template<typename Ar> Ar& load(Ar& ar, blob_info& i){
		return ar & i.flag & i.size & i.offset;
	}
	template<typename Ar> Ar& save(Ar& ar, const blob_info& i){
		return ar & i.flag & i.size & i.offset;
	}

	template<typename Ar> Ar& load(Ar& ar, blob_info_map& infos){
		blob_info item;
		version_type version;
		while (ar.readable()){
			ar & version & item;
            infos.emplace(std::move(version), std::move(item));
		}
		return ar;
	}

	template<typename Ar> Ar& save(Ar& ar, const blob_info_map& infos){
		for (auto & i: infos)
			ar & i.first & i.second;
		return ar;
	}

	///// Other s11n  ////////////////////
	template<typename Ar> Ar& save(Ar& ar, const BlobMetadata& i){
		ar & i.flag & i.tflag & i.target;
		io::writer& wr = ar;
		save(wr, i.metadata);
		return ar;
	}

	template<typename Ar> Ar& load(Ar& ar, BlobMetadata& i, type::Xirang& xr){
		uint32_t flag = bt_none;
		ar & flag;
		if (flag != i.flag) XR_THROW(bad_repository_exception)
			("bad blob metadata flag");
			
		ar & i.tflag & i.target;
		io::reader& rd = ar;
		load(rd, i.metadata, xr, xr.root());
		return ar;
	}

	/// \note don't include version field!!!!!
	template<typename Ar> Ar& load(Ar& ar, Submission& sub){
		uint32_t flag;

		ar & flag;
		if (flag != sub.flag) XR_THROW(bad_repository_exception)
			("bad blob submission flag");

		ar & sub.prev & sub.tree & sub.time
			& sub.author & sub.submitter & sub.description;
		return ar;
	}

	/// wrapper for load_blob
	template<typename Ar> Ar& load(Ar& ar, Submission& sub, type::Xirang&){
		return load(ar, sub);
	}

	template<typename Ar> Ar& save(Ar& ar, const Submission& sub){
		ar & sub.flag & sub.prev & sub.tree & sub.time
			& sub.author & sub.submitter & sub.description;
		return ar;
	}

	template<typename Ar> Ar& save(Ar& ar, const FileHistoryItem& fhi){
		return ar & fhi.submission & fhi.version;
	}
	template<typename Ar> Ar& load(Ar& ar, FileHistoryItem& fhi){
		return ar & fhi.submission & fhi.version;
	}

    struct path_versions_type : public std::vector<FileHistoryItem> {
	//	history;
    };
	struct path_map_type :  std::unordered_map<file_path, path_versions_type, hash_file_path>{
	};

	struct proxy_state_selector{
		typedef const VfsState value_type;
		typedef const VfsState* pointer;
		typedef const VfsState& reference;

		IVfs* vfs;
		mutable VfsState fst;

		proxy_state_selector(IVfs* vfs_) : vfs(vfs_){}
		const VfsState& operator()(const VfsState& item) const{
			fst = item;
			fst.owner_fs = vfs;
			return fst;
		}
	};


	static const sub_file_path K_blob_idx = sub_file_path(literal("#blob.idx"));
	static const sub_file_path K_head = sub_file_path(literal("#head"));
	static const sub_file_path K_data_file = sub_file_path(literal("#content"));
	static const sub_file_path K_remote_head = sub_file_path(literal("#remote"));

	class LocalRepositoryImp;
	void copy_to_vfsstate(VfsState& dest, const TreeItemInfo& src, bool with_object, const LocalRepositoryImp* impfs);
	struct local_repo_file_selector{
		typedef const VfsState value_type;
		typedef const VfsState* pointer;
		typedef const VfsState& reference;


		const LocalRepositoryImp* vfs_imp;
		io_option option;
		mutable VfsState fst;

		local_repo_file_selector(LocalRepository* vfs_, const LocalRepositoryImp* impfs, io_option opt)
			: vfs_imp(impfs), option(opt)
		{
			fst.owner_fs = vfs_;
		}
		const VfsState& operator()(const tree_blob::value_type& item) const{

			copy_to_vfsstate(fst, item.second, option == io::ao_metadata, vfs_imp);
			fst.path = item.first;

			return fst;
		}
	};


	struct tree_item_selector{
		typedef const TreeItem value_type;
		typedef const TreeItem* pointer;
		typedef const TreeItem& reference;

		mutable TreeItem  cache;

		const TreeItem& operator()(const tree_blob::value_type& item) const{
			TreeItemInfo& base = cache;
			base = item.second;
			cache.name = item.first;

			return cache;
		}

	};

	// this class works for 
	class TreeVfs : IVfs{
		public:
			TreeVfs(LocalRepositoryImp& repo, const version_type& ver); 
	};

	class LocalRepositoryImp {
		public:
			void sync(){ return m_underlying.sync();}
			const string& resource() const{ return m_prefix.str();}
			RootFs* root() const{ return m_root;}
			bool mounted() const { return m_root != 0;}
			file_path mountPoint() const { return m_root ? m_root->mountPoint(*m_host) : file_path();}

			LocalRepositoryImp(IVfs& vfs, const file_path& prefix, type::Xirang& xr, LocalRepository* host)
				: m_underlying(vfs), m_prefix(prefix), m_xr(xr), m_host(host), m_root()
			{
				m_data_file = m_underlying.create<io::reader, io::writer,
							io::random, io::read_map, io::write_map>(prefix/K_data_file, io::of_open);

				auto ar_head = m_underlying.create<io::reader>(prefix/K_head, io::of_open);
				if (!ar_head) XR_THROW(bad_repository_exception)("failed to open #head file");
				if (ar_head.get<io::reader>().readable()){
					auto s_head = io::exchange::as_source(ar_head.get<io::reader>());
					s_head & m_head;
				}

				auto ar_blob_idx = m_underlying.create<io::read_map>(prefix/K_blob_idx, io::of_open);
				if (!ar_blob_idx) XR_THROW(bad_repository_exception)("failed to open #blob.idx file");
				auto& rd_map = ar_blob_idx.get<io::read_map>();
				auto view = rd_map.view_rd(ext_heap::handle(0, rd_map.size()));
				io::buffer_in bin(view.get<io::read_view>().address());
				iref<io::reader> mrd(bin);
				// index_count is an estimated number of blobs, used to set hash buckets,  has no need to know the exactly number.
				auto index_count = rd_map.size() / sizeof(blob_info);
				auto reserved_bucket_count = index_count + index_count/2;
				m_blob_infos.reserve(reserved_bucket_count);
				if (mrd.get<io::reader>().readable()){
					auto s_blob_idx = io::exchange::as_source(mrd.get<io::reader>());
					s_blob_idx & m_blob_infos;
				}
			}

			VfsNodeRange children(sub_file_path path, io_option option) const{
				auto tree = try_load_tree_(get_actual_version_(path));
				if (!tree) return VfsNodeRange();

				local_repo_file_selector sel(m_host, this, option);

				typedef VfsNodeRange::iterator iterator;
				return VfsNodeRange(iterator(make_select_iterator(tree->begin(), sel))
						, iterator(make_select_iterator(tree->end(), sel)));
			}
			VfsState state(sub_file_path path, io_option option) const{
				VfsState fst (m_host, fs::st_not_found, path);

				auto binfo = try_load_target_(get_actual_version_(path));
				if (!binfo) return fst;

				auto& meta = load_metadata_(binfo->first);
				fst.metadata.file_type = meta.file_type;
				if (option == io::ao_metadata)
					fst.metadata.assign_metadata(meta.metadata());

				if (binfo->second.flag == bt_file){
					fst.state = fs::st_regular;
					fst.size = binfo->second.size;
				} else if (binfo->second.flag == bt_tree){
					fst.state = fs::st_dir;
				}
				return fst;
			}

			any getopt(int id, const any & optdata) const {
				if (id == io::ao_metadata){
					const file_path* path = xirang2::any_cast<const file_path>(&optdata);
					if (!path)  return any();
					auto st = state(*path, io::ao_metadata);
					return st.state < fs::st_error_placeholder ? any(st.metadata) : any();
				}
				return any();
			}

			any setopt(int /*id*/, const any & /*optdata*/, const any & /*indata*/)
			{ return any();}

			void** do_create(unsigned long long mask,
					void** base, unique_ptr<void>& owner, sub_file_path path, int /* flag */){
				//FIXME: should throw exception when failed, except flag was set with of_nothrow
				void ** ret = 0;
				if ((mask & detail::get_mask<io::reader, io::read_map>::value) == 0)
					return nullptr;

				auto file_version = get_actual_version_(path);
				auto binfo = try_load_target_(file_version);
				if (!binfo || binfo->second.flag != bt_file)
					return nullptr;

				if (binfo->second.offset != long_size_t(-1)){
					auto real_offset = binfo->second.offset + 4 + 8; // 4: flag size, 8: real file size
					auto adaptor = io::decorate<io::sub_archive
						, io::sub_reader_p
						, io::sub_read_map_p
						>(m_data_file,
								real_offset, real_offset + binfo->second.size - 4 - 8);
					m_data_file.get<io::random>().seek(real_offset);
					iauto<io::reader, io::read_map> res (std::move(adaptor));
					ret = copy_interface<io::reader, io::read_map>::apply(mask, base, res, res.target_ptr.get());
					unique_ptr<void>(std::move(res.target_ptr)).swap(owner);
					return ret;
				}
				// for big file, put under folder '#data'
				static const file_path data_dir("#data");
				return m_underlying.raw_create(mask, base, owner, m_prefix/ data_dir / file_path(sha1_to_string(file_version.id), pp_none), io::of_open);
			}
			void setRoot(RootFs* r){
				XR_PRE_CONDITION(!mounted() || r == 0);
				m_root = r;
			}

			const Submission* getSubmission(const version_type& ver) const{
				auto& version = is_empty(ver)? m_head : ver;

				auto subm = try_load_submission_(version);
				if (!subm)
					return nullptr;
				return subm;
			}

			BlobType blobType(const version_type& id) const{
				auto pos = m_blob_infos.find(id);
				if (pos == m_blob_infos.end())
					return bt_none;
				return BlobType(pos->second.flag);
			}

			TreeItemList treeItems(const version_type& id) const{
				auto tree = try_load_tree_(id);
				if (!tree)
					return TreeItemList();

				typedef TreeItemList::iterator iterator;
				tree_item_selector sel;
				return TreeItemList(iterator(make_select_iterator(tree->begin(), sel))
						, iterator(make_select_iterator(tree->end(), sel)));
			}

			const version_type& getFileVersion(const version_type& commit_id, sub_file_path p) const{
				static const version_type empty_version;

				auto& version = is_empty(commit_id)? m_head : commit_id;
				auto cm = try_load_submission_(version);
				if (!cm) return empty_version;
				if (p.empty()) return cm->tree;

				auto tree = &load_tree_(cm->tree);
				for (auto i(p.begin()), last(p.end());  i != last;){
					auto idir = tree->find(*i);
					if (idir == tree->end())
						break;

					if (++i == last) return idir->second.version;

					auto& binfo = load_target_(idir->second.version);
					if (binfo.second.flag != bt_tree) return empty_version;

					XR_POST_CONDITION(binfo.second.cache);
					tree = static_cast<tree_blob*>(binfo.second.cache.get());;
				}
				return empty_version;
			}

			IVfs& underlying() const{ return m_underlying;}
			const file_path& prefix() const{ return m_prefix;}

			struct Context{
				iauto<io::random, io::writer> idx_file;
				IWorkspace& wk;
				int64_t now;

				std::vector<file_path> changed_paths;

				explicit Context(IWorkspace& w) 
					: wk(w)
					, now(std::chrono::nanoseconds(std::chrono::system_clock::now().time_since_epoch()).count())
				{ 
					for(auto& path : wk.allRemoved())
						changed_paths.push_back(path);
					for (auto& element : wk.allMetadataChange())
						changed_paths.push_back(element.name);
					std::sort(changed_paths.begin(), changed_paths.end(), path_less());
				}

				bool affected_by_child(const file_path& path) const{
					auto pos = std::lower_bound(changed_paths.begin(), changed_paths.end(), path, path_less());
					return pos != changed_paths.end()
						&& pos->under(path);
				}
			};
            void init_repo_() {
                InMemory memfs;
                Workspace wk(memfs, literal("init"));
                Context ctx(wk);
                ctx.now = 0;
                ctx.idx_file = m_underlying.create<io::writer, io::random>(m_prefix / K_blob_idx, io::of_create_or_open);
                if (!ctx.idx_file) XR_THROW(bad_repository_exception)("failed to open #blob.idx file");
                auto & seek = ctx.idx_file.get<io::random>();
                seek.seek(seek.size());
                Submission sub;
                sub.time = 0;

                sub.tree = save_tree_blob_(tree_blob(), MetadataValue(), ctx).version;
                save_submission_(std::move(sub), ctx);
            }
			const Submission* commit(IWorkspace& wk, const string& description, const version_type& base){
				auto & base_version = is_empty(base) ? m_head : base;
				auto base_commit = try_load_submission_(base_version);
                if (!base_commit) return nullptr;

				Context ctx(wk);

				ctx.idx_file = m_underlying.create<io::writer, io::random>(m_prefix/K_blob_idx, io::of_create_or_open);
				if (!ctx.idx_file) XR_THROW(bad_repository_exception)("failed to open #blob.idx file");
				auto & seek = ctx.idx_file.get<io::random>();
				seek.seek(seek.size());

				Submission sub;
				sub.description = description;
				sub.prev = base_version;
				sub.time = ctx.now;

				auto btree = try_load_tree_(base_commit->tree);
				if (!btree)
					XR_THROW(bad_repository_exception)("root of submission is not a tree");

				TreeItemInfo root_item = {load_blob_(base_commit->tree).second.flag, 0, ctx.now, ctx.now, base_commit->tree};
				VfsState st {&wk, fs::st_dir, file_path()};
				tree_blob dummy;
				merge_tree_(ctx,  file_path(), wk, st, root_item, dummy);
				sub.tree = dummy.begin()->second.version;

				return save_submission_(std::move(sub), ctx);
			}

			const MetadataValue& load_metadata(const version_type& ver) const {
				return load_metadata_(ver);
			}
		private:
			void add_workspace_(Context& ctx, const file_path& current_path, IWorkspace& wk, const VfsState& st, tree_blob& parent_tree){
				if (st.state == fs::st_regular){
					 parent_tree.emplace(st.path, save_file_(wk, st, current_path, ctx));
				}
				else {
					tree_blob new_tree;
					for (auto& i : wk.children(current_path))
						add_workspace_(ctx, current_path/i.path, wk, i, new_tree);
					parent_tree.emplace(st.path, save_tree_blob_(std::move(new_tree), st.metadata, ctx));
				}
			}
            void add_blob_tree_(Context& ctx, IWorkspace& wk, const file_path& current_path, const tree_blob::value_type& item, tree_blob& parent_tree) {
                if (wk.isMarkedRemove(current_path)) return;

                auto new_meta = wk.getMetadataChange(current_path);
                auto& target = load_target_(item.second.version);
                if (target.second.flag == bt_file || !ctx.affected_by_child(current_path)) {
                    if (new_meta) {
                        auto new_item = item;
                        new_item.second.version = new_meta->empty()
                            ? target.first
                            : save_metadata_blob_(target.second.flag, target.first, *new_meta, ctx);
                        parent_tree.emplace(std::move(new_item));
                    }
                    else
                        parent_tree.emplace(item);
                }
                else {
                    // bt_tree, rebuild
                    auto& sub_tree = load_tree_(target.first);
                    tree_blob new_subtree;
                    for (auto& i : sub_tree) {
                        add_blob_tree_(ctx, wk, current_path / i.first, i, new_subtree);
                    }
                    auto& old_meta = load_metadata_(item.second.version);
                    parent_tree.emplace(item.first, save_tree_blob_(std::move(new_subtree), new_meta ? *new_meta : old_meta, ctx));
                }
			}
			// TODO: avoid to use recursively imp
			void merge_tree_(Context& ctx, const file_path& path_in_repo, IWorkspace& wk, 
					const VfsState& st, const TreeItemInfo& old_info, tree_blob& parent_tree){

				XR_PRE_CONDITION(st.state == fs::st_dir);

				auto& src_dir = load_tree_(old_info.version);
				tree_blob::const_iterator tb_itr, tb_end;
				tb_itr = src_dir.begin();
				tb_end = src_dir.end();

				std::map<file_path, VfsState> children;
				for(auto& i : wk.children(path_in_repo, io::ao_metadata))
					children.emplace(i.path, i);
				auto wk_itr = children.begin();
				auto wk_end = children.end();

				tree_blob new_tree;

				for(; tb_itr!= tb_end || wk_itr != wk_end;){
					if (tb_itr == tb_end){
						for(;wk_itr != wk_end; ++wk_itr)
							add_workspace_(ctx, path_in_repo/wk_itr->first, wk,  wk_itr->second, new_tree);
						break;
					}
					if (wk_itr == wk_end){
						for(;tb_itr != tb_end; ++tb_itr)
							add_blob_tree_(ctx, wk, path_in_repo/tb_itr->first, *tb_itr, new_tree);
						break;
					}
					if(tb_itr->first < wk_itr->first){
						add_blob_tree_(ctx, wk, path_in_repo/tb_itr->first, *tb_itr, new_tree);
						++tb_itr;
					}
					else if (wk_itr->first < tb_itr->first){
						add_workspace_(ctx, path_in_repo/wk_itr->first, wk,  wk_itr->second, new_tree);
						++wk_itr;
					}
					else{
						if (wk_itr->second.state == fs::st_dir){
							if(tb_itr->second.flag == bt_tree)
								merge_tree_(ctx, path_in_repo/tb_itr->first, wk, wk_itr->second, tb_itr->second, new_tree);
							else
								add_workspace_(ctx, path_in_repo/wk_itr->first, wk, wk_itr->second, new_tree);
						}
						else{
							if (tb_itr->second.flag == bt_tree)
								add_workspace_(ctx, path_in_repo/wk_itr->first, wk, wk_itr->second, new_tree);
							else {
								auto full_path = path_in_repo/wk_itr->first;
								if (wk_itr->second.metadata.empty()){
									auto meta = wk.getMetadataChange(full_path);
									if (meta)	
										wk_itr->second.metadata = *meta;
									else if (tb_itr->second.flag == bt_metadata)
										wk_itr->second.metadata = load_metadata_(tb_itr->second.version);
								}
								add_workspace_(ctx, full_path, wk, wk_itr->second, new_tree);
							}
						}
						++tb_itr;
						++wk_itr;
					}
				}

				// metadata: if has new, use new. if meta changed, use changed. if has old, then old. else use empty.
				const MetadataValue* new_meta = &st.metadata;
				new_meta = new_meta->empty() ? wk.getMetadataChange(path_in_repo) : new_meta;
				// because the new item would be a tree, if existing item is a file, the new should not derive metadata from the old. 
				if (!new_meta && old_info.flag == bt_metadata)
					new_meta = &load_metadata_(old_info.version);
				else 
					new_meta = &st.metadata;	// use an empty metadata

				parent_tree.emplace(st.path, save_tree_blob_(std::move(new_tree), *new_meta, ctx));
			}

			TreeItemInfo save_tree_blob_(tree_blob&& tree, const MetadataValue& meta, Context& ctx){
				TreeItemInfo ret{bt_tree, 0, ctx.now, ctx.now, version_of_object(tree)};

				if (m_blob_infos.count(ret.version) == 0){
					auto & seek = m_data_file.get<io::random>();
					auto offset = seek.size();
					seek.seek(offset);
					auto sink = io::exchange::as_sink(m_data_file.get<io::writer>());
					sink & tree;

					auto cache = unique_ptr<tree_blob>(new tree_blob(std::move(tree)));
					save_blob_idx_(bt_tree, ret.version, seek.size() - offset, offset, ctx, std::move(cache));
				}

				if (!meta.empty())
					ret.version = save_metadata_blob_(bt_tree, ret.version, meta, ctx);

				return ret;
			}

			TreeItemInfo save_file_(IWorkspace& wk, const VfsState& st, const file_path& path, Context& ctx) {
				TreeItemInfo ret{bt_file, st.size, ctx.now, ctx.now, version_type()};

				auto src = wk.create<io::read_map>(path, io::of_open);
				if (!src) XR_THROW(fs::open_failed_exception)("failed to open file in workdir");
				auto& src_map = src.get<io::read_map>();

				ret.version = version_of_archive(src.get<io::read_map>());
				if (m_blob_infos.count(ret.version) == 0){
					auto& seek = m_data_file.get<io::random>();
					long_size_t offset = seek.size();
					seek.seek(offset);
					auto sink = io::exchange::as_sink(m_data_file.get<io::writer>());
					sink & uint32_t(bt_file) & uint64_t(src_map.size());
					copy_data(src_map, m_data_file.get<io::writer>());

					save_blob_idx_(bt_file, ret.version, seek.size() - offset, offset, ctx, unique_ptr<void>());
				}
				if (!st.metadata.empty())
					ret.version = save_metadata_blob_(bt_tree, ret.version, st.metadata, ctx);

				return ret;
			}
			version_type save_metadata_blob_(uint32_t flag, const version_type& target, const MetadataValue& meta, Context& ctx){
				BlobMetadata blob{flag, target, meta};
				auto version = version_of_object(blob);
				if (m_blob_infos.count(version) != 0)
					return version;
				
				auto& seek = m_data_file.get<io::random>();
				auto offset = seek.size();
				seek.seek(offset);
				auto sink = io::exchange::as_sink(m_data_file.get<io::writer>());
				sink & blob;

				auto cache = unique_ptr<BlobMetadata>(new BlobMetadata(std::move(blob)));
				save_blob_idx_(bt_metadata, version, seek.size() - offset, offset, ctx, std::move(cache));
				return version;
			}
			const Submission* save_submission_(Submission&& sub, Context& ctx){
				sub.version = version_of_object(sub);	// Submission serialier ignore version field

				auto sub_exist = try_load_submission_(sub.version);
				if (sub_exist)	return sub_exist;

				auto& seek = m_data_file.get<io::random>();
				auto offset = seek.size();
				seek.seek(offset);
				auto sink = io::exchange::as_sink(m_data_file.get<io::writer>());
				sink & sub;

				auto cache = unique_ptr<Submission>(new Submission(sub));
				auto& binfo = save_blob_idx_(bt_submission, sub.version, seek.size() - offset, offset, ctx, std::move(cache));

				auto ar_head = m_underlying.create<io::writer, io::random>(m_prefix/K_head, io::of_create_or_open);
				if (!ar_head) XR_THROW(bad_repository_exception)("failed to create #head file");

				auto sink2 = io::exchange::as_sink(ar_head.get<io::writer>());
				sink2 & sub.version;
				m_head = sub.version;

				return static_cast<Submission*>(binfo.cache.get());
			}

			blob_info& save_blob_idx_(uint32_t flag, const version_type& ver, uint64_t size,
					long_size_t offset, Context& ctx, unique_ptr<void>&& cache){
				XR_PRE_CONDITION(m_blob_infos.count(ver) == 0);

				blob_info binfo;
				binfo.flag = flag;
				binfo.size = size;
				binfo.offset = offset;
				binfo.cache = std::move(cache);

				auto sink = io::exchange::as_sink(ctx.idx_file.get<io::writer>());
				sink & ver & binfo;

				return m_blob_infos.emplace(ver, std::move(binfo)).first->second;
			}

			version_type get_actual_version_(sub_file_path path) const{
				auto str_version = path.version();
				return str_version.empty()
					? getFileVersion(m_head, path.exclude_version())
					: version_type(str_version.str());
			}

			/////========== Load Blob methods ========== /////

			template<typename T> const T&  load_blob_obj_(const blob_info& binfo) const {
				XR_PRE_CONDITION(binfo.flag == T::flag);

				if(!binfo.cache){
					auto& seek = m_data_file.get<io::random>();
					seek.seek(binfo.offset);
					auto source = io::exchange::as_source(m_data_file.get<io::reader>());
					unique_ptr<T> b(new T);
					load(source, *b, m_xr);
					binfo.cache = std::move(b);
				}
				return *static_cast<T*>(binfo.cache.get());
			}

			const blob_info_map::value_type* try_load_blob_(const version_type& ver) const {
				auto pos = m_blob_infos.find(ver);
				if (pos == m_blob_infos.end())	return nullptr;

				switch(pos->second.flag){
					case bt_tree:
						load_blob_obj_<tree_blob>(pos->second);
						break;
					case bt_submission:
						{
							auto& subm = load_blob_obj_<Submission>(pos->second);
							const_cast<Submission&>(subm).version = ver;
						}
						break;
					case bt_metadata:
						load_metadata_(ver);
						break;
					case bt_file:
						// do nothing
						break;
					default:
						XR_THROW(bad_repository_exception)("bad blob info flag");
				};

				return &*pos;
			}

			const blob_info_map::value_type& load_blob_(const version_type& ver) const {
				auto ret = try_load_blob_(ver);
				if (!ret) XR_THROW(bad_repository_exception)("failed to load blob");
				return *ret;
			}

			const blob_info_map::value_type* try_load_target_(const version_type& ver) const {
				auto* binfo = try_load_blob_(ver);

				if (binfo && binfo->second.flag == bt_metadata){
					auto& b = load_blob_obj_<BlobMetadata>(binfo->second);
					return &load_blob_(b.target);
				}
				return binfo;
			}
			const blob_info_map::value_type& load_target_(const version_type& ver) const {
				auto ret = try_load_target_(ver);
				if (!ret) XR_THROW(bad_repository_exception)("failed to load blob");
				return *ret;
			}

			const tree_blob* try_load_tree_(const version_type& ver) const {
				auto binfo = try_load_target_(ver);
				if (!binfo || binfo->second.flag != bt_tree) return nullptr;

				return &load_blob_obj_<tree_blob>(binfo->second);
			}
			const tree_blob& load_tree_(const version_type& ver) const {
				auto ret = try_load_tree_(ver);
				if (!ret) XR_THROW(bad_repository_exception)("failed to tree blob");
				return *ret;
			}

			const MetadataValue& load_metadata_(const version_type& ver) const {
				static const MetadataValue dummy;

				auto binfo = try_load_blob_(ver);
				if (!binfo || binfo->second.flag != bt_metadata) return dummy;

				return load_blob_obj_<BlobMetadata>(binfo->second).metadata;
			}

			Submission* try_load_submission_(const version_type& ver) const {
				auto binfo = try_load_blob_(ver);
				if (binfo && binfo->second.flag == bt_submission)
					return static_cast<Submission*>(binfo->second.cache.get());
				return nullptr;
			}

			IVfs& m_underlying;
			file_path m_prefix;
			type::Xirang& m_xr;
			LocalRepository* m_host;
			RootFs* m_root;
			blob_info_map m_blob_infos;
			path_map_type m_path_map;
			version_type m_head;

			iauto<io::reader, io::writer, io::random, io::read_map, io::write_map> m_data_file;
	};

	void copy_to_vfsstate(VfsState& dest, const TreeItemInfo& src, bool with_object, const LocalRepositoryImp* impfs){
		dest.state = src.flag == bt_tree ? fs::st_dir : fs::st_regular;
		dest.size = src.size > 12 ? src.size - 12 : src.size; // for bt_file: 12 = flag(4) + size(8)
		dest.create_time = src.ctime;
		dest.modified_time = src.mtime;
		if (src.flag == bt_metadata){
			auto& meta = impfs->load_metadata(src.version);
			dest.metadata.file_type = meta.file_type;
			if (with_object)
				dest.metadata.assign_metadata(meta.metadata());
		}
	}

	LocalRepository::LocalRepository(IVfs& vfs, const file_path& prefix, type::Xirang& xr)
		: m_imp(new LocalRepositoryImp(vfs, prefix, xr, this))
	{}
	fs_error LocalRepository::do_remove(sub_file_path /* path */){
		return fs::er_permission_denied;
	}
	fs_error LocalRepository::do_createDir(sub_file_path /* path */){
		return fs::er_permission_denied;
	}
	fs_error LocalRepository::do_copy(sub_file_path /* from */, sub_file_path /* to */ ){
		return fs::er_permission_denied;
	}
	fs_error LocalRepository::do_move(sub_file_path /* from */, sub_file_path /* to */ ){
		return fs::er_permission_denied;
	}
	fs_error LocalRepository::do_truncate(sub_file_path /* path */, long_size_t /* s */){
		return fs::er_permission_denied;
	}
	void LocalRepository::do_sync(){
		m_imp->sync();
	}
	const string& LocalRepository::do_resource() const{
		return m_imp->resource();
	}
	RootFs* LocalRepository::do_root() const{ return m_imp->root(); }
	bool LocalRepository::do_mounted() const{ return m_imp->mounted();}
	file_path LocalRepository::do_mountPoint() const{ return m_imp->mountPoint();}
	VfsNodeRange LocalRepository::do_children(sub_file_path path, io_option option) const{ return m_imp->children(path, option);}
	VfsState LocalRepository::do_state(sub_file_path path, io_option option) const{ return m_imp->state(path, option);}
	any LocalRepository::do_getopt(int id, const any & optdata) const { return m_imp->getopt(id, optdata);}
	any LocalRepository::do_setopt(int id, const any & optdata,  const any & indata) { return m_imp->setopt(id, optdata, indata);}
	void** LocalRepository::do_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
		return m_imp->do_create(mask, ret, owner, path, flag);
	}
	void LocalRepository::do_setRoot(RootFs* r){ m_imp->setRoot(r); }

	unique_ptr<IVfs> LocalRepository::getVfs(const version_type& ver) const{
		XR_PRE_CONDITION(false && "not implementd");
		unuse(ver);
		return unique_ptr<IVfs>();
	}
	const Submission* LocalRepository::getSubmission(const version_type& ver) const{ return m_imp->getSubmission(ver);}
	BlobType LocalRepository::blobType(const version_type& id) const{
		return m_imp->blobType(id);
	}
	TreeItemList LocalRepository::treeItems(const version_type& id) const{
		return m_imp->treeItems(id);
	}
	version_type LocalRepository::getFileVersion(const version_type& commit_id, sub_file_path p) const{
		return m_imp->getFileVersion(commit_id, p);
	}

	void LocalRepository::push(){ }	// do nothing for local repo
	void LocalRepository::pull(){ } // do nothing for local repo
	void LocalRepository::fetch(const version_type& /* ver */, int /* level */){}
	IVfs& LocalRepository::underlying() const{ return m_imp->underlying();}
	const file_path& LocalRepository::prefix() const{ return m_imp->prefix();}
	const Submission* LocalRepository::commit(IWorkspace& wk, const string& description, const version_type& base){
		return m_imp->commit(wk, description, base);
	}

	class WorkspaceImp{
		public:
			IVfs& underlying;
			string resource;
			RootFs * root;

			std::unordered_set<file_path, hash_file_path> remove_list;
			std::unordered_map<file_path, MetadataValue, hash_file_path> metadata_changed_list;

			WorkspaceImp(IVfs& fs, const string& res)
				: underlying(fs)
				  , resource(res), root()
		{}
	};

	Workspace::Workspace(IVfs& ws, const string& res)
		: m_imp(new WorkspaceImp(ws, res))
	{}

	// IVfs API
	fs_error Workspace::do_remove(sub_file_path path){ return m_imp->underlying.remove(path); }
	fs_error Workspace::do_createDir(sub_file_path path){ return m_imp->underlying.createDir(path);}
	fs_error Workspace::do_copy(sub_file_path from, sub_file_path to){ return m_imp->underlying.copy(from, to);}
	fs_error Workspace::do_move(sub_file_path from, sub_file_path to){ return m_imp->underlying.move(from, to);}
	fs_error Workspace::do_truncate(sub_file_path path, long_size_t s){ return m_imp->underlying.truncate(path, s);}
	void Workspace::do_sync(){ return m_imp->underlying.sync();}
	const string& Workspace::do_resource() const{ return m_imp->resource;}
	RootFs* Workspace::do_root() const{ return m_imp->root;}
	bool Workspace::do_mounted() const{ return m_imp->root !=0;}
	file_path Workspace::do_mountPoint() const{ return root()? root()->mountPoint(*this) : file_path();}

	/// \param path can be "~repo/a/b/#version"
	/// FUTURE: or  "~repo/<#submission or root version>/a/b" or "~repo/a/<#tree version>/b"
	VfsNodeRange Workspace::do_children(sub_file_path path, io_option option) const{ 
		XR_PRE_CONDITION(!path.is_absolute());

		proxy_state_selector sel(const_cast<Workspace*>(this));
		auto children =  m_imp->underlying.children(path, option);
		return VfsNodeRange(
				VfsNodeRange::iterator(make_select_iterator(children.begin(), sel)),
				VfsNodeRange::iterator(make_select_iterator(children.end(), sel))
				);
	}
	VfsState Workspace::do_state(sub_file_path path, io_option) const{
		auto ret = m_imp->underlying.state(path);
		ret.owner_fs = const_cast<Workspace*>(this);
		return ret;
	}
	any Workspace::do_getopt(int id, const any & optdata) const {
		return m_imp->underlying.getopt(id, optdata);
	}
	any Workspace::do_setopt(int id, const any & optdata,  const any & indata){
		return m_imp->underlying.setopt(id, optdata, indata);
	}
	void** Workspace::do_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
		return m_imp->underlying.raw_create(mask, ret, owner, path, flag);
	}

	// the p can be regular file or directory. if it's a directory, it means all children need to be removed recursively.
	bool Workspace::markRemove(const file_path& p){
		return m_imp->remove_list.insert(p).second;
	}

	// should remove the parent directory if the parent are not affected.
	bool Workspace::unmarkRemove(const file_path& p){
		return m_imp->remove_list.erase(p) != 0;
	}

	// return true if user added a same path as p exactly  via markRemove;
	bool Workspace::isMarkedRemove(const file_path& p) const{
		return m_imp->remove_list.count(p) != 0;
	}
	RemovedList Workspace::allRemoved() const{
		typedef RemovedList::iterator iterator;
		return RemovedList(iterator(m_imp->remove_list.begin()), iterator(m_imp->remove_list.end()));
	}

	bool Workspace::markMetadataChange(const file_path& p, const MetadataValue& m){
		auto pos = m_imp->metadata_changed_list.find(p);
		bool not_found = pos == m_imp->metadata_changed_list.end();
		if (not_found)
			m_imp->metadata_changed_list[p] = m;
		else
			pos->second = m;
		return not_found;
	}
	bool Workspace::unmarkMetadataChange(const file_path& p){
		return m_imp->metadata_changed_list.erase(p) != 0;
	}
	const MetadataValue* Workspace::getMetadataChange(const file_path& p){
		auto pos = m_imp->metadata_changed_list.find(p);
		return pos == m_imp->metadata_changed_list.end()
			? nullptr
			: &pos->second;
	}

	struct pair_meta_changed_converter{
		typedef const MetadataElement value_type;
		typedef const MetadataElement* pointer;
		typedef const MetadataElement& reference;

		mutable MetadataElement value;

		const MetadataElement& operator()(const std::unordered_map<file_path, MetadataValue, hash_file_path>::value_type& item) const{
			value.name = item.first;
			value.value = item.second;
			return value;
		}
	};

	MetadataChangeList Workspace::allMetadataChange(){
		typedef MetadataChangeList::iterator iterator;
		pair_meta_changed_converter sel;
		return MetadataChangeList(iterator(make_select_iterator(m_imp->metadata_changed_list.begin(), sel)), 
				iterator(make_select_iterator(m_imp->metadata_changed_list.end(), sel)));
	}

	void Workspace::do_setRoot(RootFs* r){
		XR_PRE_CONDITION(!mounted() || r == 0);
		m_imp->root = r;
	}

	static bool createEmptyFile(IVfs& vfs, const file_path& p){
		auto f = vfs.create<io::ioctrl, io::writer>(p, io::of_create_or_open);
		if (!f) return false;
		f.get<io::ioctrl>().truncate(0);
		return true;
	}

	static const uint32_t K_repo_file_version = 1;
	static const uint32_t K_repo_file_sig = 0x4f504552; //'REPO'

	fs_error initRepository(IVfs& vfs, sub_file_path dir){
		if (vfs.state(dir / K_data_file).state != fs::st_not_found){
			return fs::er_exist;
		}

		if (createEmptyFile(vfs, dir / K_data_file)
				&& createEmptyFile(vfs, dir / K_blob_idx)
				&& createEmptyFile(vfs, dir / K_head)
		   ){
			auto f = vfs.create<io::ioctrl, io::writer>(dir / K_data_file, io::of_create_or_open);
			auto sink = io::exchange::as_sink(f.get<io::writer>());
			sink & K_repo_file_sig & K_repo_file_version;
            f.reset();

            type::Xirang xr("repo", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());
            LocalRepositoryImp repo(vfs, file_path(), xr, nullptr);
            repo.init_repo_();

			return fs::er_ok;
		}
		return fs::er_create;
	}
}}

