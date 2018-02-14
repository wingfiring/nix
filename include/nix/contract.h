#ifndef NIX_CONTRACT_H__
#define NIX_CONTRACT_H__

/** @file 

	contract programming support class
	includes runtime invariants check and compile time check.
	three invariant check are implemented: NIX_PRE_CONDITION, NIX_POST_CONDITION, NIX_CONTRACT.
*/

#include <nix/config.h>
#include <nix/exception.h>
#include <nix/macro_helper.h>

namespace nix
{
	/**	includes invariant checkers.
	*/
	/** supported invatiant contract_categorys 
		@see http://en.wikipedia.org/wiki/Design_by_contract
		@see http://www.artima.com/intv/contracts.html
	*/
	enum class contract_category
	{				
		expect= 0,
		ensure
	};

	struct contract_tag {};
	struct expects_tag : contract_tag {};
	struct ensures_tag : contract_tag {};


	/** if needs customize error processing method, create a new engine dirived from this class*/
	class NIX_INTERFACE contract_handler
	{
	public:
		/** if invariants broken, this function called
			@param type invariant check type
			@param expr invariant check expression
			@param sourcefile source file name
			@param function in function name
			@param line line number
			@param message addtional message
			@return if the impementation ignores a error, return true, else return false.
			@note generally, process should print diagnosis info and then abort or active debuger.
			if necessary, process can throw an exception instead of abort, such as in release version.
			but be careful, use exceptions instead of invariants is almost always a bad practice.
		*/
		NIX_API static bool process(contract_category type
			, const char* expr
			, const char* sourcefile
			, const char* function
			, int line
			, const char* message);

		virtual bool do_process(contract_category type
			, const char* expr
			, const char* sourcefile
			, const char* function
			, int line
			, const char* message) = 0;


		/** derivedable */
		virtual ~contract_handler(){};

		/** set error report engine
			@param engine new error report engine
		*/
		NIX_API static void set_handle(contract_handler* engine);

		/** get error report engine
			@return current error report engine
		*/
		NIX_API static contract_handler* get_handle();
	};

	/** it's used to keep the current contract_handler pointer and restore it when exit the scope.
	*/
	struct contract_handler_saver
	{
		/// \ctor Default ctor, save the result of contract_handler::get_handle() call
		contract_handler_saver() 
			: m_old(contract_handler::get_handle())
		{}

		/// \ctor same as the default ctor but set the current handler to new_handler
		explicit contract_handler_saver(contract_handler* new_handler ) 
			: m_old(contract_handler::get_handle())
		{
			contract_handler::set_handle(new_handler);
		}

		/// \dtor restore the saved handler.
		~contract_handler_saver() { contract_handler::set_handle(m_old);}
	private:
		contract_handler* m_old;
	};

	/** treat invariant as exception, base of assert_exception_impl
	*/
	NIX_EXCEPTION_TYPE(contract_exception);
	
	NIX_EXCEPTION_TYPE_EX(expect_exception, contract_exception);

	NIX_EXCEPTION_TYPE_EX(ensure_exception, contract_exception);
	
	/** convert assert check failure to exceptions*/
	class NIX_API exception_reporter : public contract_handler
	{
		/** @copydoc nix::contract::contract_handler::do_process */
		virtual bool do_process(contract_category type, 
				const char* expr, 
				const char* sourcefile, 
				const char* function, 
				int line, 
				const char* message) override;
	};

	/** default error report engine */
	class NIX_API console_reporter : public contract_handler
	{
	public:
		/** @copydoc nix::contract::contract_handler::do_process */
		virtual bool do_process(contract_category type, 
				const char* expr, 
				const char* sourcefile, 
				const char* function, 
				int line, 
				const char* message) override;
	};

	NIX_API void aio_assert(const char* expr, const char* sourcefile,
		const char* function, int line, const char* message);
}

#if !defined(NDEBUG) || defined(NIX_KEEP_CONTRACT)
#  define NIX_CONTRACT_COMM_EX(contract_category, expr, msg)\
	((expr) ||\
		::nix::contract_handler::process(contract_category, \
		NIX_STRING(expr), __FILE__,\
			NIX_FUNCTION, __LINE__, msg))

#  define NIX_CONTRACT_COMM(contract_category, expr)\
	NIX_CONTRACT_COMM_EX(contract_category, expr, 0)

#else
#  define NIX_CONTRACT_COMM_EX(contract_category, expr, msg) 
#  define NIX_CONTRACT_COMM(contract_category, expr) 

#endif //end NDEBUG

#define NIX_EXPECTS_EX(expr, msg)\
	NIX_CONTRACT_COMM_EX(nix::contract_category::expect, expr, msg)

#define NIX_ENSURE_EX(expr, msg)\
	NIX_CONTRACT_COMM_EX(nix::contract_category::ensure, expr, msg)

#define NIX_EXPECTS(expr)\
	NIX_CONTRACT_COMM(nix::contract_category::expect, expr)

#define NIX_ENSURE(expr)\
	NIX_CONTRACT_COMM(nix::contract_category::ensure, expr)

#endif // end NIX_CONTRACT_H__

