//NIX_LICENSE_PLACE_HOLDER

#ifndef NIX_COMMON_CONFIG_H__
#define NIX_COMMON_CONFIG_H__

#include <nix/config/config.h>

#if defined (NIX_COMMON_DLL_EXPORT)
#	define NIX_API NIX_DLL_EXPORT
#	define NIX_INTERFACE NIX_DLL_INTERFACE
#elif defined (NIX_COMMON_DLL_IMPORT)
#	define NIX_API NIX_DLL_IMPORT
#	define NIX_INTERFACE NIX_DLL_INTERFACE
#else
#	define NIX_API			//for static
#	define NIX_INTERFACE 
#endif

#endif //end NIX_COMMON_CONFIG_H__


