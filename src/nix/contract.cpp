#if 1
#include <nix/contract.h>
#include <nix/context_except.h>

//STL
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <atomic>

namespace nix
{
	namespace
	{
		std::atomic<contract_handler*> g_engine ={ 0};		
		
		const char* const category_name[] = 
		{
			"expects:",
			"ensure:"
		};

		/* ensure cat is expect or ensure */
		contract_category ensure_valid_contract_category(contract_category cat)
		{
			return cat != contract_category::ensure 
				? contract_category::expect : cat;
		}
	}
 
	NIX_API [[noreturn]] void aio_assert(const char* expr, const char* sourcefile, 
			const char* function, int line, const char* message) 
	{
		fprintf(stderr, "Assert failed: %s:%d %s:%s %s\n", sourcefile, line, function, expr, message);
		std::abort();
	}

	NIX_API void contract_handler::set_handle(contract_handler* engine){
        g_engine.store(engine);
	}

	NIX_API contract_handler* contract_handler::get_handle(){
        return g_engine.load();
	}

#if defined(MSVC_COMPILER_)
#pragma warning(disable: 4702) // "unreachable code" after aio_assert calling in release build
#endif

	NIX_API bool contract_handler::process(contract_category cat
			, const char* expr
			, const char* sourcefile
			, const char* function
			, int line
			, const char* message)
	{
		contract_handler* engine = g_engine.load();
		if (!engine)
			aio_assert(expr, sourcefile, function, line, message);

		return engine->do_process(ensure_valid_contract_category(cat), expr, sourcefile, function, line, message);
	}

	namespace 
	{
		const char* ensure_not_null(const char* src)
		{
			return src ? src : "";
		}

		//local link
		std::ostream& default_format(std::ostream& os
				, contract_category type
				, const char* expr
				, const char* sourcefile
				, const char* function
				, int line
				, const char* message)
		{
			return os << "\n" << ensure_not_null(sourcefile) << "(" << line << ") : [" 
				<< ensure_not_null(function) << "]\n"
				   << category_name[int(ensure_valid_contract_category(type))] 
				   << ensure_not_null(message) << ":" << ensure_not_null(expr);
		}
	}

	bool console_reporter::do_process(contract_category type, const char* expr, const char* sourcefile, 
			const char* function, int line, const char* message)
	{
		default_format(std::cerr, type, expr, sourcefile, function, line, message) 
			<< std::endl;
		return false;
	}

	bool exception_reporter::do_process(contract_category type, const char* expr, const char* sourcefile, 
			const char* function, int line, const char* message)
	{
		std::stringstream sstr;
		default_format(sstr, type, expr, sourcefile, function, line, message);

		switch(type)
		{
			case contract_category::expect:
				NIX_THROW(expect_exception) << sstr.str();
				break;
			case contract_category::ensure:
				NIX_THROW(ensure_exception) << sstr.str();
				break;
		}

		return false;
	}

}

#endif
