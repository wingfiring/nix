#include "commandcontainer.h"
#include "cmdcontext.h"
#include "commandutils.h"
#include "jsonserializer.h"
#include "jsondeserializer.h"
#include "contentservicert.h"

namespace CS{

using namespace xirang2;

// class CommandBase
//
CommandBase::CommandBase(ContentServiceRT& rt)
:
m_rt(rt)
{}

// class CommandList
//
CommandList::CommandList(ContentServiceRT& rt)
: CommandBase(rt)
{}

void CommandList::operator() (CmdContext& context) const
{
    auto pos = context.path.find("/type");
    if (pos != std::string::npos)
    {
        CommandUtils::response_header(context.os);
        JSONSerializer().response_schema_list(context.os, m_rt.xr.root());
    }
}

// class CommandGet
//
CommandGet::CommandGet(ContentServiceRT& rt)
: CommandBase(rt)
{}

void CommandGet::operator() (CmdContext& context) const
{
    xirang2::file_path filepath(context.path.c_str());
    if (filepath.parent() == sub_file_path(literal("/lib/adsk/cm/asset")))
    {
	CommandUtils::response_header(context.os);
	JSONSerializer().response_object_list(context.os, m_rt.xr.root(), filepath);
	return;
    }
    else
    {
	auto obj = CommandUtils::locateObject(m_rt.xr.root(), filepath);
	if (obj.valid())
	{
	    CommandUtils::response_header(context.os);
            JSONSerializer().response_obj(context.os, obj);
	    return;
	}
	auto type = m_rt.xr.root().locateType(filepath);
	if (type.valid())
	{
	    CommandUtils::response_header(context.os);
	    JSONSerializer().response_type(context.os, type);
	    return;
	}
	// third, is it a file?
	auto ar = m_rt.rootFs.create<io::read_map>(filepath, io::of_open);
	CommandUtils::response_file(context.os, ar);
    }
    // failed: not found
    CommandUtils::response_error(context.os, 404, filepath);
}

// class CommandGetMult
//
CommandGetMult::CommandGetMult(ContentServiceRT& rt)
: CommandBase(rt)
{}

void CommandGetMult::operator() (CmdContext& context) const
{
    //TODO: For future work.
#ifdef WIN32
	context;
#endif
}

// class CommandPut
//
CommandPut::CommandPut(ContentServiceRT& rt)
: CommandBase(rt)
{}

void CommandPut::operator() (CmdContext& context) const
{
    xirang2::file_path filepath(context.path.c_str());
    if (filepath.parent() == sub_file_path(literal("/lib/adsk/cm/asset")))
    {
	
	//http://technologyconversations.com/2014/08/12/rest-api-with-json/
	CommandUtils::response_header(context.os, 412);
	//return;
    }
    else
    {
	auto obj = CommandUtils::locateObject(m_rt.xr.root(), filepath);
	if (obj.valid())
	{
	    CommandUtils::response_header(context.os);
	    CommandUtils::response_header(context.os, 412);

	    std::string param_str = context.params;
	    std::string js_format("Json:");
    	    auto pos = param_str.find_first_of(js_format);
    	    if (pos != std::string::npos)
	    {
		std::string js_str = param_str.substr(pos+js_format.length(), param_str.length()-pos-js_format.length());
		JSONDeserializer().convert_obj(context.os, context.path, js_str, m_rt);
            	//JSONSerializer().response_obj(context.os, obj);
	    	return;
	    }
	}
	/*auto type = m_rt.xr.root().locateType(filepath);
	if (type.valid())
	{
	    CommandUtils::response_header(context.os);
	    //JSONSerializer().response_type(context.os, type);
	    return;
	}*/
	//else
	{
	    CommandUtils::response_error(context.os, 404, filepath);
    	}
    }
}

} //namespace
