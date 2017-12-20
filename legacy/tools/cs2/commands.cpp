#include "commands.h"
#include "configfile.h"

#include <xirang2/string_algo/uri.h>
#include <xirang2/string_algo/json.h>
#include <xirang2/string_algo/string.h>
#include <xirang2/type/s11njson.h>
#include <xirang2/type/typeio.h>
#include <xirang2/serialize/s11nbasetype.h>
#include <xirang2/serialize/path.h>
#include <xirang2/versionedvfs.h>
#include <xirang2/vfs/local.h>
#include <xirang2/fsutility.h>
#include <unordered_map>
#include <map>


#include <iostream>
#include <boost/lexical_cast.hpp>

#include <boost/algorithm/searching/boyer_moore.hpp>

#include <future>
#include <vector>
#include <xirang2/io/file.h>
#include <chrono>

#define NOT_IMPLEMENTED \
::xirang2::fcgi::StreamGroup sg(request);\
context.response_header(sg.fout());\
auto m = request.getParam("REQUEST_METHOD"); \
sg.fout() << m << " " << command.str() << " is not implemented\n";


using namespace xirang2;
//using namespace std;

namespace cs2{
    namespace {
        const xirang2::file_path path_metabase(".metabase");
    }

	template<typename ACont, typename K, typename V> V&& get_value(ACont&& cont, K&& key, V&& default_){
		auto pos = cont.find(key);
		return std::forward<V&&>(pos == cont.end() ? default_ : pos->second);
	}
		
#if 1

typedef boost::algorithm::boyer_moore<const char*> bm_type;
   
std::vector<const char*> search(const char* first, const char* last, const bm_type& bm, const bm_type& bm_dim, std::size_t  p_size) {
    std::vector<const char*> result;
    auto end = last - p_size + 1;
    for (;first < end;) {
        auto found = bm(first, last);
        if (found != last) { //found!
            auto id_end = bm_dim(found, last);
            found = xirang2::rfind(first, id_end, 0);
            result.push_back(found + 1);

            first = id_end;
        }
        else break;
    }

    return result;
}

std::vector<const char*> para_search(const std::vector<std::pair<const char*, const char*>>& table, const bm_type& bm, const bm_type& bm_dim, std::size_t  p_size) {
    
    std::vector<std::future<std::vector<const char*>>> futures;
    for (auto& p : table)
        futures.push_back(
            std::async(std::launch::async, search, p.first, p.second, bm, bm_dim, p_size)
            );

    std::vector<const char*> result;
    for (auto& f : futures) {
        auto v = f.get();
        result.insert(result.end(), v.begin(), v.end());
    }

    return result;
}
	void show_file(xirang2::fcgi::StreamGroup& sg, vfs::IVfs& fs, const file_path& dir){
		int first = 1;
		for (auto& i: fs.children(dir)){
			if (first != 1)
				sg.fout()<<",";
			file_path path = dir/i.path;
			if (fs.state(path).state == fs::st_regular){
				//cout << "\t" << path.str() << std::endl;
				sg.fout() << "\"" <<path.str() << "\"";
			}
			else
				show_file(sg, fs, path);
			first = 0;
		}
	}

	std::map<xirang2::const_range_string, xirang2::const_range_string> parse_query_str(xirang2::const_range_string str){
		std::map<xirang2::const_range_string, xirang2::const_range_string> result;
		for(auto first = str.begin();;){
			auto last = std::find(first, str.end(), '&');
			xirang2::const_range_string field(first, last);
			if (!field.empty()){
				auto equal_pos = std::find(field.begin(), field.end(), '=');
				xirang2::const_range_string name(field.begin(), equal_pos);
				if (equal_pos != field.end()) ++equal_pos;
				xirang2::const_range_string value(equal_pos, field.end());
				result[name] = value;
			}
			if (last == str.end())	break;
			first = ++last;
		}

		return result;
	}

