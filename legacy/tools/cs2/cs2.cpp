#include <iostream>
#include <fcgigear/fcgiserver.h>
#include <fcgio.h>

#include <xirang2/io/memory.h>
#include <xirang2/io/file.h>

#include <boost/property_tree/info_parser.hpp>  // boost::property_tree::info_parser::read_info
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include "commands.h"
#include "configfile.h"

using namespace xirang2;

//hack
namespace cs2{
    extern void init_json_load_table(xirang2::type::Namespace root);
}

int main(int argc, char** argv ){

    if (argc != 2){
        std::cerr << "Usage: cmserver <config file> \n";
        return 1;
    }
    
    fcgi::Context context;
    context.methods = xirang2::fcgi::GetDefaultMethodMap();
    

    cs2::LoadConfigAndInit(context, xirang2::file_path(argv[1]));
    
    auto& user_context = static_cast<cs2::CMContext&>(*context.user_context);
    cs2::init_json_load_table(user_context.runtime.root());
    
    FCGX_Init();
    
    fcgi::Run(context);
    
    return 0;
}


