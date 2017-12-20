#ifndef CMD_CONTEXT_H__
#define CMD_CONTEXT_H__

#include <string>

namespace CS{

struct CmdContext
{
    std::string path;
    std::string params;
    std::ostream& os;
    explicit CmdContext(std::ostream& outstream);
};

} //namespace

#endif //CMD_CONTEXT_H__