	ADD_GET_COMMAND
    ("fs/file")
    {
        ::xirang2::fcgi::StreamGroup sg(request);
        
        CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
        
        auto ar = user_context.runtime.rootFs().create<xirang2::io::read_map>(path, xirang2::io::of_open | xirang2::io::of_nothrow);
        if (!ar){
            context.response_error(sg.fout(),  404, path);
            return;
        }
        
        auto & rmap = ar.get<xirang2::io::read_map>();
        auto view = rmap.view_rd(xirang2::ext_heap::handle(0, rmap.size()));
        auto address = view.get<xirang2::io::read_view>().address();
        
        xirang2::string header = xirang2::literal("Content-Disposition: attachment; filename=\"") << path.filename().str() << xirang2::literal("\"\r\n");
        
        if (path.has_version())
            context.response_header(sg.fout(), 200, header << xirang2::literal("Cache-Control: public, max-age=7776000\r\n"));
        else
            context.response_header(sg.fout(), 200, header << xirang2::literal("Cache-Control: public, max-age=604800\r\n"));
        
        sg.fout().write((const char*)address.begin(), address.size());
    };
    

    ADD_GET_COMMAND
    ("fs/content")
    {
        ::xirang2::fcgi::StreamGroup sg(request);
        
        CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
        
        
        
        if (path.under(xirang2::sub_file_path(xirang2::literal("/lib/adsk/cm/1.0/resource")))){        //file
            auto ar = user_context.runtime.rootFs().create<xirang2::io::read_map>(path, xirang2::io::of_open | xirang2::io::of_nothrow);
            if (!ar){
                context.response_error(sg.fout(),  404, path);
                return;
            }
            auto & rmap = ar.get<xirang2::io::read_map>();
            auto view = rmap.view_rd(xirang2::ext_heap::handle(0, rmap.size()));
            auto address = view.get<xirang2::io::read_view>().address();
            
            xirang2::string header = xirang2::literal("Content-Disposition: attachment; filename=\"") << path.filename().str() << xirang2::literal("\"\r\n");
            
            if (path.has_version())
                context.response_header(sg.fout(), 200, header << xirang2::literal("Cache-Control: public, max-age=7776000\r\n"));
            else
                context.response_header(sg.fout(), 200, header << xirang2::literal("Cache-Control: public, max-age=604800\r\n"));
            
            sg.fout().write((const char*)address.begin(), address.size());
            return;
        }
        
            
        if (path.under(xirang2::sub_file_path(xirang2::literal("/lib/adsk/cm/1.0/sys")))){         // type
            auto ret = user_context.runtime.rootFs().locate(path.exclude_version());

            xirang2::file_path dir = path.parent();
            dir.remove_absolute();
            
            xirang2::version_type type_version(path.version().str());
            if (xirang2::is_empty(type_version)){
                type_version = user_context.cm_repo->getFileVersion(xirang2::version_type(), ret.path);
                
            }
            
            if (user_context.runtime.rootFs().state(path).state != xirang2::fs::st_regular){
                context.response_error(sg.fout(),  404, path);
                return;
            }
            
            
            xirang2::type::NamespaceBuilder nsb;
            //auto dest_ns = getOrCreateNamespace(nsb.get(), dir);
            
            xirang2::type::BinaryTypeLoader loader(user_context.runtime.rootFs(), user_context.runtime.root());
            xirang2::type::Type t = loader.load(path.exclude_version(), type_version, nsb.get());
            
            context.response_header(sg.fout());
            sg.fout() << "{\"arg\":"
            << xirang2::json::escape_string(path.str())
            << ",\"result\":"
            << xirang2::io::json_obj::to_json(t, 0)
            << "}";
            
            return;
        }
        
        if (path.under(xirang2::sub_file_path(xirang2::literal("/lib/adsk/cm/1.0")))){        // object
            std::unordered_map<xirang2::file_path, std::tuple<xirang2::file_path, xirang2::version_type>, xirang2::hash_file_path> type_table;
            
            auto dir = path.parent();
            auto res = user_context.runtime.rootFs().locate(dir);
            
            auto metar = user_context.runtime.rootFs().create<xirang2::io::reader>(dir / path_metabase, xirang2::io::of_open | xirang2::io::of_nothrow);
            if (!metar) {
                context.response_error(sg.fout(),  404, path);
                return;
            }
            
            auto& rd = metar.get<xirang2::io::reader>();
            auto source = xirang2::io::exchange::as_source(rd);
            xirang2::file_path obj_name;
            while(rd.readable()){
                source & obj_name;
                auto& tp = type_table[obj_name];
                source & std::get<0>(tp) & std::get<1>(tp);
            }
            
            xirang2::type::NamespaceBuilder nsb;
            //auto dest_ns = getOrCreateNamespace(nsb.get(), dir);
            xirang2::type::BinaryTypeLoader loader(user_context.runtime.rootFs(), user_context.runtime.root());
            
            auto pos = type_table.find(path.filename());
            if (pos == type_table.end()) {
                context.response_error(sg.fout(),  404, path);
                return;
            }
            auto& tp = pos->second;
            auto objar = user_context.runtime.rootFs().create<xirang2::io::reader>(path, xirang2::io::of_open|xirang2::io::of_nothrow);
            if (!objar){
                context.response_error(sg.fout(),  404, path);
                return;
            }
            xirang2::type::Type t = loader.load(std::get<0>(tp), std::get<1>(tp), nsb.get());
            XR_PRE_CONDITION(t);
            xirang2::type::ScopedObjectCreator obj_creator(t, user_context.runtime);
            t.methods().deserialize(objar.get<xirang2::io::reader>(), obj_creator.get(), user_context.runtime.get_heap(), user_context.runtime.get_ext_heap());
            
            context.response_header(sg.fout());
            sg.fout() << "{\"arg\":"
            << xirang2::json::escape_string(path.str())
            << ",\"result\":"
            << xirang2::io::json_obj::to_json(obj_creator.get(), 0)
            << "}";
            

            return;
        }
        
        
        context.response_error(sg.fout(),  404, path);
    };

