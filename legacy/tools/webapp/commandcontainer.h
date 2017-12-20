
#ifndef COMMAND_CONTAINER_H__
#define COMMAND_CONTAINER_H__

#include "contentservicefwd.h"

namespace CS{

class CommandBase
{
public:
    explicit CommandBase(ContentServiceRT& rt);
protected:
    ContentServiceRT& m_rt;
};

class CommandList : public CommandBase
{
public:
    explicit CommandList(ContentServiceRT& rt);
    void operator() (CmdContext& context) const;
};

class CommandGet : public CommandBase
{
public:
    explicit CommandGet(ContentServiceRT& rt);
    void operator() (CmdContext& context) const;
};

class CommandGetMult : public CommandBase
{
public:
    explicit CommandGetMult(ContentServiceRT& rt);
    void operator() (CmdContext& context) const;
};

class CommandPut : public CommandBase
{
public:
    explicit CommandPut(ContentServiceRT& rt);
    void operator() (CmdContext& context) const;
};

} //namespace

#endif  //COMMAND_CONTAINER_H__
