//NIX_LICENSE_PLACE_HOLDER

#ifndef NIX_CONTEXT_EXCEPTION_H__
#define NIX_CONTEXT_EXCEPTION_H__
#include <nix/config.h>
#include <nix/macro_helper.h>
#include <nix/string.h>
#include <sstream>

namespace nix
{
	NIX_API extern string stacktrace();

	/// collect and hold exception point runtime information
	/// \param Base usually, user just need to define a empty exception class
	/// with or without base class. all nix exception class should derived from ::nix::exception
	template<typename Base>
	class context_exception : public Base
	{
	public:
		/// \param msg returned by what()
		/// \param stack_str stack info
		explicit context_exception(std::string msg, string stack_str)
			: m_msg(msg)
		  , m_stack(stack_str)
		{
		}
		/// just for nothow compatible
		virtual ~context_exception() noexcept {}

		/// \return exception information, return as  char string
		virtual const char* what() const  noexcept {
			return this->m_msg.c_str();
		}

		/// \return stack info, could be empty
		virtual const string& stack() const noexcept{
			return this->m_stack;
		}

	private:
		std::string m_msg;
		string m_stack;
	};

	/// class for throw exception, help to collect runtime information
	template<typename E> class throw_exception
	{
	public:
		/// instrument to yield a throw
		struct do_throw{};

		/// \param file file path or name
		/// \param lineno code line number
		/// \param stack_info stack info, string represented
		throw_exception(const char* file, unsigned lineno, string stack_info)
			: m_stack(std::move(stack_info))
		{
			m_stream << file << '(' << lineno << ')';
		}
		/// dtor
		~throw_exception() = default;

		void apply_throw(){
			throw context_exception<E>(m_stream.str(), std::move(m_stack));
		}
	private:
		std::stringstream m_stream;
		string m_stack;

		/// help to collect data
		template<typename T> friend throw_exception<E>& operator<<(throw_exception& th, T&& t){
			th.m_stream << t;
			return th;
		}
		/// help to collect data
		template<typename T> friend throw_exception<E>& operator<<(throw_exception&& th, T&& t){
			th.m_stream << t;
			return th;
		}

		/// helper overloaded for NIX_THROW
		friend void operator,(do_throw, throw_exception<E>& th){ th.apply_throw(); }
		/// helper overloaded for NIX_THROW
		friend void operator,(do_throw, throw_exception<E>&& th){ th.apply_throw(); }
	};
}

/// throw exception of type ex, with stacktrace info
/// \param ex type of exception
#define NIX_THROW(ex) ::nix::throw_exception<ex>::do_throw(), ::nix::throw_exception<ex>(__FILE__, __LINE__, ::nix::stacktrace())

/// throw exception of type ex, without stacktrace info
/// \param ex type of exception
#define NIX_THROW_LITE(ex) ::nix::throw_exception<ex>::do_throw(), ::nix::throw_exception<ex>(__FILE__, __LINE__, {})

/// generate named value pair
#define NIX_VAR(x) NIX_STRING(x) ": " << x

#endif //end NIX_CONTEXT_EXCEPTION_H__
