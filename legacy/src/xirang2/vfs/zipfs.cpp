#include <xirang2/vfs/zipfs.h>
#include <xirang2/zip.h>
#include <xirang2/vfs/vfs_common.h>
#include <xirang2/io/memory.h>
#include <xirang2/io/adaptor.h>

#include <map>
#include <vector>
#include <ctime>
#include <chrono>

namespace xirang2{ namespace io{
	typedef decltype(decorate<map_to_stream_archive
			, proxy_read_map_p
			, read_map_to_reader_p
			, read_map_to_random_p
			>(iauto<read_map>(), 0)) archive_type_;

	struct mix_random_co : public random
	{ 
		virtual long_size_t offset() const{
			return get_cobj<archive_type_>(this).offset();
		}
		virtual long_size_t size() const{
			random& r = get_cobj<archive_type_>(this);
			return r.size();
		}
		virtual long_size_t seek(long_size_t off){
			return get_cobj<archive_type_>(this).seek(off);
		}

	};
	mix_random_co get_interface_map(random*, archive_type_*);

	struct mix_read_map_co : public read_map
	{
		virtual iauto<read_view> view_rd(ext_heap::handle h){
			return get_cobj<archive_type_>(this).view_rd(h);
		}
		virtual long_size_t size() const{
			read_map& r = get_cobj<archive_type_>(this);
			return r.size();
		}
	};
	mix_read_map_co get_interface_map(read_map*, archive_type_*);

}}

namespace xirang2{ namespace vfs{
	std::time_t dos2time_t(uint16_t date, uint16_t time){
		std::tm timeinfo{};
		timeinfo.tm_year = (date >> 9) + 80;
		timeinfo.tm_mon = (date & 0x1ff) >> 5;	// bits [8, 5]
		timeinfo.tm_mday = (date & 0x1f);	// bits [4, 0]
		
		timeinfo.tm_hour = time >> 11;
		timeinfo.tm_min = (time & 0x7ff) >> 11;	// bits [10, 5]
		timeinfo.tm_sec = (time & 0x1f);			//bits [4, 0]

		return std::mktime(&timeinfo);
	}
	uint64_t dos2chrono(uint16_t date, uint16_t time){
		return std::chrono::nanoseconds(std::chrono::system_clock::from_time_t(dos2time_t(date, time)).time_since_epoch()).count();
	}
	struct EntryInfo{
		zip::file_header header ;
		file_path cache_name;
		fs::file_state type = fs::st_dir;
		bool is_dirty = false;
		bool is_force_store = false;
	};
	EntryInfo make_entryinfo(const file_path& name){
		EntryInfo info;
		info.header.name = name;
		return info;
	}

	class ZipFsImp 
	{
		public:
			explicit ZipFsImp(const iref<io::read_map, io::write_map>& ar, IVfs* cache, const string& resource, ZipFs* host)
				: m_resource(resource)
				  , m_read_map(&ar.get<io::read_map>())
				  , m_cache(cache)
				  , m_host(host)
				  , m_war(ar)
		{
			init_load_();
		}
			explicit ZipFsImp(iauto<io::read_map, io::write_map> ar, IVfs* cache, const string& resource, ZipFs* host)
				: m_resource(resource)
				  , m_read_map(&ar.get<io::read_map>())
				  , m_cache(cache)
				  , m_host(host)
				  , m_war(ar)
				  , m_archive(std::move(ar))
		{
			init_load_();
		}
			explicit ZipFsImp(const iref<io::read_map>& ar, IVfs* cache, const string& resource, ZipFs* host)
				: m_resource(resource)
				  , m_read_map(&ar.get<io::read_map>())
				  , m_cache(cache)
				  , m_host(host)
		{
			init_load_();
		}
			explicit ZipFsImp(iauto<io::read_map> ar, IVfs* cache, const string& resource, ZipFs* host)
				: m_resource(resource)
				  , m_read_map(&ar.get<io::read_map>())
				  , m_cache(cache)
				  , m_host(host)
				  , m_archive(std::move(ar))
		{
			init_load_();
		}

