#include <iostream>
#include "commandcontainer.h"
#include "fcgxserver.h"
#include "contentservicert.h"

using namespace xirang2;
using namespace CS;

int main(int argc, char** argv){
	if (argc != 2){
		std::cerr << "Usage: protein wordir\n";
		return 1;
	}

	ContentServiceRTParam param;
	param.xrName = "demo";
	param.rootfsName = "test";
	file_path work_dir(argv[1]);
	param.workDir = work_dir.str();
	param.resPath = "lib/adsk/cm/resource";
	param.dataFile = "cm.data";
	param.metabasePath = ".metabase";
	try{
	    ContentServiceRT rt(param);
	    FCGXServer server;
	    server.Register("List", CommandList(rt));
	    server.Register("Get", CommandGet(rt));
	    server.Register("GetMult", CommandGetMult(rt));
	    server.Register("Put", CommandPut(rt));
	    server.ListenFCGX();
	}
	catch(fs::open_failed_exception&)
	{
	    std::cerr << "fs::open_failed_exception happens!\n";
	}
	catch(...)
	{
	    std::cerr << "exception happens!\n";
	}
	return 0;
}
