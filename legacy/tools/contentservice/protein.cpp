#include <iostream>
#include "commandcontainer.h"
//#include "fcgxserver.h"
#include "contentservicert.h"
#include "jsonserializer.h"
#include "jsondeserializer.h"
#include "commandutils.h"
#include <fstream>

using namespace xirang2;
using namespace CS;

int main(int , char** ){
	/*
	if (argc != 2){
		std::cerr << "Usage: protein wordir\n";
		return 1;
	}
	*/

	ContentServiceRTParam param;
	param.xrName = "demo";
	param.rootfsName = "test";
	//file_path work_dir(argv[1]);  // Hard code in Windows for easy testing
	file_path work_dir("C:/CM_FY14/PlatformSDK/sandbox/sunjac/xirang2/trunk/tools/webapp");
	param.workDir = work_dir.str();
	param.resPath = "lib/adsk/cm/resource";
	param.dataFile = "cm.data";
	param.metabasePath = ".metabase";
	try{
	    ContentServiceRT rt(param);



		std::ofstream fout;
		std::string path("/lib/adsk/cm/asset/PrismMetalSchema/metadata/Prism-017");

		xirang2::file_path filepath(path.c_str());
		auto obj = CommandUtils::locateObject(rt.xr.root(), filepath);
		JSONSerializer().response_obj(fout, obj);


		std::string js_str("{\"BaseSchema\":\"PrismMetalSchema\",\"Hidden\":true,\"UIName\":\"LeeTest-Prism-017\",\"VersionGUID\":\"Prism-017\",\"category\":[\"Metal/Aluminum\",\"LeeTest-Default\"],\"description\":\"Aluminum-brushed\",\"keyword\":[\"Metal; Aluminum\",\"materials\",\"metal\"],\"thumbnail\":[\"/cm/lib/adsk/cm/resource/Mats/PrismMetal/Presets/t_Prism-017.png\"],\"tags\":[],\"dict\":\"\"}");
		JSONDeserializer().convert_obj(fout, path, js_str, rt);

		auto objAfter = CommandUtils::locateObject(rt.xr.root(), filepath);
		JSONSerializer().response_obj(fout, objAfter);

		
		/*
	    FCGXServer server;
	    server.Register("List", CommandList(rt));
	    server.Register("Get", CommandGet(rt));
	    server.Register("GetMult", CommandGetMult(rt));
	    server.Register("Put", CommandPut(rt));
	    server.ListenFCGX();
		*/
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
