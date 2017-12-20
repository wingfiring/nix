#include <xirang2/assert.h>
#include <xirang2/context_except.h>

//STL
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <atomic>

namespace xirang2
{
	namespace
	{
		std::atomic<contract_handler*> g_engine ={ 0};		
		
		const char_utf8* const category_name[] = 
		{
			"precondition:",
			"postcondition:",
			"invariant:"
		};

		/* ensure precondition <= cat <= invariant */
		contract_category ensure_valid_contract_category(contract_category cat)
		{
			return cat > contract_category::invariant 
				? contract_category::invariant 
				: (cat < contract_category::pre? contract_category::pre: cat);
		}
	}
 
	XR_API void aio_assert(const char_utf8* expr, const char_utf8* sourcefile, 
			const char_utf8* function, int line, const char_utf8* message) 
	{
		fprintf(stderr, "Assert failed: %s:%d %s:%s %s\n", sourcefile, line, function, expr, message);
		std::abort();
	}

	XR_API void contract_handler::set_handle(contract_handler* engine){
        g_engine.store(engine);
	}

	XR_API contract_handler* contract_handler::get_handle(){
        return g_engine.load();
	}

#if defined(MSVC_COMPILER_)
#pragma warning(disable: 4702) // "unreachable code" after aio_assert calling in release build
#endif

	XR_API bool contract_handler::process(contract_category cat
			, const char_utf8* expr
			, const char_utf8* sourcefile
			, const char_utf8* function
			, int line
			, const char_utf8* message)
	{
		contract_handler* engine = g_engine.load();
		if (engine)
			return engine->do_process(ensure_valid_contract_category(cat), expr, sourcefile, function, line, message);

		aio_assert(expr, sourcefile, function, line, message);
		return false;
	}

	namespace 
	{
		const char_utf8* ensure_not_null(const char_utf8* src)
		{
			return src ? src : "";
		}

		//local link
		std::ostream& default_format(std::ostream& os
				, contract_category type
				, const char_utf8* expr
				, const char_utf8* sourcefile
				, const char_utf8* function
				, int line
				, const char_utf8* message)
		{
			return os << "\n" << ensure_not_null(sourcefile) << "(" << line << ") : [" 
				<< ensure_not_null(function) << "]\n"
				   << category_name[int(ensure_valid_contract_category(type))] 
				   << ensure_not_null(message) << ":" << ensure_not_null(expr);
		}
	}

	bool console_reporter::do_process(contract_category type, const char_utf8* expr, const char_utf8* sourcefile, 
			const char_utf8* function, int line, const char_utf8* message)
	{
		default_format(std::cerr, type, expr, sourcefile, function, line, message) 
			<< std::endl;
		return false;
	}

	bool exception_reporter::do_process(contract_category type, const char_utf8* expr, const char_utf8* sourcefile, 
			const char_utf8* function, int line, const char_utf8* message)
	{
		std::stringstream sstr;
		default_format(sstr, type, expr, sourcefile, function, line, message);

		switch(type)
		{
			case contract_category::pre:
				XR_THROW(pre_exception)(sstr.str().c_str());
				break;
			case contract_category::post:
				XR_THROW(post_exception)(sstr.str().c_str());
				break;
			case contract_category::invariant:
				XR_THROW(invariant_exception)(sstr.str().c_str());
				break;
		}

		return false;
	}

}

