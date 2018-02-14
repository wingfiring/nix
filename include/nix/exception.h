#ifndef NIX_EXCEPTION_H__
#define NIX_EXCEPTION_H__
#include <nix/config.h>
#include <exception>
#include <string_view>

namespace nix
{
	/// base of all nix exception classes
	struct NIX_API exception : public std::exception
	{
		virtual const char* what() const noexcept  = 0;
		virtual ~exception() noexcept {};
	};
}

#define NIX_EXCEPTION_TYPE_EX(type, base) \
	struct NIX_API type : public base {\
	virtual ~type() noexcept {}\
	}

#define NIX_EXCEPTION_TYPE(type) \
	NIX_EXCEPTION_TYPE_EX(type, ::nix::exception)

#define NIX_INTERNAL_EXCEPTION_TYPE_EX(type, base) \
	struct type : public base {\
	virtual ~type() noexcept {}\
	}

#define NIX_INTERNAL_EXCEPTION_TYPE(type) \
	NIX_INTERNAL_EXCEPTION_TYPE_EX(type, ::nix::exception)

#endif //end NIX_EXCEPTION_H__
