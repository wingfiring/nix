#ifndef XIRANG2_NO_COPY_H_
#define XIRANG2_NO_COPY_H_
#include <xirang2/config.h>
#include <type_traits>

namespace xirang2 {
    template<typename T>
    struct no_copy {
        static_assert(std::is_default_constructible<T>::value, "T does not match is_default_constructible");
        ~no_copy() = default;

        no_copy() = default;
        no_copy(const no_copy&) : value(){};
        no_copy& operator=(const no_copy&) { return *this;}
        
        template<typename U>
        no_copy(U&& v) : value(std::move(v)) {}

        template<typename U>
        no_copy& operator=(U&& v){
            value = std::forward<U>(v);
            return *this;
        };

        operator T&() noexcept { return value; }
        operator const T&() const noexcept{ return value; }
        T value;
    };

}

#endif //end XIRANG2_NO_COPY_H_
