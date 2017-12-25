#define NIX_ABI_PREFIX <nix/config/abi/msvc_prefix.h>
#define NIX_ABI_SUFFIX <nix/config/abi/msvc_suffix.h>

#define NIX_FUNCTION __FUNCTION__ 

#define NIX_DLL_EXPORT __declspec( dllexport )
#define NIX_DLL_IMPORT __declspec( dllimport )

#define NIX_DLL_INTERFACE __declspec(novtable)