    // ==== For single file/dir
    
    //input:
    //p file_path
    //
    // output: in header
    // file_size <standard http header>
    // file_type regular, dir, etc
    // mime_type <standard http header>
    // version sha1
    //
    // error:
    // 404,403
    
    
    
    // input:
    // p    file_path
    // output: as json
    // type
    // size
    // mime_type
    // version
    // error: 404,403
    ADD_GET_COMMAND("fs/info"){
        NOT_IMPLEMENTED;
    };
    
    ADD_GET_COMMAND("fs/history"){
        NOT_IMPLEMENTED;
    };
    
    // ==== for multiple files/dirs
    // GET input:
    // p a,b,c...
    // fmt zip;simple;json;
    // POST input:
    // json
    ADD_GET_COMMAND("fs/contents"){
        NOT_IMPLEMENTED;
    };
    
    
    //GET input:
    // p: a,b,c...
    // fmt
    ADD_GET_COMMAND("fs/ginfos"){
        NOT_IMPLEMENTED;
    };
    
    // for authrized service only
    ADD_GET_COMMAND("fs/tree"){
        NOT_IMPLEMENTED;
    };
    // for authrized service only
    ADD_GET_COMMAND("fs/treeinfo"){
        NOT_IMPLEMENTED;
    };
    // ==== for single object/type/namespace
    
    ADD_GET_COMMAND("rt/type"){
        ::xirang2::fcgi::StreamGroup sg(request);
        
        CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
        auto type = user_context.runtime.root().locateType(path.exclude_version());
        
        if (!type.valid()){
            context.response_error(sg.fout(),  404, path);
            return;
        }
        
        context.response_header(sg.fout());
        sg.fout() << "{\"arg\":"
        << xirang2::json::escape_string(path.str())
        << ",\"result\":"
        << xirang2::io::json_obj::to_json(type, 0)
        << "}";
    };
    
    ADD_GET_COMMAND("rt/object"){
        ::xirang2::fcgi::StreamGroup sg(request);
        
        
        CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
        auto ns = user_context.runtime.root().locateNamespace(path.parent());

        if (!ns.valid()){
            context.response_error(sg.fout(),  404, path);
            return;
        }
        
        auto obj = ns.findObject(path.filename());

        if (!obj.name){
            context.response_error(sg.fout(),  404, path);
            return;
        }
        context.response_header(sg.fout());
        sg.fout() << "{\"arg\":"
            << xirang2::json::escape_string(path.str())
            << ",\"result\":"
            << xirang2::io::json_obj::to_json(obj.value, 0)
            << "}";
    };
    
