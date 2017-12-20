#include <xirang2/vfs/metadatacache.h>
#include <xirang2/lrumap.h>
#include <xirang2/type/itypebinder.h>
#include <xirang2/serialize/exchs11n.h>
#include <xirang2/serialize/path.h>
#include <xirang2/serialize/versiontype.h>

namespace xirang2{ namespace vfs{
	///////////// MetadataKey  ////////////////////
	struct MetadataKey{
		IVfs* fs;
		file_path path;
        
        bool operator==(const MetadataKey& rhs) const{
            return fs == rhs.fs && path == rhs.path;
        }
	};

	struct MetadataKeyHash{
		std::size_t operator()(const MetadataKey& key) const {
			return std::size_t(key.fs) ^ hash_file_path()(key.path);
		}
	};

	///////////// MetadataList  ////////////////////
    struct MetadataListImp : public std::unordered_map<file_path, MetadataValue, hash_file_path> {
		bool is_dirty = false;
	};

	struct pair_converter{
		typedef const MetadataElement value_type;
		typedef const MetadataElement* pointer;
		typedef const MetadataElement& reference;

		mutable MetadataElement value;

		const MetadataElement& operator()(const MetadataListImp::value_type& item) const{
			value.name = item.first;
			value.value = item.second;
			return value;
		}
	};

	MetadataList::MetadataList() : m_imp(new MetadataListImp){}
	MetadataList::MetadataList(const MetadataList& rhs) : m_imp(new MetadataListImp(*rhs.m_imp)){}
	MetadataList::MetadataList(MetadataList&& rhs) {
        std::swap(m_imp, rhs.m_imp);
	}

	MetadataList& MetadataList::operator=(const MetadataList& rhs){
		if (&rhs != this){
			MetadataList(rhs).swap(*this);
		}
		return *this;
	}
	MetadataList& MetadataList::operator=(MetadataList&& rhs){
		if (&rhs != this){
			rhs.swap(*this);
		}
		return *this;

	}
	MetadataList::~MetadataList(){
		delete m_imp;
	}

	bool MetadataList::empty() const{ return m_imp->empty();}
	std::size_t MetadataList::size() const { return m_imp->size();}

	MetadataList::iterator MetadataList::begin() const{
		return iterator(make_select_iterator(m_imp->begin(), pair_converter()));
	}

	MetadataList::iterator MetadataList::end() const{
		return iterator(make_select_iterator(m_imp->end(), pair_converter()));
	}

	bool MetadataList::emplace(file_path&& path, MetadataValue&& value){
		return m_imp->emplace(std::move(path), std::move(value)).second;
	}
	bool MetadataList::insert(const file_path& path, const MetadataValue&& value){
		return m_imp->emplace(std::make_pair(path, value)).second;
	}

	bool MetadataList::erase(const file_path& path){
		return m_imp->erase(path) != 0;
	}
	void MetadataList::swap(MetadataList& rhs){
        std::swap(m_imp, rhs.m_imp);
	}

	///////////// MetadataCache  ////////////////////
	DefaultMetadataCacheHandler::DefaultMetadataCacheHandler(type::Xirang& xr_) : xr(xr_){}

	DefaultMetadataCacheHandler::~DefaultMetadataCacheHandler(){}

	MetadataList DefaultMetadataCacheHandler::load_list(IVfs* fs, sub_file_path dir){
		file_path db_path = dir / file_path(".metadata.db", pp_none);

		auto ar = fs->create<io::reader>(db_path, io::of_open | io::of_nothrow);
		MetadataList lists;
		if (ar){
			auto& rd = ar.get<io::reader>();
			auto source = io::exchange::as_source(rd);
			while(rd.readable()){
				file_path file_name;
				source & file_name;

				MetadataValue value;
				load(rd, value, xr, xr.root());
				lists.emplace(std::move(file_name), std::move(value));
			}
		}
		return lists;
	}

	void DefaultMetadataCacheHandler::save_list(IVfs* fs, sub_file_path dir, const MetadataList& list)
	{
		if (list.empty()){
			remove(fs, dir);
			return;
		}
		file_path db_path = dir / file_path(".metadata.db", pp_none);

		auto ar = fs->create<io::writer, io::ioctrl>(db_path, io::of_create_or_open | io::of_nothrow);
		if (!ar)	XR_THROW(failed_create_metadatadb_exception);

		ar.get<io::ioctrl>().truncate(0);
		auto& wr = ar.get<io::writer>();
		auto sink = io::exchange::as_sink(wr);
		for(auto&v : list){
			sink & v.name;
			save(wr, v.value);
		}
	}

	void  DefaultMetadataCacheHandler::remove(IVfs* fs, sub_file_path dir){
		file_path db_path = dir / file_path(".metadata.db", pp_none);
		auto st = fs->remove(db_path);
		if (st != fs::er_ok &&  st !=  fs::er_not_found)
			XR_THROW(failed_remove_metadata_db_exception);
	}


	///////////// MetadataCache  ////////////////////
	struct PseudoMetadataList{
		void* holder;
	};
	class MetadataCacheImp{
    public:
		typedef lru_map<MetadataKey, MetadataListImp, MetadataKeyHash> cache_type;
		cache_type m_cache;
		std::unordered_map<IVfs*, MetadataCacheHandler*> m_all_vfs;

