#ifndef XIRANG2_VFS_METADATA_MGR_H_
#define XIRANG2_VFS_METADATA_MGR_H_
#include <xirang2/config.h>
#include <xirang2/path.h>
#include <xirang2/vfs.h>
#include <xirang2/type/xirang.h>

namespace xirang2{ namespace vfs{
	XR_EXCEPTION_TYPE(failed_create_metadatadb_exception);
	XR_EXCEPTION_TYPE(failed_remove_metadata_db_exception);
	XR_EXCEPTION_TYPE(failed_resolve_type_exception);


	struct XR_API MetadataElement{
		file_path name;
		MetadataValue value;
	};

	struct MetadataListImp;
	class XR_API MetadataList{
		public:
			MetadataList();
			MetadataList(const MetadataList& rhs);
			MetadataList(MetadataList&& rhs);

			MetadataList& operator=(const MetadataList& rhs);
			MetadataList& operator=(MetadataList&& rhs);
			~MetadataList();

			bool empty() const;
			std::size_t size() const;

			typedef forward_iterator<const_itr_traits<MetadataElement> > iterator;
			iterator begin() const;
			iterator end() const;

			bool emplace(file_path&& path, MetadataValue&& value);
			bool insert(const file_path& path, const MetadataValue&& value);

			bool erase(const file_path& path);

			void swap(MetadataList& rhs);

		private:
			MetadataListImp* m_imp;
			friend class MetadataCacheImp;
	};

	struct XR_INTERFACE MetadataCacheHandler{
		virtual MetadataList load_list(IVfs* fs, sub_file_path dir) = 0;
		virtual void save_list(IVfs* fs, sub_file_path dir, const MetadataList& list) = 0;
		virtual void remove(IVfs* fs, sub_file_path dir) = 0;
		protected:
			virtual ~MetadataCacheHandler(){}
	};

	struct XR_API DefaultMetadataCacheHandler : public MetadataCacheHandler{
		explicit DefaultMetadataCacheHandler(type::Xirang& xr_) ;
		virtual MetadataList load_list(IVfs* fs, sub_file_path dir);
		virtual void save_list(IVfs* fs, sub_file_path dir, const MetadataList& list);
		virtual void remove(IVfs* fs, sub_file_path dir);
		virtual ~DefaultMetadataCacheHandler();
		private:
		type::Xirang& xr;
	};

	class MetadataCacheImp;
	class XR_API MetadataCache{
		unique_ptr<MetadataCacheImp> m_imp;

		MetadataCache(const MetadataCache&) = delete;
		MetadataCache& operator=(const MetadataCache&) = delete;

		public:
		explicit MetadataCache(std::size_t item_capacity = 65536);
		~MetadataCache();

		// item info
		std::size_t missed() const; 
		std::size_t hitted() const;
		std::size_t capacity() const;
		std::size_t size() const;

		const MetadataValue& fetch(IVfs* fs, sub_file_path path);

		const MetadataValue* try_fetch(IVfs* fs, sub_file_path path);

		// can be add or update
		// write through or sync dirty?
		// Imp: mark the affected item as dirty, flush() will sync changes to vfs. 
		void set(IVfs* fs, sub_file_path path, MetadataValue value);

		bool remove_item(IVfs* fs, sub_file_path path);

		void flush();

		void register_fs(IVfs* fs, MetadataCacheHandler& handler);

		void clear_fs(IVfs* fs);
	};

}}
#endif //end XIRANG2_VFS_METADATA_MGR_H_