    ADD_GET_COMMAND("rt/objectbyid"){
        ::xirang2::fcgi::StreamGroup sg(request);
        
		CMContext& user_context = static_cast<CMContext&>(*context.user_context);
		auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
		auto iter = user_context.object_path_table.find(path);
		if (iter == user_context.object_path_table.end()) {
			context.response_error(sg.fout(), 404, path);
			return;
		}
		auto& fullpath = iter->second;
		auto ns = user_context.runtime.root().locateNamespace(fullpath.parent());
		if (!ns.valid()){
			context.response_error(sg.fout(),  404, path);
			return;
		}

		auto obj = ns.findObject(path.filename());

		if (!obj.name){
			context.response_error(sg.fout(),  404, path);
			return;
		}
		context.response_header(sg.fout());
		sg.fout() << "{\"arg\":"
			<< xirang2::json::escape_string(path.str())
			<< ",\"result\":"
			<< xirang2::io::json_obj::to_json(obj.value, 0)
			<< "}";
	};

    ADD_GET_COMMAND("rt/namespace"){
        ::xirang2::fcgi::StreamGroup sg(request);
        
        
        CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
        auto ns = user_context.runtime.root().locateNamespace(path);
        
        
        if (!ns.valid()){
            context.response_error(sg.fout(),  404, path);
            return;
        }
        
        context.response_header(sg.fout());
        
        
        sg.fout() << "{\"arg\":" << xirang2::json::escape_string(ns.fullName().str())
            << ",\"result\":{\"namespace\":[";
        
        bool is_first = true;
        for(auto i :  ns.namespaces())
        {
            if (!is_first)
                sg.fout() << ",";
            else
                is_first = false;
            
            sg.fout() << xirang2::json::escape_string(i.name().str());
        }
        sg.fout() << "],\"type\":[";
        
        is_first = true;
        for(auto i : ns.types()){
            if (!is_first)
                sg.fout() << ",";
            else
                is_first = false;
            
            sg.fout() << xirang2::json::escape_string(i.current().name().str());
        }
        sg.fout() << "],\"object\":[";
        
        is_first = true;
        for(auto i : ns.objects()){
            if (!is_first)
                sg.fout() << ",";
            else
                is_first = false;
            
            sg.fout() << xirang2::json::escape_string(i.name->str());
        }
        sg.fout() << "]}}";
        
    };
    
    ADD_GET_COMMAND("rt/alias"){
        NOT_IMPLEMENTED;
    };
    
    // ==== for multiple objects/types/namespaces
    ADD_GET_COMMAND("rt/types"){
        ::xirang2::fcgi::StreamGroup sg(request);
        
        
        CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
        auto ns = user_context.runtime.root().locateNamespace(path);
        
        if (!ns.valid()){
            context.response_error(sg.fout(),  404, path);
            return;
        }
        
        context.response_header(sg.fout());
        sg.fout() << "{\"arg\":"
        << xirang2::json::escape_string(path.str())
        << ",\"result\":{";
        bool is_first = true;
        for(auto i : ns.types()){
            if (is_first)
                is_first = false;
            else
                sg.fout() << ",";
            sg.fout() << xirang2::json::escape_string(i.current().name().str()) << ":" << xirang2::io::json_obj::to_json(i.current(), 0);
        }
        sg.fout() << "}}";

        
    };

    void list_objects(std::ostream& os, const xirang2::file_path& prefix, xirang2::type::Namespace ns, bool& is_first, bool is_recursively){
        for (auto obj : ns.objects()){
            if (is_first)
                is_first = false;
            else
                os << ",";
            os << xirang2::json::escape_string((prefix/ *obj.name).str()) << ":" << xirang2::io::json_obj::to_json(obj.value, 0);
        }

		if (is_recursively)
			for (auto sub_ns : ns.namespaces())
				list_objects(os, prefix/sub_ns.name(), sub_ns, is_first, is_recursively);
    }
    


