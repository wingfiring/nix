#include <xirang2/versionedvfs.h>
#include <xirang2/fsutility.h>
#include <xirang2/vfs/local.h>
#include <xirang2/vfs/inmemory.h>

#include <xirang2/io/memory.h>

#include <unordered_map>
#include <sstream>
#include <iostream>
#include <random>
#include <ctime>

using namespace xirang2;
using std::cout;
using std::cerr;
using std::endl;

const size_t K_Count = 1024;

int do_main(const char* repo_dir, long seed){
    file_path dir(repo_dir);
    
    auto err = fs::recursive_create_dir(dir);
    if (err != fs::er_ok){
        std::cout << "Failed to create dir\n";
        return 1;
    }
    
    vfs::LocalFs vfs(dir);
    err = vfs::initRepository(vfs, file_path());
    
    if (err != fs::er_ok){
        std::cout << "Error:" << "failed to init repository" << std::endl;
        return 1;
    }
    
    std::mt19937 engine(seed);
    //std::normal_distribution<long> distribution(500, 200);
    std::uniform_int_distribution<unsigned long> distribution(200, 1024);
    
    xirang2::buffer<xirang2::byte> data;
    data.resize(10240, byte(0));
    
    
    vfs::InMemory wkfs;
    vfs::Workspace wk(wkfs, string());
    
    file_path data_dir("data");
    cout << "prepare workspace ..."; cout.flush();
    auto start_time = clock();
    for (size_t i = 0; i < K_Count; ++i){
        std::stringstream sstr;
        sstr << i;
        
        file_path sub_dir = data_dir / file_path(sstr.str().c_str());
        
        for (size_t j = 0; j < K_Count; ++j){
            std::stringstream sstr2;
            sstr2 << j;
            
            data.resize(distribution(engine));
//            if (data.size() > 10240)
//                data.resize(10240);
            
            xirang2::io::buffer_in in_ar(data);
            
            xirang2::iref<xirang2::io::read_map> src(in_ar);
            
            long* p = (long*) &data[0];
            p += (distribution(engine) % (data.size() -1)) >> 2;
            *p = distribution(engine);
            
            file_path fpath  = sub_dir / file_path(sstr2.str().c_str());
            
            auto dest = vfs::recursive_create<io::write_map>(wk, fpath, io::of_create_or_open);
            copy_data(src.get<io::read_map>(), dest.get<io::write_map>());
        }
    }
    auto end_time = clock();
    cout << " Done\n Time:" << double(end_time - start_time) / CLOCKS_PER_SEC << "\nCommit ..."; cout.flush();
    
    type::Xirang xr("repo_perf", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());

    vfs::LocalRepository repo(vfs, file_path(), xr);
    auto base = repo.getSubmission(version_type())->version;
    
    auto c = repo.commit(wk, string("test"), base);
    if (!c || c->flag == vfs::bt_none){
        cout << "Commit failed\n";
        return 1;
    }
    
    cout << " Done\n Time: ";
    cout << double(clock() - end_time) / CLOCKS_PER_SEC << endl;
    
    cout << c->version.id << endl;
    
    return 0;
}

int main(int argc, char** argv){
    if (argc < 2){
        cout << "usage: repo_perf dir [seed]" << endl;
        return 1;
    }
    
    try{
        auto seed = time(0);
        if (argc == 3){
            std::stringstream sstr(argv[2]);
            sstr >> seed;
        }
        return do_main(argv[1], seed);
    }catch(exception& e){
        cerr << e.what() << endl;
        
    }
    return 1;
}

