#ifndef SRC_XIRANG2_VFS_FILE_TREE_H
#define SRC_XIRANG2_VFS_FILE_TREE_H

#include <xirang2/vfs.h>
#include <xirang2/string_algo/string.h>

#include <algorithm>
#include <unordered_map>
#include <chrono>

namespace xirang2{ namespace vfs{ namespace private_{

	template<typename T> struct file_node
	{
		typedef file_node<T> node_type;
		VfsState state; // size field is not used, use data.size() directly

		typedef std::unordered_map<file_path, std::unique_ptr<node_type>, hash_file_path> children_type;
		children_type children;
		node_type* parent;

		T data;

		file_node(node_type* parent_ = 0) :
			parent(parent_),
			data()
		{
			state.state = fs::st_dir;
		}
	};

	template<typename T> struct locate_result{
		file_node<T>* node;
		sub_file_path not_found;
	};

	template <typename T>
	locate_result<T> locate(file_node<T>& root, sub_file_path path)
	{
		file_node<T>* pos = &root;

		auto itr = path.begin();
		auto end(path.end());
		for (; itr != end; ++itr ){
			auto child = pos->children.find(*itr);
			if (child != pos->children.end())
				pos = child->second.get();
			else
				break;
		}

		typedef locate_result<T> return_type;
		return return_type {
			pos, itr == end ? sub_file_path() : sub_file_path(itr->str().begin(), path.str().end())
		};
	}


	template<typename T>
	fs_error removeNode(file_node<T>* node)
	{
		XR_PRE_CONDITION(node);
		if (!node->parent)
			return fs::er_is_root;
		XR_PRE_CONDITION(node->parent->children.count(node->state.path));
        if (!node->children.empty())
            return fs::er_not_empty;


		node->parent->children.erase(node->state.path);
		return fs::er_ok;
	}


	template<typename T>
	file_node<T>* create_node(IVfs* host, const locate_result<T>& pos, file_state type, bool whole_path){
		XR_PRE_CONDITION(pos.node);
		XR_PRE_CONDITION(!pos.not_found.empty());
		XR_PRE_CONDITION(pos.node->children.count(pos.not_found) == 0);

		bool first_create = true;

		auto node = pos.node;
		for (auto& item : pos.not_found){
			XR_PRE_CONDITION(!node->children.count(item));

			if (!whole_path && !first_create){
				node->parent->children.erase(node->state.path);
				return 0;
			}

			std::unique_ptr<file_node<T> > fnode(new file_node<T>(node));
			fnode->state.path = item;
			fnode->state.owner_fs = host;
			fnode->state.state = fs::st_dir;
			auto now = std::chrono::nanoseconds(std::chrono::system_clock::now().time_since_epoch()).count();
			fnode->state.create_time = now;
			fnode->state.modified_time = now;
			auto& new_node = pos.node->children[fnode->state.path];
			new_node = std::move(fnode);
			node = new_node.get();
			first_create = false;
		}
		node->state.state = type;
		return node;
	}
	template<typename T> file_node<T>* create_node(IVfs* host, file_node<T>& root, sub_file_path path, file_state type, bool whole_path)
	{
		XR_PRE_CONDITION(!path.empty());

		auto result = locate(root, path);
		if (result.not_found.empty())
			return 0;

		return create_node(host, result, type, whole_path);
	}

}}}

#endif //end SRC_XIRANG2_VFS_FILE_TREE_H

