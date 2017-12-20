#include <xirang2/type/xirang.h>
#include <xirang2/protein/datatype.h>
#include <xirang2/io/memory.h>


#include <chrono>
#include <iostream>

static const xirang2::file_path K_protein_sys_type_path ("lib/adsk/cm/1.0/sys/type/color4");

int main(){
    using namespace xirang2::type;
    
    Xirang xr("demo", xirang2::memory::get_global_heap(), xirang2::memory::get_global_ext_heap());
    SetupXirang(xr);
    xirang2::protein::SetupSysTypes(xr);
    
    auto t_color4 = xr.root().locateType(K_protein_sys_type_path);
    ScopedObjectCreator obj(t_color4, xr);
    
    auto obj_color4 = obj.get();
    
    auto& color4 =  *xirang2::type::Ref<xirang2::protein::color4>(obj_color4);
    color4.r = 0.3;
    color4.g = 0.6;
    color4.b = 0.9;
    color4.a = 0;
    
    xirang2::io::mem_archive mar;
    
    xirang2::iref<xirang2::io::writer> sink(mar);
    auto& wr = sink.get<xirang2::io::writer>();
    
    xirang2::iref<xirang2::io::reader> source(mar);
    auto& rd = source.get<xirang2::io::reader>();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000*1000; ++i){
        mar.seek(0);

        t_color4.methods().serialize(wr, obj_color4);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "encode time: " << diff.count() << " s" << std::endl;
    
    start = end;
    for (int i = 0; i < 1000*1000; ++i){
    
        mar.seek(0);
        
        t_color4.methods().deserialize(rd, obj_color4, xr.get_heap(), xr.get_ext_heap());
        
    }
    end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    std::cout << "decode time: " << diff.count() << " s" << std::endl;
}