			fs_error remove(sub_file_path path){
				XR_PRE_CONDITION(!path.is_absolute());
				if (is_readonly_() ||path.empty())
					return fs::er_permission_denied;

				auto pos = m_items.find(path);
				if (pos == m_items.end())
					return fs::er_not_found;
				if (pos->second.type == fs::st_dir
						&& !children_(path).empty()){
					return fs::er_not_empty;
				}
				m_items.erase(pos);
				return fs::er_ok;
			}

			fs_error createDir(sub_file_path path){
				XR_PRE_CONDITION(!path.is_absolute());
				if (is_readonly_() ||path.empty())
					return fs::er_permission_denied;

				auto parent = path.parent();
				if (!parent.empty()){
					auto pos = m_items.find(parent);
					if (pos == m_items.end())
						return fs::er_not_found;
				}

				m_items.insert(std::make_pair(file_path(path), make_entryinfo(path)));
				return fs::er_ok;
			}
			fs_error copy(sub_file_path from, sub_file_path to){
				XR_PRE_CONDITION(!to.is_absolute());
				XR_PRE_CONDITION(!from.is_absolute());
				if (is_readonly_() ||to.empty())
					return fs::er_permission_denied;

				VfsState from_node(m_host, fs::st_invalid,  from);
				VfsState to_node(m_host, fs::st_invalid, to);
				return xirang2::vfs::copyFile(from_node, to_node);
			}

			fs_error move(sub_file_path from, sub_file_path to){
				XR_PRE_CONDITION(!to.is_absolute());
				XR_PRE_CONDITION(!from.is_absolute());
				if (is_readonly_() ||to.empty())
					return fs::er_permission_denied;

				auto pos = m_items.find(from);
				if (pos == m_items.end())
					return fs::er_not_found;

				if (from.contains(to))
					return fs::er_invalid;

				typedef std::pair<file_path, EntryInfo> value_type; 

				if (pos->second.type == fs::st_dir){	// process children first
					string pfirst = from.str() << literal("/");
					string plast = from.str() << literal("//");
					auto pos1 = m_items.lower_bound(file_path(pfirst, pp_none));
					auto pos2 = m_items.lower_bound(file_path(plast, pp_none));

					std::vector<value_type> new_entries;
					for(auto itr = pos1; itr != pos2; ++itr){
						value_type entry = *itr;
						entry.first.replace_prefix(from, to);
						entry.first.remove_absolute();
						entry.second.header.name = entry.first;
						m_items.insert(entry);
					}

					m_items.erase(pos1, pos2);
					m_items.insert(new_entries.begin(), new_entries.end());
				}

				value_type entry = *pos;
				entry.first = to;
				entry.second.header.name = to;
				m_items.erase(pos);
				m_items.insert(entry);

				return fs::er_ok;
			}

			fs_error truncate(sub_file_path path, long_size_t s){
				XR_PRE_CONDITION(!path.is_absolute());
				if (is_readonly_() ||path.empty())
					return fs::er_permission_denied;

				auto pos = m_items.find(path);
				if (pos == m_items.end())
					return fs::er_not_found;
				if (pos->second.type != fs::st_regular)
					return fs::er_not_regular;

				if (pos->second.cache_name.empty()){
					auto ret = extract_file_(pos->second);
					if (ret != fs::er_ok)	return ret;
				}

				pos->second.is_dirty = true;
				return m_cache->truncate(pos->second.cache_name, s);
			}

			fs_error extract_file_(EntryInfo& info){
				auto fin = zip::open_raw(info.header);
                auto fout = temp_file<io::write_map, io::read_map>(*m_cache, sub_file_path(), sub_file_path()
						, io::of_remove_on_close, &info.cache_name);
                
                if (info.header.method == 0){       //0: store
                    auto s = io::copy_data(fin.get<io::read_map>(), fout.get<io::write_map>());
                    if (s != fin.get<io::read_map>().size()){
                        info.cache_name = file_path();
                        return fs::er_system_error;
                    }
                }
                else if(info.header.method == 8){     // 8: deflate
                    auto ret = xirang2::zip::inflate(fin.get<xirang2::io::read_map>(), fout.get<xirang2::io::write_map>());
                    if (ret.err != 0)
                        return fs::er_data_error;
                    // else
                    // verify crc: auto crc = xirang2::zip::crc32(fout.get<xirang2::io::read_map>());
                    
                }
                else
                    return fs::er_data_error;   // TODO: unsupport compression type

				return fs::er_ok;
			}

