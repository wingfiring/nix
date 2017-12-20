#ifndef XIRANG2_VAR_SAVER_H_
#define XIRANG2_VAR_SAVER_H_
#include <xirang2/config.h>
#include <type_traits>

namespace xirang2 {
    template<typename T>
    struct var_saver {
        static_assert(std::is_nothrow_assignable<T&, T&&>::value, "T does not match is_nothrow_assignable");

        ~var_saver() noexcept 
        { 
            if (dest) *dest = std::move(old); 
        }

        var_saver() noexcept(std::is_nothrow_default_constructible<T>::value)
            : dest(nullptr)
            , old()
        {}
        explicit var_saver(T& t) noexcept(std::is_nothrow_constructible<T, T&>::value)
            :dest(&t), old(t) 
        {}

        template<typename U>
        explicit var_saver(T& t, U&& set) noexcept(std::is_nothrow_constructible<T, T&>::value && std::is_nothrow_assignable<T&, U&&>::value)
            : dest(&t)
            , old(dest) 
        { 
            *dest = std::forward<U>(set); 
        }

        var_saver(var_saver&& rhs) noexcept
            : dest(rhs.dest)
            , old(rhs.old) 
        { rhs.dest == nullptr; }

        var_saver& operator=(var_saver&& rhs) noexcept {
            if (this != rhs) {
                dest = rhs.dest;
                old = std::move(rhs.old);
                rhs.dest = nullptr;
            }
            return *this;
        }
        
        operator T&() const noexcept{
            return *dest;
        }

        void swap(var_saver& rhs) {
            swap(std::move(rhs));
        }
        void swap(var_saver&& rhs) {
            std::swap(dest, rhs.dest);
            std::swap(old, rhs.old);
        }
    private:
        T* dest;
        T old;
    };
    template<typename T> var_saver<T> var_save(T& t) noexcept(std::is_nothrow_constructible<var_saver<T>, T&>::value)
    { return var_saver<T>(t); }
    template<typename T, typename U> var_saver<T> var_save(T& t, U&& set) noexcept(std::is_nothrow_constructible<var_saver<T>, U&&>::value)
    { return var_saver<T>(t, std::forward<U>(set)); }
}

#endif //end XIRANG2_VAR_SAVER_H_