    ADD_GET_COMMAND("rt/objects"){
        ::xirang2::fcgi::StreamGroup sg(request);
        
        CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto query_str = xirang2::uri::http_decode_string(request.getParam("QUERY_STRING"));
		auto param = parse_query_str(query_str);
		auto path = xirang2::file_path(get_value(param, xirang2::literal("p"), xirang2::const_range_string()));

        auto ns = user_context.runtime.root().locateNamespace(path);
        
        if (!ns.valid()){
            context.response_error(sg.fout(),  404, path);
            return;
        }
        
		context.response_header(sg.fout());
		sg.fout() << "{\"arg\":"
			<< xirang2::json::escape_string(path.str())
			<< ",\"result\":{";

		bool is_first = true;
		list_objects(sg.fout(), xirang2::file_path(), ns, is_first,  (param.count(xirang2::literal("recursively")) != 0));
        sg.fout() << "}}";
    };
    
    const char dim[2] = { 0,0 };
    const bm_type bm_dim(dim, dim + sizeof(dim));

    ADD_GET_COMMAND("search/objects"){
		::xirang2::fcgi::StreamGroup sg(request);

		CMContext& user_context = static_cast<CMContext&>(*context.user_context);
		auto query_str = xirang2::uri::http_decode_string(request.getParam("QUERY_STRING"));
		auto param = parse_query_str(query_str);
		auto keyword = xirang2::file_path(get_value(param, xirang2::literal("p"), xirang2::const_range_string()));

		auto ns = user_context.runtime.root();
		if (!ns.valid()){
			context.response_error(sg.fout(),  404, keyword);
			return;
		}

		auto key = keyword.str();

		bm_type bm(&*key.begin(), &*key.begin() + key.size());
		auto hit =  para_search(user_context.idx_table, bm, bm_dim, key.size());

		context.response_header(sg.fout());
		sg.fout() << "{\"arg\":"
			<< xirang2::json::escape_string(key)
			<< ",\"result\":{";

		static const xirang2::file_path prefix("/lib/adsk/cm/1.0/");
		bool is_first = true;
		for (auto& id : hit)
		{
			if (is_first) is_first = false;
			else sg.fout() << ",";

			xirang2::file_path path(prefix/xirang2::file_path(id, xirang2::pp_none));
			auto parent = ns.locateNamespace(path.parent());
			auto obj = parent.findObject(path.filename());
			sg.fout() << xirang2::json::escape_string(path.str()) << ":" << xirang2::io::json_obj::to_json(obj.value, 0);

		}
		sg.fout() << "}}";
	};

    ADD_GET_COMMAND
    ("rp/submissions")
    {
        ::xirang2::fcgi::StreamGroup sg(request);
        
	CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
        

	auto repo = user_context.cm_repo;
	auto sub = repo->getSubmission(version_type());
	if (!sub || sub->version == version_type()){
		std::cout << "Empty repo\n";
	}
		
            context.response_header(sg.fout());
            sg.fout() << "{\"items\":[";
	int first  = 1;
	for (;;){
	    if (first != 1)
	    {
	    	sg.fout() << ",";
	    }
	    sg.fout() << "{";
	    sg.fout() << "\"version\":" << "\"" << sub->version.id << "\"" << ","
		<< "\"author\":" << "\"" << sub->author << "\"" << ","
		<< "\"id\":" << "\"" << sub->tree.id << "\"" << ","
		<< "\"description\":" << "\"" << sub->description << "\"" << ","
		<< "\"submitter\":" << "\"" << sub->submitter << "\"";
	    sg.fout() << "}";
		if (!xirang2::is_empty(sub->prev))
			sub = repo->getSubmission(sub->prev);
		else
			break;
	    first = 0;
	}
	    sg.fout() << "]}";
    };