			void sync(){
				if (is_readonly_()) return;

				m_war.get<io::write_map>().truncate(0);
				zip::reader_writer zip_writer(m_war);
				for (auto& i : m_items){
					auto &info = i.second;
					if (info.type == fs::st_dir){
						if (info.is_force_store){
							zip::file_header h;
							h.name = i.first;
							h.method = zip::cm_store;
							h.flags = 0x800;
							h.external_attrs = 0x10;	//dir
							io::empty empty;
							iref<io::reader> in(empty);
							zip_writer.append(in.get<io::reader>(), h);
						}
						continue;
					}
					if (info.is_dirty){
						auto src = m_cache->create<io::read_map>(info.cache_name, io::of_open);
						zip_writer.append(src.get<io::read_map>(), i.first);
					}
					else{
						auto src = zip::open_raw(info.header);
						zip_writer.append(src.get<io::read_map>(), info.header, zip::ft_defalted);
					}
				}
			}
			const string& resource() const { return m_resource;}
			RootFs* root() const{ return m_root;}
			bool mounted() const { return m_root != 0; }
			file_path mountPoint() const {
				return m_root ? m_root->mountPoint(*m_host) : file_path();
			}

			struct zipfs_state_selector{
				typedef const VfsState value_type;
				typedef const VfsState* pointer;
				typedef const VfsState& reference;

				mutable VfsState fst;
				IVfs* cache;

				zipfs_state_selector(IVfs* vfs_, IVfs* cache_) : cache(cache_){
					fst.owner_fs = vfs_;
				}
				const VfsState& operator()(const std::pair<const file_path, EntryInfo>& item) const{
					fst.path = item.first.filename();
					fst.state = item.second.type;
					if (item.second.cache_name.empty()){
						fst.size = item.second.header.uncompressed_size;
						fst.modified_time = dos2chrono(item.second.header.modified_date, item.second.header.modified_time);
						fst.create_time = fst.modified_time;
					}
					else{
						auto st = cache->state(item.second.cache_name);
						fst.size = st.size;
						fst.create_time = st.create_time;
						fst.modified_time = st.modified_time;
					}
					return fst;
				}
			};
			typedef std::map<file_path, EntryInfo, path_less>::const_iterator item_iterator;
#if 0	// TODO: remove after passed build
			class FileIterator
			{
				public:
					FileIterator() : m_itr() {
						m_node.owner_fs = 0;
					}

					explicit FileIterator(item_iterator itr, IVfs* vfs)
						: m_itr(itr)
					{ 
						m_node.owner_fs = vfs;
					}

					const VfsState& operator*() const
					{
						m_node.path = m_itr->first.filename();
						m_node.state = m_itr->second.type;
						if (m_itr->second.cache_name.empty()){
							m_node.size = m_itr->second.header.uncompressed_size;
							m_node.modified_time = dos2chrono(m_itr->second.header.modified_date, m_itr->second.header.modified_time);
							m_node.create_time = m_node.modified_time;
						}
						else{
							auto st = m_cache->state(m_itr->second.cache_name);
							m_node.size = st.size;
							m_node.create_time = st.create_time;
							m_node.modified_time = st.modified_time;
						}
						return m_node;
					}

					const VfsState* operator->() const
					{
						return &**this;
					}

					FileIterator& operator++()	{ ++m_itr; return *this;}
					FileIterator operator++(int) { 
						FileIterator ret = *this; 
						++*this; 
						return ret;
					}

					FileIterator& operator--(){ return *this;}
					FileIterator operator--(int){ return *this;}

					bool operator==(const FileIterator& rhs) const
					{
						return m_itr == rhs.m_itr;
					}
				private:
					item_iterator m_itr;
					mutable VfsState m_node;
			};
#endif

