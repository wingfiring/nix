#ifndef NIX_NO_COPY_H_
#define NIX_NO_COPY_H_
#include <nix/config.h>
#include <nix/stl/type_traits.h>

namespace nix {
	// intents to hold a data member and disable default copy & assignment
    template<typename T> struct no_copy {
        static_assert(std::is_default_constructible<T>::value, "T does not match std::is_default_constructible");
        constexpr no_copy() = default;
        constexpr no_copy(const no_copy&) noexcept(std::is_nothrow_constructible_v<T>) : value(){};
        constexpr no_copy& operator=(const no_copy&) noexcept { return *this;}
        
        constexpr operator T&() noexcept { return value; }
        constexpr operator const T&() const noexcept{ return value; }

        T value;
    };

}

#endif //end NIX_NO_COPY_H_
