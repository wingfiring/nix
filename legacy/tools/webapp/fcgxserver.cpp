
#include "fcgxserver.h"
#include "assert.h"
#include <fcgio.h>
#include "commandutils.h"
#include "cmdcontext.h"
#include "contentservicert.h"

namespace CS{

using namespace xirang2;

// struct ServerEnv
//
ServerEnv::ServerEnv(std::istream& in, std::ostream& out)
: fin(in),
  fout(out)
{}

// class FCGXServer
//
FCGXServer::FCGXServer()
{}

void FCGXServer::ListenFCGX()
{
	FCGX_Request request;

	FCGX_Init();
	FCGX_InitRequest (&request, 0, 0);

	while (FCGX_Accept_r (&request) == 0){
	    fcgi_streambuf in_buf(request.in);
	    fcgi_streambuf out_buf(request.out);
	    std::istream fin(&in_buf);
	    std::ostream fout(&out_buf);
	    try{
		ServerEnv env(fin, fout);
		auto s = FCGX_GetParam("PATH_INFO", request.envp);
		file_path path = s ? file_path(s) : file_path();
		if (!path.is_absolute())
		{
		    CommandUtils::response_error(fout, 404, path);
		    continue;
		}
		std::string pathStr(s);
		auto q = FCGX_GetParam("QUERY_STRING", request.envp);
		std::string query(q);
		std::string requestString = pathStr  + '?' + query;
                Run_(env, requestString);
	    }
	    catch(fs::not_found_exception&)
	    {
		CommandUtils::response_error(fout, 404, file_path(""));
	    }
	    catch(...){}
	}
}

void FCGXServer::Register(const std::string& cmdName, CommandHandler handler)
{
    cmdMap.insert(std::make_pair(cmdName, handler));
}

bool FCGXServer::ExecuteCommand_(const std::string& cmdName, ServerEnv& env, const std::string& path, const std::string& params)
{
    std::map<std::string, CommandHandler>::iterator iterEnd = cmdMap.end();
    std::map<std::string, CommandHandler>::iterator iter = cmdMap.find(cmdName);
    if (iter == iterEnd)
        return false;
    CommandHandler handler = iter->second;
    if (handler == NULL)
        return false;
    CmdContext context(env.fout);
    context.path = path;
    context.params = params;
    handler(context);
    return true;
}

// std::string request1("/lib/adsk/cm/type?List");
// std::string request3("/lib/adsk/cm/asset/PrismMetalSchema?Get");
// std::string request5("/lib/adsk/cm/asset/PrismMetalSchema?GetMult&Set:(%22Prism-017%22%20%22Prism-018%22%20%22Prism-002%22)");
bool FCGXServer::Run_(ServerEnv& env, const std::string& input)
{
    // Split the result as "path", "cmd", "parameters"
    //
    std::string cmd;
    std::string path;
    std::string parameters;
    auto pos = input.find_first_of("?");
    assert(pos != std::string::npos);
    path = input.substr(0, pos);
    auto pos2 = input.find_first_of("&");
    if (pos2 == std::string::npos)
    {
        cmd = input.substr(pos+1, input.length()-pos-1);
    }
    else
    {
        cmd = input.substr(pos+1, pos2-pos-1);
        parameters = input.substr(pos2+1, input.length()-pos2-1);
    }
     
    return ExecuteCommand_(cmd, env, path, parameters);
}

} //namespace