    ADD_GET_COMMAND
    ("rp/cat")
    {
        ::xirang2::fcgi::StreamGroup sg(request);
        
	CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
        
std::string p = path.str().c_str();
auto pos = p.find("&");
if (pos == std::string::npos)
	return;

	std::string idStr = p.substr(0, pos);
	std::string filePath = p.substr(pos+1, p.length()-pos-1);

	std::cout<<"filePath="<<filePath<<std::endl;
	std::cout<<"idStr="<<idStr<<std::endl;

	/*vfs::LocalFs vfs(file_path("/home/lee_src/PlatformSDK/sandbox/test/mygit"));
   	
	vfs::LocalRepository repo(vfs, file_path());*/
	version_type id(sha1_from_string(as_range_string(idStr.c_str())));
	std::cout<<path.str()<<std::endl;
	std::cout<<"After print path"<<std::endl;

	vfs::LocalRepository& repo = *(user_context.cm_repo);
	auto type =  repo.blobType(id);
	//auto type = (*user_context->cm_repo).blobType(id);
	switch(type){

	case vfs::bt_tree:
	{
            context.response_header(sg.fout());
sg.fout() << "{\"arg\":"
          << xirang2::json::escape_string(xirang2::string(filePath.c_str()));
	    //sg.fout() << "{\"items\":[";
	    sg.fout() << ",\"items\":[";
	    int first = 1;
	    for (auto& i: repo.treeItems(id)){
			std::cout << i.version.id << "\t" << i.name.str() << std::endl;
	    if (first != 1)
	    {
	    	sg.fout() << ",";
	    }
	    sg.fout() << "{";
	    sg.fout() << "\"id\":" << "\"" << i.version.id << "\"" << ","
	              << "\"description\":" << "\"" << i.name.str() << "\"";
	    sg.fout() << "}";
	    first = 0;
	    }
	    sg.fout() << "]}";	
	}
	break;
	case vfs::bt_submission:
	{
	    auto sub = repo.getSubmission(id);
            context.response_header(sg.fout());
            sg.fout() << "{\"items\":[";
	    sg.fout() << "{";
	    sg.fout() << "\"author\":" << "\"" << sub->author << "\"" << ","
		<< "\"id\":" << "\"" << sub->tree.id << "\"" << ","
		<< "\"description\":" << "\"" << sub->description << "\"" << ","
		<< "\"submitter\":" << "\"" << sub->submitter << "\"";
	    sg.fout() << "}";
	    sg.fout() << "]}";	
	    
	}
	break;
	default:
	std::cout << "Not found\n";
	}

    };

    ADD_GET_COMMAND
    ("rp/content")
    {
        ::xirang2::fcgi::StreamGroup sg(request);
        
	CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));

std::string p = path.str().c_str();
auto index = p.find("&");
if (index == std::string::npos)
	return;

	std::string idStr = p.substr(0, index);
	std::string filePath = p.substr(index+1, p.length()-index-1);

	xirang2::file_path realPath(filePath.c_str());
	auto dir = realPath.parent();
	std::unordered_map<xirang2::file_path, std::tuple<xirang2::file_path, xirang2::version_type>, xirang2::hash_file_path> type_table;
	    auto metar = user_context.runtime.rootFs().create<xirang2::io::reader>(dir / path_metabase, xirang2::io::of_open | xirang2::io::of_nothrow);
            if (!metar) {
				std::cout<<"metar == null"<<std::endl;
                context.response_error(sg.fout(),  404, path);
                return;
            }
            
            auto& rd = metar.get<xirang2::io::reader>();
            auto source = xirang2::io::exchange::as_source(rd);
            xirang2::file_path obj_name;
            while(rd.readable()){
                source & obj_name;
                auto& tp = type_table[obj_name];
                source & std::get<0>(tp) & std::get<1>(tp);
            }
xirang2::type::NamespaceBuilder nsb;
            xirang2::type::BinaryTypeLoader loader(user_context.runtime.rootFs(), user_context.runtime.root());
            

	    auto pos = type_table.find(realPath.filename());
            if (pos == type_table.end()) {
				std::cout<<"pos == type_table.end()"<<std::endl;
                context.response_error(sg.fout(),  404, realPath);
                return;
            }

	vfs::LocalRepository& repo = *(user_context.cm_repo);
	//xirang2::file_path id("#764c08d293f32bc5e1d7932599254cc6a87887ca");
	std::string id_stdStr = "#" + idStr;
	//xirang2::file_path id("#764c08d293f32bc5e1d7932599254cc6a87887ca");
	xirang2::file_path id(id_stdStr.c_str());
	auto objar = repo.create<xirang2::io::reader>(id, xirang2::io::of_open|xirang2::io::of_nothrow);
	if (!objar){

		std::cout<<"objar == null"<<std::endl;
                context.response_error(sg.fout(),  404, path);
                return;
            }

	auto& tp = pos->second;