			VfsNodeRange children(sub_file_path path) const{
				auto ret = children_(path);
				zipfs_state_selector sel(m_host, m_cache);
				return VfsNodeRange(
						//VfsNodeRange::iterator(FileIterator(ret.begin(), m_host)),
						//VfsNodeRange::iterator(FileIterator(ret.end(), m_host))
						VfsNodeRange::iterator(make_select_iterator(ret.begin(), sel)),
						VfsNodeRange::iterator(make_select_iterator(ret.end(), sel))
						);
			}
			VfsState state(sub_file_path path) const{
				XR_PRE_CONDITION(!path.is_absolute());
				auto pos = m_items.find(path);
				if (pos != m_items.end()){
					auto fst = zipfs_state_selector(m_host, m_cache)(*pos);
					fst.path = path;
					return fst;
#if 0
					fst.state = pos->second.type;

					if (pos->second.is_dirty && pos->second.type == fs::st_regular){
						auto st = m_cache->state(m_itr->second.cache_name);
						fst.size = st.size;
						fst.create_time = st.create_time;
						fst.modified_time = st.modified_time;
					}
					else {
						fst.size = pos->second.header.uncompressed_size;
						fst.modified_time = dos2chrono(pos->second.header.modified_date, m_itr->second.header.modified_time);
						fst.create_time = fst.modified_time;
					}
#endif
				}
				return VfsState(m_host, fs::st_not_found, path);
			}


			void** do_create(unsigned long long mask,
					void** base, unique_ptr<void>& owner, sub_file_path path, int flag){
				XR_PRE_CONDITION(!path.is_absolute());
				bool need_write = (mask & detail::get_mask<io::writer, io::write_map>::value ) != 0;
				if ((is_readonly_()  && need_write) ||path.empty())
					XR_THROW(fs::permission_denied_exception);

				auto parent = path.parent();
				if (!parent.empty() && m_items.count(parent) == 0)
					XR_THROW(fs::not_found_exception);

				auto pos = m_items.find(path);

				auto f = (flag & io::of_open_create_mask);
				if (pos == m_items.end()){
					if(f == io::of_open) 
						XR_THROW(fs::not_found_exception);

					EntryInfo info;
					info.header.name = path;
					info.type = fs::st_regular;
					auto fout = temp_file<io::write_map>(*m_cache, sub_file_path(), sub_file_path()
							, io::of_remove_on_close, &info.cache_name);
					if (!fout)
						XR_THROW(fs::system_error_exception);

					pos = m_items.insert(std::make_pair(file_path(path), info)).first;
				}
				else{
					if(f == io::of_create) XR_THROW(fs::exist_exception);
					if(m_cache && pos->second.cache_name.empty() && extract_file_(pos->second) != fs::er_ok)
						XR_THROW(fs::system_error_exception);
				}

				if (mask & detail::get_mask<io::writer, io::write_map>::value )
					pos->second.is_dirty = true;

				if (m_cache)
					return m_cache->raw_create(mask, base, owner, pos->second.cache_name, (flag | io::of_create_or_open));
				else {
					auto adaptor = io::decorate<io::map_to_stream_archive
						, io::proxy_read_map_p
						, io::read_map_to_reader_p
						, io::read_map_to_random_p
						>(zip::open_raw(pos->second.header), 0);

					iauto<io::reader, io::read_map, io::random > res (std::move(adaptor));

					void** ret = copy_interface<io::reader, io::random, io::read_map>::apply(mask, base, res, res.target_ptr.get()); 
					unique_ptr<void>(std::move(res.target_ptr)).swap(owner);
					return ret;
				}
			}

			void setRoot(RootFs* r){
				XR_PRE_CONDITION(!mounted() || r == 0);
				m_root = r;
			}