		explicit MetadataCacheImp(std::size_t item_capacity )
			: m_cache(item_capacity)
		{}

		// item info
		std::size_t missed() const { return m_cache.missed();}
		std::size_t hitted() const { return m_cache.hitted();}
		std::size_t capacity() const { return m_cache.capacity();}
		std::size_t size() const { return m_cache.size();}

		const MetadataValue& fetch(IVfs* fs, sub_file_path path){
			XR_PRE_CONDITION(fs);
			auto ret = try_fetch(fs, path);
			XR_PRE_CONDITION(ret);
			return *ret;
		}
		const MetadataValue* try_fetch(IVfs* fs, sub_file_path path){
			auto result = fetch_or_load(fs, path.parent());
			if (!result)	return nullptr;

			auto pos = result->find(path.filename());
			return pos == result->end() ? nullptr : &pos->second;
		}

		// can be add or update
		// write through or sync dirty?
		// Imp: mark the affected item as dirty, flush() will sync changes to vfs. 
		void set(IVfs* fs, sub_file_path path, MetadataValue&& value){
			auto result = fetch_or_load(fs, path.parent());
			if (!result)	return;

			(*result)[path.filename()] = std::move(value);
			result->is_dirty = true;
		}

		bool remove_item(IVfs* fs, sub_file_path path){
			auto ret = m_cache.erase(MetadataKey{fs, file_path(path)});
			bool bExist = !ret.empty();
			if (bExist)
				m_all_vfs.find(fs)->second->remove(fs, path);

			auto result = fetch_or_load(fs, path.parent());
			if (!result)	return bExist;

			auto filename = path.filename();
			auto pos = result->find(filename);
			if (pos != result->end())
			{
				result->is_dirty = true;
				result->erase(filename);
			}
			return bExist;
		}

		void flush(){
			m_cache.for_each(
					[&](cache_type::list_type::value_type& v){
					auto h = m_all_vfs.find(v.first.fs)->second;
					store_(v, h);
					});
		}

		void register_fs(IVfs* fs, MetadataCacheHandler& handler){
			XR_PRE_CONDITION(!registered_(fs));
			m_all_vfs[fs] = &handler;
		}

		void clear_fs(IVfs* fs){
			XR_PRE_CONDITION(!registered_(fs));
			cache_type::list_type output;
			m_cache.erase_if([fs](cache_type::index_value_type& v){ return v.first.fs == fs; }
					, output);

			for(auto& v : output){
				auto pos = m_all_vfs.find(v.first.fs);
				XR_PRE_CONDITION(pos != m_all_vfs.end());
				store_(v, pos->second);
			}
			m_all_vfs.erase(fs);
		}

	private:
		void store_(cache_type::list_type::value_type& item, MetadataCacheHandler* h){
			if (!item.second.is_dirty)
				return;

			PseudoMetadataList list{&item.second};
			void* plist = &list;

			h->save_list(item.first.fs, item.first.path, *static_cast<MetadataList*>(plist));
			item.second.is_dirty = false;
		}

		MetadataListImp* load_(MetadataKey& key){
			if (key.fs->state(key.path).state == fs::st_not_found)	return nullptr;

			auto h = m_all_vfs.find(key.fs)->second;

			MetadataList lists = h->load_list(key.fs, key.path);

			cache_type::list_type output;
			m_cache.set(key, std::move(*lists.m_imp), output);
			for(auto& v : output){
				auto pos = m_all_vfs.find(v.first.fs);
				XR_PRE_CONDITION(pos != m_all_vfs.end());
				store_(v, pos->second);
			}
			return m_cache.try_fetch(key);
		}

		MetadataListImp* fetch_or_load(IVfs* fs, sub_file_path dir){
			XR_PRE_CONDITION(fs && registered_(fs));

			MetadataKey key{fs, dir};
			auto result = m_cache.try_fetch(key);
			if (result != nullptr)	return result;

			return load_(key);
		}

		bool registered_(IVfs* fs) const{
			return m_all_vfs.count(fs) != 0;
		}
	};

	MetadataCache::MetadataCache(std::size_t item_capacity)
		: m_imp(new MetadataCacheImp(item_capacity))
	{}

	// item info
	std::size_t MetadataCache::missed() const { return m_imp->missed();}
	std::size_t MetadataCache::hitted() const { return m_imp->hitted();}
	std::size_t MetadataCache::capacity() const { return m_imp->capacity();}
	std::size_t MetadataCache::size() const { return m_imp->size();}

	const MetadataValue& MetadataCache::fetch(IVfs* fs, sub_file_path path){
		return m_imp->fetch(fs, path);
	}
	const MetadataValue* MetadataCache::try_fetch(IVfs* fs, sub_file_path path){
		return m_imp->try_fetch(fs, path);
	}
	void MetadataCache::set(IVfs* fs, sub_file_path path, MetadataValue value){
		m_imp->set(fs, path, std::move(value));
	}

	bool MetadataCache::remove_item(IVfs* fs, sub_file_path path){
		return m_imp->remove_item(fs, path);
	}

	void MetadataCache::flush(){
		m_imp->flush();
	}

	void MetadataCache::register_fs(IVfs* fs, MetadataCacheHandler& handler){
		m_imp->register_fs(fs, handler);
	}

	void MetadataCache::clear_fs(IVfs* fs){
		m_imp->clear_fs(fs);
	}

}}