xirang2::type::Type t = loader.load(std::get<0>(tp), std::get<1>(tp), nsb.get());
            XR_PRE_CONDITION(t);
            xirang2::type::ScopedObjectCreator obj_creator(t, user_context.runtime);
            t.methods().deserialize(objar.get<xirang2::io::reader>(), obj_creator.get(), user_context.runtime.get_heap(), user_context.runtime.get_ext_heap());
            
            context.response_header(sg.fout());

	    sg.fout() << "{\"arg\":"
            //<< xirang2::json::escape_string(path.str())
            << xirang2::json::escape_string(realPath.str())
            << ",\"result\":"
            << xirang2::io::json_obj::to_json(obj_creator.get(), 0)
            << "}";

    };

    ADD_GET_COMMAND
    ("rp/workspace")
    {
        ::xirang2::fcgi::StreamGroup sg(request);
        
	CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
   	
        context.response_header(sg.fout());

	sg.fout() << "{\"Added\":[";
	show_file(sg, *user_context.cm_workspace, file_path()); 
	sg.fout() << "]}";
    };

    ADD_GET_COMMAND("rt/namespaces"){
    };
    
    ADD_GET_COMMAND("rt/aliases"){
    };
    

    extern void convert_obj(const std::string& js, xirang2::type::CommonObject obj);
    ADD_COMMAND("POST", "rt/object"){
        ::xirang2::fcgi::StreamGroup sg(request);
        
        CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        auto size = boost::lexical_cast<std::size_t>(request.getParam ("CONTENT_LENGTH"));
        
        std::string json;
        json.resize(size+1);
        sg.fin().read(&json[0], size);
        json.resize(size);
        
        auto path = xirang2::file_path(xirang2::uri::http_decode_string(request.getParam("QUERY_STRING")));
        auto ns = user_context.runtime.root().locateNamespace(path.parent());
        
        if (!ns.valid()){
            context.response_error(sg.fout(),  404, path);
            return;
        }
        
        auto obj = ns.findObject(path.filename());
        
        if (!obj.name){
            context.response_error(sg.fout(),  404, path);
            return;
        }
        
        
        //1. load from json to object
        
        convert_obj(json, obj.value);
        
        //2. serialize the object to workspace
        auto res = user_context.runtime.rootFs().locate(path);
        xirang2::vfs::recursive_create_dir(*user_context.cm_workspace, res.path.parent());
        auto ar = user_context.cm_workspace->create<xirang2::io::writer, xirang2::io::ioctrl>(res.path, xirang2::io::of_create_or_open);
        ar.get<xirang2::io::ioctrl>().truncate(0);
        obj.value.type().methods().serialize(ar.get<xirang2::io::writer>(), obj.value);
        
        
        context.response_header(sg.fout());
        sg.fout() << "{\"result\": \"success\"}";
        
    };
    
    ADD_COMMAND("POST", "rp/commit"){
        
        ::xirang2::fcgi::StreamGroup sg(request);
        
        CMContext& user_context = static_cast<CMContext&>(*context.user_context);
        xirang2::version_type base = user_context.cm_repo->getSubmission(xirang2::version_type())->version;

        auto description = xirang2::uri::http_decode_string(request.getParam("QUERY_STRING"));
        
        
        auto c = user_context.cm_repo->commit(*user_context.cm_workspace, description, base);
        if (!c || c->flag == xirang2::vfs::bt_none){
            context.response_error(sg.fout(), 412, xirang2::file_path(""));
        }
        else {
            std::vector<xirang2::file_path> paths;
            for (auto dir :  user_context.cm_workspace->children(xirang2::file_path()))
                paths.push_back(dir.path);
                 
            for (auto dir :  paths){
                xirang2::vfs::recursive_remove(*user_context.cm_workspace, dir);
            }
            context.response_header(sg.fout());
            sg.fout() << "{\"result\": \"" << c->version.id <<"\"}";
        }

    };

#endif
}
