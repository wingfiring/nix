#ifndef NIX_ABI_PREFIX_H__
# error Header nix/config/abi_suffix.h must only be used after nix/config/abi_prefix.h
#else
#undef NIX_ABI_PREFIX_H__
#endif

#ifdef NIX_H__AS_ABI_H__EADERS
#  include NIX_ABI_SUFFIX
#endif


