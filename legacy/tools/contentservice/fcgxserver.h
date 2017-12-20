
#ifndef FCGXSERVER_H__
#define FCGXSERVER_H__

#include "contentservicefwd.h"
#include <map>
#include <functional>

namespace CS{

typedef std::function<void (CmdContext& context)> CommandHandler; 

struct ServerEnv
{
    std::istream& fin;
    std::ostream& fout;
    explicit ServerEnv(std::istream& in, std::ostream& out);
};

class FCGXServer
{
public:
    FCGXServer();
    void ListenFCGX();
    void Register(const std::string& cmdName, CommandHandler handler);

private:
    bool ExecuteCommand_(const std::string& cmdName, ServerEnv& env, const std::string& path, const std::string& params);
    bool Run_(ServerEnv& env, const std::string& input);
private:
    std::map<std::string, CommandHandler> cmdMap;
};

} //namespace

#endif  //FCGXSERVER_H__