			IVfs* host_() const { return m_host;}
			bool is_readonly_() const{
				return !m_war;
			}
			void init_load_(){

				zip::reader zip_reader(*m_read_map);
				auto items = zip_reader.items();
				for (auto&i : items){
					EntryInfo info;
					info.header = i;
					info.header.name.remove_absolute();
					info.type = (i.external_attrs & 0x10) ? fs::st_dir : fs::st_regular;

					create_parent_(i.name);
					auto parent = i.name.parent();
					if (!parent.empty()){
						auto pos = m_items.find(parent);
						XR_PRE_CONDITION(pos != m_items.end());
						if (pos->second.type != fs::st_dir)
							continue;
					}

					auto res = m_items.insert(std::make_pair(i.name, info));
					XR_INVARIANT(res.second);
					unuse(res);
				}
			}
			void create_parent_(const file_path& path){
				file_path parent = path.parent();
				if(!parent.empty()){
					auto pos = m_items.find(parent);
					if (pos == m_items.end()){
						create_parent_(parent);
						m_items.insert(std::make_pair(parent, make_entryinfo(parent)));
					}
				}
			}
			range<item_iterator> children_(sub_file_path path) const{
				if (path.empty()){
					auto pos1 = m_items.lower_bound(file_path());
					auto pos2 = m_items.lower_bound(file_path(literal("/"), pp_none));
					return make_range(pos1, pos2);
				}
				else{
					string pfirst = path.str() << literal("/");
					string plast = path.str() << literal("//");
					auto pos1 = m_items.lower_bound(file_path(pfirst, pp_none));
					auto pos2 = m_items.lower_bound(file_path(plast, pp_none));
					return make_range(pos1, pos2);
				}
			}
		private:
			string m_resource;
			std::map<file_path, EntryInfo, path_less> m_items;
			io::read_map* m_read_map;
			IVfs* m_cache;
			ZipFs* m_host;
			RootFs* m_root;
			iref<io::read_map, io::write_map> m_war;
			iauto<io::read_map> m_archive;
	};
	ZipFs::ZipFs(const iref<io::read_map, io::write_map>& ar, IVfs* cache, const string& resource/*  = string() */)
		: m_imp(new ZipFsImp(ar, cache, resource, this))
	{
	}
	ZipFs::ZipFs(iauto<io::read_map, io::write_map> ar, IVfs* cache, const string& resource /* = string() */)
		: m_imp(new ZipFsImp(std::move(ar), cache, resource, this))
	{}
	ZipFs::ZipFs(const iref<io::read_map>& ar, IVfs* cache, const string& resource /* = string() */)
		: m_imp(new ZipFsImp(ar, cache, resource, this))
	{}
	ZipFs::ZipFs(iauto<io::read_map> ar, IVfs* cache, const string& resource /* = string() */)
		: m_imp(new ZipFsImp(std::move(ar), cache, resource, this))
	{}

	ZipFs::~ZipFs(){}

	fs_error ZipFs::do_remove(sub_file_path path){
		return m_imp->remove(path);
	}

	fs_error ZipFs::do_createDir(sub_file_path path){
		return m_imp->createDir(path);
	}

	fs_error ZipFs::do_copy(sub_file_path from, sub_file_path to){
		return m_imp->copy(from, to);
	}
	fs_error ZipFs::do_move(sub_file_path from, sub_file_path to){
		return m_imp->move(from, to);
	}

	fs_error ZipFs::do_truncate(sub_file_path path, long_size_t s){
		return m_imp->truncate(path, s);
	}

	void ZipFs::do_sync(){ return m_imp->sync(); }
	const string& ZipFs::do_resource() const{ return m_imp->resource(); }
	RootFs* ZipFs::do_root() const{ return m_imp->root(); }
	bool ZipFs::do_mounted() const{ return m_imp->mounted(); }
	file_path ZipFs::do_mountPoint() const{ return m_imp->mountPoint(); }

	VfsNodeRange ZipFs::do_children(sub_file_path path, io_option) const{
		return m_imp->children(path);
	}

	VfsState ZipFs::do_state(sub_file_path path, io_option) const{
		return m_imp->state(path);
	}

	void** ZipFs::do_create(unsigned long long mask,
			void** base, unique_ptr<void>& owner, sub_file_path path, int flag){
		return m_imp->do_create(mask, base, owner, path, flag);
	}

	void ZipFs::do_setRoot(RootFs* r){ m_imp->setRoot(r);}
}}

